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
/*	@(#)devnm.c	1.3	*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

main(argc, argv)
char	**argv;
{

	struct stat sbuf;
	struct direct dbuf;
	short	dv, dv1, dv2, fno;
	short found;
	char fname[20];

	if (chdir("/dev") == -1)
		goto  err;
	if ((dv = open(".", 0)) < 0) {
err:
		fprintf(stderr, "Cannot open /dev\n");
		exit(1);
	}
	dv1 = open("/dev/dsk", 0);
	dv2 = open("/dev/rdsk", 0);
	while(--argc) {
		found = 0;
		if (stat(*++argv, &sbuf) == -1) {
			fprintf(stderr,"devnm: ");
			perror(*argv);
			continue;
		}
		fno = sbuf.st_dev;
		lseek(dv, (long)0, 0);
		while(read(dv, &dbuf, sizeof dbuf) == sizeof dbuf) {
			if (!dbuf.d_ino) continue;
			if (stat(dbuf.d_name, &sbuf) == -1) {
				fprintf(stderr, "/dev stat error\n");
				exit(1);
			}
			if ((fno != sbuf.st_rdev) || ((sbuf.st_mode & S_IFMT) !=
				S_IFBLK)) continue;
			if (strcmp(dbuf.d_name,"swap") != 0)
				found++;
			printf("%s %s\n", dbuf.d_name, *argv);
		}
		if (!found && dv1 > 0) {
			lseek(dv1, (long)0, 0);
			while(read(dv1, &dbuf, sizeof dbuf) == sizeof dbuf) {
				if (!dbuf.d_ino) continue;
				sprintf(fname,"dsk/%s",dbuf.d_name);
				if (stat(fname, &sbuf) == -1) {
					fprintf(stderr, "/dev stat error\n");
					exit(1);
				}
				if ((fno != sbuf.st_rdev) ||
				 ((sbuf.st_mode & S_IFMT) != S_IFBLK)) continue;
				found++;
				printf("%s %s\n", fname, *argv);
			}
		}
		if (!found && dv2 > 0) {
			lseek(dv2, (long)0, 0);
			while(read(dv2, &dbuf, sizeof dbuf) == sizeof dbuf) {
				if (!dbuf.d_ino) continue;
				sprintf(fname,"rdsk/%s",dbuf.d_name);
				if (stat(fname, &sbuf) == -1) {
					fprintf(stderr, "/dev stat error\n");
					exit(1);
				}
				if ((fno != sbuf.st_rdev) ||
				 ((sbuf.st_mode & S_IFMT) != S_IFBLK)) continue;
				printf("%s %s\n", fname, *argv);
			}
		}
	}
	exit (0);
}
