/* @(#)superp.c	1.1 */
superp(fdi,s1,s2)

int fdi /* file descriptor for input */;
#include "super16.h"
#include "super32.h"

struct pdp16 *s1;
struct pdp32 *s2;

{
#include <stdio.h>
	/*
		 * This subroutine converts a VAX formatted superblock to a PDP 11/70 
		 * superblock.
		 */

	int condi,i;
	char ans[20];

	/* Position file to superblock */
	lseek(fdi,(long)512*1,0);

	/* Read the super block in the VAX format and do conversion */
	condi = read(fdi,s2,512);
	if (condi <= 0) {
		perror("Error reading super block");
		printf("condi %d\n",condi);
		exit(1);
	};

	/* Converting to pdp, make sure alingment words are both zero */
	if(s2->fill1 || s2->fill2) {
		printf("Input file appears to be in PDP 11 format\n");
		printf("If input file is already in PDP 11 format;");
		printf(" conversion will corrupt the output file\n");
		do {
			printf("Do you wish to continue? (y/n): ");
			scanf("%s",ans);
		} 
		while (ans[0] != 'y' && ans[0] != 'n');
		if(ans[0] == 'n') exit(1);
		else exit(1);
	}
	/* Convert data the hard way */

	s1->b16_isize = s2->b32_isize;
	s1->b16_fsize[1] = s2->b32_fsize[0];
	s1->b16_fsize[0] = s2->b32_fsize[1];
	s1->b16_nfree = s2->b32_nfree;
	for (i = 0; i < 100; i += 2) {
		s1->b16_free[i] = s2->b32_free[i+1];
		s1->b16_free[i+1] = s2->b32_free[i];
	};
	s1->b16_ninode = s2->b32_ninode;
	for (i = 0; i < 100; i++) s1->b16_inode[i] = s2->b32_inode[i];
	s1->b16_flock = s2->b32_flock;
	s1->b16_ilock = s2->b32_ilock;
	s1->b16_fmod = s2->b32_fmod;
	s1->b16_ronly = s2->b32_ronly;
	s1->b16_time[1] = s2->b32_time[0];
	s1->b16_time[0] = s2->b32_time[1];
	for (i = 0; i < 4; i++) s1->b16_dinfo[i] = s2->b32_dinfo[i];
	s1->b16_tfree[1] = s2->b32_tfree[0];
	s1->b16_tfree[0] = s2->b32_tfree[1];
	s1->b16_tinode = s2->b32_tinode;
	for (i = 0; i < 6; i++) {
		s1->b16_fname[i] = s2->b32_fname[i];
		s1->b16_fpack[i] = s2->b32_fpack[i];
	};

	return(0);
}
