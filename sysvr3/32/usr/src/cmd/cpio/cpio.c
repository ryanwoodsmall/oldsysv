/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cpio:cpio.c	1.30.3.23"
/*	/sccs/src/cmd/s.cpio.c
	cpio.c	1.30.3.23	6/9/87 17:33:41
	Reworked cpio which uses getopt(3) to interpret flag arguments and
	changes reels to the save file name.
	Performance and size improvements.
*/

/*	cpio	COMPILE:	cc -O cpio.c -s -i -o cpio -lgen
	cpio -- copy file collections

*/
#include "errmsg.h"
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <varargs.h>
#include <sys/stat.h>

#define TRUE 1
#define EQ(x,y)	(strcmp(x,y)==0)

				/* MKSHORT:  for VAX, Interdata, ...	*/
				/* Take a 4-byte long, lv, and turn it	*/
				/* into an array of two 2-byte shorts, v*/
#define MKSHORT(v,lv) {U.l=1L;if(U.c[0]) U.l=lv,v[0]=U.s[1],v[1]=U.s[0]; else U.l=lv,v[0]=U.s[0],v[1]=U.s[1];}

#define BINARY	0		/* Cflag setting */
#define ASCII	1		/* Cflag setting */
#define NONE	2		/* Cflag setting not verified */
#define M_ASCII "070707"	/* ASCII  magic number */
#define M_BINARY 070707		/* Binary magic number */
#define M_STRLEN 6		/* number bytes in ASCII magic number */
#define PATHSIZE 256		/* maximum PATH length */
#define IN	'i'		/* copy in */
#define OUT	'o'		/* copy out */
#define PASS	'p'		/* direct copy */
#define HDRSIZE	(Hdr.h_name - (char *)&Hdr)	/* header size minus filename field */
#define LINKS	500		/* no. of links allocated per bunch */
#define CHARS	76		/* ASCII header size minus filename field */
#define BUFSIZE 512		/* In u370, can't use BUFSIZ or BSIZE */
#define CPIOBSZ 4096		/* file read/write */
#define MK_USHORT(a)	(a & 00000177777)	/* Make unsigned shorts for portable  */
						/* header.  Hardware may only know    */
						/* integer operations and sign extend */
						/* the large unsigned short resulting */
						/* in 8 rather than 6 octal char in   */
						/* the header.			      */

#define	FORMAT	"%b %e %H:%M:%S %Y"     /* Date time formats */
					/* b - abbreviated month name */
					/* e - day of month ( 1 - 31) */
					/* H - hour (00 -23) */
					/* M - minute (00 - 59) */
					/* S - seconds (00 -59) */
					/* Y - year as ccyy */

static struct	stat	Statb, Xstatb, Astatb;

	/* Cpio header format */
static struct header {
	short	h_magic;
	short	h_dev;
	ushort	h_ino;
	ushort	h_mode,
		h_uid,
		h_gid;
	short	h_nlink;
	short	h_rdev;
	short	h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[256];
} Hdr;

static unsigned	Bufsize = BUFSIZE;		/* default record size */
static char	*Buf, *Cbuf;
static char	*Cp;


static
short	Option,
	Dir,
	Uncond,
	PassLink,
	Rename,
	Toc,
	Verbose,
	Mod_time,
	Acc_time,
	Cflag,
	fflag,
	kflag,
	Swap,
	byteswap,
	bothswap,
	halfswap;

static
int	Ifile,
	Ofile,
	orig_umask,	/* umask inherited from cpio's parent process */
	df_mode = 0777,	/* default file/directory protection modes */
	Input = 0,
	Output = 1;
			/* sBlocks: short Blocks.  Cumulative character   */
			/* count for short reads in bread().  Encountered */
			/* with communication lines and pipes as in:      */
			/* split -100 cpio_archive; cat xa* | cpio -icd   */
static
long	sBlocks,
	Blocks,
	Longfile,
	Longtime;

static
char	Fullname[256],
	Name[256];
static
int	Pathend;
static
char	*swfile;
static
char	*eommsg = "Change to part %d and press RETURN key. [q] ";

static
FILE	*Rtty,
	*Wtty;
static
char	ttyname[] = "/dev/tty";

static
char	**Pattern = 0;
static
char	Chdr[500];
static
short	Dev;
ushort	Uid,
	A_directory,
	A_special,
	Filetype = S_IFMT;

extern	errno;
extern	void exit();
extern	ushort getuid();
extern	void perror();
extern	char *sys_errlist[];
extern	long lseek();
extern	long ulimit();
char	*malloc();
FILE 	*popen();

static
union {
	long l;
	short s[2];
	char c[4];
} U;

static	char	time_buf[50]; 		/* array to hold date and time */

/* for VAX, Interdata, ... */
static
long mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0])
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return U.l;
}

main(argc, argv)
char **argv;
{
	void pwd();
	register ct;
	register char *fullp;
	register i;
	long	filesz, maxsz;
	int ans,  rc, first;
	short select;			/* set when files are selected */
	extern char	*optarg;
	extern int	optind;

	errsource( *argv );
	errverb("notag,notofix");
	errexit( 2 );
	signal(SIGSYS, 1);
	if(*argv[1] != '-')
		usage();
	Uid = getuid();
	orig_umask = umask(0);

	while( (ans = getopt( argc, argv, "aBC:ifopcdklmrSsbtuvVM:6eI:O:")) != EOF ) {

		switch( ans ) {
		case 'a':		/* reset access time */
			Acc_time++;
			break;
		case 'B':		/* change record size to 5120 bytes */
			Bufsize = 5120;
			break;
		case 'C':		/* reset buffer size to arbitrary value */
			Bufsize = atoi( optarg );
			if( Bufsize == 0 )
				errmsg( EERROR,
					"Illegal argument to -%c, '%s'.",
					ans, optarg );
			break;
		case 'i':
			Option = IN;
			break;
		case 'f':	/* copy files not matched by patterns */
			fflag++;
			break;
		case 'o':
			Option = OUT;
			break;
		case 'p':
			Option = PASS;
			break;
		case 'c':		/* ASCII header */
			Cflag = ASCII;
			break;
		case 'd':		/* create directories when needed */
			Dir++;
			break;
		case 'l':		/* link files, when necessary */
			PassLink++;
			break;
		case 'm':		/* retain mod time */
			Mod_time++;
			break;
		case 'r':		/* rename files interactively */
			Rename++;
			Rtty = fopen(ttyname, "r");
			Wtty = fopen(ttyname, "w");
			if(Rtty==NULL || Wtty==NULL) {
				errmsg( EERROR, "Cannot rename (%s missing)",
					ttyname );
			}
			break;
		case 'S':		/* swap halfwords */
			halfswap++;
			Swap++;
			break;
		case 's':		/* swap bytes */
			byteswap++;
			Swap++;
			break;
		case 'b':		/* swap both bytes and halfwords */
			bothswap++;
			Swap++;
			break;
		case 't':		/* table of contents */
			Toc++;
			break;
		case 'u':		/* copy unconditionally */
			Uncond++;
			break;
		case 'v':		/* verbose - print out file names */
			Verbose = 1;
			break;
		case 'V':		/* print a dot '.' for each file */
			Verbose = 2;
			break;
		case 'M':		/* alternate message for end-of-media */
			eommsg = optarg;
			break;
		case '6':		/* for old, sixth-edition files */
			Filetype = 060000;
			break;
		case 'I':
			chkswfile( swfile, ans, Option );
			close( Input );
			if( open( optarg, O_RDONLY ) != Input )
				cannotopen( optarg, "input" );
			swfile = optarg;
			break;
		case 'O':
			chkswfile( swfile, ans, Option );
			close( Output );
			if( open( optarg, O_WRONLY | O_CREAT | O_TRUNC, 0666 )
				!= Output)
				cannotopen( optarg, "output" );
			swfile = optarg;
			break;
		case 'k':
			kflag++;
			break;
		default:
			usage();
		}
	}
	switch(Option) {
		case IN:
			Cp = Cbuf = (char *)zmalloc(EERROR, Bufsize + CHARS + PATHSIZE) + CHARS + PATHSIZE;
			break;
		case OUT:
			Cp = Cbuf = (char *)zmalloc( EERROR, Bufsize);
			(void)fstat(Output, &Astatb);
			if (kflag)
				fperr("`k' option is irrelevant with the `-o' option\n");
			break;
		case PASS:
			if(Rename)
				errmsg( EERROR, "Pass and Rename cannot be used together.");
			if(kflag)
				fperr("`k' option is irrelevant with the `-p' option\n");
			if( Bufsize != BUFSIZE ) {
				fperr("`B' or `C' option is irrelevant with the '-p' option\n");
				Bufsize = BUFSIZE;
			}
			break;
		default:
			errmsg( EERROR, "Options must include one: -o, -i, -p.");
	}

	Buf = (char *)zmalloc(EERROR, CPIOBSZ);

	argc -= optind;
	argv += optind;

	switch(Option) {
	case OUT:
		if(argc != 0)
			usage();
		/* get filename, copy header and file out */
		while(getname()) {
			if(!A_special && ident(&Statb, &Astatb))
				continue;
			if( mklong(Hdr.h_filesize) == 0L) {
				if( Cflag )
					bwrite(Chdr,CHARS+Hdr.h_namesize);
				else
					bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
				if(Verbose)
					verbdot( stderr, Hdr.h_name);
				continue;
			}
			if((Ifile = open(Hdr.h_name, O_RDONLY)) < 0) {
				fperr("<%s> ?\n", Hdr.h_name);
				continue;
			}
			if ( Cflag )
				bwrite(Chdr,CHARS+Hdr.h_namesize);
			else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				errno = 0;
				if(read(Ifile, Buf, (unsigned)ct) < 0) {
					fperr("Cannot read %s\n", Hdr.h_name);
					continue;
				}
				bwrite(Buf,ct);
			}
			close(Ifile);
			if(Acc_time)
				utime(Hdr.h_name, &Statb.st_atime);
			if(Verbose)
				verbdot( stderr, Hdr.h_name);
		}

	/* copy trailer, after all files have been copied */
		strcpy(Hdr.h_name, "TRAILER!!!");
		Hdr.h_magic = M_BINARY;
		MKSHORT(Hdr.h_filesize, 0L);
		Hdr.h_namesize = strlen("TRAILER!!!") + 1;
		if ( Cflag )  {
			bintochar(0L);
			bwrite(Chdr, CHARS+Hdr.h_namesize);
		}
		else
			bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
		bwrite(Cbuf, Bufsize);
		break;

	case IN:
		if(argc > 0 ) {	/* save patterns, if any */
			Pattern = argv;
		}
		Cflag = NONE;
		pwd();
		maxsz = 512 * ulimit(1);
		while(gethdr()) {
			if( (select = ckname(Hdr.h_name))  &&  !Toc )
				Ofile = openout(Hdr.h_name);
			else
				Ofile = 0;
			filesz=mklong(Hdr.h_filesize);
			if(select && maxsz < filesz) {
				select = Ofile = 0;
				fperr("%s skipped: exceeds ulimit by %d bytes\n",
					Hdr.h_name, filesz - maxsz);
			}
			first = 1;
			if (Cflag == BINARY && (filesz % 2) == 1)
				i = 1;
			else
				i = 0;
			for(; filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				errno = 0;
				if(bread(Buf, ct, filesz + i) == -1) {
					fperr("cpio: i/o error, %s is corrupt\n",
						Hdr.h_name);
					break;
				}
						/* skip to next file */
				if(Ofile) {
					if(Swap)
						swap(Buf,ct);
					errno = 0;
					if((rc = write(Ofile, Buf, ct)) < ct) {
						if(rc < 0) {
							if(first++ == 1) {
					 			fperrno(
								"Cannot write %s\n",
								Hdr.h_name);
							}
							if (errno == EFBIG || errno == ENOSPC)
								continue;
							else
								exit(2);
						}
						else {
					 		fperr("%s truncated\n",
								Hdr.h_name);
							continue;
						}
					}
				}
			}
			if( Ofile ) {
				zclose( EERROR, Ofile );
				zchmod( EWARN, Hdr.h_name, Hdr.h_mode);
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			}
			if(select) {
				if(Verbose)
					if(Toc)
						pentry(Hdr.h_name);
					else
						verbdot( stdout, Hdr.h_name);
				else if(Toc)
					puts(Hdr.h_name);
			}
		}
		break;

	case PASS:		/* move files around */
		if(argc != 1)
			usage();
		if(access(argv[0], 2) == -1) {
			errmsg( EERROR, "cannot write in <%s>", argv[0]);
		}
		strcpy(Fullname, argv[0]);	/* destination directory */
		zstat( EERROR, Fullname, &Xstatb );
		if((Xstatb.st_mode&S_IFMT) != S_IFDIR)
			errmsg( EERROR, "<%s> not a directory.", Fullname );
		Dev = Xstatb.st_dev;
		if( Fullname[ strlen(Fullname) - 1 ] != '/' )
			strcat(Fullname, "/");
		fullp = Fullname + strlen(Fullname);

		while(getname()) {
			if (A_directory && !Dir)
				fperr("Use `-d' option to copy <%s>\n",
					Hdr.h_name);
			if(!ckname(Hdr.h_name))
				continue;
			i = 0;
			while(Hdr.h_name[i] == '/')
				i++;
			strcpy(fullp, &(Hdr.h_name[i]));

			if( PassLink  &&  !A_directory  &&  Dev == Statb.st_dev ) {
				if(link(Hdr.h_name, Fullname) < 0) {
					switch(errno) {
						case ENOENT:
							if(missdir(Fullname) != 0) {
								fperr(
									"cpio: cannot create directory for <%s>: %s\n",
									Fullname, sys_errlist[errno]);
								continue;
							}
							break;
						case EEXIST:
							if(unlink(Fullname) < 0) {
								fperr(
									"cpio: cannot unlink <%s>: %s\n",
									Fullname, sys_errlist[errno]);
								continue;
							}
							break;
						default:
							fperr(
								"Cpio: cannot link <%s> to <%s>: %s\n",
								Hdr.h_name, Fullname, sys_errlist[errno]);
							continue;
						}
					if(link(Hdr.h_name, Fullname) < 0) {
						fperr(
							"cpio: cannot link <%s> to <%s>: %s\n",
							Hdr.h_name, Fullname, sys_errlist[errno]);
						continue;
					}
				}

				goto ckverbose;
			}
			if(!(Ofile = openout(Fullname)))
				continue;
			if((Ifile = zopen( EWARN, Hdr.h_name, 0)) < 0) {
				close(Ofile);
				continue;
			}
			filesz = Statb.st_size;
			for(; filesz > 0; filesz -= CPIOBSZ) {
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				errno = 0;
				if(read(Ifile, Buf, (unsigned)ct) < 0) {
					fperr("Cannot read %s\n", Hdr.h_name);
					break;
				}
				if (Ofile) {
					errno = 0;
					if (write(Ofile, Buf, ct) < 0) {
						fperr("Cannot write %s\n", Hdr.h_name);
						break;
					}
				}
					/* Removed u370 ifdef which caused cpio */
					/* to report blocks in terms of 4096 bytes. */

				Blocks += ((ct + (BUFSIZE - 1)) / BUFSIZE);
			}
			close(Ifile);
			if(Acc_time)
				utime(Hdr.h_name, &Statb.st_atime);
			if(Ofile) {
				close(Ofile);
				zchmod( EWARN, Fullname, Hdr.h_mode);
				set_time(Fullname, Statb.st_atime, mklong(Hdr.h_mtime));
ckverbose:
				if(Verbose)
					verbdot( stdout, Fullname );
			}
		}
	}
	/* print number of blocks actually copied */
	Blocks += ((sBlocks + (BUFSIZE - 1)) / BUFSIZE);
	fperr("%ld blocks\n", Blocks * (Bufsize>>9));
	exit(0);
}

static
usage()
{
	errusage("%s\n\t%s %s\n\t%s %s\n\t%s %s\n\t%s %s\n",
		    "-o[acvVB] [-Cbufsize] [-Mmessage] <name-list >collection",
	Err.source, "-o[acvVB] -Ocollection [-Cbufsize] [-Mmessage] <name-list",
	Err.source, "-i[bcdkmrsStuvVfB6] [-Cbufsize] [-Mmessage] [pattern ...] <collection",
	Err.source, "-i[bcdkmrsStuvVfB6] -Icollection [-Cbufsize] [-Mmessage] [pattern ...]",
	Err.source, "-p[adlmruvV] directory <name-list");
}

static
chkswfile( sp, c, option )
char	*sp;
char	c;
short	option;
{
	if( !option )
		errmsg( EERROR, "-%c must be specified before -%c option.",
			c == 'I' ? 'i' : 'o', c );
	if( (c == 'I'  &&  option != IN)  ||  (c == 'O'  &&  option != OUT) )
		errmsg( EERROR, "-%c option not permitted with -%c option.", c,
			option );
	if( !sp )
		return;
	errmsg( EERROR, "No more than one -I or -O flag permitted.");
}

static
cannotopen( sp, mode )
char	*sp, *mode;
{
	errmsg( EERROR, "Cannot open <%s> for %s.", sp, mode );
}

static
getname()		/* get file name, get info for header */
{
	register char *namep = Name;
	register ushort ftype;
	long tlong;

	for(;;) {
		if(gets(namep) == NULL)
			return 0;
		while(*namep == '.' && namep[1] == '/') {
			namep++;
			while(*namep == '/') namep++;
		}
		strcpy(Hdr.h_name, namep);
		if(zstat( EWARN, namep, &Statb) < 0) {
			continue;
		}
		ftype = Statb.st_mode & Filetype;
		A_directory = (ftype == S_IFDIR);
		A_special = (ftype == S_IFBLK)
			|| (ftype == S_IFCHR)
			|| (ftype == S_IFIFO);
		Hdr.h_magic = M_BINARY;
		Hdr.h_namesize = strlen(Hdr.h_name) + 1;
		Hdr.h_uid = Statb.st_uid;
		Hdr.h_gid = Statb.st_gid;
		Hdr.h_dev = Statb.st_dev;
		Hdr.h_ino = Statb.st_ino;
		Hdr.h_mode = Statb.st_mode;
		MKSHORT(Hdr.h_mtime, Statb.st_mtime);
		Hdr.h_nlink = Statb.st_nlink;
		tlong = (Hdr.h_mode&S_IFMT) == S_IFREG? Statb.st_size: 0L;
		MKSHORT(Hdr.h_filesize, tlong);
		Hdr.h_rdev = Statb.st_rdev;
		if( Cflag )
			bintochar(tlong);
		return 1;
	}
}

static
bintochar(t)		/* ASCII header write */
long t;
{
	sprintf(Chdr,"%.6o%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		M_BINARY, MK_USHORT(Statb.st_dev), MK_USHORT(Statb.st_ino), Statb.st_mode, Statb.st_uid,
		Statb.st_gid, Statb.st_nlink, MK_USHORT(Statb.st_rdev),
		Statb.st_mtime, (short)strlen(Hdr.h_name)+1, t, Hdr.h_name);
}

static
chartobin()		/* ASCII header read */
{
	sscanf(Chdr, "%6ho%6ho%6ho%6ho%6ho%6ho%6ho%6ho%11lo%6ho%11lo",
		&Hdr.h_magic, &Hdr.h_dev, &Hdr.h_ino, &Hdr.h_mode, &Hdr.h_uid,
		&Hdr.h_gid, &Hdr.h_nlink, &Hdr.h_rdev, &Longtime,
		&Hdr.h_namesize, &Longfile);
	MKSHORT(Hdr.h_filesize, Longfile);
	MKSHORT(Hdr.h_mtime, Longtime);
}


static
gethdr()		/* get file headers */
{
	void synch();
	register ushort ftype;

	synch();
	if(EQ(Hdr.h_name, "TRAILER!!!"))
		return 0;
	ftype = Hdr.h_mode & Filetype;
	A_directory = (ftype == S_IFDIR);
	A_special = (ftype == S_IFBLK)
		||  (ftype == S_IFCHR)
		||  (ftype == S_IFIFO);
	return 1;
}

static
ckname(namep)	/* check filenames with patterns given on cmd line */
register char *namep;
{
	char	buf[sizeof Hdr.h_name];

	if(fflag ^ !nmatch(namep, Pattern)) {
		return 0;
	}
	if(Rename && !A_directory) {	/* rename interactively */
		fprintf(Wtty, "Rename <%s>\n", namep);
		fflush(Wtty);
		fgets(buf, sizeof buf, Rtty);
		if(feof(Rtty))
			exit(2);
		buf[strlen(buf) - 1] = '\0';
		if(EQ(buf, "")) {
			strcpy(namep,buf);
			printf("Skipped\n");
			return 0;
		}
		else if(EQ(buf, "."))
			printf("Same name\n");
		else
			strcpy(namep,buf);
	}
	return  1;
}

static
openout(namep)	/* open files for writing, set all necessary info */
register char *namep;
{
	register f;
	register char *np;
	int ans;

	if(!strncmp(namep, "./", 2))
		namep += 2;
	np = namep;
	if(A_directory) {
		if( !Dir  ||  Rename  ||  EQ(namep, ".")  ||  EQ(namep, "..") )
			/* do not consider . or .. files */
			return 0;
		if(stat(namep, &Xstatb) == -1) {

/* try creating (only twice) */
			(void) umask(orig_umask);
			ans = 0;
			do {
				if (!mkdir(namep, df_mode)) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(namep) == 0);
			(void) umask(0);
			if(ans == 1) {
				fperrno("Cannot create directory for <%s>",
					namep);
				return(0);
			}else if(ans == 2) {
				fperrno("Cannot create directory <%s>", namep);
				return(0);
			}
		}

ret:
		zchmod( EWARN, namep, Hdr.h_mode);
		if(Uid == 0)
			zchown( EWARN, namep, Hdr.h_uid, Hdr.h_gid);
		set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
		return 0;
	}
	if(Hdr.h_nlink > 1)
		if(!postml(namep, np))
			return 0;
	if(stat(namep, &Xstatb) == 0) {
		if(Uncond && !((!(Xstatb.st_mode & S_IWRITE) || A_special) && (Uid != 0))) {
			if(unlink(namep) < 0) {
				fperrno("cannot unlink current <%s>", namep);
			}
		}
		if(!Uncond && (mklong(Hdr.h_mtime) <= Xstatb.st_mtime)) {
		/* There's a newer or same aged version of file on destination */
			fperr("current <%s> newer or same age\n", np);
			return 0;
		}
	}
	if( Option == PASS
		&& Hdr.h_ino == Xstatb.st_ino
		&& Hdr.h_dev == Xstatb.st_dev) {
		errmsg( EERROR, "Attempt to pass file to self!");
	}
	if(A_special) {
		if((Hdr.h_mode & Filetype) == S_IFIFO)
			Hdr.h_rdev = 0;

/* try creating (only twice) */
		ans = 0;
		do {
			if(mknod(namep, Hdr.h_mode, Hdr.h_rdev) < 0) {
				ans += 1;
			}else {
				ans = 0;
				break;
			}
		}while(ans < 2 && missdir(np) == 0);
		if(ans == 1) {
			fperrno("Cannot create directory for <%s>", namep);
			return(0);
		}else if(ans == 2) {
			fperrno("Cannot mknod <%s>", namep);
			return(0);
		}

		goto ret;
	}

/* try creating (only twice) */
	ans = 0;
	do {
		if((f = creat(namep, Hdr.h_mode)) < 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	}while(ans < 2 && missdir(np) == 0);
	if(ans == 1) {
		fperrno("Cannot create directory for <%s>", namep);
		return(0);
	}else if(ans == 2) {
		fperrno("Cannot create <%s>", namep);
		return(0);
	}

	if(Uid == 0)
		zchown( EWARN, namep, Hdr.h_uid, Hdr.h_gid);
	return f;
}


/*	Shared by bread(), synch() and rstbuf()
*/
static int	nleft = 0;	/* unread chars left in Cbuf and expansion buffer*/
static char	*ip;		/* pointer to next char to be read from Cbuf and */
				/* expansion buffer*/
static int	filbuf = 0;	/* flag to bread() to fill buffer but transfer	*/
				/* no characters				*/

	/* bread() is called by rstbuf() in order to read the next block	*/
	/* from the input archive into Cbuf.  bread() is called by main(),	*/
	/* case: IN, in order to read in file content to then be written out.	*/
	/* In case of I/O error bread() exits with return code 2 unless kflag	*/
	/* is set.  If kflag is set then bread attempts a max of 10 times to	*/
	/* successfully lseek then read good data.  If 10 consecutive reads	*/
	/* fail bread() exits with return code 2.  Upon a successful read()	*/
	/* bread() leaves nleft and the pointers ip and p set correctly but no	*/
	/* characters are copied and bread() returns -1.  In the case of no I/O	*/
	/* errors bread() reads from the input archive to Cbuf and then copies	*/
	/* chars to a target buffer unless filbuf is set by rstbuf().		*/

	/* d, distance to lseek if I/O error encountered with -k option.*/
	/* Converted to a multiple of Bufsize				*/
static
bread(b, c, d)
register char	*b;
register int	c;
long d;
{
	register char	*p = ip;
	register int	dcr;
	register int	rv;
	int rc = 0, delta = 0, i = 0;

	if (filbuf == 1) {	/* fill buffer, memcpy no chars */
		nleft = 0;
		c = 0;
	}
	if( !Cflag ) {
		/* round c up to an even number */
		c = (c+1)/2;
		c *= 2;
	}
	while ( c || filbuf )  {
		while (nleft == 0 ) {
			while (TRUE) {
				errno = 0;
				rv = read(Input, Cbuf, Bufsize);
				if ((rv == 0) || ((rv == -1) && (errno == ENOSPC || errno == ENXIO)))
					Input = chgreel(0, Input, rv);
				else
					break;
			}
			if( rv == -1 ) {
				int s, rvl;
				if( kflag ) {
					if (i++ > 10) {
						fperr("cpio: cannot recover from I/O error, %s\n",
							sys_errlist[errno]);
						exit(2);
					}
					rvl = lseek(Input,
						(s = d / Bufsize) == 0 ? Bufsize : s * Bufsize, 1); 
					if(i == 1) {
						if (rvl != -1 && (d % Bufsize)
						   && d > Bufsize)
							delta = (d % Bufsize);
						d = 0;
						rc = -1;
					}
					else
						delta = 0;
				}
				else
					errmsg( EERROR,
						"Read() in bread() failed\n");
			}
			else {
				if (rv == Bufsize)
					++Blocks;
				else 				/* short read */
					sBlocks += rv;
				if (rv > delta) {
					nleft = rv - delta;
					p = Cbuf + delta;
				}
				else {
					nleft = 0;
					delta -= rv;
				}
			}
		}
		if (filbuf || i > 0) {
			filbuf = 0;
			break;
		}
		if (nleft <= c)
			dcr = nleft;
		else
			dcr = c;
		memcpy( b, p, dcr );
		c -= dcr;
		d -= dcr;
		b += dcr;
		p += dcr;
		nleft -= dcr;
	}
	ip = p;
	return(rc);
}
		/* synch() searches for headers.  Any Cflag specification by the */
		/* user is ignored.  Cflag is set appropriately after a good    */
		/* header is found.  It searches for and verifies all headers.  */
		/* Unless kflag is set only one failure of the header causes an */
		/* exit(2).  I/O errors during examination of any part of the   */
		/* header causes synch() to throw away current data and begin	*/
		/* again.  Other errors during examination of any part of the   */
		/* header causes synch() to advance a single byte and continue   */
		/* the examination.						*/
static
void
synch()
{
	register char	*magic;
	register int	hit = NONE, cnt = 0;
	int hsize, offset, align = 0;
	static int min = CHARS;
	union {
		char bite[2];
		ushort temp;
	} mag;

	if (Cflag == NONE) {
		filbuf = 1;
		if (bread(Chdr, 0, 0) == -1)
			fperr("cpio: I/O error, searching to next header\n");
	}
	magic = ip;
	do {
		while (nleft < min) {
			if (rstbuf(magic) == -1) ;
			magic = ip;
		}
		if (Cflag == ASCII || Cflag == NONE) {
			if (strncmp(magic, M_ASCII, M_STRLEN) == 0) {
				memcpy(Chdr, magic, CHARS);
				chartobin();
				hit = ASCII;
				hsize = CHARS + Hdr.h_namesize;
			}
		}
		if (Cflag == BINARY || Cflag == NONE) {
			mag.bite[0] = magic[0];
			mag.bite[1] = magic[1];
			if (mag.temp == M_BINARY) {
				memcpy(&Hdr, magic, HDRSIZE);
				hit = BINARY;
				hsize = HDRSIZE + Hdr.h_namesize;
				align =	((Hdr.h_namesize % 2) == 0 ? 0 : 1);
			}
		}
		if (hit == NONE) {
			magic++;
			nleft--;
		}
		else {
			if (hdck() == -1) {
				magic++;
				nleft--;
				hit = NONE;
				if (kflag)
				    fperr(
				    "cpio: header corrupted.  File(s) may be lost\n");
			}
			else {			/* consider possible alignment byte */
				while (nleft < hsize + align) {
					if (rstbuf(magic) == -1) {
						magic = ip;
						hit = NONE;
						break;
					}
					else 
						magic = ip;
				}
				if (hit == NONE)
					continue;
				if (*(magic + hsize - 1) != '\0') {
					magic++;
					nleft--;
					hit = NONE;
					continue;
				}
			}
		}
		if (cnt++ == 2)
			fperr(
			"cpio: out of sync.  searching for magic number/header\n");
	}
	while (hit == NONE && kflag);

	if (cnt > 2)
		fperr("cpio: re-synchronized on magic number/header\n");
	if (hit == NONE) {
		if (Cflag == NONE)
			errmsg( EERROR,
				"This is not a cpio file.  Bad header.");
		else
			errmsg( EERROR,
				"Out of sync.  bad magic number/header.");
	}	
	if (hit == ASCII) {
		memcpy(Hdr.h_name, magic + CHARS, Hdr.h_namesize);
		Cflag = ASCII;
	}
	else {
		memcpy(Hdr.h_name, magic + HDRSIZE, Hdr.h_namesize + align);
		Cflag = BINARY;
		min = HDRSIZE;
	}
	offset = min + Hdr.h_namesize + align;
	ip = magic + offset;
	nleft -= offset;
}

		/* rstbuf(), reset bread() buffer,  moves incomplete potential	*/
		/* headers from Cbuf to an expansion buffer to the left of Cbuf.*/
		/* It then forces bread() to replenish Cbuf.  Rstbuf() returns	*/
		/* the value returned by bread() to warn synch() of I/O errors.	*/
		/* nleft and ip are updated to reflect the new data available.	*/
static
int
rstbuf(ptr)
char *ptr;
{
	int rc = 0, eleft;			/* eleft: amt in expansion buffer */

	smemcpy(Cbuf - nleft, ptr, nleft);	/* mv leftover bytes to expansion */
	eleft = nleft;				/*			   buffer */
	filbuf = 1;				/* force fill of Cbuf		  */

	if ((rc = bread(Chdr, 0, 0)) == -1)
		;				/* ip & nleft are ok */
	else {
		nleft += eleft;
		ip -= eleft;
	}
	return(rc);
}

			
	/* hdck() sanity checks the fixed length portion of the cpio header	*/
	/* -1 indicates a bad header and 0 indicates a good header		*/
static
int
hdck()
{
	if (Hdr.h_nlink < 1 ||
		mklong(Hdr.h_filesize) < 0 ||
		Hdr.h_namesize <= 0 ||
		Hdr.h_namesize >= PATHSIZE)
		return(-1);
	else
		return(0);
}

static
bwrite(rp, c)
register char *rp;
register c;
{
	register char	*cp = Cp;
	static unsigned	Ccnt = 0;
	register unsigned Cleft;
	register int	rv;

	if( !Cflag ) {
		/* round c up to an even number */
		c = (c+1)/2;
		c *= 2;
	}
	while( c )  {
		if( (Cleft = Bufsize - Ccnt) <= c ) {
			memcpy( cp, rp, Cleft );
			errno = 0;
			rv = write(Output, Cbuf, Bufsize);
			if( rv <= 0 )
				rv = eomchgreel(rv);
			if( rv == Bufsize )
				{
				Ccnt = 0;
				cp = Cbuf;
				}
			else if( rv < Bufsize )
				{
				Output = chgreel(1, Output, 0);
				smemcpy( Cbuf, &Cbuf[ Bufsize - rv ], rv );
				Ccnt = Bufsize - rv;
				cp = &Cbuf[ rv ];
				}
			else
				errmsg( EHALT,
					"Impossible return from write(), %d\n",
					rv );
			++Blocks;
			rp += Cleft;
			c -= Cleft;
		}
		else {
			memcpy( cp, rp, c );
			Ccnt += c;
			cp += c;
			rp += c;
			c = 0;
		}
	}
	Cp = cp;
}


static int	reelcount = 1;	/* used below and in chgreel() */

/*	Change reel due to reaching end-of-media.
	Keep trying to get a successful write before considering the
	change-of-reel as successful.
*/
static
int
eomchgreel(rv)
int rv;
{
	while( 1 ) {
		Output = chgreel(1, Output, rv);
		errno = 0;
		rv = write(Output, Cbuf, Bufsize);
		if( rv == Bufsize )
			return  rv;
		fperr( "Unable to write this medium.  Try again.\n" );
		reelcount--;
	}
	/*NOTREACHED*/
}


static
postml(namep, np)		/* linking function:  Postml() is called after*/
register char *namep, *np;	/* namep is created.  Postml() checks to see  */
{				/* if namep should be linked to np.  If so,   */
				/* postml() removes the independent instance  */
	register i;		/* of namep and links namep to np.	      */
	static struct ml {
		short	m_dev;
		ushort	m_ino;
		char	m_name[2];
	} **ml = 0;
	register struct ml	*mlp;
	static unsigned	mlsize = 0;
	static unsigned	mlinks = 0;
	char		*lnamep;
	int		ans;

	if( !ml ) {
		mlsize = LINKS;
		ml = (struct ml **) zmalloc( EERROR, mlsize * sizeof(struct ml));
	}
	else if( mlinks == mlsize ) {
		mlsize += LINKS;
		ml = (struct ml **) zrealloc( EERROR, ml, mlsize * sizeof(struct ml));
	}
	for(i = 0; i < mlinks; ++i) {
		mlp = ml[i];
		if(mlp->m_ino==Hdr.h_ino  &&  mlp->m_dev==Hdr.h_dev) {
			if(Verbose == 1)
				fperr("%s linked to %s\n", mlp->m_name, np);
			if(Verbose && Option == PASS)
				verbdot(stdout, np);
			unlink(namep);
			if(Option == IN && *(mlp->m_name) != '/') {
				Fullname[Pathend] = '\0';
				strcat(Fullname, mlp->m_name);
				lnamep = Fullname;
			}
			lnamep = mlp->m_name;

/* try linking (only twice) */
			ans = 0;
			do {
				if(link(lnamep, namep) < 0) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(np) == 0);
			if(ans == 1) {
				fperrno("Cannot create directory for <%s>", np);
				return(0);
			}else if(ans == 2) {
				fperrno("Cannot link <%s> & <%s>", lnamep, np);
				return(0);
			}

			set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			return 0;
		}
	}
	if( !(ml[mlinks] = (struct ml *)zmalloc( EWARN, strlen(np) + 2 + sizeof(struct ml)))) {
		static int first=1;

		if(first)
			fperr("No memory for links (%d)\n", mlinks);
		first = 0;
		return 1;
	}
	ml[mlinks]->m_dev = Hdr.h_dev;
	ml[mlinks]->m_ino = Hdr.h_ino;
	strcpy(ml[mlinks]->m_name, np);
	++mlinks;
	return 1;
}

static
pentry(namep)		/* print verbose table of contents */
register char *namep;
{

	static short lastid = -1;
#include <pwd.h>
	static struct passwd *pw;
	struct passwd *getpwuid();

	printf("%-7o", MK_USHORT(Hdr.h_mode));
	if(lastid == Hdr.h_uid)
		printf("%-6s", pw->pw_name);
	else {
		setpwent();
		if(pw = getpwuid((int)Hdr.h_uid)) {
			printf("%-6s", pw->pw_name);
			lastid = Hdr.h_uid;
		} else {
			printf("%-6d", Hdr.h_uid);
			lastid = -1;
		}
	}
	printf("%7ld ", mklong(Hdr.h_filesize));
	U.l = mklong(Hdr.h_mtime);
	cftime(time_buf, FORMAT, (long *)&U.l);
	printf(" %s  %s\n", time_buf, namep);
}


		/* pattern matching functions */
static
nmatch(s, pat)
char *s, **pat;
{
	if( !pat )
		return 1;
	while(*pat) {
		if((**pat == '!' && !gmatch(s, *pat+1))
		|| gmatch(s, *pat))
			return 1;
		++pat;
	}
	return 0;
}



static
swap(buf, ct)		/* swap halfwords, bytes or both */
register ct;
register char *buf;
{
	register char c;
	register union swp { long	longw; short	shortv[2]; char charv[4]; } *pbuf;
	int savect, n, i;
	char *savebuf;
	short cc;

	savect = ct;	savebuf = buf;
	if(byteswap || bothswap) {
		if (ct % 2) buf[ct] = 0;
		ct = (ct + 1) / 2;
		while (ct--) {
			c = *buf;
			*buf = *(buf + 1);
			*(buf + 1) = c;
			buf += 2;
		}
		if (bothswap) {
			ct = savect;
			pbuf = (union swp *)savebuf;
			if (n = ct % sizeof(union swp)) {
				if(n % 2)
					for(i = ct + 1; i <= ct + (sizeof(union swp) - n); i++) pbuf->charv[i] = 0;
				else
					for (i = ct; i < ct + (sizeof(union swp) - n); i++) pbuf->charv[i] = 0;
			}
			ct = (ct + (sizeof(union swp) -1)) / sizeof(union swp);
			while(ct--) {
				cc = pbuf->shortv[0];
				pbuf->shortv[0] = pbuf->shortv[1];
				pbuf->shortv[1] = cc;
				++pbuf;
			}
		}
	}
	else if (halfswap) {
		pbuf = (union swp *)buf;
		if (n = ct % sizeof(union swp))
			for (i = ct; i < ct + (sizeof(union swp) - n); i++) pbuf->charv[i] = 0;
		ct = (ct + (sizeof(union swp) -1)) / sizeof(union swp);
		while (ct--) {
			cc = pbuf->shortv[0];
			pbuf->shortv[0] = pbuf->shortv[1];
			pbuf->shortv[1] = cc;
			++pbuf;
		}
	}
}


static
set_time(namep, atime, mtime)	/* set access and modification times */
register char *namep;
time_t atime, mtime;
{
	static time_t timevec[2];

	if(!Mod_time)
		return;
	timevec[0] = atime;
	timevec[1] = mtime;
	utime(namep, timevec);
}



static
chgreel(x, fl, rv)
{
	register f;
	char str[BUFSIZ];
	struct stat statb;
	int tmperrno;

	tmperrno = errno;
	fstat(fl, &statb);
	if ((statb.st_mode&S_IFMT) != S_IFCHR)
		{
		if (x && rv == -1)
			switch (tmperrno)
				{
				case EFBIG:	fperr("cpio: ulimit reached for output file\n");
						break;
				case ENOSPC:	fperr("cpio: no space left for output file\n");
						break;
				default:	errmsg( EERROR,
							"write() in bwrite() failed\n");
				}
		else
			fperr( "Can't read input:  end of file encountered \
prior to expected end of archive.\n");
		exit(2);
		}
	if ((rv == 0) || ((rv == -1) && (tmperrno == ENOSPC  ||  tmperrno == ENXIO)))
		fperr( "\007Reached end of medium on %s.\n", x? "output":"input" );
	else
		{
		fperrno( "\007Encountered an error on %s", x? "output":"input" );
		exit(2);
		}
	if( Rtty == NULL )
		Rtty = zfopen( EERROR, ttyname, "r");
	close(fl);
	reelcount++;
again:
	if( swfile ) {
	    askagain:
		fperr( eommsg, reelcount );
		fgets(str, sizeof str, Rtty);
		switch( *str ) {
		case '\n':
			strcpy( str, swfile );
			break;
		case 'q':
			exit(2);
		default:
			goto askagain;
		}
	}
	else {
		fperr("If you want to go on, type device/file name when ready.\n");
		fgets(str, sizeof str, Rtty);
		str[strlen(str) - 1] = '\0';
		if(!*str)
			exit(2);
	}
	if((f = open(str, x? 1: 0)) < 0) {
		fperr("That didn't work, cannot open \"%s\"\n", str);
		if( errno )
			perror("");
		goto again;
	}
	return f;
}



static
missdir(namep)
register char *namep;
{
	register char *np;
	register ct = 2;

	for(np = namep; *np; ++np)
		if(*np == '/') {
			if(np == namep) continue;	/* skip over 'root slash' */
			*np = '\0';
			if(stat(namep, &Xstatb) == -1) {
				if(Dir) {
					(void) umask(orig_umask);
					ct = mkdir(namep, df_mode);
					(void) umask(0);
					if (ct != 0) {
						*np = '/';
						return(ct);
					}
				}else {
					fperr("missing 'd' option\n");
					return(-1);
				}
			}
			*np = '/';
		}
	if (ct == 2) ct = 0;		/* the file already exists */
	return ct;
}



static
void
pwd()		/* get working directory */
{
	FILE *dir;

	if((dir = popen("pwd", "r")) == NULL) {
		fperr("cpio: popen() failed, cannot determine working directory\n");
		exit(2);
	}
	fgets(Fullname, sizeof Fullname, dir);
	if(pclose(dir))
		exit(2);
	Pathend = strlen(Fullname);
	Fullname[Pathend - 1] = '/';
}


static int	verbcount = 0;
static FILE	*verbout = 0;
/*
	In -V verbose mode, print out a dot for each file processed.
*/
static
verbdot( fp, cp )
FILE	*fp;
char	*cp;
{

	if( !verbout )
		verbout = fp;
	if( Verbose == 2 ) {
		fputc( '.', fp );
		if( ++verbcount >= 50 ) {
			/* start a new line of dots */
			verbcount = 0;
			fputc( '\n', fp );
		}
	}
	else {
		fputs( cp, fp );
		fputc( '\n', fp );
	}
}


/*
	print message on the stderr
*/
static
fperr( va_alist )
va_dcl
{
	va_list	args;
	char	*fmt;

	resetverbcount();
	va_start( args );
	fmt = va_arg( args, char * );
	vfprintf( stderr, fmt, args );
	fflush( stderr );
}

/*
	print message on the stderr followed by error number and meaning.
*/
static
fperrno( va_alist )
va_dcl
{
	va_list	args;
	char	*fmt;

	resetverbcount();
	va_start( args );
	fmt = va_arg( args, char * );
	vfprintf( stderr, fmt, args );
	fprintf( stderr, ", errno %d, ", errno );
	fflush( stderr );
	perror("");
}


static
resetverbcount()
{
	if( Verbose == 2  &&  verbcount ) {
		fputc( '\n', verbout );
		verbcount = 0;
	}
}
			/* ident() accepts pointers to two stat structures and	  */
			/* determines if they correspond to identical files.	  */
			/* ident() assumes that if the device and inode are the	  */
			/* same then files are identical.  ident()'s purpose is to*/
			/* prevent putting the archive name in the output archive.*/
static
int
ident(in, ot)
struct stat *in, *ot;
{
	if (in->st_ino == ot->st_ino &&
	    in->st_dev == ot->st_dev)
		return(1);
	else
		return(0);
}
