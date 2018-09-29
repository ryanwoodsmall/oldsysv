/* @(#)cpyblk.c	1.1 */
cpyblk(num,fdi,fdo)
int num,fdi,fdo;
{
	/*
	 * This subroutine copies "num" blocks from input file descriptor fdi
	 * to output file descriptor fdo.
	 */

#include <stdio.h>
#include "params.h"

	int in,out;
	char buff[BUFNOS*512];

	/* Make sure too many blocks not read */
	if (BUFNOS < num) {
		printf("cpyblk: Illegal number of blocks\n");
		return(-1);
	};

	in = read(fdi,buff,num*512);
	if (in < 0) {
		perror("Error copying block from input file");
		return(in);
	};

	if (in > 0) {
		out = write(fdo,buff,in);
		if (in != out) {
			perror("Error copying block to output file");
			return(-1);
		};
	};
	return(in);
}
