/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)clri.c	1.4	*/
/*
 * clri filsys inumber ...
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ino.h>
#include <sys/filsys.h>

/* file system block size */
#if (vax || ns32000 || u3b || u3b5) && (FsTYPE == 3)
#define FSBSIZE	(BSIZE*2)
#else
#define	FSBSIZE BSIZE
#endif

#define ISIZE	(sizeof(struct dinode))
#define	NBINODE	(FSBSIZE/ISIZE)

struct filsys sblock;
int	bsize, nbinode;

struct	ino
{
	char	junk[ISIZE];
};
struct	ino	buf[NBINODE];
int	status;

main(argc, argv)
char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;

	if(argc < 3) {
		printf("usage: clri filsys inumber ...\n");
		exit(4);
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	if(lseek(f, (long)SUPERBOFF, 0) < 0
	   || read(f, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		printf("cannot read super-block of %s\n", argv[1]);
		exit(4);
	}
	if(sblock.s_magic == FsMAGIC) {
		if(sblock.s_type == Fs1b) {
			bsize = BSIZE;
		} else if(sblock.s_type == Fs2b) {
			bsize = BSIZE*2;
		} else {
			printf("bad block type in %s\n", argv[1]);
			exit(4);
		}
	} else {
		bsize = BSIZE;
	}
	nbinode = bsize / ISIZE;
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		if(read(f, (char *)buf, bsize) != bsize) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if(status)
		exit(status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		read(f, (char *)buf, bsize);
		j = (n-1)%nbinode;
		for(k=0; k<ISIZE; k++)
			buf[j].junk[k] = 0;
		lseek(f, off, 0);
		write(f, (char *)buf, bsize);
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
	return(1);
}
