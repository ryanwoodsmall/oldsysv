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
/*	@(#)df.c	1.8	*/
#include <stdio.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ino.h>
#include <mnttab.h>
#include <ustat.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#include <sys/fblk.h>

#define EQ(x,y,z) (strncmp(x,y,z)==0)

struct	mnttab	M[NMOUNT];
struct stat	S;
struct filsys	sblock;
struct ustat	Fs_info, *Fs;

int	physblks;
long	bsize;
int 	bshift;
int	Flg, tflag;
int	fd;
daddr_t	blkno	= 1;
daddr_t	alloc();
char nfsmes[] = "          (%-10.10s): is not a file system\n";

main(argc, argv)
char **argv;
{
	register fi, i;
	register char	c;
	char	 dev[40];
	int	 j, k;

	for(j=1;j < argc;j++) {
		if(argv[j][0] != '-') {
			break;
		}
		for(k=1;argv[j][k] != '\0';k++) {
			switch(c = argv[j][k]) {
				case 'f':
					Flg++;
					break;

				case 'q':
					break;

				case 't':
					tflag=1;
					break;

				default:
					fprintf(stderr,"df: illegal arg -%c\n",c);
					exit(1);
			}
		}
	}
	if((fi = open("/etc/mnttab", 0)) < 0) {
		fprintf(stderr,"df: cannot open /etc/mnttab\n");
		exit(1);
	}
	i = read(fi, M, sizeof M);
	close(fi);

	for(i /= sizeof M[0]; --i >= 0;) {
		if(!M[i].mt_dev[0]) /* it's been umount'ed */
			continue;
		sprintf(dev, "/dev/%.32s", M[i].mt_dev);
		if(argc > j) {
			for(k=j;k < argc; ++k) {
				if(EQ(argv[k], dev, 36)
				|| EQ(argv[k], M[i].mt_dev, 32)
				|| EQ(argv[k], M[i].mt_filsys, 14)) {
					printit(dev, M[i].mt_filsys);
					argv[k][0] = '\0';
				}
			}
		} else
			printit(dev, M[i].mt_filsys);
	}
	for(i = j; i < argc; ++i) {
		if(argv[i][0])
			printit(argv[i], "\0");
	}
	exit(0);
}

printit(dev, fs_name)
char *dev, *fs_name;
{
	if((fd = open(dev, 0)) < 0) {
		fprintf(stderr,"df: cannot open %s\n",dev);
		return;
	}
	if(!Flg) {
		Fs = &Fs_info;
		if(stat(dev, &S) < 0) {
			fprintf(stderr, "df: cannot stat %s\n", dev);
			return;
		}
		if((S.st_mode & S_IFMT) != S_IFBLK || ustat(S.st_rdev, Fs) < 0) {
			Fs = (struct ustat *)&sblock.s_tfree;
		}
	} else {
		Fs = (struct ustat *)&sblock.s_tfree;
		sync();
	}
	if(lseek(fd, (long)SUPERBOFF, 0) < 0
	   || read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		return;
	}
	if(sblock.s_magic == FsMAGIC) {
		if(sblock.s_type == Fs1b) {
			physblks = 1;
			bsize = BSIZE;
			bshift = BSHIFT;
		} else if(sblock.s_type == Fs2b) {
			physblks = 2;
			bsize = BSIZE*2;
			bshift = BSHIFT + 1;
		} else {
			printf("          (%-12s): bad block type\n", dev);
			return;
		}
	} else {
		physblks = 1;
		bsize = BSIZE;
		bshift = BSHIFT;
	}
/* ustat returns physical blocks,
 * filsys contains logical blocks,
 * make correction
 */
	if(Fs == (&Fs_info))
		Fs->f_tfree /= physblks;
/* file system sanity test */
	if(sblock.s_fsize <= sblock.s_isize
	   || sblock.s_fsize < Fs->f_tfree
	   || sblock.s_isize < Fs->f_tinode*sizeof(struct dinode)/bsize
	   || (long)sblock.s_isize*bsize/sizeof(struct dinode) > 0xffffL) {
/* inodes are assumed to be valid up to the largest unsigned short */
		printf(nfsmes, dev);
		return;
	}
	

	if(!Flg) {
		if(*fs_name == '\0') {
			printf("  %-6.6s  ", sblock.s_fname);
		}else {
			printf("%-10s", fs_name);
		}
		printf("(%-12s): %8ld blocks %7u i-nodes\n",
			dev, Fs->f_tfree*physblks,
			Fs->f_tinode);
	}
	else {
		daddr_t	i;

		i = 0;
		while(alloc())
			i++;
		printf("%-10s(%-12s): %8ld blocks\n",fs_name, dev,
			i*physblks);
	}
	if(tflag) {
		printf("                   total: %8ld blocks %7u i-nodes\n",
			sblock.s_fsize*physblks, (unsigned)((sblock.s_isize - 2)*bsize/sizeof(struct dinode)));
	}
	close(fd);
}

daddr_t
alloc()
{
	int i;
	daddr_t	b;
	struct fblk buf;

	i = --sblock.s_nfree;
	if(i<0 || i>=NICFREE) {
		printf("bad free count, b=%ld\n", blkno);
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<sblock.s_isize || b>=sblock.s_fsize) {
		printf("bad free block (%ld)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {
		bread(b, &buf, sizeof(buf));
		blkno = b;
		sblock.s_nfree = buf.df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.df_free[i];
	}
	return(b);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	int n;
	extern errno;

	lseek(fd, bno<<bshift, 0);
	if((n=read(fd, buf, cnt)) != cnt) {
		printf("read error %ld\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		exit(0);
	}
}
