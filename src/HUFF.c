/* ---------------------------------------------------------------------- *//*!
   
   \file  HUFF.c
   \brief Huffman Encode/Decode, implementation file
   \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim

  \warning
   In their current state, these routines are not fit for Flight use.
   They can be used to determine compression factors and, in general,
   study the properties of Huffman encoding/decoding. However, they
   malloc all over the place and involve recursive calling techniques,
   neither of is appropriate for Flight code.
                                                                          */
/* ---------------------------------------------------------------------- */


#include <limits.h>
#include <stdlib.h>
#include <string.h>
//#include <netinet/in.h>

#include "DFC/HUFF.h"
#include "DFC/BTREE.h"
#include "DFC/HEAP.h"


#ifndef CMX_DOXYGEN  /* Internal only routine */
static __inline int  bit_get (const unsigned char *bits, int pos);
static __inline void bit_set (      unsigned char *bits, int pos, int state);


static int bit_get (const unsigned char *bits, int pos)
{
   unsigned char c = bits[pos / 8];
   return (c >> (pos % 8)) & 1;
}


static __inline void bit_set (unsigned char *bits, int pos, int state)
{
   unsigned char c = bits[pos / 8];
   unsigned char b = 0x80 >> (pos % 8);
   
   if (state) c |=  b;
   else       c &= ~b;

   bits[pos / 8] = c;
}


static __inline int destroy (void *data, BTREE *tree, HEAP *heap)
{
   if (data) free (data);
   if (tree) { BTREE_free (tree); free (tree); }
   if (heap) {  HEAP_free (heap);              }
   return -1;
}

       

static int compare_freq(const void *tree1, const void *tree2)
{

   HUFF_node           *root1,
                       *root2;


   /*  Compare the frequencies stored in the root nodes of two binary trees */
   root1 = (HUFF_node *)BTREE__data(BTREE__root((BTREE *)tree1));
   root2 = (HUFF_node *)BTREE__data(BTREE__root((BTREE *)tree2));

   if      (root1->freq < root2->freq)   return 1;
   else if (root1->freq > root2->freq)   return -1;
   return 0;
}


static void destroy_tree(void *tree)
{
    /*  Destroy and free one binary tree from the priority queue of trees. */
    BTREE_free(tree);
    free(tree);
    return;
}



static int build_tree(BTREE **tree, const int *freqs, int count)
{

   BTREE              *init,
                     *merge,
                      *left,
                     *right;

   HEAP               heap;

   HUFF_node         *data;

   int                status,
                      size,
                      c;

   
   /*  Initialize the priority queue of binary trees. */
   *tree = NULL;
    HEAP_init(&heap, compare_freq, destroy_tree);

    for (c =0; c < count; c++)
    {
        if (freqs[c] != 0)
        {
            /*  Set up a binary tree for current symbol and its frequency. */

            if ((init = (BTREE *)malloc(sizeof(BTREE))) == NULL)
            {
                return destroy (NULL, NULL, &heap);
            }

            BTREE_init(init, free);

            if ((data = (HUFF_node *)malloc(sizeof(HUFF_node))) == NULL)
            {
                return destroy (NULL, NULL, &heap);
            }

            
            data->symbol = c;
            data->freq   = freqs[c];

            if (BTREE_insertL (init, NULL, data) != 0)
            {
                return destroy (data, init, &heap);
            }

            /*  Insert the binary tree into the priority queue. */
            if (HEAP_insert(&heap, init) != 0)
            {
                return destroy (NULL, init, &heap);
            }
        }
    }


    /*  Build a Huffman tree by merging trees in the priority queue. */
    size = HEAP__size(&heap);

    for (c = 1; c <= size - 1; c++)
    {
        /*  Allocate storage for the next merged tree. */
        if ((merge = (BTREE *)malloc(sizeof(BTREE))) == NULL)
        {
           HEAP_free (&heap);
           return -1;
        }

        
        /*  Extract the two trees whose root nodes have smallest frequencies.*/
        if (HEAP_extract(&heap, (void **)&left) != 0)
        {
            HEAP_free (&heap);
            free (merge);
            return -1;
        }

        
        if (HEAP_extract(&heap, (void **)&right) != 0)
        {
            HEAP_free (&heap);
            free (merge);
            return -1;
        }
        
        
        /*  Allocate storage for the data in root node of the merged tree.*/
        if ((data =(HUFF_node *)malloc(sizeof(HUFF_node))) == NULL)
        {
            HEAP_free (&heap);
            free(merge);
            return -1;
        }

        memset(data, 0, sizeof(HUFF_node));

        
        /*  Sum frequencies in the root nodes of the trees being merged. */
        data->freq = ((HUFF_node *)BTREE__data (BTREE__root ( left)))->freq 
                   + ((HUFF_node *)BTREE__data (BTREE__root (right)))->freq;
        data->symbol = 0xaa;
        
        
        /*  Merge the two trees. */
        if (BTREE_merge(merge, left, right, data) != 0)
        {
            HEAP_free (&heap);
            free(merge);
            return -1;
        }

        /*  Insert the merged tree into the priority queue and free others.*/
        if (HEAP_insert(&heap, merge) != 0)
        {
            HEAP_free  (&heap);
            BTREE_free (merge);
            free(merge);
            return -1;
        }

        free (left);
        free (right);
    }


    /*  The last tree in the priority queue is the Huffman tree.  */
    status = HEAP_extract(&heap, (void **)tree);
    HEAP_free (&heap);
    return status;
}


static void build_table (BTREE_node    *node,
                         unsigned short code,
                         unsigned int   size,
                         HUFF_code    *table)
{
   if (!BTREE__is_eob (node))
   {

       if (!BTREE__is_eob(BTREE__left(node)))
       {
           /*  Move to the left and append 0 to the current. */
           build_table (BTREE__left(node), code << 1, size + 1, table);
       }

       
       if (!BTREE__is_eob(BTREE__right(node)))
       {
           /*  Move to the right and append 1 to the current code. */
           build_table (BTREE__right(node),
                        (code << 1) | 0x0001,
                        size + 1,
                        table);
       }

       
       if (BTREE__is_eob (BTREE__left(node)) &&
           BTREE__is_eob (BTREE__right(node)))
       {
           int symbol = ((HUFF_node *)BTREE__data (node))->symbol;
       
           /*  Ensure that the current code is in big-endian format. */
           //code = htons(code);
           
           /*  Assign the current code to the symbol in the leaf node. */
           table[symbol].used = 1;
           table[symbol].code = code;
           table[symbol].size = size;
       }

   }

   return;

}
#endif


/* ---------------------------------------------------------------------- *//*!
   
  \fn  void HUFF_accumulate (int                    *freqs,
                             const unsigned char *original,
                             int                      size)
  \brief Accumulates the frequency distribution 
  \param    freqs  The accumulated and appropriately scaled frequency
                   distribution. Must be at 256 characters in width
  \param original  The input set of characters. The distribution is
                   created from this set
  \param     size  The number of characters in \a original
								          */
/* ---------------------------------------------------------------------- */  
void HUFF_accumulate (int                    *freqs,
                      const unsigned char *original,
                      int                      size)
{
   int    c;
   int  max;
   int ipos;
   
 
   /*  Get the frequency of each symbol in the original data. */
   for (c =0; c <= UCHAR_MAX; c++) freqs[c] = 0;

   ipos = 0;
   if (size > 0)
   {
       while (ipos < size)
       {
           freqs[original[ipos]]++;
           ipos++;
       }
   }


   /*  Scale the frequencies to fit into one byte. */
   max = UCHAR_MAX;
   for (c = 0; c <= UCHAR_MAX; c++)
   {
       if (freqs[c] > max)  max = freqs[c];
   }

   
   for (c = 0; c <= UCHAR_MAX; c++)
   {
       int scale;
       scale = (int)(freqs[c] / ((double)max / (double)UCHAR_MAX));
       if (scale == 0 && freqs[c] != 0) freqs[c] = 1;
       else                             freqs[c] = scale;
   }

   return;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     int HUFF_build (HUFF_code *table, const int *freqs, int count)
  \brief  Builds a Huffman code table from the input frequency distribution
  \return Status

  \param table The table, must have at least \a count elements
  \param freqs The frequency distribution
  \param count The number of elements in the frequency distribution
									  */
/* ---------------------------------------------------------------------- */
int HUFF_build (HUFF_code *table, const int *freqs, int count)
{
   int       c;
   BTREE *tree;
 
 
   /*  Build the Huffman tree and table of codes for the data. */
   if (build_tree(&tree, freqs, count) != 0)  return -1;

   
   for (c = 0; c < count; c++)
   {
       memset(&table[c], 0, sizeof(HUFF_code));
   }
       
   build_table (BTREE__root (tree), 0x0000, 0, table);

   BTREE_free (tree);
   free (tree);

   return 0;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     int HUFF_compress(const unsigned char *original,
                            int                      size,
                            const HUFF_code        *table, 
                            unsigned char    **compressed)
  \brief  Compresses the input array \a original into the output array
  \return Size of the compressed data in bytes.

  \param   original  The array of bytes to compress
  \param       size  The number of bytes in original to compress
  \param      table  A previously built Huffman table used to encode the data
  \param compressed  Returned as a pointer to the compressed data
						                          */
/* ---------------------------------------------------------------------- */
int HUFF_compress(const unsigned char *original,
                  int                      size,
                  const HUFF_code        *table, 
                  unsigned char    **compressed)

{
   int             freqs[UCHAR_MAX + 1],
                   hsize,
                   ipos,
                   opos,
                   cpos,
                   c,
                   i;

   unsigned char   *comp,
                   *temp;

   /*  Initially there is no buffer of compressed data. */
   *compressed = NULL;

   
   /*  Write the header information. */
   hsize = sizeof(int) + (UCHAR_MAX + 1);
   
   if ((comp = (unsigned char *)malloc(hsize)) == NULL) return -1;

   memcpy(comp, &size, sizeof(int));

   for (c = 0; c <= UCHAR_MAX; c++)
   {
       comp[sizeof(int) + c] = (unsigned char)freqs[c];
   }
       

   /*  Compress the data. */
   ipos = 0;
   opos = hsize * 8;
   while (ipos < size)
   {
       /*  Get the next symbol in the original data. */
       c = original[ipos];

           
       /*  Write the code for the symbol to buffer of compressed data. */
       for (i = 0; i < table[c].size; i++)
       {
           if (opos % 8 == 0)
           {
               /*  Allocate another byte for buffer of compressed data. */
               temp = (unsigned char *)realloc(comp,(opos / 8) + 1);
               if (temp == NULL)
               {
                   free(comp);
                   return -1;
               }

               comp = temp;

           }

           cpos = (sizeof(short) * 8) - table[c].size + i;
           bit_set(comp,
                   opos,
                   bit_get((unsigned char *)&table[c].code, cpos));
           opos++;
       }

       ipos++;

   }

   /*  Point to the buffer of compressed data. */
   *compressed = comp;

   /*  Return the number of bytes in the compressed data.  */
   
   return ((opos - 1) / 8) + 1;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     int HUFF_uncompress (const unsigned char *compressed,
                               unsigned char        **original)
  \brief  Decompresses the input byte stream \a compressed
  \return The number of decompressed bytes, \e i.e. the size of original

  \param  compressed The compressed bit stream
  \param    original Returned as a pointer to the decompressed data
									  */
/* ---------------------------------------------------------------------- */
int HUFF_uncompress (const unsigned char *compressed,
                     unsigned char        **original)
{
   BTREE             *tree;
   BTREE_node        *node;
   int                freqs[UCHAR_MAX + 1],
                      hsize,
                      size,
                      ipos,
                      opos,
                      state,
                      c;

   unsigned char     *orig,
                     *temp;


   /*  Initially there is no buffer of original data. */
   *original = orig = NULL;

   
   /*  Get the header information from the buffer of compressed data. */
   hsize = sizeof(int) + (UCHAR_MAX + 1);
   memcpy(&size, compressed, sizeof(int));

   
   for (c = 0; c <= UCHAR_MAX; c++) freqs[c] = compressed[sizeof(int) + c];

   
   /*  Rebuild the Huffman tree used previously to compress the data. */
   if (build_tree(&tree, freqs, 256) != 0)  return -1;

   
   /*  Uncompress the data. */
   ipos = hsize * 8;
   opos = 0;
   node = BTREE__root (tree);

   
   while (opos < size)
   {
       /*  Get the next bit in the compressed data. */
       state = bit_get(compressed, ipos);
       ipos++;
       if (state == 0)
       {
           /*  Move to the left. */
           if (BTREE__is_eob (node) || BTREE__is_eob (BTREE__left(node)))
           {
               BTREE_free (tree);
               free (tree);
               return -1;
           }
           else
           {
               node = BTREE__left(node);
           }
       }
       
       else
       {
           /*  Move to the right. */
           if (BTREE__is_eob (node) || BTREE__is_eob (BTREE__right(node)))
           {
               BTREE_free (tree);
               free(tree);
               return -1;
           }
           else
           {
               node = BTREE__right(node);
           }
       }
       

       
       if (BTREE__is_eob (BTREE__left  (node)) &&
           BTREE__is_eob (BTREE__right (node)))
       {
           /*  Write the symbol in leaf node to buffer of original data. */
           if (opos > 0)
           {
               if ((temp =  (unsigned char *)realloc(orig, opos + 1)) == NULL)
               {
                   BTREE_free (tree);
                   free (tree);
                   free (orig);
                   return -1;
               }

               orig = temp;
           }
           else
           {
               if ((orig = (unsigned char *)malloc(1)) == NULL)
               {
                   BTREE_free (tree);
                   free(tree);
                   return -1;
               }
           }
       }
       

       orig[opos] = ((HUFF_node *)BTREE__data(node))->symbol;
       opos++;

       /*  Move back to the top of the tree. */
       node = BTREE__root (tree);
       
   }
   

   BTREE_free (tree);
   free(tree);

   /*  Point to the buffer of original data. */
   *original = orig;

   /*  Return the number of bytes in the original data. */
   return opos;

}
/* ---------------------------------------------------------------------- */

