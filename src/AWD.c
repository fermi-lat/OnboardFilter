/* ---------------------------------------------------------------------- *//*!
   
   \file  AWD.c
   \brief Arithmetic Byte Decoder, implementation file
   \author JJRussell - russell@slac.stanford.edu

    Implementation of the routines to decode bit streams using an 
    arithmetic probability encoding technique. The encoding tables are
    32-bits wide. This allows probabilities to (in principle) hold a range
    of 1/2**32. Contrast this with the ABD routines which only use a range
    of 1/2**16. 

    Because one uses 'all the bits', on a 32-bit machine, overflows must
    be carefully controlled. For this reason, these routines are a little
    more than just a straight transcription of the 16-bit routines.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/AWD.h"
#ifdef WIN32
#define Int64 __int64
#else
#define Int64 long long
#endif

/*
 |  These objects are all internal variables. There is no need to 
 |  publically document these objects.
*/
#ifndef CMX_DOXYGEN

/* Code value parameters                                                  */
#define AWD_K_NBITS  32                      /* # of bits in code value   */ 
#define AWD_K_HI    (unsigned int)0xffffffff /* Largest code value        */
#define AWD_K_Q1    (unsigned int)(1 << (AWD_K_NBITS - 2)) 
                                             /* Point after first quarter */ 
#define AWD_K_HALF  (unsigned int)(2  *  AWD_K_Q1) 
                                             /* Point after first half    */
#define AWD_K_Q3    (unsigned int)(3  *  AWD_K_Q1)
                                             /* Point after third quarter */


/* INPUT A BIT. */
#define add_input_bit(_v, _in, _buffer, _bits_to_go) \
do                                                   \
{   _bits_to_go -= 1;                                \
    if (_bits_to_go < 0)                             \
    {                                                \
        _buffer = *_in++;                            \
        _bits_to_go = 7;                             \
    }                                                \
    _v <<= 1;                                        \
    /* printf ("Get bit %1.1x\n", (_buffer >> _bits_to_go) & 1); */ \
    _v  |= (_buffer >> _bits_to_go) & 1;             \
} while (0)


static __inline unsigned int scale (unsigned int range,
                                    unsigned int   val,
                                    unsigned int   den);

static __inline unsigned int scale (unsigned int range,
                                    unsigned int   val,
                                    unsigned int   den)
{
  unsigned Int64   r = range ? range : 0x100000000;
  unsigned Int64   v = val;
  unsigned Int64 num = r * v;

  /* If the denominator is 0, it really wanted to be 1 << 32 */
  return den ? (num / den) : (num >> 32);
}

static __inline unsigned int scale_m1 (unsigned int range,
                                       unsigned int   val,
                                       unsigned int   den);

static __inline unsigned int scale_m1 (unsigned int range,
                                       unsigned int   val,
                                       unsigned int   den)
{
  unsigned Int64   r = range;
  unsigned Int64   v = val;
  unsigned Int64 num = r * v - 1;
  
  /* If the denominator is 0, it really wanted to be 1 << 32 */
  return den ? (num / den) : (num >> 32);
}
#endif


/* ---------------------------------------------------------------------- *//*!

  \fn   void AWD_start (AWD_dtx            *dtx,
                        const unsigned char *in,
                        unsigned int       boff)
  \brief Begins a decoding session
  
  \param  dtx  The decoding context to be initialized
  \param   in  The input bit stream
  \param boff  The bit offset into the input bit stream
									  */
/* ---------------------------------------------------------------------- */
extern void AWD_start (AWD_dtx            *dtx,
                       const unsigned char *in,
                       unsigned int       boff)
{
  int                   i;
  unsigned int     buffer;

  unsigned int      value = 0;
  int          bits_to_go = 0;


  /* Move to the nearest byte offset */
  in       += boff >> 3; 
  boff     &= 0x7;


  /* Pick up the shard, this can be from 1 to 8 bits */
  dtx->beg   =  in;
  buffer     = *in++;
  bits_to_go =  8 - boff;
  dtx->bbeg  =  boff;

  //printf ("Input buffer = %8.8x\n", buffer);

  /* Input bits to fill the code value word */
  for (i = 1; i <= AWD_K_NBITS; i++)
  {	
    add_input_bit (value, in, buffer, bits_to_go);
  }

  dtx->lo     = 0;
  dtx->hi     = AWD_K_HI;
  dtx->togo   = bits_to_go;
  dtx->value  = value;
  dtx->buffer = buffer;
  dtx->cur    = in;

  return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int AWD_bdecompress (unsigned char        *out,
                               int                   cnt,
		               const unsigned char   *in,
                               unsigned int         boff,
		              const unsigned int *table)
  \brief  Convenience routine to decode a bit stream using the specified
          table
  \return The number of decoded bits 

  \param  out  The output buffer
  \param  cnt  The number of bytes in the output buffer
  \param   in  The input buffer
  \param boff  The bit offset to start at in the input buffer
  \param table The decoding table

   This is a convenience routine, combining AWD_start, AWD_decode and
   AWD_finish. This routine can only be used if the decoding table is
   the same for all symbols in the input stream.
									  */
/* ---------------------------------------------------------------------- */
int  AWD_bdecompress (unsigned char        *out,
		      int                   cnt,
		      const unsigned char   *in,
		      unsigned int         boff,
		      const unsigned int *table)
{
    AWD_dtx dtx;


    AWD_start (&dtx, in, boff);

    while (--cnt >= 0)
    {
      int symbol;
      *out++ = symbol = AWD_decode (&dtx, table);
    }
    
    return AWD_finish (&dtx);
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn      unsigned int  AWD_decode     (AWD_dtx              *dtx,
		                         const unsigned int *table)
  \brief   Decodes the next symbol
  \return  The decoded symbol

  \param   dtx   The decoding context
  \param table   The table to use in the decoding
									  */
/* ---------------------------------------------------------------------- */
extern unsigned int  AWD_decode (AWD_dtx              *dtx,
				 const unsigned int *table)
{
    register int                cum;
    register const unsigned int * p;			
    int                      symbol;
    long                      range;

    unsigned int         lo = dtx->lo;
    unsigned int         hi = dtx->hi;
    int          bits_to_go = dtx->togo;
    unsigned int      value = dtx->value;
    unsigned int     buffer = dtx->buffer;
    const unsigned char *in = dtx->cur;
    const unsigned     *top = table + table[0];
    unsigned int        max = *top;

        
    range = (long)(hi - lo) + 1;
    cum   = scale_m1 (value - lo + 1, max, range);
    //    cum   = (((long)(value - lo) + 1) * max - 1) / range;
    table = table + 1;
	
        
    /* Find the symbol */
    p     = top;
    while (1)
    {
      // printf (" %x > %x %x > %x\n", p[-1], cum, p[-1]*range, cum*range);
      if (*--p > cum) continue;
      else            break;
    }
        
        
    symbol = p - table;

    /*
     |  Narrow the code region  to that alloted to this symbol 
     |  This must be done independent of whether the input stream 
     |  is exhausted, so that the bit count can be correctly
     |  calculated in AWD_finish
    */
    hi = lo + scale (range, table[symbol+1], max) - 1;
    lo = lo + scale (range, table[symbol  ], max);
        
    while (1)
    {    
       /* Loop to get rid of bits. */

       // printf ("Val = %x Lo:Hi = [%8.8x,%8.8x)\n", value, lo, hi);
            
       if      (hi <  AWD_K_HALF)
       {
	 /* Expand low half.         */
	 /* nothing */                    
       } 
       else if (lo >= AWD_K_HALF)
       {
        /* Expand high half, subtract offset to top.*/
	 value -= AWD_K_HALF;
	 lo    -= AWD_K_HALF;		
	 hi    -= AWD_K_HALF;
       }
       else if (lo >= AWD_K_Q1 && hi < AWD_K_Q3)
       {
	 /* Expand middle half, subtract offset to middle*/
	 value -= AWD_K_Q1;
	 lo    -= AWD_K_Q1;
	 hi    -= AWD_K_Q1;
       }
       else 
       {
         /* Otherwise exit loop.     */
	 break;
       }
            
       /* Scale up code range.     */
       lo <<= 1;
       hi <<= 1;
       hi  |= 1;		        

       /* Move in next input bit.  */
       add_input_bit(value, in, buffer, bits_to_go);
    }


    // printf ("Val = %x Lo:Hi = [%8.8x,%8.8x)\n", value, lo, hi);
    dtx->lo     = lo;
    dtx->hi     = hi;
    dtx->togo   = bits_to_go;
    dtx->value  = value;
    dtx->buffer = buffer;
    dtx->cur    = in;

    /*
    printf ("Decoded Symbol = %x bits_to_go = %d\n", 
	    symbol, dtx->togo);
    */
    
    return symbol;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     int AWD_finish (AWD_dtx *dtx)
  \brief  Finishes the decoding, cleaning up any inprogress context
  \return The number of bits decoded
									  */
/* ---------------------------------------------------------------------- */
int AWD_finish (AWD_dtx *dtx)
{
  return 8*(dtx->cur - dtx->beg)  /* This is the number of bits 'read'   */
         - dtx->togo              /* Subtract off those not yet consumed */
         - AWD_K_NBITS            /* Subtract off the initial read       */
         - dtx->bbeg              /* Number of unused bits in byte 0     */
         + 1                      /* Last bit still in code word         */
         + 1;                     /* Haven't figure this 1 out yet       */

} 
