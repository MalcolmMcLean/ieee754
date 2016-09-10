#include <stdio.h>

/**
  Portable binary integer read routines

  Portable binary integer read routines are slightly tricky to
  write. ANSI C guarantees that an int shall have at least 16 bits
  and a long at least 32 bits, but not that CHAR_BIT (the number
  of bits ina  byte) shall be 8.

  The routines assume that the filesystem reads back binary files in
  bytes of 8 bits when written to in bytes of 8 bits. 
*/

/**
  Get a 16-bit big-endian signed integer form a stream.

  Does not break, regardless of host integer representation.

  @param[in] fp - pointer to a stream opened for reading in binary mode
  @ returns the 16 bit value as an integer
*/
int fget16be(FILE *fp)
{
	int c1, c2;

	c2 = fgetc(fp);
	c1 = fgetc(fp);

	return ((c2 ^ 128) - 128) * 256 + c1;
}

/**
Get a 32-bit big-endian signed integer form a stream.

Does not break, regardless of host integer representation.

@param[in] fp - pointer to a stream opened for reading in binary mode
@ returns the 16 bit value as an integer
*/
long fget32be(FILE *fp)
{
	int c1, c2, c3, c4;

	c4 = fgetc(fp);
	c3 = fgetc(fp);
	c2 = fgetc(fp);
	c1 = fgetc(fp);
	return ((c4 ^ 128) - 128) * 256 * 256 * 256 + c3 * 256 * 256 + c2 * 256 + c1;
}

/**
Get a 16-bit little-endian signed integer form a stream.

Does not break, regardless of host integer representation.

@param[in] fp - pointer to a stream opened for reading in binary mode
@ returns the 16 bit value as an integer
*/
int fget16le(FILE *fp)
{
	int c1, c2;

	c1 = fgetc(fp);
	c2 = fgetc(fp);

	return ((c2 ^ 128) - 128) * 256 + c1;
}

/**
Get a 32-bit little-endian signed integer form a stream.

Does not break, regardless of host integer representation.

@param[in] fp - pointer to a stream opened for reading in binary mode
@ returns the 16 bit value as an integer
*/
long fget32le(FILE *fp)
{
	int c1, c2, c3, c4;

	c1 = fgetc(fp);
	c2 = fgetc(fp);
	c3 = fgetc(fp);
	c4 = fgetc(fp);
	return ((c4 ^ 128) - 128) * 256 * 256 * 256 + c3 * 256 * 256 + c2 * 256 + c1;
}
