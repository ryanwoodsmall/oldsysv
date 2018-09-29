/* @(#)fscv.c	1.1 */
main(argc, argv)
int argc;
char *argv[];

{
	/*
	 * use: fscv -v|p input [output]
	 *
	 *	-v	convert to VAX file structure from 11
	 *	-p	convert to PDP 11 file structure from VAX
	 *
	 *	input	input file structure (from file)
	 *	output 	output file structure (to file)
	 *	Note:	If output file is not specified an in-place update is done
	 */

	/*
	 * This program converts file systems between VAX and PDP 11s.
	 * Only the file structure is converted, not the data included in
	 * any files.  This conversion is due to the difference in internal
	 * storage of Long integers and boundary alignments on the VAX.
	 */


	/*
	 * The file system contains no information concerning the type of
	 * machine on which it was created.  A rudimentary check of the
	 * file structure is made after exmaning the command line.
	 * The operator is questioned if conversion appears risky.
	 * The conversion should never be made from a mounted file system as
	 * any file activity on the input file will in all probability destroy
	 * the output file.
	 */

#include "params.h"
#include <stdio.h>

	/***** data *****/
	long numbks;
	int fdi,fdo,tovax,notinp,rc;

	/*
	 * The following two structures are hardcoded copies of the superblocs
	 * for the VAX and PDP 11 machines with longs = short[2] and boundary
	 * alignments included so compilation will be independent of the sort of 
	 * machine.
	 */

	/*
	 * The subroutines superv and superp load the following two buffers with
	 * the VAX and PDP 11 super blocks respectively.
	 */

#include "super16.h"
#include "super32.h"
	static union un1 {
		struct pdp16 s1;
		char super16[512];
	} 
	sup16;

	static union un2 {
		struct pdp32 s2;
		char super32[512];
	} 
	sup32;


	/***** program *****/

	/* initialization section */

	/* check command line */

	if (*argv[1] != '-' || *(argv[1]+2) != 0) {
		fprintf(stderr,"Use : fscv -v|p input [output]\n");
		exit(1);
	}
	/*
	 * See if an in-place update is being done
	 */

	if(argc == 3) notinp = 0;
	else if(argc == 4) notinp = 1;
	else {
		fprintf(stderr,"Uset - fscv -v|p input [output]");
		exit(1);
	}

	/*
	 * Set variable tovax to true if converting to a VAX file structure
	 * else set to false making sure -v or -p was used.
	 */
	if (tovax = *(argv[1]+1) == 'v') ;
	else if (*(argv[1]+1) != 'p' ) {
		fprintf(stderr,"Flag error: -v converts to VAX\n");
		fprintf(stderr,"            -p converts to PDP 11\n");
		exit(1);
	};


	/* Open input and output files */

	fdi = open(argv[2],0);
	if (fdi == -1) {
		perror("Error opening input file");
		exit(1);
	};
	fdo = open(argv[2+notinp],2);
	if (fdo == -1) {
		perror("Error opening output file");
		exit(1);
	};

	/* Check to see if conversion should be done */
	ckmnt(argv[2]);
	if(notinp) ckmnt(argv[3]);

	/* Read the super block in two formats  */
	tovax ?
	(superv(fdi,sup16.super16,sup32.super32)) :
	(superp(fdi,sup16.super16,sup32.super32)) ;

	/*
	 * If doing an in-place update just convert necessary blocks, else
	 * copy entire file system first.
	 */
	if(notinp) {
		/* 
		 * Reposition file to beginning
		 */
		lseek(fdi,(long)0,0);
		printf("Copying file system\n");
#ifdef vax
		numbks = *(long *)sup32.s2.b32_fsize;
#else
		numbks = *(long *)sup16.s1.b16_fsize;
#endif
		while(numbks-BUFNOS > 0) {
			cpyblk(BUFNOS,fdi,fdo);
			numbks -= BUFNOS;
		}
		cpyblk((int)numbks,fdi,fdo);
	}

	/*
	 * Write the proper super block to disk
	 */
	lseek(fdo,(long)512*1,0);
	tovax ?
	(rc = write(fdo,sup32.super32,512)) :
	(rc = write(fdo,sup16.super16,512)) ;
	if(rc < 512) perror("Error writing super block");

	/* Convert the free list */
	/* The first indirect block number must be passed as a long integer */
	printf("Converting free list\n");
#ifdef vax
	tovax ?
	indfv(fdo,&sup32.s2.b32_free[0]) :
	indfp(fdo,&sup32.s2.b32_free[0]) ;

#else
	tovax ?
	indfv(fdo,&sup16.s1.b16_free[0]) :
	indfp(fdo,&sup16.s1.b16_free[0]) ;
#endif

	/* Convert the indodes */
	/*
	 * Isize is passed as the third argument so the number of blocks
	 * containing inodes can be calculated.
	 */
	printf("Converting Inodes\n");
	cnvino(tovax,fdo,sup16.s1.b16_isize);
	/*
	 * Sync in case block special device converted
	 */
	sync();

}
