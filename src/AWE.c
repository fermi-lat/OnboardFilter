/* ---------------------------------------------------------------------- *//*!
   
   \file  AWE.c
   \brief Arithmetic Byte Encoder, implementation file
   \author JJRussell - russell@slac.stanford.edu

    Implementation of the routines to encode symbols using an arithmetic
    word encoding technique. The \e word refers to the width of the
    encoding tables, in this case 32-bits. This allows probabilities with
    ranges up to 1:2**32 to be encoded. If the range is less than 2**16,
    consider using the ABx routines. They are quicker, but may sacrifice
    some compression if the probability range exceeds 2**16.

   \warning 
    Technically the use of \e probability \e range is somewhat of a 
    misnomer. The actual condition is that the sum of the entries in the
    frequency distribution table handed to AWE_build is less than 2**16
    or 2**32.
                                                                          */
/* ---------------------------------------------------------------------- */


/*
 |  These objects are all internal variables. There is no need to 
 |  publically document these objects.
*/
#ifndef CMX_DOXYGEN
#include "DFC/AWE.h"

#ifdef WIN32
#define Int64 __int64
#else
#define Int64 long long
#endif

/* Code value parameters                                                  */
#define AWE_K_NBITS  32                      /* # of bits in code value   */ 
#define AWE_K_HI    (unsigned int)0xffffffff /* Largest code value        */
#define AWE_K_Q1    (unsigned int)(1 << (AWE_K_NBITS - 2)) 
                                             /* Point after first quarter */ 
#define AWE_K_HALF  (unsigned int)(2  *  AWE_K_Q1) 
                                             /* Point after first half    */
#define AWE_K_Q3    (unsigned int)(3  *  AWE_K_Q1)
                                             /* Point after third quarter */

/* 
 | This sets the normalization. It cannot be greater than half the range
 | of the code value word. If it is more than half the range, there is a
 | danger that, due to rounding, the HI limit of the range will be less
 | than the LO limit.
 */
#define AWE_K_NORM  (1 << (AWE_K_NBITS - 1))


static __inline unsigned int scale (unsigned int range, 
                                    unsigned int   val,
                                    unsigned int   max);

static __inline unsigned int scale (unsigned int range, 
                                    unsigned int   val,
                                    unsigned int   max)
{
  unsigned Int64 v = val;
  unsigned Int64 r = range;
  unsigned Int64 a = ((r * v) + v) / max;
  

  //printf ("(%8.8llx * %8.8llx) + %8.8llx/ %8.8x = %8.8llx\n", r, v, v, max, a);

  return a;
}
#endif


/* ---------------------------------------------------------------------- *//*!

  \fn     unsigned int AWE_build (unsigned int          *table,
                                  const unsigned int        *f,
		                  int                      cnt)
  \brief  Creates an appropriately scaled probability table based on the 
          the frequency distribution, \a f.
  \return The sum of the frequency distribution (not all that interesting)

  \param  table  The table to be filled out. This must be an array of
                 \a cnt + 1 values.
  \param      f  The frequency distribution of the symbols. There must
                 be one entry, indexed by the symbol value, for each symbol
  \param    cnt  The number of entries (or the number of symbols, they are
                 equivalent) in \a f.

   As a performance hint, symbols that have 0 probability of occurring 
   should not be present in the frequency distribution. In this case, one
   should define a lookup table mapping only the non-zero values. A lookup
   table is almost mandatory since the frequency table must be sorted by
   decreasing frequency. 
									  */
/* ---------------------------------------------------------------------- */
unsigned int AWE_build (unsigned int          *table,
			const unsigned int        *f,
			int                      cnt)
{
   unsigned int        *tbl;
   unsigned int         sum;
   int                 zcnt;
   int           correction;
   int                 left;
   int                  idx;
   int                 iter;


   /* Entry 0 is always filled with the element count */
   table[0] = cnt + 1;


   /*
    | Count the number of hard zeros, these will be changed to a 1
    | but never anything bigger or smaller 
   */
   zcnt = 0;

   /* 
    | Get the sum, 
    | Fill 0 entries with a 1 so these symbols can be represented
   */
   for (tbl = table + 1, sum = idx = 0; idx < cnt; idx++)
   {
     unsigned int v;

     v = f[idx];

     /* If value is 0, make it 1 so these symbols can be represented */
     if (v == 0) { zcnt++; v = 1; }

     tbl[idx] = v;
     /* printf ("f[%d] = %d: tbl[%d] = %d sum = %d\n", 
              idx, f[idx], 
              idx, tbl[idx],
              sum); */

     sum  += v;
   }

   /*
    | The goal is to normalize the sum to AWE_K_NORM
    | As one will see, this is not easy to do in integer arithmetic. There
    | are two problems with doing a simple scaling
    |
    |      1. Round off error
    |      2. Values that scale to 0, must be set to at least 1.
    |
    | Both these conspire to make finding the right scale factor difficult.
    | In fact, a stronger statement can be made, there is no guarantee that
    | there is a solution without somewhat ad hocing the correction. For
    | example, consider a 4 entry frequency table
    |
    |            2,1,1,1
    | 
    | The sum of these is 5, so scaling up to 2**16 
    |
    |                                            Table
    |            2 * 2**16/5 = 26214.4   26214   0
    |            1 * 2**16/5 = 13107.2   13107   26214
    |            1 * 2**16/5 = 13107.2   13107   39321
    |            1 * 2**16/5 = 13107.2   13107   52428
    |                          65536     65535   65535 
    |
    | Still need a correction factor of 1. The only sensible thing one can
    | do is spread the correction factor over each value in proportion to
    | that entries probability, but always starting a the most probable value
    | and always applying a correction of at least 1. One keeps going down
    | the list until the correction is exhausted.
    |
    | In this example one would take. Since one is working in the integrated
    | space, whatever accumulated adjustment that has been made, must be
    | applied to the remaining entries
    |
    | Interval 0  Correction       New Delta    Adjustment   Cremaining
    | 26214 * 1 / 65535 = 0 => 1   26214 + 1             1            0
    |                              39322
    |                              52429
    |                              65536
    |
   */



   /* Keep rescaling, until the sum hits the target */
   iter = 0;
   while ((correction = (AWE_K_NORM) - sum))
   {
     int den;
     /* 
      | Set the denominator of the scale factor. The values with hard
      | zeros never are scaled, so don't count these. The correction
      | will be distributed only over the non-zero entries.
     */
     den  = sum - zcnt;
     if (den == 0) den = 1;
     left = correction;
     /*printf ("AWE_build: need to scale sum = 0x%8.8x\n"
     	     "                        zcnt = %d\n"
             "                         den = %d\n"
	     "                  correction = %d\n",
	     sum,
	     zcnt,
	     den,
	     correction);*/

      /*
       | Now apply the remaining correction to each element, always moving
       | the value by at least 1 (in the direction of the correction), but
       | never applying the correction if the value goes to zero
      */
     sum = 0;
     for (idx = 0;  idx < cnt; idx++)
      {
	int nfreq = tbl[idx];
	//printf ("Tbl[%3d] = %d\n", idx, nfreq);

	/* 
         | Only apply scaling to non-zero entries or to all entries in the
         | degenerate case where they are all 0.
        */
	if (f[idx] || (cnt == zcnt))
	{
	  int         d;
          Int64 num = nfreq;

	  num *= correction;
	  d    =  num / den;
	  /* printf ("Nfreq = %u\n"
		  "Num   = %lld\n"
		  "    d = %d\n",
		  nfreq,
		  num,
		  d); */

	  /* Apply non-zero correction in the direction of the correction */
	  if (d == 0)  d = correction < 0 ? -1 : 1;
	 
	  //printf ("   d = %d\n", d);

	  /* Don't let the correction take the interval to zero */
	  if (nfreq + d == 0) d += 1;

	  nfreq       += d;
	  left        -= d;
	  sum         += nfreq;
	  tbl[idx    ] = nfreq;
	  //printf ("tbl[%3d]   = %d  sum = %d\n", idx, tbl[idx], sum);

	  /* If nothing left */
	  if (left == 0) goto DONE;
	}
	else sum += nfreq;
      }
   }     

   DONE:

   for (sum = 0, idx = 0; ; idx++) 
   {
     unsigned int v = tbl[idx];
     tbl[idx] = sum;
     //printf ("Tbl[%3d] = %d:%d\n", idx, sum, v);
     if (idx >= cnt) break;
     sum     += v; 
   }

   return sum;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */

#ifndef CMX_DOXYGEN

/* OUTPUT A BIT. */
#define output_bit(_out, _buffer, _bits_to_go, _bit)  \
do                                                    \
{                                                     \
    _buffer <<= 1;                                    \
     /*printf ("Outputing %x\n", _bit); */            \
    _buffer |= _bit;                                  \
    _bits_to_go -= 1;                                 \
    if (_bits_to_go==0)                               \
    {                                                 \
         /*printf ("Storing %x\n", _buffer&0xff); */  \
        *_out++ = _buffer;                            \
        _buffer = 0;                                  \
        _bits_to_go = 8;                              \
    }                                                 \
} while (0)




/* OUTPUT BITS PLUS FOLLOWING OPPOSITE BITS. */
#define bit_plus_follow(_out, _buffer, _bits_to_go, _bit, _bits_to_follow)  \
    do                                                                      \
    {   output_bit(_out, _buffer, _bits_to_go, _bit);                       \
        while (_bits_to_follow > 0)                                         \
        {                                                                   \
            output_bit(_out, _buffer, _bits_to_go, !_bit);                  \
            _bits_to_follow -= 1;                                           \
        }                                                                   \
    }                                                                       \
    while(0)
#endif
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     unsigned int AWE_bcompress (unsigned char         *out,
                                      unsigned int          boff,
		                      const unsigned int  *table,
			              const unsigned char    *in,
			              int                    cnt)
  \brief  Convenience routine to compress a byte stream
  \return The number of bits needed to encoded the byte stream

  \param   out  The output buffer
  \param  boff  The current bit offset
  \param table  The encoding table
  \param    in  The input buffer of symbols.
  \param   cnt  The number of input symbols.

   This is a convenience routine, combining AWE_start(), AWE_encode() and
   AWE_finish(). It can be used iff the same table is used to encode
   all the symbols.
									  */
/* ---------------------------------------------------------------------- */
unsigned int AWE_bcompress (unsigned char         *out,
                            unsigned int          boff,
			    const unsigned int  *table,
			    const unsigned char    *in,
			    int                    cnt)
{
   AWE_etx etx;

   AWE_start (&etx, out, boff);

   while (--cnt >= 0)  AWE_encode (&etx, table, *in++);

   return AWE_finish (&etx);
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    void AWE_start (AWE_etx *etx, unsigned char *out, unsigned int boff)
  \brief Initializes the encoding context.

  \param etx  The encoding context to initialize
  \param out  The output buffer
  \param boff The bit offset into the output buffer

   Note that since the AWx routines are bit encoders/decoders, one needs to
   specify the offsets into the buffers as bit offsets.
									  */
/* ---------------------------------------------------------------------- */
void AWE_start (AWE_etx *etx, unsigned char *out, unsigned int boff)
{
   int togo;

   etx->lo       = 0;
   etx->hi       = AWE_K_HI;
   

   /* Move to the nearest byte boundary */
   out          += boff >> 3;
   boff         &= 0x7;
   togo          = 8 - boff;


   /* Extract the first group of bits, left justifying the value */
   etx->buffer   = boff ? *out >> togo : 0;
   etx->bbeg     = boff;
   etx->beg      = out;
   etx->out      = out;
   etx->togo     = togo;
   etx->tofollow = 0;

   return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    int AWE_encode   (AWE_etx              *etx,
 	                   const unsigned int *table,
		           unsigned int          sym)
  \brief  Encodes one symbol
  \retval  0, if successful
  \retval -1, if error, essentially this is an internal error from which
              there is no recovery.

  \param   etx  The encoding context
  \param table  The encoding table
  \param   sym  The symbol to encode
									  */
/* ---------------------------------------------------------------------- */
int AWE_encode   (AWE_etx              *etx,
		  const unsigned int *table,
		  unsigned int          sym)
{
   unsigned int  range;
   unsigned int  orglo;
   unsigned int  orghi;
   unsigned int buffer = etx->buffer;
   unsigned int     lo = etx->lo;
   unsigned int     hi = etx->hi;
   unsigned char  *out = etx->out;
   int            togo = etx->togo;
   int        tofollow = etx->tofollow;
   unsigned int    max = table[table[0]];


   //printf ("Encoding %x\n", sym);
   table  = table + 1; 

   /* 
    | The range is really hi - lo + 1, but the + 1 could cause an overflow
    | so the + 1 is added in the 'scale' function.
   */
   range  = hi - lo;

   orglo  = lo;
   orghi  = hi;

   hi     = lo + scale(range, table[sym+1], max) - 1;
   lo     = lo + scale(range, table[sym  ], max);
   //printf ("Lo:Hi %8.8x:%8.8x\n", lo, hi);
   if (lo > hi)
   {
     /*  printf ("\n"
	       "Error lo > hi %8.8x > %8.8x max = %8.8x\n"
	       "Org   lo   hi %8.8x   %8.8x\n"
	       "Table[%3d+1] = %8.8x\n"
	       "Table[%3d  ] = %8.8x\n",
	       lo,
	       hi,
	       max,
	       orglo,
	       orghi,
	       sym, table[sym+1],
	       sym, table[sym]); */
       return -1;
     }

       
   /* Keep the LO and HI in the middle of the range */
   while (1)
   {


       /* 
        | Check if MSb of the ranges are the same, if so, then output the
        | value that bit. This is because it is going to change.
       */
       int xor = (lo ^ hi) << (32 - AWE_K_NBITS);
       if (xor >= 0)
       { 
	 int bit = (unsigned int)lo >> (AWE_K_NBITS - 1);
         bit_plus_follow (out, buffer, togo, bit, tofollow);

	 /* Reduce the range */
         lo -= bit << (AWE_K_NBITS - 1);
         hi -= bit << (AWE_K_NBITS - 1);
       }
       else if (lo >= AWE_K_Q1 && hi < AWE_K_Q3)
       {
	   tofollow += 1;
	   lo       -= AWE_K_Q1;
	   hi       -= AWE_K_Q1;
       }
       else { break; }
           
       lo <<= 1;
       hi <<= 1;
       hi  |= 1;
   }

   etx->lo       = lo;
   etx->hi       = hi;
   etx->out      = out;
   etx->buffer   = buffer;
   etx->togo     = togo;
   etx->tofollow = tofollow;
   return 0;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     unsigned int AWE_finish (AWE_etx *etx)
  \brief  Finishes the encoding by flushing out any currently buffered bits
  \return The number of encoded bits.

  \param  etx  The encoding context
									  */
/* ---------------------------------------------------------------------- */
unsigned int AWE_finish (AWE_etx *etx)
{

   unsigned int          lo = etx->lo;
   int                 togo = etx->togo;
   int             tofollow = etx->tofollow + 1;
   unsigned int      buffer = etx->buffer;
   const unsigned char *beg = etx->beg;
   unsigned char       *out = etx->out;

   if (lo < AWE_K_Q1)       bit_plus_follow(out,
                                            buffer,
                                            togo,
                                            0,
                                            tofollow);
                                                /* select the quarter that  */
   else                     bit_plus_follow(out,
                                            buffer,
                                            togo,
                                            1,
                                            tofollow);

   
   if (togo != 8) 
   {

     // printf ("Storing %x bits_to_go/follow = %d:%d\n",
     //	     (buffer >> togo) & 0xff,
     //       togo,
     //	     tofollow);
     *out = buffer << togo;
   }
   
   /* Calculate the length, accounting for both the initial and final shard */
   return (out - beg) * 8 + (8 - togo) - etx->bbeg;
}
/* ---------------------------------------------------------------------- */


