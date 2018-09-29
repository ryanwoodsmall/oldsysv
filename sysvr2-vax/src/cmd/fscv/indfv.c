/* @(#)indfv.c	1.1 */
indfv(fdo,fblock)
long *fblock;
int fdo;
/* Fblock is first block to be read */
{
	/*
		 * This  subroutine reads a free list indirect block number
		 * "block" in PDP 11/70 format and converts it to VAX format.
		 */

#include <stdio.h>

	int i,in,out;
	long block;

#include "indir16.h"
#include "indir32.h"

	static struct ind16 i16;
	static struct ind32 i32;

	/* Get first block */
	block = *fblock;

	while (block != 0) {

		/* Position file to desired block */
		lseek(fdo,block*512,0);

		/* Read in block and convert */
		in = read(fdo,&i16,512);
		if (in <= 0) {
			perror("Error reading free list");
			return(1);
		};

		i32.b32_num = i16.b16_num;

		for ( i = 0; i < 50; i++) {
			i32.b32_blk[2*i] = i16.b16_blk[2*i+1];
			i32.b32_blk[2*i+1] = i16.b16_blk[2*i];
		};

		/* Write out indirect block */
		lseek(fdo,block*512,0);
		out = write(fdo,&i32,in);
		if (out != in ) {
			perror("Error writing free list");
			return(1);
		};

		/*
				 * Set "block" to long integer for next time through loop.
				 * This requires knowledge on the machine on which we are running
				 * since both formats are available.
				 */
#ifdef vax
		block = *(long *)&i32.b32_blk[0] ;
#else
		block = *(long *)&i16.b16_blk[0] ;
#endif

	};
	return(i);
}
