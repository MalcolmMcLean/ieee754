/*
  IEEE 754 portable read / write routines

  IEEE 754 is the standard for floating point arithmetic, implemented by most but not all floating point
  units. These functions will read / write doubles in IEE 754, regardless of the underlying representation
  of the machine. They also take a flag for endian-ness.

  So they should be used whenever you want to write real values in binary, portably.


  By Malcolm McLean

*/

#include <stdio.h>
#include <math.h>
#include <float.h>

/*
* read a double from a stream in ieee754 format regardless of host
*  encoding.
*  fp - the stream
*  bigendian - set to if big bytes first, clear for little bytes
*              first
*
*/
double freadieee754(FILE *fp, int bigendian)
{
	unsigned char buff[8];
	int i;
	double fnorm = 0.0;
	unsigned char temp;
	int sign;
	int exponent;
	double bitval;
	int maski, mask;
	int expbits = 11;
	int significandbits = 52;
	int shift;
	double answer;

	/* read the data */
	for (i = 0; i < 8; i++)
		buff[i] = fgetc(fp);
	/* just reverse if not big-endian*/
	if (!bigendian)
	{
		for (i = 0; i < 4; i++)
		{
			temp = buff[i];
			buff[i] = buff[8 - i - 1];
			buff[8 - i - 1] = temp;
		}
	}
	sign = buff[0] & 0x80 ? -1 : 1;
	/* exponet in raw format*/
	exponent = ((buff[0] & 0x7F) << 4) | ((buff[1] & 0xF0) >> 4);

	/* read inthe mantissa. Top bit is 0.5, the successive bits half*/
	bitval = 0.5;
	maski = 1;
	mask = 0x08;
	for (i = 0; i < significandbits; i++)
	{
		if (buff[maski] & mask)
			fnorm += bitval;

		bitval /= 2.0;
		mask >>= 1;
		if (mask == 0)
		{
			mask = 0x80;
			maski++;
		}
	}
	/* handle zero specially */
	if (exponent == 0 && fnorm == 0)
		return 0.0;

	shift = exponent - ((1 << (expbits - 1)) - 1); /* exponent = shift + bias */
	/* nans have exp 1024 and non-zero mantissa */
	if (shift == 1024 && fnorm != 0)
		return sqrt(-1.0);
	/*infinity*/
	if (shift == 1024 && fnorm == 0)
	{

#ifdef INFINITY
		return sign == 1 ? INFINITY : -INFINITY;
#endif
		return	(sign * 1.0) / 0.0;
	}
	if (shift > -1023)
	{
		answer = ldexp(fnorm + 1.0, shift);
		return answer * sign;
	}
	else
	{
		/* denormalised numbers */
		if (fnorm == 0.0)
			return 0.0;
		shift = -1022;
		while (fnorm < 1.0)
		{
			fnorm *= 2;
			shift--;
		}
		answer = ldexp(fnorm, shift);
		return answer * sign;
	}
}


float freadieee754f(FILE *fp, int bigendian)
{
   unsigned long buff = 0;
   unsigned long buff2 = 0;
   unsigned long mask;
   int sign;
   int exponent;
   int shift;
   int i;
   int significandbits = 23;
   int expbits = 8;
   double fnorm = 0.0;
   double bitval;
   double answer;

   for(i=0;i<4;i++)
     buff = (buff << 8) | fgetc(fp);
   if(!bigendian)
   {
     for(i=0;i<4;i++)
     {
	   buff2 <<= 8;
       buff2 |= (buff & 0xFF);
       buff >>= 8;
     }
     buff = buff2; 
   }

   sign = (buff & 0x80000000) ? -1 : 1;
   mask = 0x00400000;
   exponent = (buff & 0x7F800000) >> 23;
   bitval = 0.5;
   for(i=0;i<significandbits;i++)
   {
     if(buff & mask)
        fnorm += bitval;
     bitval /= 2;
     mask >>= 1;
   }
   if(exponent == 0 && fnorm == 0.0)
     return 0.0f;
   shift = exponent - ((1 << (expbits - 1)) - 1); /* exponent = shift + bias */

   if(shift == 128 && fnorm != 0.0)
     return (float) sqrt(-1.0);
   if(shift == 128 && fnorm == 0.0)
   {
#ifdef INFINITY
     return sign == 1 ? INFINITY : -INFINITY;
#endif
     return (sign * 1.0f)/0.0f;
   }
   if(shift > -127)
   {
     answer = ldexp(fnorm + 1.0, shift);
     return (float) answer * sign;
   }
   else
   {
     if(fnorm == 0.0)
     {
       return 0.0f;
     }
	 shift = -126;
	 while (fnorm < 1.0)
	 {
		 fnorm *= 2;
		 shift--;
	 }
	 answer = ldexp(fnorm, shift);
	 return (float) answer * sign;
   }
}

/*
* write a double to a stream in ieee754 format regardless of host
*  encoding.
*  x - number to write
*  fp - the stream
*  bigendian - set to write big bytes first, elee write litle bytes
*              first
*  Returns: 0 or EOF on error
*  Notes: different NaN types and negative zero not preserved.
*         if the number is too big to represent it will become infinity
*         if it is too small to represent it will become zero.
*/
int fwriteieee754(double x, FILE *fp, int bigendian)
{
	int shift;
	unsigned long sign, exp, hibits, hilong, lowlong;
	double fnorm, significand;
	int expbits = 11;
	int significandbits = 52;

	/* zero (can't handle signed zero) */
	if (x == 0)
	{
		hilong = 0;
		lowlong = 0;
		goto writedata;
	}
	/* infinity */
	if (x > DBL_MAX)
	{
		hilong = 1024 + ((1 << (expbits - 1)) - 1);
		hilong <<= (31 - expbits);
		lowlong = 0;
		goto writedata;
	}
	/* -infinity */
	if (x < -DBL_MAX)
	{
		hilong = 1024 + ((1 << (expbits - 1)) - 1);
		hilong <<= (31 - expbits);
		hilong |= (1 << 31);
		lowlong = 0;
		goto writedata;
	}
	/* NaN - dodgy because many compilers optimise out this test, but
	*there is no portable isnan() */
	if (x != x)
	{
		hilong = 1024 + ((1 << (expbits - 1)) - 1);
		hilong <<= (31 - expbits);
		lowlong = 1234;
		goto writedata;
	}

	/* get the sign */
	if (x < 0) { sign = 1; fnorm = -x; }
	else { sign = 0; fnorm = x; }

	/* get the normalized form of f and track the exponent */
	shift = 0;
	while (fnorm >= 2.0) { fnorm /= 2.0; shift++; }
	while (fnorm < 1.0) { fnorm *= 2.0; shift--; }

	/* check for denormalized numbers */
	if (shift < -1022)
	{
		while (shift < -1022) { fnorm /= 2.0; shift++; }
		shift = -1023;
	}
	/* out of range. Set to infinity */
	else if (shift > 1023)
	{
		hilong = 1024 + ((1 << (expbits - 1)) - 1);
		hilong <<= (31 - expbits);
		hilong |= (sign << 31);
		lowlong = 0;
		goto writedata;
	}
	else
		fnorm = fnorm - 1.0; /* take the significant bit off mantissa */

	/* calculate the integer form of the significand */
	/* hold it in a  double for now */

	significand = fnorm * ((1LL << significandbits) + 0.5f);


	/* get the biased exponent */
	exp = shift + ((1 << (expbits - 1)) - 1); /* shift + bias */

	/* put the data into two longs (for convenience) */
	hibits = (long)(significand / 4294967296);
	hilong = (sign << 31) | (exp << (31 - expbits)) | hibits;
	x = significand - hibits * 4294967296;
	lowlong = (unsigned long)(significand - hibits * 4294967296);

writedata:
	/* write the bytes out to the stream */
	if (bigendian)
	{
		fputc((hilong >> 24) & 0xFF, fp);
		fputc((hilong >> 16) & 0xFF, fp);
		fputc((hilong >> 8) & 0xFF, fp);
		fputc(hilong & 0xFF, fp);

		fputc((lowlong >> 24) & 0xFF, fp);
		fputc((lowlong >> 16) & 0xFF, fp);
		fputc((lowlong >> 8) & 0xFF, fp);
		fputc(lowlong & 0xFF, fp);
	}
	else
	{
		fputc(lowlong & 0xFF, fp);
		fputc((lowlong >> 8) & 0xFF, fp);
		fputc((lowlong >> 16) & 0xFF, fp);
		fputc((lowlong >> 24) & 0xFF, fp);

		fputc(hilong & 0xFF, fp);
		fputc((hilong >> 8) & 0xFF, fp);
		fputc((hilong >> 16) & 0xFF, fp);
		fputc((hilong >> 24) & 0xFF, fp);
	}
	return ferror(fp);
}


int fwriteieee754f(float x, FILE *fp, int bigendian)
{
	int shift;
	unsigned long sign, exp, hibits, buff;
	double fnorm, significand;
	int expbits = 8;
	int significandbits = 23;

	/* zero (can't handle signed zero) */
	if (x == 0)
	{
		buff = 0;
		goto writedata;
	}
	/* infinity */
	if (x > FLT_MAX)
	{
		buff = 128 + ((1 << (expbits - 1)) - 1);
		buff <<= (31 - expbits);
		goto writedata;
	}
	/* -infinity */
	if (x < -FLT_MAX)
	{
		buff = 128 + ((1 << (expbits - 1)) - 1);
		buff <<= (31 - expbits);
		buff |= (1 << 31);
		goto writedata;
	}
	/* NaN - dodgy because many compilers optimise out this test, but
	*there is no portable isnan() */
	if (x != x)
	{
		buff = 128 + ((1 << (expbits - 1)) - 1);
		buff <<= (31 - expbits);
		buff |= 1234;
		goto writedata;
	}

	/* get the sign */
	if (x < 0) { sign = 1; fnorm = -x; }
	else { sign = 0; fnorm = x; }

	/* get the normalized form of f and track the exponent */
	shift = 0;
	while (fnorm >= 2.0) { fnorm /= 2.0; shift++; }
	while (fnorm < 1.0) { fnorm *= 2.0; shift--; }

	/* check for denormalized numbers */
	if (shift < -126)
	{
		while (shift < -126) { fnorm /= 2.0; shift++; }
		shift = -1023;
	}
	/* out of range. Set to infinity */
	else if (shift > 128)
	{
		buff = 128 + ((1 << (expbits - 1)) - 1);
		buff <<= (31 - expbits);
		buff |= (sign << 31);
		goto writedata;
	}
	else
		fnorm = fnorm - 1.0; /* take the significant bit off mantissa */

	/* calculate the integer form of the significand */
	/* hold it in a  double for now */

	significand = fnorm * ((1LL << significandbits) + 0.5f);


	/* get the biased exponent */
	exp = shift + ((1 << (expbits - 1)) - 1); /* shift + bias */

	hibits = (long)(significand);
	buff = (sign << 31) | (exp << (31 - expbits)) | hibits;

writedata:
	/* write the bytes out to the stream */
	if (bigendian)
	{
		fputc((buff >> 24) & 0xFF, fp);
		fputc((buff >> 16) & 0xFF, fp);
		fputc((buff >> 8) & 0xFF, fp);
		fputc(buff & 0xFF, fp);
	}
	else
	{
		fputc(buff & 0xFF, fp);
		fputc((buff >> 8) & 0xFF, fp);
		fputc((buff >> 16) & 0xFF, fp);
		fputc((buff >> 24) & 0xFF, fp);
	}
	return ferror(fp);
}

