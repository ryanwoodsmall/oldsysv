/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fstyp:S51Kfstyp.c	1.2"
/* S51Kfstyp */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fsid.h>
#include <sys/ino.h>
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>

#define  SUCCESS	 0
#define  FAILURE	-1
#define PHYSBLKSZ 512

void	exit();
long	lseek();

main(argc,argv)
int	argc;
char	*argv[];
{

	int	fd;
	int	result;
	long	bsize = 0;
	char	*dev;
	char	*usage = "S51Kfstyp special";
	struct	stat	buf;
	struct	filsys	sblock;


	if (argc != 2) {
		fprintf(stderr,"Usage: %s\n",usage);
		exit(1);
	}

	dev = argv[1];
	if (stat(dev, &buf) < 0) {
		fprintf(stderr, "S51Kfstyp: cannot stat <%s>\n",dev);
		exit(1);
	}

	if (((buf.st_mode & S_IFMT) != S_IFBLK) 
		&& ((buf.st_mode & S_IFMT) != S_IFCHR)) {
		fprintf(stderr,"S51Kfstyp: <%s> is not a block, or a character device\n",dev);
		exit(1);
	}

	/*
	 *	Read the super block associated with the device. 
	 */

	if ((fd = open(dev, O_RDONLY)) < 0) {
		fprintf(stderr,"S51Kfstyp: cannot open <%s>\n",dev);
		exit(1);
	}
	
	if (lseek(fd, (long)SUPERBOFF, 0) < 0
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		fprintf(stderr,"S51Kfstyp: cannot read superblock\n");
		close(fd);
		exit(1);
	}
	
	/*
	 *	Determine block size and if it is type S51K
 	 */
	
	if (sblock.s_magic == FsMAGIC) {
		switch (sblock.s_type) {
			case Fs1b:
				bsize = 512;
				break;
			case Fs2b:
				result = s5bsize(fd);
				if (result == Fs2b)
					bsize = 1024;
				else if (result == Fs4b) {
					fprintf(stderr, "S51Kfstyp: <%s> is not type S51K, invalid block size for type S51K\n", dev);
					exit(1);
				}
				else {
					fprintf(stderr, "S51Kfstyp: cannot determine block size\n");
					exit(1);
				}
				break;
			case Fs4b:
				fprintf(stderr, "S51Kfstyp: <%s> is not type S51K, invalid block size for type S51K\n", dev);
				exit(1);
				break;
			default:
				fprintf(stderr,"S51Kfstyp: cannot determine file system type\n");
				exit(1);
		}
	} else {
		fprintf(stderr,"S51Kfstyp: <%s> is not a System V file system\n",dev);
		exit(1);
	}
	close(fd);

	if (sanity_check(&sblock,bsize) == FAILURE) {
		fprintf(stderr,"S51Kfstyp: System V file system sanity check failed\n");
		exit(1);
	}
	fprintf(stdout,"S51K\n");
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

/* heuristic function to determine logical block size of System V file system */

s5bsize(fd)
int fd;
{

	int results[3];
	int count;
	long address;
	long offset;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	
	results[1] = 0;
	results[2] = 0;

	buf = (char *)malloc(PHYSBLKSZ);

	for (count = 1; count < 3; count++) {

		address = 2048 * count;
		if (lseek(fd, address, 0) != address)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		inodes = (struct dinode *)buf;
		if ((inodes[1].di_mode & IFMT) != IFDIR)
			continue;
		if (inodes[1].di_nlink < 2)
			continue;
		if ((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		offset = address << (count + 9);
		if (lseek(fd, offset, 0) != offset)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		dirs = (struct direct *)buf;
		if (dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if (strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;
		results[count] = 1;
		}
	free(buf);
	
	if(results[1])
		return(Fs2b);
	if(results[2])
		return(Fs4b);
	return(-1);
}
