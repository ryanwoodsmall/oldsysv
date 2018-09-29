/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/ncpio.c	1.2.1.2"
/*
	Reworked cpio which uses getopt(3) to interpret flag arguments and
	changes reels to the save file name.
*/

/*	based on (#)cpio.c	1.7	*/
/*	cpio	COMPILE:	cc -O cpio.c -s -i -o cpio 
	cpio -- copy file collections

*/
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#ifdef	RT
#include <rt/macro.h>
#include <rt/types.h>
#include <rt/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#define EQ(x,y)	(strcmp(x,y)==0)
/* for VAX, Interdata, ... */
#define MKSHORT(v,lv) {U.l=1L;if(U.c[0]) U.l=lv,v[0]=U.s[1],v[1]=U.s[0]; else U.l=lv,v[0]=U.s[0],v[1]=U.s[1];}
#define MAGIC	070707		/* cpio magic number */
#define IN	'i'		/* copy in */
#define OUT	'o'		/* copy out */
#define PASS	'p'		/* direct copy */
#define HDRSIZE	(Hdr.h_name - (char *)&Hdr)	/* header size minus filename field */
#define LINKS	1000		/* max no. of links allowed */
#define CHARS	76		/* ASCII header size minus filename field */
#define BUFSIZE 512		/* In u370, can't use BUFSIZ nor BSIZE */
#define CPIOBSZ 4096		/* file read/write */
#ifdef RT
extern long filespace;
#endif

struct	stat	Statb, Xstatb;

	/* Cpio header format */
struct header {
	short	h_magic,
		h_dev;
	ushort	h_ino,
		h_mode,
		h_uid,
		h_gid;
	short	h_nlink,
		h_rdev,
		h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[256];
} Hdr;

unsigned	Bufsize = BUFSIZE;		/* default record size */
short	Buf[CPIOBSZ/2], *Dbuf;
char	BBuf[CPIOBSZ], *Cbuf;
int	Wct, Wc;
short	*Wp;
char	*Cp;

#ifdef RT
short	Actual_size[2];
#endif

short	Option,
	Dir,
	Uncond,
	Link,
	Rename,
	Toc,
	Verbose,
	Select,
	Mod_time,
	Acc_time,
	Cflag,
	fflag,
#ifdef RT
	Extent,
#endif
	Swap,
	byteswap,
	bothswap,
	halfswap;

int	Ifile,
	Ofile,
	Input = 0,
	Output = 1;
long	Blocks,
	Longfile,
	Longtime;

char	Fullname[256],
	Name[256];
int	Pathend;
char	*swfile;
char	*pgm_name;
char	*eommsg = "Change to part %d and press RETURN key. [q] ";

FILE	*Rtty,
	*Wtty;

char	*Pattern[100];
char	Strhdr[500];
char	*Chdr = Strhdr;
short	Dev,
	Uid,
	Gid,
	A_directory,
	A_special,
#ifdef RT
	One_extent,
	Multi_extent,
#endif
	Filetype = S_IFMT;

extern	errno;
char	*malloc();
char 	*cd();
/*	char	*Cd_name;	*/
FILE 	*popen();

union { long l; short s[2]; char c[4]; } U;

/* for VAX, Interdata, ... */
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
	register ct;
	long	filesz;
	register char *fullp;
	register i;
	int ans;
	extern char	*optarg;
	extern int	optind;

	pgm_name = *argv;
	signal(SIGSYS, 1);
	if(*argv[1] != '-')
		usage();
	Uid = getuid();
	umask(0);
	Gid = getgid();
	Pattern[0] = "*";

	while( (ans = getopt( argc, argv, "aBifopcdlmrSsbtuvVM:6eI:O:")) != EOF ) {

		switch( ans ) {
		case 'a':		/* reset access time */
			Acc_time++;
			break;
		case 'B':		/* change record size to 5120 bytes */
			Bufsize = 5120;
			break;
		case 'i':
			Option = IN;
			break;
		case 'f':	/* do not consider patterns in cmd line */
			fflag++;
			break;
		case 'o':
			Option = OUT;
			break;
		case 'p':
			Option = PASS;
			break;
		case 'c':		/* ASCII header */
			Cflag++;
			break;
		case 'd':		/* create directories when needed */
			Dir++;
			break;
		case 'l':		/* link files, when necessary */
			Link++;
			break;
		case 'm':		/* retain mod time */
			Mod_time++;
			break;
		case 'r':		/* rename files interactively */
			Rename++;
			Rtty = fopen("/dev/tty", "r");
			Wtty = fopen("/dev/tty", "w");
			if(Rtty==NULL || Wtty==NULL) {
				error("Cannot rename (/dev/tty missing)\n");
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
		case 'b':
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
#ifdef RT
		case 'e':
			Extent++;
			break;
#endif
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
		default:
			usage();
		}
	}
	if(!Option) {
		error("Options must include o|i|p\n");
	}
#ifdef RT
	setio(-1,1);	/* turn on physio */
#endif

	if(Option == PASS) {
		if(Rename) {
			error("Pass and Rename cannot be used together\n");
		}
		if(Bufsize == 5120) {
			printf("`B' option is irrelevant with the '-p' option\n");
			Bufsize = BUFSIZE;
		}

	}else  {
		if(Cflag)
		    Cp = Cbuf = (char *)malloc(Bufsize);
		else
		    Wp = Dbuf = (short *)malloc(Bufsize);
	}
	argc -= optind;
	argv += optind;
	Wct = Bufsize >> 1;
	Wc = Bufsize;

	switch(Option) {
	case OUT:
		if(argc != 0)
			usage();
		/* get filename, copy header and file out */
		while(getname()) {
			if( mklong(Hdr.h_filesize) == 0L) {
			   if( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			   else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
#ifdef RT
			if (One_extent || Multi_extent) {
			   actsize(0);
			   if( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			   else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			}
#endif
				continue;
			}
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fperr("<%s> ?\n", Hdr.h_name);
				continue;
			}
			if ( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
#ifdef RT
			if (One_extent || Multi_extent) {
			   actsize(Ifile);
			   if(Cflag)
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			   else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			   Hdr.h_filesize[0] = Actual_size[0];
			   Hdr.h_filesize[1] = Actual_size[1];
			}
#endif
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				if(read(Ifile, Cflag? BBuf: (char *)Buf, ct) < 0) {
					fperr("Cannot read %s\n", Hdr.h_name);
					continue;
				}
				Cflag? writehdr(BBuf,ct): bwrite(Buf,ct);
			}
			close(Ifile);
			if(Acc_time)
				utime(Hdr.h_name, &Statb.st_atime);
			if(Verbose)
				if( Verbose == 2 )
					verbdot( stderr );
				else
					fperr("%s\n", Hdr.h_name);
		}

	/* copy trailer, after all files have been copied */
		strcpy(Hdr.h_name, "TRAILER!!!");
		Hdr.h_magic = MAGIC;
		MKSHORT(Hdr.h_filesize, 0L);
		Hdr.h_namesize = strlen("TRAILER!!!") + 1;
		if ( Cflag )  {
			bintochar(0L);
			writehdr(Chdr,CHARS+Hdr.h_namesize);
		}
		else
			bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
		Cflag? writehdr(Cbuf, Bufsize): bwrite(Dbuf, Bufsize);
		break;

	case IN:
		if(argc > 0 ) {	/* save patterns, if any */
			for(i = 0; i < argc; ++i)
				Pattern[i] = argv[i];
		}
		pwd();
		while(gethdr()) {
			Ofile = ckname(Hdr.h_name)? openout(Hdr.h_name): 0;
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				Cflag? readhdr(BBuf,ct): bread(Buf, ct);
				if(Ofile) {
					if(Swap)
					   Cflag? swap(BBuf,ct): swap(Buf,ct);
					if(write(Ofile, Cflag? BBuf: (char *)Buf, ct) < 0) {
					 fperr("Cannot write %s\n", Hdr.h_name);
					 continue;
					}
				}
			}
			if(Ofile) {
				close(Ofile);
				if(chmod(Hdr.h_name, Hdr.h_mode) < 0) {
					fperrno("Cannot chmod <%s>",
						Hdr.h_name);
				}
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			}
			if(!Select)
				continue;
			if(Verbose)
				if(Toc)
					pentry(Hdr.h_name);
				else
					if( Verbose == 2 )
						verbdot( stdout );
					else
						puts(Hdr.h_name);
			else if(Toc)
				puts(Hdr.h_name);
		}
		break;

	case PASS:		/* move files around */
		if(argc != 1)
			usage();
		if(access(argv[0], 2) == -1) {
	    accerr:
			error("cannot write in <%s>\n", argv[0]);
		}
		strcpy(Fullname, argv[0]);	/* destination directory */
		strcat(Fullname, "/");
		stat(Fullname, &Xstatb);
		if((Xstatb.st_mode&S_IFMT) != S_IFDIR)
			goto accerr;
		Dev = Xstatb.st_dev;
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

			if(Link
			&& !A_directory
			&& Dev == Statb.st_dev)  {
/* ???			&& (Uid == Statb.st_uid || !Uid)) {*/
				if(link(Hdr.h_name, Fullname) < 0) { /* missing dir.? */
					unlink(Fullname);
					missdir(Fullname);
					if(link(Hdr.h_name, Fullname) < 0) {
						fperr("Cannot link <%s> & <%s>\n",
						 Hdr.h_name, Fullname);
						continue;
					}
				}

/* try creating (only twice) */
				ans = 0;
				do {
					if(link(Hdr.h_name, Fullname) < 0) { /* missing dir.? */
						unlink(Fullname);
						ans += 1;
					}else {
						ans = 0;
						break;
					}
				}while(ans < 2 && missdir(Fullname) == 0);
				if(ans == 1) {
					fperrno("Cannot create directory for <%s>",
						Fullname);
				}else if(ans == 2) {
					fperr("Cannot link <%s> & <%s>\n",
						Hdr.h_name, Fullname);
				}

				if(chmod(Hdr.h_name, Hdr.h_mode) < 0) {
					fperrno("Cannot chmod <%s>",
						Hdr.h_name);
				}
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
				goto ckverbose;
			}
#ifdef RT
			if (One_extent || Multi_extent)
				actsize(0);
#endif
			if(!(Ofile = openout(Fullname)))
				continue;
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fperr("<%s> ?\n", Hdr.h_name);
				close(Ofile);
				continue;
			}
			filesz = Statb.st_size;
			for(; filesz > 0; filesz -= CPIOBSZ) {
				ct = filesz>CPIOBSZ? CPIOBSZ: filesz;
				if(read(Ifile, Buf, ct) < 0) {
					fperr("Cannot read %s\n", Hdr.h_name);
					break;
				}
				if(Ofile)
					if(write(Ofile, Buf, ct) < 0) {
					 fperr("Cannot write %s\n", Hdr.h_name);
					 break;
					}
#ifndef u370
				Blocks += ((ct + (BUFSIZE - 1)) / BUFSIZE);
#else
				++Blocks;
#endif
			}
			close(Ifile);
			if(Acc_time)
				utime(Hdr.h_name, &Statb.st_atime);
			if(Ofile) {
				close(Ofile);
				if(chmod(Fullname, Hdr.h_mode) < 0) {
					fperrno("Cannot chmod <%s>", Fullname);
				}
				set_time(Fullname, Statb.st_atime, mklong(Hdr.h_mtime));
ckverbose:
				if(Verbose)
					if( Verbose == 2 )
						verbdot( stdout );
					else
						puts(Fullname);
			}
		}
	}
	/* print number of blocks actually copied */
	   fperr("%ld blocks\n", Blocks * (Bufsize>>9));
	exit(0);
}
usage()
{
	fprintf(stderr,"Usage:\t%s %s\n\t%s %s\n\t%s %s\n\t%s %s\n\t%s %s\n",
	pgm_name, "-o[acvB] <name-list >collection",
	pgm_name, "-o[acvB] -Ocollection <name-list",
	pgm_name, "-i[cdmrstuvfB6] [pattern ...] <collection",
	pgm_name, "-i[cdmrstuvfB6] -Icollection [pattern ...]",
	pgm_name, "-p[adlmruv] directory <name-list");
	exit(2);
}

chkswfile( sp, c, option )
char	*sp;
char	c;
short	option;
{
	if( !option )
		error( "-%c must be specified before -%c option\n",
			c == 'I' ? 'i' : 'o', c );
	if( (c == 'I'  &&  option != IN)  ||  (c == 'O'  &&  option != OUT) )
		error( "-%c option not permitted with -%c option\n", c,
			option );
	if( !sp )
		return;
	error( "No more than one -I or -O flag permitted.\n");
}

cannotopen( sp, mode )
char	*sp, *mode;
{
	error( "Cannot open <%s> for %s\n.", sp, mode );
}

getname()		/* get file name, get info for header */
{
	register char *namep = Name;
	register ushort ftype;
	long tlong;

	for(;;) {
		if(gets(namep) == NULL)
			return 0;
		if(*namep == '.' && namep[1] == '/')
			namep += 2;
		strcpy(Hdr.h_name, namep);
		if(stat(namep, &Statb) < 0) {
			fperr("< %s > ?\n", Hdr.h_name);
			continue;
		}
		ftype = Statb.st_mode & Filetype;
		A_directory = (ftype == S_IFDIR);
		A_special = (ftype == S_IFBLK)
			|| (ftype == S_IFCHR)
			|| (ftype == S_IFIFO);
#ifdef RT
		A_special |= (ftype == S_IFREC);
		One_extent = (ftype == S_IF1EXT);
		Multi_extent = (ftype == S_IFEXT);
#endif
		Hdr.h_magic = MAGIC;
		Hdr.h_namesize = strlen(Hdr.h_name) + 1;
		Hdr.h_uid = Statb.st_uid;
		Hdr.h_gid = Statb.st_gid;
		Hdr.h_dev = Statb.st_dev;
		Hdr.h_ino = Statb.st_ino;
		Hdr.h_mode = Statb.st_mode;
		MKSHORT(Hdr.h_mtime, Statb.st_mtime);
		Hdr.h_nlink = Statb.st_nlink;
		tlong = (Hdr.h_mode&S_IFMT) == S_IFREG? Statb.st_size: 0L;
#ifdef RT
		if (One_extent || Multi_extent) tlong = Statb.st_size;
#endif
		MKSHORT(Hdr.h_filesize, tlong);
		Hdr.h_rdev = Statb.st_rdev;
		if( Cflag )
		   bintochar(tlong);
		return 1;
	}
}

bintochar(t)		/* ASCII header write */
long t;
{
	sprintf(Chdr,"%.6o%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		MAGIC,Statb.st_dev,Statb.st_ino,Statb.st_mode,Statb.st_uid,
		Statb.st_gid,Statb.st_nlink,Statb.st_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(Hdr.h_name)+1,t,Hdr.h_name);
}

chartobin()		/* ASCII header read */
{
	sscanf(Chdr,"%6ho%6ho%6ho%6ho%6ho%6ho%6ho%6ho%11lo%6ho%11lo",
		&Hdr.h_magic,&Hdr.h_dev,&Hdr.h_ino,&Hdr.h_mode,&Hdr.h_uid,
		&Hdr.h_gid,&Hdr.h_nlink,&Hdr.h_rdev,&Longtime,&Hdr.h_namesize,
		&Longfile);
	MKSHORT(Hdr.h_filesize, Longfile);
	MKSHORT(Hdr.h_mtime, Longtime);
}

gethdr()		/* get file headers */
{
	register ushort ftype;

	if (Cflag)  {
		readhdr(Chdr,CHARS);
		chartobin();
	}
	else
		bread(&Hdr, HDRSIZE);

	if(Hdr.h_magic != MAGIC) {
		error("Out of phase--get help\n");
	}
	if(Cflag)
		readhdr(Hdr.h_name, Hdr.h_namesize);
	else
		bread(Hdr.h_name, Hdr.h_namesize);
	if(EQ(Hdr.h_name, "TRAILER!!!"))
		return 0;
	ftype = Hdr.h_mode & Filetype;
	A_directory = (ftype == S_IFDIR);
	A_special =(ftype == S_IFBLK)
		|| (ftype == S_IFCHR)
		|| (ftype == S_IFIFO);
#ifdef RT
	A_special |= (ftype == S_IFREC);
	One_extent = (ftype == S_IF1EXT);
	Multi_extent = (ftype == S_IFEXT);
	if (One_extent || Multi_extent) {
		Actual_size[0] = Hdr.h_filesize[0];
		Actual_size[1] = Hdr.h_filesize[1];
		if (Cflag)  {
			readhdr(Chdr,CHARS);
			chartobin();
		}
		else
			bread(&Hdr, HDRSIZE);
	
		if(Hdr.h_magic != MAGIC) {
			error("Out of phase--get RT help\n");
		}
		if(Cflag)
			readhdr(Hdr.h_name, Hdr.h_namesize);
		else
			bread(Hdr.h_name, Hdr.h_namesize);
	}
#endif
	return 1;
}

ckname(namep)	/* check filenames with patterns given on cmd line */
register char *namep;
{
	char	buf[sizeof Hdr.h_name];

	++Select;
	if(fflag ^ !nmatch(namep, Pattern)) {
		Select = 0;
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
	return !Toc;
}

openout(namep)	/* open files for writing, set all necessary info */
register char *namep;
{
	register f;
	register char *np;
	int ans;

	if(!strncmp(namep, "./", 2))
		namep += 2;
	np = namep;
/*
	if(Option == IN)
		Cd_name = namep = cd(namep);
*/
	if(A_directory) {
		if(!Dir
		|| Rename
		|| EQ(namep, ".")
		|| EQ(namep, ".."))	/* do not consider . or .. files */
			return 0;
		if(stat(namep, &Xstatb) == -1) {

/* try creating (only twice) */
			ans = 0;
			do {
				if(makdir(namep) != 0) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(namep) == 0);
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
		if(chmod(namep, Hdr.h_mode) < 0) {
			fperrno("Cannot chmod <%s>", namep);
		}
		if(Uid == 0)
			if(chown(namep, Hdr.h_uid, Hdr.h_gid) < 0) {
				fperrno("Cannot chown <%s>", namep);
			}
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
		/* There's a newer version of file on destination */
			if(mklong(Hdr.h_mtime) < Xstatb.st_mtime)
				fperr("current <%s> newer\n", np);
			return 0;
		}
	}
	if(Option == PASS
	&& Hdr.h_ino == Xstatb.st_ino
	&& Hdr.h_dev == Xstatb.st_dev) {
		error("Attempt to pass file to self!\n");
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
#ifdef RT
	if(One_extent || Multi_extent) {

/* try creating (only twice) */
		ans = 0;
		do {
			if((f = falloc(namep, Hdr.h_mode, longword(Hdr.h_filesize[0]))) < 0) {
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

		if(filespace < longword(Hdr.h_filesize[0])){
			fperr("Cannot create contiguous file <%s> proper size\n", namep);
			fperr("    <%s> will be created as a regular file\n", namep);
			if(unlink(Fullname) != 0)
				fperr("s> not removed\n", namep);
			Hdr.h_mode = (Hdr.h_mode & !S_IFMT) | S_IFREG;
			One_extent = Multi_extent = 0;
		}
	Hdr.h_filesize[0] = Actual_size[0];
	Hdr.h_filesize[1] = Actual_size[1];
	}
	if (!(One_extent || Multi_extent)) {
#endif

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

#ifdef RT
	}
#endif
	if(Uid == 0)
		chown(namep, Hdr.h_uid, Hdr.h_gid);
	return f;
}

bread(b, c)
register c;
register short *b;
{
	static nleft = 0;
	static short *ip;
	register int rv;
	register short *p = ip;
	register int in;

	c = (c+1)>>1;
	while(c--) {
		if(nleft == 0) {
			in = 0;
			while((rv=read(Input, &(((char *)Dbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if(rv <= 0) {
					Input = chgreel(0, Input);
					continue;
				}
				in += rv;
				nleft += (rv >> 1);
			}
			nleft += (rv >> 1);
			p = Dbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

readhdr(b, c)
register c;
register char *b;
{
	static nleft = 0;
	static char *ip;
	register int rv;
	register char *p = ip;
	register int in;

	while(c--)  {
		if(nleft == 0) {
			in = 0;
			while((rv=read(Input, &(((char *)Cbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if(rv <= 0) {
					Input = chgreel(0, Input);
					continue;
				}
				in += rv;
				nleft += rv;
			}
			nleft += rv;
			p = Cbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

bwrite(rp, c)
register short *rp;
register c;
{
	register short *wp = Wp;

	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
again:
			if(write(Output, Dbuf, Bufsize)<0) {
				Output = chgreel(1, Output);
				goto again;
			}
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

writehdr(rp, c)
register char *rp;
register c;
{
	register char *cp = Cp;

	while(c--)  {
		if(!Wc)  {
again:
			if(write(Output,Cbuf,Bufsize)<0)  {
				Output = chgreel(1,Output);
				goto again;
			}
			Wc = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Wc;
	}
	Cp = cp;
}

postml(namep, np)		/* linking function */
register char *namep, *np;
{
	register i;
	static struct ml {
		short	m_dev,
			m_ino;
		char	m_name[2];
	} *ml[LINKS];
	static	mlinks = 0;
	char *mlp;
	int ans;

	for(i = 0; i < mlinks; ++i) {
		if(mlinks == LINKS) break;
		if(ml[i]->m_ino==Hdr.h_ino &&
			ml[i]->m_dev==Hdr.h_dev) {
			if(Verbose == 1)
			  printf("%s linked to %s\n", ml[i]->m_name,
				np);
			unlink(namep);
			if(Option == IN && *ml[i]->m_name != '/') {
				Fullname[Pathend] = '\0';
				strcat(Fullname, ml[i]->m_name);
				mlp = Fullname;
			}
			mlp = ml[i]->m_name;

/* try linking (only twice) */
			ans = 0;
			do {
				if(link(mlp, namep) < 0) {
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
				fperr("Cannot link <%s> & <%s>.\n", ml[i]->m_name, np);
				return(0);
			}

			set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			return 0;
		}
	}
	if(mlinks == LINKS
	|| !(ml[mlinks] = (struct ml *)malloc(strlen(np) + 2 + sizeof(struct ml)))) {
		static int first=1;

		if(first)
			if(mlinks == LINKS)
				fperr("Too many links (%d)\n", LINKS);
			else
				fperr("No memory for links (%d)\n", mlinks);
		mlinks = LINKS;
		first = 0;
		return 1;
	}
	ml[mlinks]->m_dev = Hdr.h_dev;
	ml[mlinks]->m_ino = Hdr.h_ino;
	strcpy(ml[mlinks]->m_name, np);
	++mlinks;
	return 1;
}

pentry(namep)		/* print verbose table of contents */
register char *namep;
{

	static short lastid = -1;
#include <pwd.h>
	static struct passwd *pw;
	struct passwd *getpwuid();
	static char tbuf[32];
	char *ctime();

	printf("%-7o", Hdr.h_mode & 0177777);
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
	strcpy(tbuf, ctime((long *)&U.l));
	tbuf[24] = '\0';
	printf(" %s  %s\n", &tbuf[4], namep);
}

		/* pattern matching functions */
nmatch(s, pat)
char *s, **pat;
{
	if(EQ(*pat, "*"))
		return 1;
	while(*pat) {
		if((**pat == '!' && !gmatch(s, *pat+1))
		|| gmatch(s, *pat))
			return 1;
		++pat;
	}
	return 0;
}
gmatch(s, p)
register char *s, *p;
{
	register int c;
	register cc, ok, lc, scc;

	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		ok = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (ok)
					return(gmatch(++s, ++p));
				else
					return(0);

			case '-':
				ok |= ((lc <= scc) && (scc <= (cc=p[1])));
			}
			if (scc==(lc=cc)) ok++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(gmatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
register char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (gmatch(s++,p)) return(1);
	return(0);
}

makdir(namep)		/* make needed directories */
register char *namep;
{
	static status;
	register pid;

	if(pid = fork())
		while(wait(&status) != pid);
	else {
		close(2);
		execl("/bin/mkdir", "mkdir", namep, 0);
		exit(2);
	}
	return ((status>>8) & 0377)? 1: 0;
}

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
set_time(namep, atime, mtime)	/* set access and modification times */
register *namep;
long atime, mtime;
{
	static long timevec[2];

	if(!Mod_time)
		return;
	timevec[0] = atime;
	timevec[1] = mtime;
	utime(namep, timevec);
}
chgreel(x, fl)
{
	static int	reelcount = 1;
	register f;
	char str[100];
	struct stat statb;

	fstat(fl, &statb);
#ifndef RT
	if((statb.st_mode&S_IFMT) != S_IFCHR) {
#else
	if((statb.st_mode & (S_IFBLK|S_IFREC))==0) {
#endif
		fperrno("Can't %s: ", x? "write output": "read input");
		exit(2);
	}
	fperr( "\007Reached end of medium on %s.\n", x? "output":"input" );
	if( Rtty == NULL )
		if( (Rtty = fopen("/dev/tty", "r")) == NULL )
			error("Cannot open /dev/tty.\n");
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
		fperr("If you want to go on, type device/file name when ready\n");
		fgets(str, sizeof str, Rtty);
		str[strlen(str) - 1] = '\0';
		if(!*str)
			exit(2);
	}
	if((f = open(str, x? 1: 0)) < 0) {
		fperr("That didn't work");
		goto again;
	}
	return f;
}

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
					if((ct = makdir(namep)) != 0) {
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

pwd()		/* get working directory */
{
	FILE *dir;

	dir = popen("pwd", "r");
	fgets(Fullname, sizeof Fullname, dir);
	if(pclose(dir))
		exit(2);
	Pathend = strlen(Fullname);
	Fullname[Pathend - 1] = '/';
}
char * cd(n)		/* change directories */
register char *n;
{
	char *p_save = Name, *n_save = n, *p_end = 0;
	register char *p = Name;
	static char dotdot[]="../../../../../../../../../../../../../../../../../../../../../../../../../../../../../../../../";
	int slashes, ans;

	if(*n == '/') /* don't try to chdir on full pathnames */
		return n;
	for(; *p && *n == *p; ++p, ++n) { /* whatever part of strings == */
		if(*p == '/')
			p_save = p+1, n_save = n+1;
	}

	p = p_save;
	*p++ = '\0';
	for(slashes = 0; *p; ++p) { /* if prev is longer, chdir("..") */
		if(*p == '/')
			++slashes;
	}
	p = p_save;
	if(slashes) {
		slashes = slashes * 3 - 1;
		dotdot[slashes] = '\0';
		chdir(dotdot);
		dotdot[slashes] = '/';
	}

	n = n_save;
	for(; *n; ++n, ++p) {
		*p = *n;
		if(*n == '/')
			p_end = p+1, n_save = n+1;
	}
	*p = '\0';

	if(p_end) {
		*p_end = '\0';
		if(chdir(p_save) == -1) {
			if((ans = missdir(p_save)) == -1) {
				error("Cannot chdir to '%s', no `d' option\n",
					p_save);
			} else if (ans > 0)  {
				error("Cannot chdir to '%s', no write permission\n",
					p_save);
			} else if(chdir(p_save) == -1)  {
				error("Cannot chdir to '%s'\n", p_save);
			}
		}
	} else
		*p_save = '\0';
	return n_save;
}
#ifdef RT
actsize(file)
register int file;
{
	long tlong;
	long fsize();
	register int tfile;

	Actual_size[0] = Hdr.h_filesize[0];
	Actual_size[1] = Hdr.h_filesize[1];
	if (!Extent)
		return;
	if (file)
		tfile = file;
	else if ((tfile = open(Hdr.h_name,0)) < 0)
		return;
	tlong = fsize(tfile);
	MKSHORT(Hdr.h_filesize,tlong);
	if (Cflag)
		bintochar(tlong);
	if (!file)
		close(tfile);
}
#endif


static int	verbcount = 0;
/*
	In -V verbose mode, print out a dot for each file processed.
*/
static
verbdot( fp )
FILE	*fp;
{

	fputs( ".", fp );
	if( ++verbcount >= 50 ) {
		/* start a new line of dots */
		verbcount = 0;
		fputs( "\n", fp );
	}
}

/*
	print error message on the stderr and exit
*/
static
error( a, b, c, d, e, f, g, h, i, j, k, l, m )
{
	if( Verbose == 2  &&  verbcount ) {
		fprintf( stderr, "\n" );
		verbcount = 0;
	}
	fprintf( stderr, "%s:  ", pgm_name );
	fprintf( stderr, a, b, c, d, e, f, g, h, i, j, k, l, m );
	exit( 2 );
}

/*
	print message on the stderr
*/
static
fperr( a, b, c, d, e, f, g, h, i, j, k, l, m )
{
	if( Verbose == 2  &&  verbcount ) {
		fprintf( stderr, "\n" );
		verbcount = 0;
	}
	fprintf( stderr, a, b, c, d, e, f, g, h, i, j, k, l, m );
}

/*
	print message on the stderr followed by error number and meaning.
*/
static
fperrno( a, b, c, d, e, f, g, h, i, j, k, l, m )
{
	fperr( a, b, c, d, e, f, g, h, i, j, k, l, m );
	fprintf( stderr, ", errno %d, ", errno );
	perror("");
}
