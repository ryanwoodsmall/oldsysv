/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)df:df.c	1.32"

/*	Distributed Unix, FS Switching Version	*/
#include <stdio.h>
#include <sys/param.h>
#include <sys/fs/s5param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ino.h>
#include <mnttab.h>
#include <sys/stat.h>
#include <ustat.h>
#include <sys/statfs.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5dir.h>
#include <setjmp.h>
#include <sys/errno.h>
#include <string.h>

#define EQ(x,y,z) (strncmp(x,y,z)==0)
#define MNTTAB "/etc/mnttab"
#define MOUNTED fs_name[0]
#define DEVLEN sizeof(Mp.mt_dev)
#define DEVLSZ 200	/* number of unique file systems we can remember */
#define FREMOTE	0x0002	/* file is remote (see <sys/user.h>)  */

extern char *optarg;
extern int optind, opterr;
extern int errno;
char *devnm();

struct	mnttab	Mp;
struct stat	S;
struct filsys	sblock;
struct ustat	*Fs = (struct ustat *)&sblock.s_tfree;
struct statfs	Fs_info;

int	physblks;
long	bsize;
int 	bshift;
int	freflg, totflg, locflg;
int	fd;
daddr_t	blkno	= 1;
daddr_t	alloc();

char nfsmes[] = "          (%-10.32s): \
is not a file system, directory, or mounted resource\n";

char usage[] = "Usage: \
df [-lt] [-f] [{<file system> <directory> <mounted resource>}] ...\n";

char *
basename(s)
char *s;
{
	int n = 0;

	while(*s++ != '\0') n++;
	while(n-- > 0)
		if(*(--s) == '/') 
			return(++s);
	return(--s);
}

main(argc, argv)
char **argv;
{
	register fi, i;
	char	 dev[DEVLEN+1], *res_name, *s;
	int	 c, k, errcnt = 0;

	s = basename(argv[0]);
	if(!strncmp(s,"devnm",5)) {

		while(--argc) {
			if(stat(*++argv, &S) == -1) {
				fprintf(stderr,"devnm: ");
				errcnt++;
				continue;
			}
			res_name = devnm();
			if(res_name[0] != '\0')
				printf("%s %s\n", res_name, *argv);
			else {
				printf("devnm: %s not found\n", *argv);
				errcnt++;
			}
		}
		exit(errcnt);
	}
	while((c = getopt(argc,argv,"tfl")) != EOF) {
		switch(c) {

			case 'l':	/* do local resources only */
				locflg = 1;
				break;

			case 'f':	/* freelist count only */
				freflg++;
				break;

			case 't':	/* include total allocations */
				totflg = 1;
				break;

			default:
				fprintf(stderr,usage);
				exit(1);
		}
	}

	if((fi = open(MNTTAB, 0)) < 0) {
		fprintf(stderr,"df: cannot open %s\n",MNTTAB);
		exit(1);
	}
	while ((i = read(fi, &Mp, sizeof(Mp))) == sizeof(Mp)) {
		if(locflg && (Mp.mt_ro_flg & FREMOTE))
			continue;
		sprintf(dev,"%.32s",Mp.mt_dev);
		if(argc > optind) {
				/* we are looking for specific file systems
				   in the mount table */
			for(k=optind;k < argc; k++) {
				if(argv[k][0] == '\0')
					continue;
				if(stat(argv[k], &S) == -1)
					S.st_dev = -1;

					/* does the argument match either
					   the mountpoint or the device? */
				if(EQ(argv[k], Mp.mt_dev, DEVLEN)
				|| EQ(argv[k], Mp.mt_filsys, DEVLEN)) {
					errcnt += printit(dev, Mp.mt_filsys);
					argv[k][0] = '\0';
				} else	
					/* or is it on one of 
						the mounted devices? */
					if( ((S.st_mode & S_IFMT) == S_IFDIR)
					&& (EQ(devnm(), Mp.mt_dev, DEVLEN))) {
						/* must re-do stat cuz
						   devnm() wiped it out */
						if(stat(argv[k], &S) == -1)
							S.st_dev = -1;
						errcnt += printit(dev, argv[k]);
						argv[k][0] = '\0';
					}
			}
		} else {
				/* doing all things in the mount table */
			if(stat(Mp.mt_filsys, &S) == -1)
				S.st_dev = -1;
			errcnt += printit(dev, Mp.mt_filsys);
		}
	}

			/* process arguments that were not in /etc/mnttab */
	for(i = optind; i < argc; ++i) {
		if(argv[i][0])
			errcnt += printit(argv[i], "\0");
	}

	close(fi);
	exit(errcnt);
}

printit(dev, fs_name)
char *dev, *fs_name;
{
	char mark;


	if(MOUNTED && (Mp.mt_ro_flg & FREMOTE))
		if(freflg)
			return(0);

	if(!MOUNTED || freflg) {	/* Unmounted file system queries
					   and freelist querries work only
					   on UNIX file systems */
		if((fd = open(dev, 0)) < 0) {
			fprintf(stderr,"df: cannot open %s\n",dev);
			close(fd);
			return(1);
		}
		if(stat(dev, &S) < 0) {
			fprintf(stderr, "df: cannot stat %s\n", dev);
			close(fd);
			return(1);
		}

		if(((S.st_mode & S_IFMT) == S_IFREG) 
		|| ((S.st_mode & S_IFMT) == S_IFIFO)) {
			printf(nfsmes, dev);
			close(fd);
			return(1);
		}

		sync();
		if(lseek(fd, (long)SUPERBOFF, 0) < 0
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
			close(fd);
			return(1);
		}	
		if(sblock.s_magic == FsMAGIC) {
			if(sblock.s_type == Fs1b) {
				physblks = 1;
				bsize = BSIZE;
				bshift = BSHIFT;
			} else if(sblock.s_type == Fs2b) {
#ifdef u3b5
				physblks = 4;
				bsize = 4 * BSIZE;
				bshift = BSHIFT + 2;
#else
				physblks = 2;
				bsize = 2 * BSIZE;
				bshift = BSHIFT + 1;
#endif
			} else {
				printf("          (%-12s): bad block type\n", dev);
				return(1);
			}
		} else {
			physblks = 1;
			bsize = BSIZE;
			bshift = BSHIFT;
		}
				/* file system sanity test */
		if(sblock.s_fsize <= sblock.s_isize
		|| sblock.s_fsize < Fs->f_tfree
		|| sblock.s_isize < Fs->f_tinode*sizeof(struct dinode)/bsize
		|| (long)sblock.s_isize*bsize/sizeof(struct dinode) > 0xffffL) {
			printf(nfsmes, dev);
			return(1);
		}
		
	} else {	/* mounted file system */

		if(statfs(fs_name, &Fs_info, sizeof(struct statfs), 0) < 0) {
			fprintf(stderr, "df: cannot statfs %s\t", dev);
			perror(" statfs");
			return(1);
		}
			/* copy statfs info into superblock structure */
		Fs->f_tfree = Fs_info.f_bfree;
		Fs->f_tinode = Fs_info.f_ffree;
		strncpy(Fs->f_fname,Fs_info.f_fname,sizeof(Fs_info.f_fname));
		strncpy(Fs->f_fpack,Fs_info.f_fpack,sizeof(Fs_info.f_fpack));
				/* statfs returns block counts mod 512,
				so no conversion is required */
		physblks = 1;
	}

	if(!freflg) {
		if(!MOUNTED) 
			printf("  %-6.6s ", sblock.s_fname);
		else
			printf("%-9s", fs_name);
		printf(" (%-16.32s):", dev);

			/* mark 2nd and subsequent reports on the same
							remote resource.  */
		mark = (firstpik()) ? ' ' : '*';
		printf(" %8ld blocks%c %7u i-nodes\n",
			Fs->f_tfree*physblks, mark, Fs->f_tinode);
	} else {
		daddr_t	i;

		i = 0;
		while(alloc())
			i++;
		printf("%-10s(%-12s): %8ld blocks\n",fs_name, dev,
			i*physblks);
		close(fd);
	}
	if(totflg) {
		printf("                   total: ");
		if(MOUNTED && !freflg)
			printf("%8ld blocks %7u i-nodes\n",
				Fs_info.f_blocks,
				Fs_info.f_files);
		else
			printf("%8ld blocks %7u i-nodes\n",
				sblock.s_fsize*physblks, 
				(unsigned)((sblock.s_isize - 2)
					*bsize/sizeof(struct dinode)));
	}
	return(0);
}

firstpik()
{
	static dev_t devlst[DEVLSZ];
	static	int i, ndev;

	if(S.st_dev == -1)
		return(1);	/* stat() failed.. data invalid */
	if(ndev)
		for(i = 0; i < ndev; i++) 
			if(S.st_dev == devlst[i])
				return(0);
	if(ndev < DEVLSZ)
		devlst[ndev++] = S.st_dev;
	return(1);
}

daddr_t
alloc()
{
	int i;
	daddr_t	b;
		/* Would you believe the 3b20 dfc requires read counts mod 64?
		   Believe it! (see dfc1.c).
		*/
	char buf[(sizeof(struct fblk) + 63) & ~63];
	struct fblk *fb = (struct fblk *)buf;
	
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

		bread(b, buf, sizeof(buf));
		blkno = b;
		sblock.s_nfree = fb->df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = fb->df_free[i];
	}
	return(b);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	int n;

	if((lseek(fd, bno<<bshift, 0)) < 0) {
		perror("bread: seek error");
		exit(1);
	}
	if((n=read(fd, buf, cnt)) != cnt) {
		perror("bread: read error");
		printf("read error %x, count = %d\n", bno, n);
		exit(1);
	}
}

struct direct dbuf;

char *
devnm()
{
	int i;
	static short fno;
	static struct devs {
		char *devdir;
		short dfd;
	} devd[] = {		/* in order of desired search */
		"/dev/dsk",0,
		"/dev",0,
		"/dev/rdsk",0
	};
	static char devnam[DEVLEN];

	devnam[0] = '\0';
	if(!devd[1].dfd) {	/* if /dev isn't open, nothing happens */
		for(i = 0; i < 3; i++) {
			devd[i].dfd = open(devd[i].devdir, 0);
		}
	}
	fno = S.st_dev;

	for(i = 0; i < 3; i++) {
		if((devd[i].dfd >= 0)
		   && (chdir(devd[i].devdir) == 0) 
		   && (dsearch(devd[i].dfd,fno))) {
			strcpy(devnam, devd[i].devdir);
			strcat(devnam,"/");
			strncat(devnam,dbuf.d_name,DIRSIZ);
			return(devnam);
		}
	}
	return(devnam);

}

dsearch(ddir,fno)
short ddir, fno;
{
	lseek(ddir, (long)0, 0);
	while(read(ddir, &dbuf, sizeof dbuf) == sizeof dbuf) {
		if(!dbuf.d_ino) continue;
		if(stat(dbuf.d_name, &S) == -1) {
			fprintf(stderr, "devnm: cannot stat %s\n",dbuf.d_name);
			return(0);
		}
		if((fno != S.st_rdev) 
		|| ((S.st_mode & S_IFMT) != S_IFBLK)
		|| (strcmp(dbuf.d_name,"swap") == 0)
		|| (strcmp(dbuf.d_name,"pipe") == 0)
			) continue;
		return(1);
	}
	return(0);
}
