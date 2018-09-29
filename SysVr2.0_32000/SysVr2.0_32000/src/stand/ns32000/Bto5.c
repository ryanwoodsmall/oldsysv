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
#include <stand.h>
#include <sys/filsys.h>
#include "Bfilsys.h"
#include <sys/dcu.h>

struct filsys f;
struct Bfilsys b;

main (argc, argv)
char **argv;
{
	extern struct dcu_iocb erriocb;
	char *strcpy(), *strcmp();
	char buf[50];
	static char devname[] = "/dev/dsk/1s0";
	int i, j, fdes;
	struct dtab *dp;
	extern struct dtab _dtab[];

again:
	if (argc == 2) { 
		argc = 0;
		strcpy (buf, argv[1]);
	}
	else {
		do {
			printf ("File System /dev/dsk/? ");
			gets (buf);
		} while  ((buf[0]<'0' || buf[0]>'7') || (buf[1]!='s') || (buf[2]<'0' || buf[2]>'7'));   
	}
	devname[9] = buf[0];
	devname[11] = buf[2];

	for (dp = &_dtab[0]; dp < &_dtab[NDEV]; ++dp) {
		if (dp->dt_name == 0)
			break;
		else if (strcmp (devname, dp->dt_name) == 0) {
			if ((fdes = open (devname, 2)) == -1) {
				printf ("Can't open %s\n", devname);
				goto again;
			}
			else
				goto doit;
		}
	}

	buf[0] -= '0';
	if (MKNOD (devname, 0, buf[0], (daddr_t) 0) < 0) {
		perror (devname);
		goto again;
	}
	else {
		if ((fdes = open (devname, 2)) == -1) {
			dp->dt_name = 0;
			goto again;
		}
	}

doit:
	lseek (fdes, 1024, 0);
	read (fdes, &b, sizeof(b));
	printf ("isize %d fsize %d\n", b.s_isize, b.s_fsize);
	lseek (fdes, 512, 0);
	read (fdes, &f, sizeof(f));
	f.s_isize = b.s_isize;
	f.s_fsize = b.s_fsize;
	f.s_nfree = b.s_nfree;
	f.s_ninode = b.s_ninode;
	f.s_time = b.s_time;
	f.s_tfree = b.s_tfree;
	f.s_tinode = b.s_tinode;
	j = b.s_nfree;
	if (j > NICFREE) {
		j = NICFREE;
		f.s_nfree = j;
	}
	for (i=0; i<j; i++)
		f.s_free[i] = b.s_free[i];
	for (i=0; i<b.s_ninode; i++)
		f.s_inode[i] = b.s_inode[i];
	f.s_magic = FsMAGIC;
	f.s_type = Fs2b;
	lseek (fdes, 512, 0);
	write (fdes, &f, 512);
}
