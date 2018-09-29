/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)labelit:labelit.c	1.17"
/* labelit with 512, 1K, and 2K block size support */
#include <stdio.h>
#include <sys/param.h>
#ifndef	RT
#include <signal.h>
#include <sys/types.h>
#endif
#include <sys/inode.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5dir.h>
#include <sys/ino.h>
#include <sys/sysmacros.h>
#include <sys/fs/s5filsys.h>
#define DEV 1
#define FSNAME 2
#define VOLUME 3
#define PHYSBLKSZ 512
 /* write fsname, volume # on disk superblock */
struct {
	char fill1[SUPERBOFF];
	union {
		char fill2[BSIZE];
		struct filsys fs;
	} f;
} super;
#ifdef RT
#define IFTAPE(s) (equal(s,"/dev/mt",7)||equal(s,"mt",2))
#define TAPENAMES "'/dev/mt'"
#else
#define IFTAPE(s) (equal(s,"/dev/rmt",8)||equal(s,"rmt",3)||equal(s,"/dev/rtp",8)||equal(s,"rtp",3))
#define TAPENAMES "'/dev/rmt' or '/dev/rtp'"
#endif

struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens,
		t_reelblks,
		t_blksize,
		t_nblocks;
	char	t_fill[472];
} Tape_hdr;

int fsi, fso;

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv) 
char **argv; 
{
	long curtime;
	int i;
#if u3b15 || u3b2
	int result;
#endif
#ifdef	RT
	static char usage = "Usage: labelit /dev/??? [fsname volume [-n]]\n";
#else
	static char *usage = "Usage: labelit /dev/r??? [fsname volume [-n]]\n";
#endif

	signal(SIGALRM, sigalrm);

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

	if(argc!=4 && argc!=2 && argc!=5)  {
	}
	if(argc==5) {
		if(strcmp(argv[4], "-n")!=0) {
			fprintf(stderr, usage);
			exit(2);
		}
		if(!IFTAPE(argv[DEV])) {
			fprintf(stderr, "labelit: `-n' option for tape only\n");
			fprintf(stderr, "\t'%s' is not a valid tape name\n", argv[DEV]);
			fprintf(stderr, "\tValid tape names begin with %s\n", TAPENAMES);
			fprintf(stderr, usage);
			exit(2);
		}
		printf("Skipping label check!\n");
		goto do_it;
	}

	if((fsi = open(argv[DEV],0)) < 0) {
		fprintf(stderr, "labelit: cannot open device\n");
		exit(2);
	}

	if(IFTAPE(argv[DEV])) {
		alarm(5);
		read(fsi, &Tape_hdr, sizeof(Tape_hdr));
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {
			fprintf(stderr, "labelit: tape not labelled!\n");
			exit(2);
		}
		printf("%s tape volume: %s, reel %d of %d reels\n",
			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		printf("Written: %s", ctime(&Tape_hdr.t_time));
		if((argc==2 && Tape_hdr.t_reel>1) || equal(Tape_hdr.t_magic,"Finc",4))
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read superblock\n");
		exit(2);
	}

#define	S	super.f.fs
	if (S.s_magic == FsMAGIC) {
		switch(S.s_type)  {
		case Fs1b:
		    printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 512b, ", 
			S.s_fname, S.s_fpack, S.s_fsize, (S.s_isize - 2) * 8);
		    break;
#if u3b15 || u3b2
		case Fs2b:
		    if(IFTAPE(argv[DEV]))
			result = tps5bsz(argv[DEV], Tape_hdr.t_blksize);
		    else
			result = s5bsize(fsi);
		    if(result == Fs2b)
			printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 2,(S.s_isize - 2) * 16);
		    else if(result == Fs4b)
			printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 4,(S.s_isize - 2) * 32);
		    else {
			fprintf(stderr,"WARNING: can't determine logical block size of %s\n         root inode or root directory may be corrupted!\n", argv[DEV]);
			printf("Current fsname: %.6s, Current volname: %.6s\n", S.s_fname, S.s_fpack);
		    }
		    break;
		case Fs4b:
		    printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 4,(S.s_isize - 2) * 32);
		    break;
#else
		case Fs2b:
		    printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 2,(S.s_isize - 2) * 16);
		    break;
#endif
		default:
		    fprintf(stderr,"labelit: bad block type\n");
		    printf("Current fsname: %.6s, Current volname: %.6s\n", S.s_fname, S.s_fpack);
		}
	}
	else
		printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 512b, ", 
			S.s_fname, S.s_fpack, S.s_fsize, (S.s_isize - 2) * 8);
	printf("Date last modified: %s", ctime(&S.s_time));
	if(argc==2)
		exit(0);
do_it:
	printf("NEW fsname = %.6s, NEW volname = %.6s -- DEL if wrong!!\n",
		argv[FSNAME], argv[VOLUME]);
	sleep(10);
	sprintf(super.f.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.f.fs.s_fpack, "%.6s", argv[VOLUME]);

	close(fsi);
	fso = open(argv[DEV],1);
	if(IFTAPE(argv[DEV])) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", argv[VOLUME]);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0)
			goto cannot;
	}
	if(write(fso, &super, sizeof(super)) < 0) {
cannot:
		fprintf(stderr, "labelit cannot write label\n");
		exit(2);
	}
	exit(0);
}
equal(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
}
#if u3b2 || u3b15
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
/* heuristic function to determine logical block size of System V file system
 *  on tape
 */
tps5bsz(dev, blksize)
char	*dev;
long	blksize;
{

	int count;
	int skipblks;
	long offset;
	long address;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	

	buf = (char *)malloc(2 * blksize);

	close(fsi);

	/* look for i-list starting at offset 2048 (indicating 1KB block size) 
	 *   or 4096 (indicating 2KB block size)
	 */
	for(count = 1; count < 3; count++) {

		address = 2048 * count;
		skipblks = address / blksize;
		offset = address % blksize;
		if ((fsi = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   start of i-list */
		read(fsi, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fsi, buf, blksize);
			skipblks--;
		}

		if (read(fsi, buf, blksize) != blksize) {
			close(fsi);
			continue;
		}
		/* if first 2 inodes cross block boundary read next block also*/
		if ((offset + 2 * sizeof(struct dinode)) > blksize) {
		    if (read(fsi, &buf[blksize], blksize) != blksize) {
			close(fsi);
			continue;
		    }
		}
		close(fsi);
		inodes = (struct dinode *)&buf[offset];
		if((inodes[1].di_mode & IFMT) != IFDIR)
			continue;
		if(inodes[1].di_nlink < 2)
			continue;
		if((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		/* look for root directory at address specified by potential */
		/*   root inode */
		address = address << (count + 9);
		skipblks = address / blksize;
		offset = address % blksize;
		if ((fsi = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   root directory */
		read(fsi, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fsi, buf, blksize);
			skipblks--;
		}

		if (read(fsi, buf, blksize) != blksize) {
			close(fsi);
			continue;
		}
		/* if first 2 directory entries cross block boundary read next 
		 *   block also 
		 */
		if ((offset + 2 * sizeof(struct direct)) > blksize) {
		    if (read(fsi, &buf[blksize], blksize) != blksize) {
			close(fsi);
			continue;
		    }
		}
		close(fsi);

		dirs = (struct direct *)&buf[offset];
		if(dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if(strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;

		if (count == 1) {
			free(buf);
			return(Fs2b);
		}
		else if (count == 2) {
			free(buf);
			return(Fs4b);
		}
	}
	free(buf);
	return(-1);
}
#endif
