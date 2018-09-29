/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)volcopy:volcopy.c	4.33.1.12"
#define LOG
#define AFLG 0 
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5dir.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/stat.h>
#ifdef u3b
#include <sys/elog.h>
#include <sys/mtc.h>
#endif
#include <stdio.h>
#include <signal.h>
#define FILE_SYS 1
#define DEV_FROM 2
#define FROM_VOL 3
#define DEV_TO 4
#define TO_VOL 5
#define INPUT 0
#define OUTPUT 1
#define	T_TYPE	0xfd187e20	/* like FsMAGIC */
#define EQ(X,Y,Z) !strncmp(X,Y,Z)
#define NOT_EQ(X,Y,Z) strncmp(X,Y,Z)
#if defined(u3b15)
/*
 * For volcopy to work on 3B[5|15] and especially on Apache,
 * it should recognize all valid path names for tape.  The string
 * "rmt/" is the common string in path names on all these machines.
 * We will also check for the "rtp/" string for compatibility.
 */
IFTAPE(dev_name)
register char * dev_name;
{
	register int length;

	for(length = strlen(dev_name); length > 4; length--, dev_name++)
		if(strncmp(dev_name, "rmt/", 4) == 0 ||
		   strncmp(dev_name, "rtp/", 4) == 0)
			return(1);
	return(0);
}
#else
#define IFTAPE(s) (EQ(s,"/dev/rmt",8)||EQ(s,"rmt",3)\
||EQ(s,"/dev/rtp",8)||EQ(s,"rtp",3))
	/* the tp is there for upwards compatibility with the old usage */
#endif	/* u3b15 */
#ifdef	u370
#define BLKSIZ	4096
#else
#define	BLKSIZ	512	/* use physical blocks */
#endif
#define _2_DAYS 172800L
#define MAX 1000000L
#ifdef	u370
#define Ft800x10	2L	/* u370 */
#define Ft1600x10	4L	/* u370 */
#define Ft6250x50	15L	/* u370 */
#else
#define Ft800x10	15L
#define Ft1600x4	22L
#define Ft1600x10	28L
#define Ft1600x16	30L
#define Ft1600x32	32L
#define Ft6250x10	90L
#define Ft6250x16	95L
#define Ft6250x32	115L
#define Ft6250x50	120L
#endif

#ifdef u3b15
/*
 * Read 1 big block from disk and break up into transfer_ratio smaller
 * blocks, which the tape can handle, to write to tape.  transfer_ratio
 * of greater than 1 should not be used when reading from tape.
 */
int transfer_ratio = 1;	/* Same size read and write by default.	*/
#endif	/* u3b15 */
#if defined(u3b15)
/*
 * Should we ask the user if he/she wants shell to be invoked after
 * hitting break or delete?  The "-nosh" option, which sets the
 * noshell_flag to 1, should be used as a security precaution when
 * the user should not be allowed to get shell through volcopy.
 */
int noshell_flag = 0;	/* Allow shell by default.	*/
#endif	/* u3b15 */
/*
 * If either of the volume arguments in the command line is '-', we
 * need a place to store the volume name that is read from the device.
 */
#define VOLUME_NAME_LENGTH	6
char to_vol[VOLUME_NAME_LENGTH + 1], from_vol[VOLUME_NAME_LENGTH + 1];	
struct Tphdr {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time;
	long	t_length;
	long	t_dens;
	long	t_reelblks;	/* u370 added field */
	long	t_blksize;	/* u370 added field */
	long	t_nblocks;	/* u370 added field */
	char	t_fill[468];
	int	t_type;		/* does tape have nblocks field? (u3b || u3b15) */
} Tape_hdr;

int	K_drive = 0;	/* 3B20 Kennedy tape drive (4 blks/rec max) */
int	T_drive = 0;	/* 3B20 Tape File Controller (50 blks/rec) */
#if defined(u3b15)
int	A_drive = 0;	/* 3B15 Accellerated Tape Ctlr (32 blks/rec) */
int	C_drive = 0;	/* 3B15 Compatibilty Mode (10 blks/rec) */
#endif
int	first = 0;
char	**args;
int	Nblocks = 0;
long	Reelblks = MAX;
int	Reels = 1;
int	reel = 1;
int	Reelsize = 0;
long	rblock = 0, reeloff = 0;
long	saveFs;
int	Bpi = 0;
int	bufflg = 0;
int	yesflg = 0;
long	Fs;
short	*Buf;
long	Fstype;
int	sts;
int	p_in, p_out;
extern	int errno;

/*

filesystem copy with propagation of volume ID and filesystem name:

  volcopy [-options]  filesystem /dev/from from_vol /dev/to to_vol

options are:
	-feet - length of tape
	-bpi  - recording density
	-reel - reel number (if not starting from beginning)
	-buf  - use double buffered i/o (if dens >= 1600 bpi)
	-a    - ask "y or n" instead of "DEL if wrong"
	-s    - inverse of -a
		From/to devices are printed followed by `?'.
		User has 10 seconds to DEL if mistaken!
	-y    - assume "yes" response to all questions

  Examples:

  volcopy root /dev/rdsk/0s2 pk5 /dev/rdsk/1s2 pk12

  volcopy u3 /dev/rdsk/1s5 pk1 /dev/rmt/0m tp123

  volcopy u5 /dev/rmt/0m -  /dev/rdsk/1s5 -
	In this example, dashed volume args mean "use label that's there."

 */
#if defined(u3b15)
/*
	-block NUM - Set the transfer block size to NUM physical blocks (512
		bytes on 3B2 and 3B15).  Note that an arbitrary block size might
		or might not work on a given system.  Also, the block size
		read from the header of an input tape silently overrides this.
	-nosh - Don't offer the user a shell after hitting break or delete.
		Only works on 3B15.
	-r NUM - Read NUM transfer blocks from the disk at once and write it
		to the output device one block at a time.  Intended only to
		boost the 3B15 EDFC disk to tape performance.  Disabled on 3B2.
*/
#endif	/* u3b15 */

long	Block;
char *Totape, *Fromtape;
FILE	*Devtty;
char	*Tape_nm;
int	pid;
long	tvec;

struct filsys	Superi, Supero, *Sptr;

extern int	read(), write();
char *tapeck();

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

sigint()
{
	extern char **environ;
	int tmpflg;
	int i = 0, ps1 = -1, ps2 = -1;

	tmpflg = yesflg;	/* override yesflg for interrupts */
	yesflg = 0;		/* cuz it's obviously an exception */
	if(pid != 1) {
#ifdef u3b15
		if( !noshell_flag && asks("Want Shell?   ")) {
#else
		if(asks("Want Shell?   ")) {
#endif	/* u3b15 */
			if(!fork()) {	/* setting the secondary prompt
					** only works if PS1 and PS2
					** are both exported */
				while(environ[i][0]) { /* change PS1 to PS2 */
					if(EQ(environ[i],"PS1",3)) 
						ps1 = i;
					if(EQ(environ[i],"PS2",3))
						ps2 = i;
					i++;
				}
				if((ps1 >=0) && (ps2 >= 0))
					environ[ps1] = environ[ps2];
				execl("/bin/sh", "/bin/sh", 0);
			}
			else
				wait((int *)0);
		}
		else if(asks("Want to quit?    ")) {
			if(pid) kilchld();
			exit(2);
		}
	}
	signal(SIGINT, sigint);
	yesflg = tmpflg;		/* reset it */
}

kilchld()
{
	kill(pid -1,9);
}

ask() 
{
	if(asks("Type `y' to override:     "))
		return;
	exit(9);
}
asks(s)
char *s;
{
	char ans[12];
	printf(s);
	if(yesflg) {
		printf("YES\n");
		return(1);
	}
	ans[0] = '\0';
	fgets(ans, 10, Devtty);
	for(;;) {
		switch(ans[0]) {

		case 'a':
		case 'A':
			if(pid == 1) { /* child process */
				write(p_out,"ABORT",1);
				exit(1);
			}
			if(pid)		/* parent process */
				kilchld();
			exit(1);
		case 'y':
		case 'Y':
			return(1);
		case 'n':
		case 'N':
			return(0);
		default:
			printf("\n(y or n)?");
			fgets(ans, 10, Devtty);
		}
	}
}

main(argc, argv) char **argv;
{
	int	fsi, fso;
#if u3b2 || u3b15
	int	result;
#endif
	struct	stat statb;
	int	altflg = AFLG;
	FILE	*popen();
	char	vol[12], dev[12], c;

	signal(SIGALRM, sigalrm);

	while(argv[1][0] == '-') {
		if(EQ(argv[1], "-bpi", 4))
			if((c = argv[1][4]) >= '0' && c <= '9')
				Bpi = getbpi(&argv[1][4]);
			else {
				++argv;
				--argc;
				Bpi = getbpi(&argv[1][0]);
			}
		else if(EQ(argv[1], "-feet", 5))
			if((c = argv[1][5]) >= '0' && c <= '9')
				Reelsize = atoi(&argv[1][5]);
			else {
				++argv;
				--argc;
				Reelsize = atoi(&argv[1][0]);
			}
		else if(EQ(argv[1],"-reel",5))
			if((c = argv[1][5]) >= '0' && c <= '9')
				reel = atoi(&argv[1][5]);
			else {
				++argv;
				--argc;
				reel = atoi(&argv[1][0]);
			}
#ifdef u3b15
		else if(EQ(argv[1], "-r", 2))	{
			if((c = argv[1][2]) >= '0' & c <= '9')
				transfer_ratio = atoi(&argv[1][2]);
			else {
				++argv;
				--argc;
				transfer_ratio = atoi(&argv[1][0]);
			}
			if(transfer_ratio == 0)	{
				fprintf(stderr,"volcopy: Need a non-zero value for the -r option\n");
				exit(1);
			}
		}
#endif	/* 3b15 */
#if defined(u3b15)
		else if(EQ(argv[1], "-block", 6))	{
			if((c = argv[1][6]) >= '0' & c <= '9')
				Nblocks = atoi(&argv[1][6]);
			else {
				++argv;
				--argc;
				Nblocks = atoi(&argv[1][0]);
			}
			if(Nblocks == 0)	{
				fprintf(stderr,"volcopy: Need a non-zero value for the -block option\n");
				exit(1);
			}
		}
#endif /* u3b15 */
#ifdef u3b15
		else if(EQ(argv[1],"-nosh",5))
			noshell_flag++;
#endif	/* u3b15 */
		else if(EQ(argv[1],"-buf",4))
			bufflg++;
		else if(EQ(argv[1],"-a",2))
			altflg++;
		else if(EQ(argv[1],"-s",2))
			altflg = 0;
		else if(EQ(argv[1],"-y",2))
			yesflg++;
		else {
			fprintf(stderr, "<%s> invalid option\n",
				argv[1]);
			exit(1);
		}
		++argv;
		--argc;
	}
	args = argv;

	Devtty = fopen("/dev/tty", "r");
	if ((Devtty == NULL) && !isatty(0))
		Devtty = stdin;
	time(&tvec);
			/* get mandatory inputs */
	if(argc!=6) {
		fprintf(stderr,"Usage: volcopy [options] fsname ");
		fprintf(stderr,"/devfrom volfrom /devto volto\n");
		exit (9);
	}

	/*
	* Argv[TO_VOL] may be as short as one character, viz., '-'.  If so,
 	* the volume name, which can be 6 characters long, is read from the
 	* device itself and stored in argv[TO_VOL] which obviously has room
 	* for only one character.  So as not to overwrite other data, we'll
 	* make argv[TO_VOL] point to a 7-character array.  Same thing is
	* done for argv[FROM_VOL].  NOTE: Volume names specified as command
	* line arguments are silently truncated to 6 charcters.
 	*/
	strncpy(to_vol, argv[TO_VOL], VOLUME_NAME_LENGTH);
	/* Null-terminate the string in case strncpy has not.	*/
	to_vol[VOLUME_NAME_LENGTH] = '\0';
	argv[TO_VOL] = &to_vol[0];
	strncpy(from_vol, argv[FROM_VOL], VOLUME_NAME_LENGTH);
	from_vol[VOLUME_NAME_LENGTH] = '\0';
	argv[FROM_VOL] = &from_vol[0];

	if((fsi = open(argv[DEV_FROM],0)) < 1)
		fprintf(stderr, "%s: ",argv[DEV_FROM]), err("cannot open");
	if((fso = open(argv[DEV_TO],0)) < 1)
		fprintf(stderr, "%s: ",argv[DEV_TO]), err("cannot open");

	if(fstat(fsi, &statb)<0 || (statb.st_mode&S_IFMT)!=S_IFCHR)
		err("From device not character-special");
	if(fstat(fso, &statb)<0 || (statb.st_mode&S_IFMT)!=S_IFCHR)
		err("To device not character-special");

	Fromtape = tapeck(argv[DEV_FROM], argv[FROM_VOL], fsi, INPUT);
	Totape = tapeck(argv[DEV_TO], argv[TO_VOL], fso, OUTPUT);
	if(Totape && Fromtape)
		err("Use dd(1) command to copy tapes");

#ifdef	u370
	Nblocks = (Totape||Fromtape)? 8:8; /* disk-disk can even be > 8 */
	Buf = (short *)malloc(BLKSIZ*(Nblocks+1));
	/* Force buffer to page boundary */
	Buf = (short *)((int)((char *)Buf+BLKSIZ) & ~(BLKSIZ-1));
#else
#if u3b
	if(K_drive)
		Nblocks = 4;
	else
		Nblocks = ((Totape||Fromtape)&&(Bpi!=6250))? 10:152;
	if(Bpi == 6250)
		if(T_drive)
			Nblocks = 50;
 		else
			Nblocks = 10;
	Buf = (short *)malloc(BLKSIZ*Nblocks);
	if((int)Buf == -1 && Nblocks == 152) {
		Nblocks = 32;
		Buf = (short *)malloc(BLKSIZ*Nblocks);
	}
#else
#if defined(u3b15)
	if(Nblocks == 0) {
		if(A_drive)
			Nblocks = 32;
		else
			if(C_drive)
				Nblocks = 10;
			else
				Nblocks = (Totape||Fromtape)? 16:152;
	}
#ifdef u3b15
	/*
	 * This line was dropped from the 3b15 part of the code when three
	 * sets of ifdefs were consolidated.  Also, the argument to malloc()
	 * was modified to get transfer_ratio blocks instead of 1 block.
	 */
	Buf = (short *)malloc( (BLKSIZ*Nblocks) * transfer_ratio);
	if(Buf == (short *) NULL)	{
		fprintf(stderr,"volcopy: cannot allocate %d * %d * %d = %d bytes for buffer.\n", Nblocks, BLKSIZ, transfer_ratio, Nblocks * BLKSIZ * transfer_ratio);
#else
	Buf = (short *)malloc(BLKSIZ*Nblocks);
	if(Buf == (short *) NULL)	{
		fprintf(stderr,"volcopy: cannot allocate %d * %d = %d bytes for buffer.\n", Nblocks, BLKSIZ, Nblocks * BLKSIZ);
#endif	/* u3b15 */
		fprintf(stderr,"Try specifying a smaller block size using the -block option.\n");
		exit(9);
	}
	if((int)Buf == -1 && Nblocks == 152) {
		Nblocks = 32;
		Buf = (short *)malloc(BLKSIZ*Nblocks);
	}
#else
	Nblocks = ((Totape||Fromtape) && (Bpi!=6250))? 10:88;
	if(Bpi == 6250) Nblocks = 50;
	Buf = (short *)malloc(BLKSIZ*Nblocks);
	if((int)Buf == -1 && Nblocks == 88) {
		Nblocks = 22;
		Buf = (short *)malloc(BLKSIZ*Nblocks);
	}
#endif	/* u3b15 */
#endif	/* u3b */
#endif	/* u370 */
	if((int)Buf == -1) {
		fprintf(stderr, "Not enough memory--get help\n");
		exit(1);
	}
	Sptr = (struct filsys *)&Buf[BLKSIZ/2];
	if(!Fromtape && !Totape) reel = 1;
	if((reel == 1) || !Fromtape) {
		if(read(fsi, Buf, 2*BLKSIZ) < 2*BLKSIZ)
			err("read error on input");
		close(fsi);
		strncpy(Superi.s_fname,  Sptr->s_fname,6);
		strncpy(Superi.s_fpack,  Sptr->s_fpack,6);

		if(Sptr->s_magic != FsMAGIC)
			Sptr->s_type = Fs1b;
		switch ((int)Sptr->s_type) {
		case Fs1b:
			Fstype = 1;
			Fs = Sptr->s_fsize;
			break;
#if u3b2 || u3b15
		case Fs2b:
			if(Fromtape)
				result = tps5bsz(argv[DEV_FROM]);
			else
				result = s5bsize(argv[DEV_FROM]);
			if (result == Fs2b) {
				Fstype = 2;
				Fs = Sptr->s_fsize * 2;
			}
			else if (result == Fs4b) {
				Fstype = 4;
				Fs = Sptr->s_fsize * 4;
			}
			else
				err("can't determine logical block size,\n    root inode or root directory may be corrupted");
			break;
		case Fs4b:
			Fstype = 4;
			Fs = Sptr->s_fsize * 4;
			break;
#else
		case Fs2b:
			Fstype = 2;
			Fs = Sptr->s_fsize * 2;
			break;
#endif
		default:
			err("File System type unknown--get help");
		}

		Superi.s_fsize = Sptr->s_fsize;
		Superi.s_time = Sptr->s_time;
	}

	if(read(fso, Buf, 2*BLKSIZ) < 2*BLKSIZ) {
		fprintf(stderr,"Read error on output\n");
		if(!Totape || !altflg) ask();
	}
	strncpy(Supero.s_fname, Sptr->s_fname,6);
	strncpy(Supero.s_fpack, Sptr->s_fpack,6);
	Supero.s_fsize = Sptr->s_fsize;
	Supero.s_time = Sptr->s_time;

	if((reel != 1) && Fromtape) {	/* if this isn't reel 1,
			the TO_FS better have been initialized */
                printf("\volcopy: IF REEL 1 HAS NOT BEEN RESTORED,");
		printf(" STOP NOW AND START OVER ***\07\n");
		if(!asks(" Continue? ")) exit(9);
		strncpy(Superi.s_fname,argv[FILE_SYS],6);
		strncpy(Superi.s_fpack,argv[FROM_VOL],6);
	}
	if(Totape) {
		Reels = Fs / Reelblks + ((Fs % Reelblks) ? 1 : 0);
		printf("You will need %d reels.\n", Reels);
		printf("(\tThe same size and density");
		printf(" is expected for all reels)\n");
			/* output vol name for tape has been validated */
		strncpy(Tape_hdr.t_volume,argv[TO_VOL],6);
		strncpy(Supero.s_fpack,argv[TO_VOL],6);
		strncpy(vol,argv[TO_VOL],6);
		strncpy(Supero.s_fname,argv[FILE_SYS],6);
	}
	if(Fromtape) {
		if((Tape_hdr.t_reel != reel
			|| Tape_hdr.t_reels!=Reels)) {
			fprintf(stderr, "Tape disagrees: Reel %d of %d",
				Tape_hdr.t_reel, Tape_hdr.t_reels);
			fprintf(stderr," : looking for %d of %d\n",
				reel,Reels);
			ask();
		}
		strncpy(vol,Tape_hdr.t_volume,6);
	}

	if(NOT_EQ(argv[FILE_SYS],Superi.s_fname, 6)) {
		printf("arg. (%.6s) doesn't agree with from fs. (%.6s)\n",
			argv[FILE_SYS],Superi.s_fname);
		if(!Totape || !altflg) ask();
	}
	if(NOT_EQ(argv[FROM_VOL],"-", 6) &&
	   NOT_EQ(argv[FROM_VOL],Superi.s_fpack, 6)) {
		printf("arg. (%.6s) doesn't agree with from vol.(%.6s)\n",
			argv[FROM_VOL],Superi.s_fpack);
		if(!Totape || !altflg) ask();
	}

	if(argv[FROM_VOL][0]=='-') argv[FROM_VOL] = Superi.s_fpack;
	if(argv[TO_VOL][0]=='-') argv[TO_VOL] = Supero.s_fpack;

	if((reel == 1) && (Supero.s_time+_2_DAYS > Superi.s_time)) {
		printf("%s less than 48 hours older than %s\n",
			argv[DEV_TO], argv[DEV_FROM]);
		printf("To filesystem dated:  %s", ctime(&Supero.s_time));
		if(!altflg) ask();
	}
	if(NOT_EQ(argv[TO_VOL],Supero.s_fpack, 6)) {
		printf("arg.(%.6s) doesn't agree with to vol.(%.6s)\n",
			argv[TO_VOL],Supero.s_fpack);
		ask();
		strncpy(Supero.s_fpack,  argv[TO_VOL],6);
	}
	if(Superi.s_fsize > Supero.s_fsize && !Totape) {
		printf("from fs larger than to fs\n");
		ask();
	}
	if(!Totape && NOT_EQ(Superi.s_fname,Supero.s_fname, 6)) {
		printf("warning! from fs(%.6s) differs from to fs(%.6s)\n",
			Superi.s_fname,Supero.s_fname);
		if(!altflg) ask();
	}

	printf("From: %s, to: %s? ",
		argv[DEV_FROM], argv[DEV_TO]);
	if(altflg) {
		if(!asks("(y or n) ")) {
			printf("\nvolcopy: STOP\n");
			exit(9);
		}
	}
	else {
                printf("(DEL if wrong)\n");
		sleep(10);   /*  10 seconds to DEL  */
	}
	close(fso);
	sync();
	fsi = open(argv[DEV_FROM], 0);
	fso = open(argv[DEV_TO], 1);
	if(Totape) {
		Tape_hdr.t_reels = Reels;
		Tape_hdr.t_reel = reel;
		Tape_hdr.t_time = tvec;
		Tape_hdr.t_reelblks = Reelblks;
		Tape_hdr.t_blksize = BLKSIZ*Nblocks;
		Tape_hdr.t_nblocks = Nblocks;
		Tape_hdr.t_type = T_TYPE;
		write(fso, &Tape_hdr, sizeof Tape_hdr);
		strcpy(vol,argv[TO_VOL]);
	} else if(Fromtape) {
		read(fsi, &Tape_hdr, sizeof Tape_hdr);
	}
	if(reel > 1) {
		Fs = (reel -1) * Reelblks + Nblocks;
		lseek(Totape ? fsi : fso,(long)(Fs * BLKSIZ),0);
		Sptr = Totape? &Superi: &Supero;
		Fs = (Sptr->s_fsize * Fstype) - Fs;
	}
	rprt(vol);

	signal(SIGINT, sigint);

	while(copy(fsi,fso))
		if(Totape)
			fso = chgreel(fso,dev,vol);
		else
			fsi = chgreel(fsi,dev,vol);
	printf("  END: %ld blocks.\n", Block);

#ifdef LOG
	fslog(argv);
#endif
	if(Block) exit(0);
	exit(1);		/* failed.. 0 blocks */
}

copy(fsi,fso)
int fsi,fso;
{
	int i, cnt;
	int p1[2], p2[2];
	char buf[20];
#ifdef u3b15
	/*
	 * This code fixes the problem of slow 3B15 disk, which doesn't
	 * keep the tape streaming.  If the -r transfer_ratio command line
	 * argument is used, multiply Nblocks by that transfer ratio, so
	 * we read a large chunk from the disk to get better performance.
	 * Since tapes have a limit on the size of I/O jobs, if the FROM
	 * device is a tape, don't do this buffering.  Also, if the TO
	 * device is a tape, use the original Nblocks in writes.
         */
	int Nblocks_for_writes;
	/* Xfer_count keeps track of the # of physical blocks written.	*/
	int xfer_count = 0;

	if ( !Totape && !Fromtape )
	/* If a disk to disk transfer, writes can be as big as reads	*/
		Nblocks_for_writes = Nblocks * transfer_ratio;
	else
		Nblocks_for_writes = Nblocks;

	if(!Fromtape)
		Nblocks = Nblocks * transfer_ratio;
#endif	/* u3b15 */

#ifdef u3b
	if(T_drive) {
		if(ioctl(Totape ? fso : fsi, BMODE) < 0)
			printf("Buffered mode failed, using normal mode\n");
		else
			T_drive = BMODE;
	}
#endif
	saveFs = Fs;
	pid = -1;
	if(bufflg && (Bpi >= 1600)) {
		if( pipe(p1) || pipe(p2)) {
			printf("\volcopy: cannot open pipe, err = %d\n",errno);
			exit(1);
		}
		pid = fork();
		if(pid) {
			close(p1[0]);
			close(p2[1]);
			p_in = p2[0];
			p_out = p1[1];
		}
		else {
			close(p1[1]);
			close(p2[0]);
			p_in = p1[0];
			p_out = p2[1];
			write(p_out,"rw",2);	/* prime the pipe */
		}
	}
	pid++;		/* pid is >0 if we forked */
			/* child has pid == 1 */

		/* copy from fsi to fso */

	while((Fs > 0) && (rblock < Reelblks)) {
#ifdef u3b15
		/* 
	 	* Are we going to write too many blocks?  That is, there
	 	* might be enough room left on the tape for one or more
	 	* write-sized blocks, but not as many as we read in one LARGE
	 	* read-sized block.  If so, reduce the size of the read block.
	 	*/
		while((rblock + Nblocks - Nblocks_for_writes) >= Reelblks)
			Nblocks -= Nblocks_for_writes;
#endif	/* u3b15 */
		Nblocks = Fs > Nblocks ? Nblocks : Fs;

		if(pid) {
			if(pid == 1) {
				Fs -= Nblocks;
				if(Fs <= 0) goto cfin;
				Block += Nblocks;
				rblock += Nblocks;
				Nblocks = Fs > Nblocks ? Nblocks : Fs;
			}
			cnt = read(p_in,buf,1);
			if(cnt < 0 || buf[0] != 'r') {
			   if(pid == 1) {
				write(p_out,"R",1);
				exit(1);
			   }
			   else {
				piperr(buf);
			   }
			}
		}

		if((sts = read(fsi, Buf, BLKSIZ * Nblocks)) != BLKSIZ * Nblocks) {
			printf("Read error on block %ld...\n", Block);
			perror("");
			for(i=0; i != Nblocks * (BLKSIZ/2); ++i) Buf[i] = 0;
			if(!Fromtape)
				lseek(fsi,(long)((Block+Nblocks) * BLKSIZ), 0);
		}

		/*
		 * This code was moved out of main (which did the
		 * first read & write) so that the same number of
		 * blocks would be written on each tape--a change
		 * required for finc, frec, and ff compatibility.
		 */
		if(!first && pid != 1 && reel == 1) {
			first++;
			strncpy(Sptr->s_fpack,  args[TO_VOL],6);
			strncpy(Sptr->s_fname,  args[FILE_SYS],6);
		}
		if(pid) {
			write(p_out,"r",1);	/* signal read complete */
			cnt = read(p_in,buf,1);
			if(cnt < 0 || buf[0] != 'w') {
				if(pid == 1) {
					write(p_out,"W",1);
					exit(1);
				}
				piperr(buf);
			}
		}

#ifdef u3b15
		while  ( xfer_count < Nblocks )	/* More small blocks to write */
		{
		if ( ( xfer_count + Nblocks_for_writes ) > Nblocks )
		/* Transfering a partial block at the end of the file system. */
	       		Nblocks_for_writes =  Nblocks - xfer_count;
		/* Buf is an array of shorts, hence the division by 2	*/
		if((sts = write(fso, &Buf[(BLKSIZ * xfer_count)/2], BLKSIZ*Nblocks_for_writes)) != BLKSIZ*Nblocks_for_writes) {
#else
		if((sts = write(fso, Buf, BLKSIZ*Nblocks)) != BLKSIZ*Nblocks) {
#endif	/* u3b15 */
			printf("Write error %d, block %ld...\n",
				errno,Block);
			if(Totape) {
				if(pid == 1) {
					write(p_out,"Tape error",10);
					exit(1);
				}
				if(asks("Want to try another tape?   ")) {
					asks("Type `y' when ready:   ");
					--reel;
					Block = reeloff;
					Fs = saveFs;
					lseek(fsi, (long)reeloff*BLKSIZ, 0);
					return(1);
				}
			} else {
				exit(9);
			}
		}
#ifdef u3b15
		xfer_count += Nblocks_for_writes;
		}
		xfer_count = 0;
#endif	/* u3b15 */

		if(pid) {
			write(p_out,"w",1);	/* signal write complete */
			if(pid != 1) {
				Fs -= Nblocks;
				Block += Nblocks;
				rblock += Nblocks;
				Nblocks = Fs > Nblocks ? Nblocks : Fs;
			}
		}
		Fs -= Nblocks;
		Block += Nblocks;
		rblock += Nblocks;
	}
	if(pid == 1) {
cfin:		write(p_out,"Done",4);
		while (cnt < 0 || buf[0] != 'D') {
			cnt = read(p_in,buf,1);
		}
		exit(0);
	}
	else if(pid) {
		cnt = read(p_in,buf,1);
/*
 * resolve some end condition problems
 */
		if((Fs + Nblocks) > 0) {
			if(cnt < 0 || buf[0] != 'r') piperr(buf);
			cnt = read(p_in, buf, 1);
			if(cnt < 0 || buf[0] != 'w') piperr(buf);
			cnt = read(p_in, buf, 1);
		}
/***/
		if(cnt < 0 || buf[0] != 'D') piperr(buf);
		write(p_out,"Done",4);
		close(p_in);
		close(p_out);
	}
#ifdef u3b
	if((T_drive == BMODE) && Totape) {
		if(ioctl(fso, BWAIT) < 0) {
			printf("write error %d, block %ld\n",errno,Block);
			return(1);
		}
	}
#endif
	return((Fs > 0) ? 1 : 0);
}

err(s) char *s; 
{
	printf("%s\n\t%d reel(s) completed\n",s,--reel);
	exit(9);
}
getbpi(inp)
char *inp;
{
#if u3b 
	if(inp[4] == 'k' || inp[4] == 'K') {
		K_drive++;
		inp[4] = '\0';
	}
#endif
#if defined(u3b15)
/*
 *	Kludge to recognize Accellerated Tape Controller usage from
 *	   letter 'a' or 'A' following density given by user.
 */
	if (inp[4] == 'a' || inp[4] == 'A') 
	{	A_drive++;
		inp[4] = '\0';
	}
/*
 *	Kludge to recognize 3B15 Compatibility Mode from
 *	   letter 'c' or 'C' following density given by user.
 */
	if (inp[4] == 'c' || inp[4] == 'C') 
	{	C_drive++;
		inp[4] = '\0';
	}
#endif
	return(atoi(inp));
}
#if defined(u3b15)
int
physical_blocks_per_ft(discount)
double discount;
{
	double d_Nblocks = Nblocks;
	double d_Bpi = Bpi;
	double d_BLKSIZ = BLKSIZ;
	double d_gap = 0.3;	/* Inter-block gap is 0.3 in.	*/

	return((int) ((d_Nblocks/((((d_Nblocks*d_BLKSIZ)/d_Bpi)+d_gap)/12.0))*discount));
}
#endif	/* u3b15 */

char *
tapeck(dev, vol, fd, ioflg)
char *dev, *vol;
int ioflg;
{
	char	resp[16];

	if(!IFTAPE(dev))
		return 0;
#ifdef u3b
	if(ioctl(fd, IFTFC) >= 0)
		T_drive++;
#endif
	Tape_nm = dev;
	Tape_hdr.t_magic[0] = '\0';	/* scribble on old data */
	alarm(5);
	if(read(fd, &Tape_hdr, sizeof Tape_hdr) <= 0)
		if(ioflg == INPUT)
			perror("input tape");
		else
			perror("output tape");
	alarm(0);
	if(NOT_EQ(Tape_hdr.t_magic, "Volcopy", 7)) {
		fprintf(stderr,"Not a labeled tape. ");
		if(ioflg == OUTPUT) {
			ask();
			mklabel();
			strncpy(Tape_hdr.t_volume, vol, 6);
			Supero.s_time = 0;
		}
		else err("Input tape not made by volcopy");
	}
	else if(Tape_hdr.t_reel == (char)0)
		if(ioflg == INPUT) {
			fprintf(stderr,"Input tape is empty\n");
			exit(9);
		}
	if(vol[0] == '-') 
		strncpy(vol, Tape_hdr.t_volume, 6);
	else if(NOT_EQ(Tape_hdr.t_volume, vol, 6)) {
		fprintf(stderr, "Header volume(%.6s) does not match (%s)\n",
			Tape_hdr.t_volume, vol);
		ask();
		strncpy(Tape_hdr.t_volume, vol, 6);
	}
tapein:
	if(ioflg == INPUT) {
		Reels = Tape_hdr.t_reels;
		Reelsize = Tape_hdr.t_length;
		Bpi = Tape_hdr.t_dens;
#if u3b
		if(Tape_hdr.t_type == T_TYPE) {
			if(Tape_hdr.t_nblocks == 4)
				K_drive++; /* use Kennedy tapedrive limit */
			if(Tape_hdr.t_nblocks == 50)
				T_drive++;
			else	T_drive = 0;
		} else
			K_drive++;
#endif
#if defined(u3b15)
		if (Tape_hdr.t_type == T_TYPE) {
			if (Tape_hdr.t_nblocks == 0) {
				Nblocks = 10;
				C_drive++;
			}
			else
				Nblocks = Tape_hdr.t_nblocks;
			if (Tape_hdr.t_nblocks == 32)
				A_drive++; /* use Accellerated Controller limit */
			else
				A_drive = 0;
		}
		else {
			A_drive = 0;
			Nblocks = 10;
			C_drive++;
		}
#endif	/* u3b15 */
	}
	else{
		Reels = 0;
	}
	if(Reelsize == 0) {
		printf("Enter size of reel in feet for <%s>:   ", vol);
		fgets(resp, 10, Devtty);
		Reelsize = atoi(resp);
	}
	if(Reelsize <= 0 || Reelsize > 3600) {
		fprintf(stderr, "Size of reel must be > 0, <= 3600\n");
		Reelsize = 0;
		goto tapein;
	}
	if(!Bpi) {
		printf("Tape density? (i.e., 800 | 1600 | 6250)?   ");
		fgets(resp, 10, Devtty);
		Bpi = getbpi(resp);
	}
	if(Bpi == 800)
		Reelblks = Ft800x10 * Reelsize;
	else if(Bpi == 1600) {
#if u3b
		if(K_drive)
			Reelblks = Ft1600x4 * Reelsize;
		else
			Reelblks = Ft1600x10 * Reelsize;
#elif defined(u3b15)
		switch (Nblocks) {

		case 0:
			/* Must be writing a new tape since Nblocks not set */
			if (A_drive)
				Reelblks = Ft1600x32 * Reelsize;
			else
				if (C_drive)
					Reelblks = Ft1600x10 * Reelsize;
				else
					Reelblks = Ft1600x16 * Reelsize;
			break;

		case 32:
			Reelblks = Ft1600x32 * Reelsize;
			break;

		case 16:
			Reelblks = Ft1600x16 * Reelsize;
			break;

		case 10:
			Reelblks = Ft1600x10 * Reelsize;
			break;
		default:
			/* 
			 * Arbitrary block sizes.  Figure out the right number
			 * of physical blocks that can be written into 1 ft.
			 * of tape discounting for the inter-block gap and a
			 * possibly short tape.  Assume the usable part of the
			 * tape is 0.85 of its length for small block sizes and
			 * 0.88 of it for large block sizes.
			 */
			if(Nblocks < 32)
				Reelblks = physical_blocks_per_ft(0.85) * Reelsize;
			else
				Reelblks = physical_blocks_per_ft(0.88) * Reelsize;
			break;
		}

#else
		Reelblks = Ft1600x10 * Reelsize;
#endif /* u3b15 */
	}
	else if(Bpi == 6250)
#if u3b
	     {
		if(T_drive)
			Reelblks = Ft6250x50 * Reelsize;
		else
			Reelblks = Ft6250x10 * Reelsize;
	     }
#elif defined(u3b15)
	{
		switch (Nblocks) {

		case 0:
			/* Must be writing a new tape since Nblocks not yet set */
			if (A_drive)
				Reelblks = Ft6250x32 * Reelsize;
			else
				if (C_drive)
					Reelblks = Ft6250x10 * Reelsize;
				else
					Reelblks = Ft6250x16 * Reelsize;
			break;

		case 32:
			Reelblks = Ft6250x32 * Reelsize;
			break;

		case 16:
			Reelblks = Ft6250x16 * Reelsize;
			break;

		case 10:
			Reelblks = Ft6250x10 * Reelsize;
			break;
		default:
			/* See the comment for the 1600 bpi case.	*/
			if(Nblocks < 32)
				Reelblks = physical_blocks_per_ft(0.85) * Reelsize;
			else
				Reelblks = physical_blocks_per_ft(0.88) * Reelsize;
			break;
		}
	}
#else
		Reelblks = Ft6250x50 * Reelsize;
#endif	/* u3b */
	else {
		fprintf(stderr, "Bpi must be 800, 1600, or 6250\n");
		Bpi = 0;
		goto tapein;
	}
	printf("\nReel %.6s",Tape_hdr.t_volume);
	Tape_hdr.t_length = Reelsize;
	printf(", %d feet",Reelsize);
	Tape_hdr.t_dens = Bpi;
	printf(", %d BPI\n",Bpi);
	return dev;
}

hdrck(fd, tvol)
char *tvol;
{
	struct Tphdr *thdr;

	thdr = (struct Tphdr *) Buf;
	alarm(15);	/* dont scan whole tape for label */
	if((read(fd, thdr, sizeof Tape_hdr)) != sizeof Tape_hdr) {
		alarm(0);
		fprintf(stderr, "Cannot read header\n");
		if(Totape) {
			ask();
			strncpy(Tape_hdr.t_volume, tvol, 6);
			return(1);
		}
		else{
			close(fd);
			return 0;
		}
	}
	alarm(0);
	Tape_hdr.t_reel = thdr->t_reel;
	if(NOT_EQ(thdr->t_volume, tvol, 6)) {
		fprintf(stderr, "Volume is <%.6s>, not <%s>.\n",
			thdr->t_volume, tvol);
		if(asks("Want to override?   ")) {
			if(Totape) {
				strncpy(Tape_hdr.t_volume, tvol, 6);
			}
			else{
				strncpy(tvol,thdr->t_volume,6);
			}
			return 1;
		}
		return 0;
	}
	return 1;
}

mklabel()
{
	int i;

	for(i = 0; i < sizeof Tape_hdr; i++)
		Tape_hdr.t_magic[i] = '\0';
	strncpy(Tape_hdr.t_magic,"Volcopy\0",8);
}

rprt(vol)
char *vol;
{
	if(Totape)
		printf("\nWriting REEL %d of %d, VOL = %.6s\n",
		  reel,Reels,vol);
	if(Fromtape)
		printf("\nReading REEL %d of %d, VOL = %.6s\n",
		  reel,Reels,vol);
}
#ifdef LOG
fslog(argv)
char *argv[];
{
	char cmd[500];

	if(access("/etc/log/filesave.log", 6) == -1) {
		fprintf(stderr,
			"volcopy: cannot access /etc/log/filesave.log\n");
		exit(0);
	}
	system("tail -200 /etc/log/filesave.log >/tmp/FSJUNK");
	system("cp /tmp/FSJUNK /etc/log/filesave.log");
	sprintf(cmd,"echo \"%s;%.6s;%.6s -> %s;%.6s;%.6s on %.24s\" >>/etc/log/filesave.log",
		argv[DEV_FROM], Superi.s_fname, Superi.s_fpack, 
		argv[DEV_TO], Supero.s_fname,
		 Supero.s_fpack, ctime(&tvec));
	system(cmd);
	system("rm /tmp/FSJUNK");
	exit(0);
}
#endif	/* LOG */

chgreel(fs,dev,vol)
int fs;
char *dev, *vol;
{
	char ctemp[21];

	rblock = 0;
	reeloff = Block;
	++reel;
again:
	if((sts = close(fs)) < 0)
		perror("chgreel");
	printf("Changing drives? (type RETURN for no,\n");
	printf("\t`/dev/rmt/??' or `/dev/rtp/??' for yes: ");
	fgets(ctemp, 20, Devtty);
	ctemp[strlen(ctemp) -1] = '\0';
	if(ctemp[0] != '\0')
		while(!IFTAPE(ctemp)) {
		  printf("\n'%s' is not a valid device",ctemp);
		  printf("\n\tenter device name `/dev/rmt/??'");
		  printf(" or `/dev/rtp/??' :");
		  fgets(ctemp, 20, Devtty);
		  ctemp[strlen(ctemp) -1] = '\0';
		}
		strcpy(dev,ctemp);
	printf("Mount tape %d\nType volume-ID when ready:   ",
		reel);
	fgets(ctemp, 10, Devtty);
	ctemp[strlen(ctemp) -1] = '\0';
	if(ctemp[0] != '\0') { /*if only <cr> - use old vol-id */
		strcpy(vol,ctemp);
		if(Totape)
			strncpy(Tape_hdr.t_volume,ctemp,6);
	}
	if(*dev)
		Tape_nm = dev;
	if(Totape) {
		fs = open(Tape_nm, 0);
		if(fs <= 0 || fs > 10)
			perror("output ERR");
		if(!hdrck(fs, vol))
			goto again;
		Tape_hdr.t_reel = reel;
		close(fs);
		sleep(2);
		fs = open(Tape_nm, 1);
		if(fs <= 0 || fs > 10)
			perror("output ERR");
		if(write(fs, &Tape_hdr, sizeof Tape_hdr) < 0) {
 				 fprintf(stderr, "Cannot re-write header");
		fprintf(stderr,"-%s\n","Try again!");
		goto again;
		}
	} else {
		fs = open(Tape_nm, 0);
		if(fs <= 0 || fs > 10)
			perror("input ERR");
		if(!hdrck(fs, vol))
			goto again;
		if(Tape_hdr.t_reel != reel) {
		  fprintf(stderr,"Need reel %d,",reel);
		  fprintf(stderr," label says reel %d\n",
			Tape_hdr.t_reel);
		  goto again;
		}
	}
	rprt(vol);
	return(fs);
}

piperr(pbuff)
char pbuff[];
{
	if(pbuff[0] == 'R')
		printf("\volcopy: read sequence error");
	
	else if(pbuff[0] == 'W')
		printf("\volcopy: write sequence error");
	
	else
		perror("\nvolcopy: pipe error");

	printf(" reel %d, %d blocks\n",reel, Block);
	kilchld();
	exit(1);
}
#if u3b2 || u3b15
/* heuristic function to determine logical block size of System V file system
 *  on disk
 */
s5bsize(dev)
char	*dev;
{

	int fd;
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

	buf = (char *)malloc(BLKSIZ);
	fd = open(dev, 0);

	for (count = 1; count < 3; count++) {

		address = 2048 * count;
		if (lseek(fd,address,0) != address)
			continue;
		if (read(fd, buf, BLKSIZ) != BLKSIZ)
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
		if (lseek(fd,offset,0) != offset)
			continue;
		if (read(fd, buf, BLKSIZ) != BLKSIZ)
			continue;
		dirs = (struct direct *)buf;
		if (dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if (strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;
		results[count] = 1;
		}
	close(fd);
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
tps5bsz(dev)
char	*dev;
{

	int fd;
	int count;
	int skipblks;
	long offset;
	long address;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	
	buf = (char *)malloc(2*BLKSIZ*Nblocks);

	/* look for i-list starting at offset 2048 (indicating 1KB block size) 
	 *   or 4096 (indicating 2KB block size)
	 */
	for(count = 1; count < 3; count++) {

		address = 2048 * count;
		skipblks = address / (BLKSIZ * Nblocks);
		offset = address % (BLKSIZ * Nblocks);
		if ((fd = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   start of i-list */
		read(fd, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fd, buf, BLKSIZ * Nblocks);
			skipblks--;
		}

		if (read(fd, buf, BLKSIZ * Nblocks) != BLKSIZ * Nblocks) {
			close(fd);
			continue;
		}
		/* if first two inodes cross block boundary read next block also
		 *  - shouldn't happen as long as BLKSIZ is a multiple of 512 
		 */
		if ((offset + 2 * sizeof(struct dinode)) > BLKSIZ * Nblocks) {
		    if (read(fd, &buf[BLKSIZ * Nblocks], BLKSIZ * Nblocks) != BLKSIZ * Nblocks) {
			close(fd);
			continue;
		    }
		}
		close(fd);
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
		skipblks = address / (BLKSIZ * Nblocks);
		offset = address % (BLKSIZ * Nblocks);
		if ((fd = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   root directory */
		read(fd, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fd, buf, BLKSIZ * Nblocks);
			skipblks--;
		}

		if (read(fd, buf, BLKSIZ * Nblocks) != BLKSIZ * Nblocks) {
			close(fd);
			continue;
		}
		/* if first 2 directory entries cross block boundary read next 
		 *   block also - shouldn't happen as long as BLKSIZ is a 
		 *   multiple of 512
		 */
		if ((offset + 2 * sizeof(struct direct)) > BLKSIZ * Nblocks) {
		    if (read(fd, &buf[BLKSIZ * Nblocks], BLKSIZ * Nblocks) != BLKSIZ * Nblocks) {
			close(fd);
			continue;
		    }
		}
		close(fd);

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
