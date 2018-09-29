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
/*	Swap administrative interface
 *	Used to add/delete/list swap devices in use by paging.
 */

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/dir.h>
#include	<sys/map.h>
#include	<sys/swap.h>
#include	<sys/page.h>
#include	<sys/stat.h>

/*	This is gross, but it will have
 *	to do for now.  How about a sysadm(2)?
 */

#ifdef u3b
#define swapfunc(X)	sys3b(3, X)
#endif

#ifdef vax
swapfunc()
{
	asm("	chmk	$58");
	asm("	bcc	.noerror");
	asm("	jmp	cerror");
	asm(".noerror:");
}
#endif

#ifdef ns32000
swapfunc()
{
	asm("	addr	58,r0");
	asm("	addr	8(fp),r1");
	asm("	svc");
	asm("	bfc	.noerror");
	asm("	nop");	/* fool optimizer */
	asm("	jump	cerror");
	asm(".noerror:");
}
#endif

typedef struct SwapDirs {
	char	*sd_name;	/* Name of directory to search.	*/
	int	sd_fd;		/* File descriptor.		*/
} SwapDirs_t;

SwapDirs_t	SwapDirs[] = {
	{"/dev/dsk"},
	{"/dev"},
};

#define	NbrSwapDirs	(sizeof(SwapDirs) / sizeof(SwapDirs_t))

typedef struct direct	dir_t;

#define	DirBufSz	(BSIZE / sizeof(dir_t))

dir_t	DirBuf[DirBufSz];

main(argc, argv)
int	argc;
char	*argv[];
{
	if(argc < 2  ||  argv[1][0] != '-'){
		usage();
		exit(1);
	}

	switch(argv[1][1]){
		case 'l':
			list();
			break;
		
		case 'd':
			if(argc < 4){
				usage();
				exit(2);
			}
			delete(argv[2], argv[3]);
			break;
		
		case 'a':
			if(argc < 5){
				usage();
				exit(2);
			}
			add(argv[2], argv[3], argv[4]);
			break;
	}
}


usage()
{
	printf("usage:\tswap -l\n");
	printf("\tswap -d <special dev> <low block>\n");
	printf("\tswap -a <special dev> <low block> <nbr of blocks>\n");
}

list()
{
	register int	swplo;
	register int	blkcnt;
	register swpt_t	*st;
	register swpi_t	*si;
	register int	i;
	swpt_t		swaptab[MSFILES];
	swpi_t		swpi;
	char		*path;
	int		length;
	char		*GetPath();

	st = swaptab;
	si = &swpi;
	si->si_cmd = SI_LIST;
	si->si_buf = (char *)st;

	if(swapfunc(si) < 0){
		perror("swap");
		exit(3);
	}

	InitPath();

	printf("path            dev  swaplo blocks   free\n");

	for(i = 0  ;  i < MSFILES  ;  i++, st++){
		if(st->st_ucnt == NULL)
			continue;
		path = GetPath(st->st_dev);
		if(path == NULL){
			printf("%.15s", "??????                 ");
		} else {
			length = 15 -strlen(path);
			if(length < 0)
				length = 0;
			printf("%s%.*s", path, length, "               ");

		}
		printf("%2d,%-2d %6d %6d %6d", 
			st->st_dev >> 8,
			st->st_dev &  0xFF,
			st->st_swplo,
			st->st_npgs << DPPSHFT,
			st->st_nfpgs << DPPSHFT);
		if(st->st_flags & ST_INDEL)
			printf(" INDEL\n");
		else
			printf("\n");
	}

}

delete(path, lo)
char	*path;
char	*lo;
{
	register int	swplo;
	register swpi_t	*si;
	swpi_t		swpi;

	swplo = atoi(lo);
	si = &swpi;
	si->si_cmd = SI_DEL;
	si->si_buf = path;
	si->si_swplo = swplo;

	if(swapfunc(si) < 0){
		perror("swap");
		exit(3);
	}
}

add(path, lo, cnt)
char	*path;
char	*lo;
char	*cnt;
{
	register int	swplo;
	register int	blkcnt;
	register swpi_t	*si;
	swpi_t		swpi;

	swplo = atoi(lo);
	blkcnt = atoi(cnt);
	si = &swpi;
	si->si_cmd = SI_ADD;
	si->si_buf = path;
	si->si_swplo = swplo;
	si->si_nblks = blkcnt;

	if(swapfunc(si) < 0){
		perror("swap");
		exit(3);
	}

}


/*	Initialization for the path search routine.
*/

InitPath()
{
	register int	i;

	for(i = 0  ;  i < NbrSwapDirs  ;  i++){
		SwapDirs[i].sd_fd =
			open(SwapDirs[i].sd_name, O_RDONLY);
		if(SwapDirs[i].sd_fd < 0){
			fprintf(stderr, "Couldn't open directory %s",
				SwapDirs[i].sd_name);
			fprintf(stderr, " - not being searched.\n");
			perror("swap");
		}
	}
}


/*	Get path name corresponding to a device.
*/

#define	MAXPATH	200

char	PathBuf[MAXPATH];

char	*
GetPath(dev)
register int	dev;
{
	register int	sdndx;
	register char	*path;
	char		*SearchDir();

	for(sdndx = 0  ;  sdndx < NbrSwapDirs  ;  sdndx++){

		/*	Skip any directories which we
		**	couldn't open.
		*/

		if(SwapDirs[sdndx].sd_fd < 0)
			continue;
		
		/*	Change to directory we want to search.
		*/

		if(chdir(SwapDirs[sdndx].sd_name) < 0){
			fprintf(stderr, "Couldn't chdir to %s",
				SwapDirs[sdndx].sd_name);
			fprintf(stderr, " - directory not searched.\n");
			perror("swap");
			SwapDirs[sdndx].sd_fd = -1;
			continue;
		}

		/*	Position at start of directory.
		*/

		lseek(SwapDirs[sdndx].sd_fd, 0, 0);

		if(path = SearchDir(&SwapDirs[sdndx], dev))
			return(path);
	}
	return(NULL);
}


char	*
SearchDir(sdp, dev)
register SwapDirs_t	*sdp;
register int		dev;
{
	register int	limit;
	register int	i;
	register dir_t	*dp;
	struct stat	statbuf;

	while(limit = read(sdp->sd_fd, DirBuf, DirBufSz)){
		limit /= sizeof(dir_t);
		dp = (dir_t *)DirBuf;
		for(i = 0  ;  i < limit  ;  i++, dp++){
			if(dp->d_ino == 0)
				continue;
			sprintf(PathBuf, "%.14s", dp->d_name);
			if(stat(PathBuf, &statbuf) <0){
				fprintf(stderr,
					"Could not stat %s.n",
					PathBuf);
				perror("swap");
				continue;
			}
			if(statbuf.st_rdev != dev)
				continue;
			if((statbuf.st_mode & S_IFMT) != S_IFBLK)
				continue;
			strcpy(PathBuf, sdp->sd_name);
			strcat(PathBuf, "/");
			sprintf(&PathBuf[strlen(PathBuf)], "%.14s",
				dp->d_name);
			return(PathBuf);
		}
	}
	return(NULL);
}

/* <@(#)swap.c	1.2> */
