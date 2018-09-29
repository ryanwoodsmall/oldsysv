/*	@(#)devnm.c	1.4	*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

struct stat sbuf;
struct direct dbuf;

main(argc, argv)
char	**argv;
{
	short	dv, dv1, dv2, fno;
	char fname[20];

	if ((chdir("/dev") == -1) || (dv = open(".", 0)) < 0) {
		fprintf(stderr, "Cannot open /dev\n");
		exit(1);
	}
	dv1 = open("/dev/dsk", 0);
	dv2 = open("/dev/rdsk", 0);
	while(--argc) {
		if (stat(*++argv, &sbuf) == -1) {
			fprintf(stderr,"devnm: ");
			perror(*argv);
			continue;
		}
		fno = sbuf.st_dev;
				/* search for standard names first (/dev/dsk?1?)
				   then for old standard names (/dev/????)
				   then for /dev/rdsk names	*/
		if ((dv1 > 0) && (dsearch(*argv,dv1,fno)))
			printf("%s dsk/%s\n", dbuf.d_name, *argv);
		else if(dsearch(dv,fno))
			printf("/dev/%s %s\n", dbuf.d_name, *argv);
		else if ((dv2 > 0) && (dsearch(*argv,dv2,fno)))
			printf("/dev/rdsk/%s rdsk/%s\n", dbuf.d_name, *argv);
	}
	exit (0);
}

dsearch(fd,fno)
short fd, fno;
{
	lseek(fd, (long)0, 0);
	while(read(fd, &dbuf, sizeof dbuf) == sizeof dbuf) {
		if (!dbuf.d_ino) continue;
		if (stat(dbuf.d_name, &sbuf) == -1) {
			fprintf(stderr, "devnm: cannot stat %s\n",dbuf.d_name);
			exit(1);
		}
		if ((fno != sbuf.st_rdev) 
			|| ((sbuf.st_mode & S_IFMT) != S_IFBLK)) continue;
		if (strcmp(dbuf.d_name,"swap") != 0)
			return(1);
	}
	return(0);
}
