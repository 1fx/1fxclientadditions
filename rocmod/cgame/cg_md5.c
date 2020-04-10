/*
===========================================================================
Copyright (C) 2002 - 2005, Caleb Gingles

This file is part of the 1fx. Client Additions source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "cg_local.h"


unsigned int	value[4];
unsigned int	datablock[16];
unsigned char	cache[128];
unsigned int	cacheSize;
unsigned int	processedSize;
char			md5_output[64];

void md5_reset( void )
{
	value[0] = 0x67452301;
	value[1] = 0xefcdab89;
	value[2] = 0x98badcfe;
	value[3] = 0x10325476;
	cacheSize = 0;
	processedSize = 0;
}

static void drawnumber( char *buffer, unsigned char number )
{
	char a;
	char b;
	char chars[] = "0123456789abcdef";

	a = chars[(number >> 4)];
	b = chars[(number & 15)];
	*buffer = a;
	buffer++;
	*buffer = b;
	buffer++;
}

static unsigned int round1( unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int t )
{
	a += ((b & c) | (~b & d)) + datablock[k] + t;
	a = (a << s) | (a >> (32 - s));
	a += b;
	return a;
}

static unsigned int round2( unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int t )
{
	a += ((b & d) | (c & ~d)) + datablock[k] + t;
	a = (a << s) | (a >> (32 - s));
	a += b;
	return a;
}

static unsigned int round3( unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int t )
{
	a += (b ^ c ^ d) + datablock[k] + t;
	a = (a << s) | (a >> (32 - s));
	a += b;
	return a;
}

static unsigned int round4( unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int t )
{
	a += (c ^ (b | ~d)) + datablock[k] + t;
	a = (a << s) | (a >> (32 - s));
	a += b;
	return a;
}

static void doblock( unsigned char *buffer, unsigned int size )
{
	unsigned int i;
	unsigned int j;
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;

	for ( i = 0; i < size; i += 64 ) {
		for ( j = 0; j < 16; j++ ) {
			datablock[j] = buffer[j * 4 + i + 0];
			datablock[j] |= buffer[j * 4 + i + 1] << 8;
			datablock[j] |= buffer[j * 4 + i + 2] << 16;
			datablock[j] |= buffer[j * 4 + i + 3] << 24;
		}

		a = value[0];
		b = value[1];
		c = value[2];
		d = value[3];

		a = round1( a, b, c, d, 0, 7, 0xd76aa478 );
		d = round1( d, a, b, c, 1, 12, 0xe8c7b756 );
		c = round1( c, d, a, b, 2, 17, 0x242070db );
		b = round1( b, c, d, a, 3, 22, 0xc1bdceee );
		a = round1( a, b, c, d, 4, 7, 0xf57c0faf );
		d = round1( d, a, b, c, 5, 12, 0x4787c62a );
		c = round1( c, d, a, b, 6, 17, 0xa8304613 );
		b = round1( b, c, d, a, 7, 22, 0xfd469501 );
		a = round1( a, b, c, d, 8, 7, 0x698098d8 );
		d = round1( d, a, b, c, 9, 12, 0x8b44f7af );
		c = round1( c, d, a, b, 10, 17, 0xffff5bb1 );
		b = round1( b, c, d, a, 11, 22, 0x895cd7be );
		a = round1( a, b, c, d, 12, 7, 0x6b901122 );
		d = round1( d, a, b, c, 13, 12, 0xfd987193 );
		c = round1( c, d, a, b, 14, 17, 0xa679438e );
		b = round1( b, c, d, a, 15, 22, 0x49b40821 );

		a = round2( a, b, c, d, 1, 5, 0xf61e2562 );
		d = round2( d, a, b, c, 6, 9, 0xc040b340 );
		c = round2( c, d, a, b, 11, 14, 0x265e5a51 );
		b = round2( b, c, d, a, 0, 20, 0xe9b6c7aa );
		a = round2( a, b, c, d, 5, 5, 0xd62f105d );
		d = round2( d, a, b, c, 10, 9,  0x2441453 );
		c = round2( c, d, a, b, 15, 14, 0xd8a1e681 );
		b = round2( b, c, d, a, 4, 20, 0xe7d3fbc8 );
		a = round2( a, b, c, d, 9, 5, 0x21e1cde6 );
		d = round2( d, a, b, c, 14, 9, 0xc33707d6 );
		c = round2( c, d, a, b, 3, 14, 0xf4d50d87 );
		b = round2( b, c, d, a, 8, 20, 0x455a14ed );
		a = round2( a, b, c, d, 13, 5, 0xa9e3e905 );
		d = round2( d, a, b, c, 2, 9, 0xfcefa3f8 );
		c = round2( c, d, a, b, 7, 14, 0x676f02d9 );
		b = round2( b, c, d, a, 12, 20, 0x8d2a4c8a );

		a = round3( a, b, c, d, 5, 4, 0xfffa3942 );
		d = round3( d, a, b, c, 8, 11, 0x8771f681 );
		c = round3( c, d, a, b, 11, 16, 0x6d9d6122 );
		b = round3( b, c, d, a, 14, 23, 0xfde5380c );
		a = round3( a, b, c, d, 1, 4, 0xa4beea44 );
		d = round3( d, a, b, c, 4, 11, 0x4bdecfa9 );
		c = round3( c, d, a, b, 7, 16, 0xf6bb4b60 );
		b = round3( b, c, d, a, 10, 23, 0xbebfbc70 );
		a = round3( a, b, c, d, 13, 4, 0x289b7ec6 );
		d = round3( d, a, b, c, 0, 11, 0xeaa127fa );
		c = round3( c, d, a, b, 3, 16, 0xd4ef3085 );
		b = round3( b, c, d, a, 6, 23,  0x4881d05 );
		a = round3( a, b, c, d, 9, 4, 0xd9d4d039 );
		d = round3( d, a, b, c, 12, 11, 0xe6db99e5 );
		c = round3( c, d, a, b, 15, 16, 0x1fa27cf8 );
		b = round3( b, c, d, a, 2, 23, 0xc4ac5665 );

		a = round4( a, b, c, d, 0, 6, 0xf4292244 );
		d = round4( d, a, b, c, 7, 10, 0x432aff97 );
		c = round4( c, d, a, b, 14, 15, 0xab9423a7 );
		b = round4( b, c, d, a, 5, 21, 0xfc93a039 );
		a = round4( a, b, c, d, 12, 6, 0x655b59c3 );
		d = round4( d, a, b, c, 3, 10, 0x8f0ccc92 );
		c = round4( c, d, a, b, 10, 15, 0xffeff47d );
		b = round4( b, c, d, a, 1, 21, 0x85845dd1 );
		a = round4( a, b, c, d, 8, 6, 0x6fa87e4f );
		d = round4( d, a, b, c, 15, 10, 0xfe2ce6e0 );
		c = round4( c, d, a, b, 6, 15, 0xa3014314 );
		b = round4( b, c, d, a, 13, 21, 0x4e0811a1 );
		a = round4( a, b, c, d, 4, 6, 0xf7537e82 );
		d = round4( d, a, b, c, 11, 10, 0xbd3af235 );
		c = round4( c, d, a, b, 2, 15, 0x2ad7d2bb );
		b = round4( b, c, d, a, 9, 21, 0xeb86d391 );

		value[0] += a;
		value[1] += b;
		value[2] += c;
		value[3] += d;
	}
}

void md5_process( unsigned char *buffer, unsigned int size )
{
	unsigned int bufferOffset;
	unsigned int bufferSize;
	unsigned int i;
	unsigned int j;

	bufferOffset = 0;
	// see if we need to add in from last time
	if ( cacheSize ) {
		for ( i = cacheSize; i < 64; ++i ) {
			cache[i] = buffer[bufferOffset++];
		}
		doblock( cache, 64 );
		processedSize += 64;
		cacheSize = 0;
	}
	// round the remaining size down to a multiple of 64
	bufferSize = size - bufferOffset;
	cacheSize = bufferSize & 63;
	bufferSize -= cacheSize;

	// process the block
	doblock( &buffer[bufferOffset], bufferSize );
	processedSize += bufferSize;

	// if theres any left, store it for next time
	if ( cacheSize ) {
		for ( j = bufferOffset + bufferSize, i = 0; i < cacheSize; ++i, ++j ) {
			cache[i] = buffer[j];
		}
	}
}

void md5_finish( void )
{
	unsigned int padding;
	unsigned int bufferSize;
	unsigned int bits;

	// process anything remaining in the cache, along with padding
	padding = (cacheSize < 56) ? (56 - cacheSize) : (120 - cacheSize);
	bufferSize = cacheSize + padding + 8;
	memset( &cache[cacheSize], 0, padding + 8 );
	cache[cacheSize] = 0x80;
	bits = (processedSize + cacheSize) * 8;
	cache[cacheSize + padding + 0] = bits & 0xff;
	cache[cacheSize + padding + 1] = (bits >> 8) & 0xff;
	cache[cacheSize + padding + 2] = (bits >> 16) & 0xff;
	cache[cacheSize + padding + 3] = (bits >> 24) & 0xff;
	doblock( cache, bufferSize );
	drawnumber( &md5_output[0], (value[0] & 0xff) );
	drawnumber( &md5_output[2], ((value[0] >> 8) & 0xff) );
	drawnumber( &md5_output[4], ((value[0] >> 16) & 0xff) );
	drawnumber( &md5_output[6], ((value[0] >> 24) & 0xff) );
	drawnumber( &md5_output[8], (value[1] & 0xff) );
	drawnumber( &md5_output[10], ((value[1] >> 8) & 0xff) );
	drawnumber( &md5_output[12], ((value[1] >> 16) & 0xff) );
	drawnumber( &md5_output[14], ((value[1] >> 24) & 0xff) );
	drawnumber( &md5_output[16], (value[2] & 0xff) );
	drawnumber( &md5_output[18], ((value[2] >> 8) & 0xff) );
	drawnumber( &md5_output[20], ((value[2] >> 16) & 0xff) );
	drawnumber( &md5_output[22], ((value[2] >> 24) & 0xff) );
	drawnumber( &md5_output[24], (value[3] & 0xff) );
	drawnumber( &md5_output[26], ((value[3] >> 8) & 0xff) );
	drawnumber( &md5_output[28], ((value[3] >> 16) & 0xff) );
	drawnumber( &md5_output[30], ((value[3] >> 24) & 0xff) );
	md5_output[32] = 0;
}
