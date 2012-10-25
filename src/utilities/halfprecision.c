/******************************************************************************
 *
 * MATLAB (R) is a trademark of The Mathworks (R) Corporation
 *
 * Function:    halfprecision
 * Filename:    halfprecision.c
 * Programmer:  James Tursa
 * Version:     1.0
 * Date:        March 3, 2009
 * Copyright:   (c) 2009 by James Tursa, All Rights Reserved
 *
 *  This code uses the BSD License:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the distribution
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * halfprecision converts the input argument to/from a half precision floating
 * point bit pattern corresponding to IEEE 754r. The bit pattern is stored in a
 * uint16 class variable. Please note that halfprecision is *not* a class. That
 * is, you cannot do any arithmetic with the half precision bit patterns.
 * halfprecision is simply a function that converts the IEEE 754r half precision
 * bit pattern to/from other numeric MATLAB variables. You can, however, take
 * the half precision bit patterns, convert them to single or double, do the
 * operation, and then convert the result back manually.
 *
 * 1 bit sign bit
 * 5 bits exponent, biased by 15
 * 10 bits mantissa, hidden leading bit, normalized to 1.0
 *
 * Special floating point bit patterns recognized and supported:
 *
 * All exponent bits zero:
 * - If all mantissa bits are zero, then number is zero (possibly signed)
 * - Otherwise, number is a denormalized bit pattern
 *
 * All exponent bits set to 1:
 * - If all mantissa bits are zero, then number is +Infinity or -Infinity
 * - Otherwise, number is NaN (Not a Number)
 *
 * Building:
 *
 *  halfprecision requires that a mex routine be built (one time only). This
 *  process is typically self-building the first time you call the function
 *  as long as you have the files halfprecision.m and halfprecision.c in the
 *  same directory somewhere on the MATLAB path. If you need to manually build
 *  the mex function, here are the commands:
 *
 * >> mex -setup
 *   (then follow instructions to select a C / C++ compiler of your choice)
 * >> mex halfprecision.c
 *
 * If you have an older version of MATLAB, you may need to use this command:
 *
 * >> mex -DDEFINEMWSIZE halfprecision.c
 *
 * Syntax
 *
 * B = halfprecision(A)
 * C = halfprecision(B,S)
 *     halfprecision(B,'disp')
 *
 * Description
 *
 * A = a MATLAB numeric array, char array, or logical array.
 *
 * B = the variable A converted into half precision floating point bit pattern.
 *     The bit pattern will be returned as a uint16 class variable. The values
 *     displayed are simply the bit pattern interpreted as if it were an unsigned
 *     16-bit integer. To see the halfprecision values, use the 'disp' option, which
 *     simply converts the bit patterns into a single class and then displays them.
 *
 * C = the half precision floating point bit pattern in B converted into class S.
 *     B must be a uint16 or int16 class variable.
 *
 * S = char string naming the desired class (e.g., 'single', 'int32', etc.)
 *     If S = 'disp', then the floating point bit values are simply displayed.
 *
 * Examples
 *
 * >> a = [-inf -1e30 -1.2 NaN 1.2 1e30 inf]
 * a =
 * 1.0e+030 *
 *     -Inf   -1.0000   -0.0000       NaN    0.0000    1.0000       Inf
 *
 * >> b = halfprecision(a)
 * b =
 * 64512  64512  48333  65024  15565  31744  31744
 *
 * >> halfprecision(b,'disp')
 *     -Inf      -Inf   -1.2002       NaN    1.2002       Inf       Inf
 *
 * >> halfprecision(b,'double')
 * ans =
 *     -Inf      -Inf   -1.2002       NaN    1.2002       Inf       Inf
 *
 * >> 2^(-24)
 * ans =
 * 5.9605e-008
 *
 * >> halfprecision(ans)
 * ans =
 *     1
 *
 * >> halfprecision(ans,'disp')
 * 5.9605e-008
 *
 * >> 2^(-25)
 * ans =
 * 2.9802e-008
 *
 * >> halfprecision(ans)
 * ans =
 *     1
 *
 * >> halfprecision(ans,'disp')
 * 5.9605e-008
 *
 * >> 2^(-26)
 * ans =
 *  1.4901e-008
 *
 * >> halfprecision(ans)
 * ans =
 *     0
 *
 * >> halfprecision(ans,'disp')
 *    0
 *
 * Note that the special cases of -Inf, +Inf, and NaN are handled correctly.
 * Also, note that the -1e30 and 1e30 values overflow the half precision format
 * and are converted into half precision -Inf and +Inf values, and stay that
 * way when they are converted back into doubles.
 *
 * For the denormalized cases, note that 2^(-24) is the smallest number that can
 * be represented in half precision exactly. 2^(-25) will convert to 2^(-24)
 * because of the rounding algorithm used, and 2^(-26) is too small and underflows
 * to zero.
 *
 ********************************************************************************/

// Includes -------------------------------------------------------------------

#include <string.h>

// Macros ---------------------------------------------------------------------

// Needed for older MATLAB versions that do not have mwSize
//#ifdef DEFINEMWSIZE
#define mwSize int
//#endif

#define  INT16_TYPE          short
#define UINT16_TYPE unsigned short
#define  INT32_TYPE          long
#define UINT32_TYPE unsigned long

// Global ---------------------------------------------------------------------

int next;  // Little Endian adjustment
int checkieee = 1;  // Flag to check for IEEE754, Endian, and word size

// Prototypes -----------------------------------------------------------------

void singles2halfp(void *target, void *source, mwSize numel);
void doubles2halfp(void *target, void *source, mwSize numel);

void halfp2singles(void *target, void *source, mwSize numel);
void halfp2doubles(void *target, void *source, mwSize numel);


//-----------------------------------------------------------------------------

void singles2halfp(void *target, void *source, mwSize numel)
{
  UINT16_TYPE *hp = (UINT16_TYPE *) target; // Type pun output as an unsigned 16-bit int
  UINT32_TYPE *xp = (UINT32_TYPE *) source; // Type pun input as an unsigned 32-bit int
  UINT16_TYPE    hs, he, hm;
  UINT32_TYPE x, xs, xe, xm;
  int hes;

  if ( source == NULL || target == NULL )
    // Nothing to convert (e.g., imag part of pure real)
  {
    return;
  }

  while ( numel-- )
  {
    x = *xp++;

    if ( (x & 0x7FFFFFFFu) == 0 )
      // Signed zero
    {
      *hp++ = (UINT16_TYPE) (x >> 16);  // Return the signed zero
    }
    else
      // Not zero
    {
      xs = x & 0x80000000u;  // Pick off sign bit
      xe = x & 0x7F800000u;  // Pick off exponent bits
      xm = x & 0x007FFFFFu;  // Pick off mantissa bits

      if ( xe == 0 )
        // Denormal will underflow, return a signed zero
      {
        *hp++ = (UINT16_TYPE) (xs >> 16);
      }
      else
        if ( xe == 0x7F800000u )
          // Inf or NaN (all the exponent bits are set)
        {
          if ( xm == 0 )
            // If mantissa is zero ...
          {
            *hp++ = (UINT16_TYPE) ((xs >> 16) | 0x7C00u); // Signed Inf
          }
          else
          {
            *hp++ = (UINT16_TYPE) 0xFE00u; // NaN, only 1st mantissa bit set
          }
        }
        else
          // Normalized number
        {
          hs = (UINT16_TYPE) (xs >> 16); // Sign bit
          hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp

          if ( hes >= 0x1F )
            // Overflow
          {
            *hp++ = (UINT16_TYPE) ((xs >> 16) | 0x7C00u); // Signed Inf
          }
          else
            if ( hes <= 0 )
              // Underflow
            {
              if ( (14 - hes) > 24 )
                // Mantissa shifted all the way off & no rounding possibility
              {
                hm = (UINT16_TYPE) 0u;  // Set mantissa to zero
              }
              else
              {
                xm |= 0x00800000u;  // Add the hidden leading bit
                hm = (UINT16_TYPE) (xm >> (14 - hes)); // Mantissa

                if ( (xm >> (13 - hes)) & 0x00000001u ) // Check for rounding
                  hm += (UINT16_TYPE) 1u; // Round, might overflow into exp bit, but this is OK
              }

              *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            }
            else
            {
              he = (UINT16_TYPE) (hes << 10); // Exponent
              hm = (UINT16_TYPE) (xm >> 13); // Mantissa

              if ( xm & 0x00001000u ) // Check for rounding
                *hp++ = (hs | he | hm) + (UINT16_TYPE) 1u; // Round, might overflow to inf, this is OK
              else
                *hp++ = (hs | he | hm);  // No rounding
            }
        }
    }
  }
}

//-----------------------------------------------------------------------------

void doubles2halfp(void *target, void *source, mwSize numel)
{
  UINT16_TYPE *hp = (UINT16_TYPE *) target; // Type pun output as an unsigned 16-bit int
  UINT32_TYPE *xp = (UINT32_TYPE *) source; // Type pun input as an unsigned 32-bit int
  UINT16_TYPE    hs, he, hm;
  UINT32_TYPE x, xs, xe, xm;
  int hes;

  xp += next;  // Little Endian adjustment if necessary

  if ( source == NULL || target == NULL )
    // Nothing to convert (e.g., imag part of pure real)
  {
    return;
  }

  while ( numel-- )
  {
    x = *xp++;
    xp++; // The extra xp++ is to skip over the remaining 32 bits of the mantissa

    if ( (x & 0x7FFFFFFFu) == 0 )
      // Signed zero
    {
      *hp++ = (UINT16_TYPE) (x >> 16);  // Return the signed zero
    }
    else
      // Not zero
    {
      xs = x & 0x80000000u;  // Pick off sign bit
      xe = x & 0x7FF00000u;  // Pick off exponent bits
      xm = x & 0x000FFFFFu;  // Pick off mantissa bits

      if ( xe == 0 )
        // Denormal will underflow, return a signed zero
      {
        *hp++ = (UINT16_TYPE) (xs >> 16);
      }
      else
        if ( xe == 0x7FF00000u )
          // Inf or NaN (all the exponent bits are set)
        {
          if ( xm == 0 )
            // If mantissa is zero ...
          {
            *hp++ = (UINT16_TYPE) ((xs >> 16) | 0x7C00u); // Signed Inf
          }
          else
          {
            *hp++ = (UINT16_TYPE) 0xFE00u; // NaN, only 1st mantissa bit set
          }
        }
        else
          // Normalized number
        {
          hs = (UINT16_TYPE) (xs >> 16); // Sign bit
          hes = ((int)(xe >> 20)) - 1023 + 15; // Exponent unbias the double, then bias the halfp

          if ( hes >= 0x1F )
            // Overflow
          {
            *hp++ = (UINT16_TYPE) ((xs >> 16) | 0x7C00u); // Signed Inf
          }
          else
            if ( hes <= 0 )
              // Underflow
            {
              if ( (10 - hes) > 21 )
                // Mantissa shifted all the way off & no rounding possibility
              {
                hm = (UINT16_TYPE) 0u;  // Set mantissa to zero
              }
              else
              {
                xm |= 0x00100000u;  // Add the hidden leading bit
                hm = (UINT16_TYPE) (xm >> (11 - hes)); // Mantissa

                if ( (xm >> (10 - hes)) & 0x00000001u ) // Check for rounding
                  hm += (UINT16_TYPE) 1u; // Round, might overflow into exp bit, but this is OK
              }

              *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            }
            else
            {
              he = (UINT16_TYPE) (hes << 10); // Exponent
              hm = (UINT16_TYPE) (xm >> 10); // Mantissa

              if ( xm & 0x00000200u ) // Check for rounding
                *hp++ = (hs | he | hm) + (UINT16_TYPE) 1u; // Round, might overflow to inf, this is OK
              else
                *hp++ = (hs | he | hm);  // No rounding
            }
        }
    }
  }
}

//-----------------------------------------------------------------------------

void halfp2singles(void *target, void *source, mwSize numel)
{
  UINT16_TYPE *hp = (UINT16_TYPE *) source; // Type pun input as an unsigned 16-bit int
  UINT32_TYPE *xp = (UINT32_TYPE *) target; // Type pun output as an unsigned 32-bit int
  UINT16_TYPE h, hs, he, hm;
  UINT32_TYPE xs, xe, xm;
  INT32_TYPE xes;
  int e;

  if ( source == NULL || target == NULL ) // Nothing to convert (e.g., imag part of pure real)
    return;

  while ( numel-- )
  {
    h = *hp++;

    if ( (h & 0x7FFFu) == 0 )
      // Signed zero
    {
      *xp++ = ((UINT32_TYPE) h) << 16;  // Return the signed zero
    }
    else
      // Not zero
    {
      hs = h & 0x8000u;  // Pick off sign bit
      he = h & 0x7C00u;  // Pick off exponent bits
      hm = h & 0x03FFu;  // Pick off mantissa bits

      if ( he == 0 )
        // Denormal will convert to normalized
      {
        e = -1; // The following loop figures out how much extra to adjust the exponent

        do
        {
          e++;
          hm <<= 1;
        }
        while ( (hm & 0x0400u) == 0 );  // Shift until leading bit overflows into exponent bit

        xs = ((UINT32_TYPE) hs) << 16; // Sign bit
        xes = ((INT32_TYPE) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
        xe = (UINT32_TYPE) (xes << 23); // Exponent
        xm = ((UINT32_TYPE) (hm & 0x03FFu)) << 13; // Mantissa
        *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
      }
      else
        if ( he == 0x7C00u )
          // Inf or NaN (all the exponent bits are set)
        {
          if ( hm == 0 )
            // If mantissa is zero ...
          {
            *xp++ = (((UINT32_TYPE) hs) << 16) | ((UINT32_TYPE) 0x7F800000u); // Signed Inf
          }
          else
          {
            *xp++ = (UINT32_TYPE) 0xFFC00000u; // NaN, only 1st mantissa bit set
          }
        }
        else
          // Normalized number
        {
          xs = ((UINT32_TYPE) hs) << 16; // Sign bit
          xes = ((INT32_TYPE) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
          xe = (UINT32_TYPE) (xes << 23); // Exponent
          xm = ((UINT32_TYPE) hm) << 13; // Mantissa
          *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
    }
  }
}

//-----------------------------------------------------------------------------

void halfp2doubles(void *target, void *source, mwSize numel)
{
  UINT16_TYPE *hp = (UINT16_TYPE *) source; // Type pun input as an unsigned 16-bit int
  UINT32_TYPE *xp = (UINT32_TYPE *) target; // Type pun output as an unsigned 32-bit int
  UINT16_TYPE h, hs, he, hm;
  UINT32_TYPE xs, xe, xm;
  INT32_TYPE xes;
  int e;

  xp += next;  // Little Endian adjustment if necessary

  if ( source == NULL || target == NULL ) // Nothing to convert (e.g., imag part of pure real)
    return;

  while ( numel-- )
  {
    h = *hp++;

    if ( (h & 0x7FFFu) == 0 )
      // Signed zero
    {
      *xp++ = ((UINT32_TYPE) h) << 16;  // Return the signed zero
    }
    else
      // Not zero
    {
      hs = h & 0x8000u;  // Pick off sign bit
      he = h & 0x7C00u;  // Pick off exponent bits
      hm = h & 0x03FFu;  // Pick off mantissa bits

      if ( he == 0 )
        // Denormal will convert to normalized
      {
        e = -1; // The following loop figures out how much extra to adjust the exponent

        do
        {
          e++;
          hm <<= 1;
        }
        while ( (hm & 0x0400u) == 0 );  // Shift until leading bit overflows into exponent bit

        xs = ((UINT32_TYPE) hs) << 16; // Sign bit
        xes = ((INT32_TYPE) (he >> 10)) - 15 + 1023 - e; // Exponent unbias the halfp, then bias the double
        xe = (UINT32_TYPE) (xes << 20); // Exponent
        xm = ((UINT32_TYPE) (hm & 0x03FFu)) << 10; // Mantissa
        *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
      }
      else
        if ( he == 0x7C00u )
          // Inf or NaN (all the exponent bits are set)
        {
          if ( hm == 0 )
            // If mantissa is zero ...
          {
            *xp++ = (((UINT32_TYPE) hs) << 16) | ((UINT32_TYPE) 0x7FF00000u); // Signed Inf
          }
          else
          {
            *xp++ = (UINT32_TYPE) 0xFFF80000u; // NaN, only the 1st mantissa bit set
          }
        }
        else
        {
          xs = ((UINT32_TYPE) hs) << 16; // Sign bit
          xes = ((INT32_TYPE) (he >> 10)) - 15 + 1023; // Exponent unbias the halfp, then bias the double
          xe = (UINT32_TYPE) (xes << 20); // Exponent
          xm = ((UINT32_TYPE) hm) << 10; // Mantissa
          *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
    }

    xp++; // Skip over the remaining 32 bits of the mantissa
  }
}
