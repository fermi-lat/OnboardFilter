/* ---------------------------------------------------------------------- *//*!
   
   \file  HEAP.c
   \brief HEAP, implementation file
   \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim

  \warning
   In their current state, these routines are not fit for Flight use.
   They can be used in studies. However they malloc all over the place
   and involve recursive calling techniques, neither of is appropriate
   for Flight code.
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdlib.h>
#include "DFC/HEAP.h"


#ifndef CMX_DOXYGEN  /* Internal only routines               */
static __inline int parent(int npos) { return (npos - 1) / 2; }
static __inline int left  (int npos) { return 2*npos + 1;     }
static __inline int right (int npos) { return 2*npos + 2;     }
#endif 


/* ---------------------------------------------------------------------- *//*!

  \fn   void HEAP_init  (HEAP           *heap,
                         HEAP_compare compare,
                         HEAP_destroy destroy)
  \brief Initializes a heap for use

  \param    heap  The heap control structure to initialize
  \param compare  Callback comparision routine
  \param destroy  Callback destruction (free) routine
									  */
/* ---------------------------------------------------------------------- */
void HEAP_init  (HEAP           *heap,
                 HEAP_compare compare,
                 HEAP_destroy destroy)
{
   heap->size    = 0;
   heap->compare = compare;
   heap->destroy = destroy;
   heap->tree    = (void *)(0);
   return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     HEAP_free (HEAP *heap)
  \brief  Frees all the nodes of the heap

  \param  heap The heap to free
									  */
/* ---------------------------------------------------------------------- */
void HEAP_free (HEAP *heap)
{
    HEAP_destroy destroy = heap->destroy;

    /* Check if have a user defined destroy function */
    if (destroy)
    {
        /* Yes, run it on each node */
        int i;
        for (i = 0; i < HEAP__size (heap); i++) (*destroy)(heap->tree[i]);
    }

    
    /* Free the storage for the heap itself */
    free (heap->tree);
    
    return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int HEAP_insert (HEAP *heap, const void *data)
  \brief  Inserts a node with the specified data on the heap
  \return Status

  \param  heap The heap to insert the node on
  \param  data The data 

   The only reason for failure is if the \a heap is specified as NULL
									  */
/* ---------------------------------------------------------------------- */
int HEAP_insert (HEAP *heap, const void *data)
{
   int             ipos;
   int             ppos;
   void          **tree;   
   HEAP_compare compare;
   
   int   size = HEAP__size(heap);

   /* Allocate storage for the new node */
   tree = (void **)realloc (heap->tree, (size + 1) * sizeof (void *));

   
   /* Check that we got the memory */
   if (tree == NULL) return -1;

   
   /* Okay, this is the tree now   */
   heap->tree = tree;
   

   /* Insert the node at the end */
   heap->tree[size] = (void *)data;
   ipos             = size;
   ppos             = parent (ipos);
   compare          = heap->compare;
   
   
   /* Bubble the node upwards */
   while (ipos > 0)
   {
       void *idat = tree[ipos];
       void *pdat = tree[ppos];
       
       if (compare (pdat, idat) >= 0) break;

       /* Swap the data */
       tree[ppos] = idat;
       tree[ipos] = pdat;
       
       ipos       = ppos;
       ppos       = parent (ipos);
   }

   
   /* New heap size */
   heap->size = size + 1;
   
   return 0;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int HEAP_extract (HEAP *heap, void **data)
  \brief  Extracts (removes) the top node of the heap
  \return Status

  \param  heap The heap to remove the node from
  \param  data Returned as a pointer to the data associated with the node

   The return value is -1 if the heap is empty or an internal error 
   occurs.
									  */
/* ---------------------------------------------------------------------- */
int HEAP_extract (HEAP *heap, void **data)
{
   void  *save;
   int    size;
   int    ipos;
   int    lpos;
   int    rpos;
   void **tree;
   HEAP_compare compare;
   
   
   size = HEAP__size (heap);
   if (size == 0) return -1;
   

   tree = heap->tree;
  *data = tree[0];
   save = tree[size - 1];

   
   if ((size - 1) > 0)
   {
       tree = (void **)realloc (tree, (size - 1) * sizeof (void *));
       if (tree == NULL) return -1;

       /* Adjust the size of the heap */
       size      -= 1;
       heap->size = size;
       heap->tree = tree;
   }
   else
   {
       free (heap->tree);
       heap->tree = NULL;
       heap->size = 0;
       return 0;
   }

   
   /* Copy the last node to the top, push the new node downwards  */   
   tree[0] = save;
   ipos    = 0;
   compare = heap->compare;

   while (1)
   {
       int   mpos;
       void *idat;
       
       
       lpos = left  (ipos);
       rpos = right (ipos);
       idat = tree[ipos];
       

       /* Select which child to swap with the current node */
       mpos = (lpos < size && compare (tree[lpos], idat) > 0) ? lpos : ipos;
       if (rpos < size && compare (tree[rpos], tree[mpos]) > 0)
       {
           mpos = rpos;
       }
       

       
       
       /* When mpos is ipos, the heap property has been restored */
       if (mpos == ipos) break;
       else
       {
           /* Else, swap the contents with the selected child */
           void *mdat = tree[mpos];
           tree[mpos] = idat;
           tree[ipos] = mdat;
       }

       
       /* Move down one level and continue */
       ipos = mpos;
   }

   return 0;
}
/* ---------------------------------------------------------------------- */
       
           

   
