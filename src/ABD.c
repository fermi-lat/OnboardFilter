/* ---------------------------------------------------------------------- *//*!
   
   \file  ABD.c
   \brief Arithmetic Byte Decoder, implementation file
   \author JJRussell - russell@slac.stanford.edu

    Implementation of the routines to decode bit streams using an 
    arithmetic halfword (16-bit) encoding technique. It is based on a
    16-bit table (hence the halfword notation) giving the relative 
    probabilities of the encoded symbols.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/ABD.h"

/*
 |  These objects are all internal variables. There is no need to 
 |  publically document these objects.
*/
#ifndef CMX_DOXYGEN

/* Code value parameters                                                  */
#define ABD_K_NBITS  16                      /* # of bits in code value   */ 
#define ABD_K_HI   ((1 <<  ABD_K_NBITS) - 1) /* Largest code value        */   
#define ABD_K_Q1    (1 << (ABD_K_NBITS - 2)) /* Point after first quarter */ 
#define ABD_K_HALF  (2  *  ABD_K_Q1)         /* Point after first half    */
#define ABD_K_Q3    (3  *  ABD_K_Q1)         /* Point after third quarter */


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
    _v  |= (_buffer >> _bits_to_go) & 1;             \
} while (0)
#endif


/* ---------------------------------------------------------------------- *//*!

  \fn   void ABD_start (ABD_dtx            *dtx,
                        const unsigned char *in,
                        unsigned int       boff)
  \brief Begins a decoding session
  
  \param  dtx  The decoding context to be initialized
  \param   in  The input bit stream
  \param boff  The bit offset into the input bit stream
									  */
/* ---------------------------------------------------------------------- */
extern void ABD_start (ABD_dtx            *dtx,
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
  for (i = 1; i <= ABD_K_NBITS; i++)
  {	
    add_input_bit (value, in, buffer, bits_to_go);
  }

  dtx->lo     = 0;
  dtx->hi     = ABD_K_HI;
  dtx->togo   = bits_to_go;
  dtx->value  = value;
  dtx->buffer = buffer;
  dtx->cur    = in;

  return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int ABD_bdecompress (unsigned char        *out,
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

   This is a convenience routine, combining ABD_start, ABD_decode and
   ABD_finish. This routine can only be used if the decoding table is
   the same for all symbols in the input stream.
									  */
/* ---------------------------------------------------------------------- */
int  ABD_bdecompress (unsigned char        *out,
                      int                   cnt,
		      const unsigned char   *in,
                      unsigned int         boff,
		      const unsigned int *table)
{
    ABD_dtx dtx;

    ABD_start (&dtx, in, boff);

    while (--cnt >= 0)
    {
      int symbol;
      *out++ = symbol = ABD_decode (&dtx, table);
      //printf ("Decoded Symbol = %x (left = %d) bits_to_go = %d\n", 
      //	  symbol, cnt, dtx.togo);
    }
    
    return ABD_finish (&dtx);
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn      unsigned int ABD_decode (ABD_dtx              *dtx,
		                    const unsigned int *table)
  \brief   Decodes the next symbol
  \return  The decoded symbol

  \param   dtx   The decoding context
  \param table   The table to use in the decoding
									  */
/* ---------------------------------------------------------------------- */
extern unsigned int ABD_decode (ABD_dtx              *dtx,
				const unsigned int *table)
{
    int                      symbol;
    register int                cum;
    register const unsigned int * p;			
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
    cum   = (((long)(value - lo) + 1) * max - 1) / range;
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
     |  calculated in ABD_finish
    */
    hi = lo + (range * table[symbol+1] ) / max - 1;
    lo = lo + (range * table[symbol  ])  / max;
        
    while (1)
    {    
       /* Loop to get rid of bits. */

       // printf ("Val = %x Lo:Hi = [%8.8x,%8.8x)\n", value, lo, hi);
            
       if      (hi <  ABD_K_HALF)
       {
	 /* Expand low half.         */
	 /* nothing */                    
       } 
       else if (lo >= ABD_K_HALF)
       {
        /* Expand high half, subtract offset to top.*/
	 value -= ABD_K_HALF;
	 lo    -= ABD_K_HALF;		
	 hi    -= ABD_K_HALF;
       }
       else if (lo >= ABD_K_Q1 && hi < ABD_K_Q3)
       {
	 /* Expand middle half, subtract offset to middle*/
	 value -= ABD_K_Q1;
	 lo    -= ABD_K_Q1;
	 hi    -= ABD_K_Q1;
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
    
    return symbol;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int ABD_finish (ABD_dtx *dtx)
  \brief  Finishes the decoding, cleaning up any inprogress context
  \return The number of bits decoded
									  */
/* ---------------------------------------------------------------------- */
int ABD_finish (ABD_dtx *dtx)
{
  return 8*(dtx->cur - dtx->beg)  /* This is the number of bits 'read'   */
         - dtx->togo              /* Subtract off those not yet consumed */
         - ABD_K_NBITS            /* Subtract off the initial read       */
         - dtx->bbeg              /* Number of unused bits in byte 0     */
         + 1                      /* Last bit still in code word         */
         + 1;                     /* Haven't figure this 1 out yet       */

} 
/* ---------------------------------------------------------------------- */
