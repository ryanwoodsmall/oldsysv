/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fstyp:s5fstyp.c	1.4"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fsid.h>
#include <sys/ino.h>
#include <sys/param.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>

#define  SUCCESS	 0
#define  FAILURE	-1
#define  cmd		s5fstyp

void	exit();
long	lseek();

main(argc,argv)
int	argc;
char	*argv[];
{

	int	fd;
	long	bsize = 0;
	char	*dev;
	char	*usage = "s5fstyp special";
	struct	stat	buf;
	struct	filsys	sblock;


	if (argc != 2) {
		fprintf(stderr,"Usage: %s\n",usage);
		exit(1);
	}

	dev = argv[1];
	if (stat(dev, &buf) < 0) {
		fprintf(stderr, "cmd: cannot stat <%s>\n",dev);
		exit(1);
	}

	if (((buf.st_mode & S_IFMT) != S_IFBLK) 
		&& ((buf.st_mode & S_IFMT) != S_IFCHR)) {
		fprintf(stderr,"cmd: <%s> is not a block, or a character device\n",dev);
		exit(1);
	}

	/*
	 *	Read the super block associated with the device. 
	 */

	if ((fd = open(dev, O_RDONLY)) < 0) {
		fprintf(stderr,"cmd: cannot open <%s>\n",dev);
		exit(1);
	}
	
	if (lseek(fd, (long)SUPERBOFF, 0) < 0
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		fprintf(stderr,"cmd: cannot read superblock\n");
		close(fd);
		exit(1);
	}
	close(fd);
	
	/*
	 *	Determine if it is a System V file system and set the
	 *	block size.
 	 */
	
	if (sblock.s_magic == FsMAGIC) {
		switch (sblock.s_type) {
			case Fs1b:
				bsize = BSIZE;
				break;
			case Fs2b:
#ifdef u3b5
				bsize = 4 * BSIZE;
#else
				bsize = 2 * BSIZE;
#endif
				break;
			default:
				fprintf(stderr,"cmd: <%s> is not a System V file system\n",dev);
				exit(1);
		}
	} else {
		fprintf(stderr,"cmd: <%s> is not a System V file system\n",dev);
		exit(1);
	}

	if (sanity_check(&sblock,bsize) == FAILURE) {
		fprintf(stderr,"cmd: System V file system sanity check failed\n");
		exit(1);
	}
	fprintf(stdout,"%s\n",S51K);
	exit(0);
}

sanity_check(sblock,bsize)
struct	filsys	*sblock;
long	bsize;
{


	sblock->s_isize -= 2;
	if (sblock->s_fsize <= sblock->s_isize
		|| sblock->s_fsize < sblock->s_tfree
		|| sblock->s_isize < sblock->s_tinode*sizeof(struct dinode)/bsize
		|| (long)sblock->s_isize*bsize/sizeof(struct dinode) > 0x10000L) {
		return(FAILURE);
	}
	return(SUCCESS);
}
