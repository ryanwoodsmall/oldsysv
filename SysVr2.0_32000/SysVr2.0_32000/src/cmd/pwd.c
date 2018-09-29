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
/*
**	Print working (current) directory
*/

/*	@(#)pwd.c	1.7	*/

#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/signal.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<sys/dir.h>

struct	stat	d, dd;
struct	direct	dir;

char	dot[]	 = ".";
char	dotdot[] = "..";
char	name[512];

FILE	*file;
int	off = -1;

main()
{
	for(;;) {
		if(stat(dot, &d) < 0) {
			fprintf(stderr, "pwd: cannot stat .!\n");
			exit(2);
		}
		if ((file = fopen(dotdot,"r")) < 0) {
			fprintf(stderr,"pwd: cannot open ..\n");
			exit(2);
		}
		if(fstat(fileno(file), &dd) < 0) {
			fprintf(stderr, "pwd: cannot stat ..!\n");
			exit(2);
		}
		if(chdir(dotdot) < 0) {
			fprintf(stderr, "pwd: cannot chdir to ..\n");
			exit(2);
		}
		if(d.st_dev == dd.st_dev) {
			if(d.st_ino == dd.st_ino)
				prname();
			do
				if (fread((char *)&dir, sizeof(dir), 1, file) < 1) {
					fprintf(stderr, "pwd: read error in ..\n");
					exit(2);
				}
			while (dir.d_ino != d.st_ino);
		}
		else do {
				if(fread((char *)&dir, sizeof(dir), 1, file) < 1) {
					fprintf(stderr, "pwd: read error in ..\n");
					exit(2);
				}
				stat(dir.d_name, &dd);
		} while(dd.st_ino != d.st_ino || dd.st_dev != d.st_dev);
		fclose(file);
		cat();
	}
}

prname()
{
	write(1, "/", 1);
	if (off<0)
		off = 0;
	name[off] = '\n';
	write(1, name, off+1);
	exit(0);
}

cat()
{
	register i, j;

	i = -1;
	while ((dir.d_name[++i] != 0) && (i < 14));
	if ((off+i+2) > 511)
		prname();
	for(j=off+1; j>=0; --j)
		name[j+i+1] = name[j];
	off=i+off+1;
	name[i] = '/';
	for(--i; i>=0; --i)
		name[i] = dir.d_name[i];
}
