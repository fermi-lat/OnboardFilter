/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_fileIn-Gleam.c
   \brief EBF File Input Utilities
   \author JJRussell - russell@slac.stanford.edu

    Reads an Event Builder File and makes it look like the in memory
    representation of data from the event builder.

\verbatim
    CVS $Id

\endverbatim    
                                                                         */
/* --------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include "windowsCompat.h"

#include "DFC/DFC_endianness.h"
#include "DFC/EBF_fileIn-Gleam.h"


/* --------------------------------------------------------------------- *//*!

  \struct _EBF_file
  \brief   Internal structure used to control the access to a EBF file
                                                                         */
/* --------------------------------------------------------------------- */
struct _EBF_file
{
    int               fd; /*!< File descriptor                     */
    const char     *name; /*!< Name of the file being processed    */
    int             size; /*!< Size of the file (in bytes)         */
    size_t      seg_size; /*!< Buffering size (unused)             */
    unsigned int  *edata; /*!< Beginning of the event data         */
    void           *free; /*!< Pointer to the memory to free       */
};
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *\

    Files read on little endian machines, must be swapped around a bit.
    There is no fixup needed on big endian machines.
        
\* --------------------------------------------------------------------- */
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_LITTLE

#include "EBF_swap.h"
#include "DFC/LCB.h"
#include "DFC/EBF_cid.h"




/* --------------------------------------------------------------------- *//*!

   \def    _ADVANCE(_ptr, _nbytes)
   \brief   Advances \a _ptr by \a _nbytes. The return value is always
            recast to the type of \a _ptr.
   \param   _ptr    The pointer to advance
   \param   _nbytes The number of nbytes to advance \a _ptr by.
   \return  The advanced pointer, recast to the type of \a _ptr.
                                                                         */
/* --------------------------------------------------------------------- */
#define _ADVANCE(_ptr, _nbytes) \
         ((unsigned char *)(_ptr) + (_nbytes))
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
static inline void halfswap  (unsigned short int *beg, int cnt);
static inline  int fixup_acd (unsigned       int *evt, int elen, int ievt);
static         int fixup     (unsigned       int *evt, int elen);
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn         int fixup (unsigned int *evt, int elen)
  \brief      Fixups up the data for little endian machines
  \param evt  Pointer to the data
  \param elen Size of the data, in bytes
  \return     Status
                                                                         */
/* --------------------------------------------------------------------- */
static int fixup (unsigned int *evt, int elen)
{
   int ievt;
   
    
   /* First swap all the 32 bit words */ 
   EBF_swap32_lclXbigN (evt, elen / sizeof (int));
   ievt = 0;
   
    
   /*
    | Unfortunately the ACD data is 16-bit data,
    | so must locate and swap it on an event by event basis
   */
   while (elen > 0)
   {
       int  esize;
       int status;
  
       /*
        |  Retrieve the total size of this event (in bytes). Since the event
        |  length word is included in the total length, the remaining number
        |  of bytes (elen) is reduced by the size of the event length word.
       */
       esize  = evt[0];
       status = fixup_acd (evt   + 1,
                           esize - sizeof (*evt),
                           ievt);
       if (status) return status;
       
       evt   = (unsigned int *)_ADVANCE (evt, esize);  /* Advance to the next event        */
       elen -= esize;                  /* Reduce number of bytes remaining */
       ievt += 1;                      /* Count number of events processed */
   }
   
   return 0;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn         int fixup_acd (unsigned int *evt, int elen)
  \brief      Fixups up the ACD data for little endian machines
  \param evt  Pointer to the ACD data
  \param elen Size of the ACD data, in bytes
  \return     Status
                                                                         */
/* --------------------------------------------------------------------- */
static int fixup_acd (unsigned int *evt, int elen, int ievt)
{
   while (elen > 0)
   {
       unsigned int hdr_clen;
       unsigned int     clen;
       unsigned int      cid;
       

       /*
        |  Get the length of the event in bytes and reduce the number of
        |  unprocessed bytes
       */
       hdr_clen = ((LCB_header *)evt)->hdr_clen; 
       clen     =  LCB_EXTRACT_CLEN (hdr_clen);
       elen    -=  clen;

       
       /*
        | If the contributor's length is precisely 0 or not a multiple of
        | an LCB packet, something's wrong
       */
       if ( (clen == 0) || (clen & LCB_M_PACKET_ALIGN) )
       {
           printf (" Bad contributor length at event = %d\n"
                   "                          length = %d\n"
                   "     Length cannot be 0 and must be an event number of\n"
                   "     LCB packets packets (16 bytes)",
                   ievt,
                   clen);
           return -1;
       }

       
       /* Check if this is the ACD */
       cid = LCB_EXTRACT_CID (hdr_clen);
       if (cid == EBF_K_CID_ACD)
       {
           unsigned short int *ptr;
           
           /* Skip over the LCB header, decrease contribution length by same */
           ptr   = (unsigned short int *)_ADVANCE (evt, sizeof (LCB_header));
           clen -= sizeof (LCB_header);
           
           halfswap (ptr, clen / sizeof (int));
           
           return 0;
       }
       
       
       /*
        | Advance the event pointer to the next contribution.
       */
       evt = (unsigned int *)_ADVANCE (evt, clen);
   }

   
   /* No ACD contribution, weird, but okay */
   return 0;
       
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*

  \fn  unsigned int *halfswap (unsigned short int *beg, int cnt)
  \brief     Swaps all the 16 bit words in the specified range.
  \param ptr The starting point to begin the swap
  \param cnt The number of longwords to swap
                                                                         */
/* --------------------------------------------------------------------- */
static void halfswap (unsigned short int *ptr, int len) 
{
    while (--len >= 0)
    {
        unsigned short int tmp0 = ptr[0];
        unsigned short int tmp1 = ptr[1];
        
        ptr[0] = tmp1;
        ptr[1] = tmp0;
        
        ptr += 2;
    }
    
    return;
}
/* --------------------------------------------------------------------- */


#else
#define fixup(evt, elen)
#endif


/* --------------------------------------------------------------------- */









/* --------------------------------------------------------------------- *//*!

  \fn     EBF_file *EBF_dmap (unsigned int *ebd)
  \brief  Prepares an in memory data structure to be read

  \param  ebd  Pointer to the in memory data structure.
  \return      If successful, the file handle,
               else NULL
                                                                         */
/* --------------------------------------------------------------------- */
EBF_file *EBF_dmapGleam (unsigned int *ebd)
{
   EBF_file *ebf;
   int      size;
   
   ebf = (EBF_file *)malloc (sizeof (*ebf));

   if (ebf == NULL)
   {
       printf ("ERROR: Unable to allocate memory\n");
       exit (1);
   }

   
   size          = ebd[0];              /* Size (in bytes) of entire data */
   ebf->fd       = -1;                  /* This is not a file             */
   ebf->name     = NULL;                /* This is not a file             */
   ebf->size     = size;                /* Total size                     */
   ebf->edata    = ebd + 1;
   ebf->free     = ebf;

   return ebf;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn     EBF_file *EBF_open (const char *fileName, int segment_size)
  \brief  Opens a EBF file for reading
  
  \param      fileName The name of the EBF file to open
  \param  segment_size The largest portion of the file that will be
                       buffered on a read. (Currently unused, the
                       whole file is read in all the time.
  \return              If successful, the file handle
                       else, NULL
                         
                                                                         */
/* --------------------------------------------------------------------- */
//EBF_file *EBF_open (const char *fileName, int segment_size)
EBF_file *EBF_openGleam(int fileSize)
{
   //int             fd;
   //int            err;
   EBF_file      *ebf;
   size_t        size;
   //struct stat  stats;
   unsigned char *ptr;
   int         nbytes;
   
   
   /* Open the data file and retrieve the stats so can find # of bytes */
   //fd = open (fileName, O_RDONLY, 0);
   //if (fd == 0)
   //{
   //    printf ("ERROR: Unable to open input file: %s\n", fileName);
   //    exit (1);
   //} 

   
   /*
    | Get the file statistics.
    | What is really needed is the size of the file.
   */
   //err = fstat (fd, &stats);
   //if (err)
   //{
   //    printf ("ERROR: Unable to retrieve stats from file: %s ERR = %x\n",
   //            fileName, err);
   //}


   /* Allocate memory big enough to hold the entire file + EB file handle */
   //size   = stats.st_size;
   size   = fileSize;
   nbytes = size + sizeof (*ebf);
   ptr    = (unsigned char *)malloc (nbytes);

   if (ptr == NULL)
   {
       printf ("ERROR: Unable to allocate memory\n");
       exit (1);
   }

   /* Position the event builder file handle at the end of the buffer */
   ebf           = (EBF_file *)(ptr + nbytes) - 1;
   ebf->fd       = 0;
   ebf->name     = "";
   ebf->size     = size;
   ebf->seg_size = 0;
   ebf->edata    = (unsigned int *)ptr;
   ebf->free     = ptr;

   
   return ebf;
}
/* --------------------------------------------------------------------- */
  



/* --------------------------------------------------------------------- *//*!

  \fn        int EBF_read (EBF_file *ebf)
  \brief     Reads the EBF file associated with the event file handle \ebf
  \param ebf A previously opened event builder file handle
  \retval    0, if successful
  \retval   -1, if not
                                                                         */
/* --------------------------------------------------------------------- */  
int EBF_readGleam (EBF_file *ebf,unsigned int *data)
{
   size_t            size;
   unsigned int      *ptr;
   

   /* Extract the size of the data and the pointer to the data */
   size  = ebf->size;
   ptr   = ebf->edata;

   
   /* If this is a file, read it in */   
   //if (ebf->fd > 0)
   //{
   //ssize_t nread;         
   //nread = read (ebf->fd, (char *)ptr, size);
   memcpy(ptr,data,size);
   /* Check what was request was actually read */
   //if (nread != size)
   //{
   //printf ("ERROR: Unable to read (%d/%d) entire file: %s\n",
   //nread,
   //size,
   //ebf->name);
   //return -1;
   //}
       
       
   /* Must swap the data to local representation */
   fixup ((unsigned int *)ptr, size);
   //}
   return 0;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn int EBF_close (EBF_file *ebf)
  \brief  Closes the file after it has been read.

   This function only closes the file after its contents have been read
   into memory. To free the memory associated with the contents of the
   file, the user must call \a EBF_free().
                                                                         */
/* --------------------------------------------------------------------- */
int EBF_closeGleam (EBF_file *ebf)
{
   return 0;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn    int EBF_free (EBF_file *ebf)
  \brief Frees the memory associated with the contents of the file.

   After calling EBF_file, the contents and the event builder file handle
   are no longer valid.
                                                                         */
/* --------------------------------------------------------------------- */
int EBF_free (EBF_file *ebf)
{
   free (ebf->free);
   return 0;

}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn       const unsigned int *EBF_edata (const EBF_file *ebf)
  \brief    Returns a pointer to the first event.

  This function is called after EBF_read to locate the first event.
  Subsequent events are found by advancing this pointer by the size
  of the event. The size of the event is stored in the first 32-bit
  word of the event.
                                                                         */
/* --------------------------------------------------------------------- */
const unsigned int *EBF_edataGleam (const EBF_file *ebf)
{
   return ebf->edata;

}
/* --------------------------------------------------------------------- */



   
/* --------------------------------------------------------------------- *//*!

  \fn     int EBF_esize (const EBF_file *ebf)
  \brief  Returns the size of the event data (in bytes)
                                                                         */
/* --------------------------------------------------------------------- */
unsigned int EBF_esizeGleam (const EBF_file *ebf)
{
   return ebf->size;
}
/* --------------------------------------------------------------------- */




