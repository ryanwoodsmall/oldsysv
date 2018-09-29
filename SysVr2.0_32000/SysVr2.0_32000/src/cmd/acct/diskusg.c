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
/*	@(#)diskusg.c	1.3 of 7/22/83	*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#include <sys/sysmacros.h>
#include <pwd.h>
#include <fcntl.h>

#ifndef Fs2BLK
#define Fs2BLK	0
#endif

#ifndef FsINOS
#define FsINOS(dev, x)	((x&~07)+1)
#endif

#define BLOCK		512	/* Block size for reporting */

#if pdp11
#define		NINODE		(INOPB * 2 * 10)
#else
#define		NINODE		(INOPB * 2 * 32)
#endif
#define		MAXUSERS	1000
#define		MAXIGN		10
#define		UNUSED		-1
#define		FAIL		-1
#define		MAXNAME		8
#define		SUCCEED		0
#define		TRUE		1
#define		FALSE		0

struct	filsys	sblock;
struct	dinode	dinode[NINODE];
struct  passwd *getpwent();

int	VERBOSE = 0;
FILE	*ufd = 0;
int	index;
unsigned ino, nfiles;

struct acct  {
	int	uid;
	long	usage;
	char	name [MAXNAME+1];
} userlist [MAXUSERS];

char	*ignlist[MAXIGN];
int	igncnt = {0};

char	*cmd;


unsigned hash();
main(argc, argv)
int argc;
char **argv;
{
	extern	int	optind;
	extern	char	*optarg;
	register c;
	register FILE	*fd;
	register	rfd;
	struct	stat	sb;
	int	sflg = {FALSE};
	char	*pfile = {"/etc/passwd"};
	int	errfl = {FALSE};

	cmd = argv[0];
	while((c = getopt(argc, argv, "vu:p:si:")) != EOF) switch(c) {
	case 's':
		sflg = TRUE;
		break;
	case 'v':
		VERBOSE = 1;
		break;
	case 'i':
		ignore(optarg);
		break;
	case 'u':
		ufd = fopen(optarg, "a");
		break;
	case 'p':
		pfile = optarg;
		break;
	case '?':
		errfl++;
		break;
	}
	if(errfl) {
		fprintf(stderr, "Usage: %s [-sv] [-p pw_file] [-u file] [-i ignlist] [file ...]\n", cmd);
		exit(10);
	}

	hashinit();
	if(sflg == TRUE) {
		if(optind == argc){
			adduser(stdin);
		} else {
			for( ; optind < argc; optind++) {
				if( (fd = fopen(argv[optind], "r")) == NULL) {
					fprintf(stderr, "%s: Cannot open %s\n", cmd, argv[optind]);
					continue;
				}
				adduser(fd);
				fclose(fd);
			}
		}
	}
	else {
		setup(pfile);
		for( ; optind < argc; optind++) {
			if( (rfd = open(argv[optind], O_RDONLY)) < 0) {
				fprintf(stderr, "%s: Cannot open %s\n", cmd, argv[optind]);
				continue;
			}
			if(fstat(rfd, &sb) >= 0){
				if ( (sb.st_mode & S_IFMT) == S_IFCHR ||
				     (sb.st_mode & S_IFMT) == S_IFBLK ) {
					ilist(argv[optind], rfd);
				} else {
					fprintf(stderr, "%s: %s is not a special file -- ignored\n", cmd, argv[optind]);
				}
			} else {
				fprintf(stderr, "%s: Cannot stat %s\n", cmd, argv[optind]);
			}
			close(rfd);
		}
	}
	output();
	exit(0);
}

adduser(fd)
register FILE	*fd;
{
	int	usrid;
	long	blcks;
	char	login[MAXNAME+10];

	while(fscanf(fd, "%d %s %ld\n", &usrid, login, &blcks) == 3) {
		if( (index = hash(usrid)) == FAIL) return(FAIL);
		if(userlist[index].uid == UNUSED) {
			userlist[index].uid = usrid;
			strncpy(userlist[index].name, login, MAXNAME);
		}
		userlist[index].usage += blcks;
	}
}

ilist(file, fd)
char	*file;
register fd;
{
	register dev_t	dev;
	register i, j;

	if (fd < 0 ) {
		return (FAIL);
	}

	sync();

	/* Fake out block size to be 512 */
	dev = 0;

	/* Read in super-block of filesystem */
	bread(fd, 1, &sblock, sizeof(sblock), dev);

	/* Check for filesystem names to ignore */
	if(!todo(sblock.s_fname))
		return;
	/* Check for size of filesystem to be 512 or 1K */
	if (sblock.s_magic == FsMAGIC && sblock.s_type == Fs2b)
		dev = Fs2BLK;	/* 1K */
	else
		dev = 0;	/* 512 */


	nfiles = (sblock.s_isize-2) * FsINOPB(dev);

	/* Determine physical block 2 */
	i = FsINOS(dev, 2);
	i = FsITOD(dev, i);

	/* Start at physical block 2, inode list */
	for (ino = 0; ino < nfiles; i += NINODE/FsINOPB(dev)) {
		bread(fd, i, dinode, sizeof(dinode), dev);
		for (j = 0; j < NINODE && ino++ < nfiles; j++)
			if (dinode[j].di_mode & S_IFMT)
				if(count(j, dev) == FAIL) {
					if(VERBOSE)
						fprintf(stderr,"BAD UID: file system = %s, inode = %u, uid = %u\n",
					    	file, ino, dinode[j].di_uid);
					if(ufd)
						fprintf(ufd, "%s %u %u\n", file, ino, dinode[j].di_uid);
				}
	}
	return (0);
}

ignore(str)
register char	*str;
{
	char	*skip();

	for( ; *str && igncnt < MAXIGN; str = skip(str), igncnt++)
		ignlist[igncnt] = str;
	if(igncnt == MAXIGN) {
		fprintf(stderr, "%s: ignore list overflow. Recompile with larger MAXIGN\n", cmd);
	}
}
bread(fd, bno, buf, cnt, dev)
register fd;
register unsigned bno;
register struct  dinode  *buf;
register dev_t dev;
{
	lseek(fd, (long)bno*FsBSIZE(dev), 0);
	if (read(fd, buf, cnt) != cnt)
	{
		fprintf(stderr, "read error %u\n", bno);
		exit(1);
	}
}

count(j, dev)
register j;
register dev_t dev;
{
	long	blocks();

	if ( dinode[j].di_nlink == 0 || dinode[j].di_mode == 0 )
		return(SUCCEED);
	if( (index = hash(dinode[j].di_uid)) == FAIL || userlist[index].uid == UNUSED )
		return (FAIL);
	userlist[index].usage += blocks(j, dev);
	return (SUCCEED);
}


output()
{
	for (index=0; index < MAXUSERS ; index++)
		if ( userlist[index].uid != UNUSED && userlist[index].usage != 0 )
			printf("%u	%s	%ld\n",
			    userlist[index].uid,
			    userlist[index].name,
			    userlist[index].usage);
}

#define SNGLIND(dev)	(FsNINDIR(dev))
#define DBLIND(dev)	(FsNINDIR(dev)*FsNINDIR(dev))
#define	TRPLIND(dev)	(FsNINDIR(dev)*FsNINDIR(dev)*FsNINDIR(dev))

long
blocks(j, dev)
register int j;
register dev_t dev;
{
	register long blks;

	blks = (dinode[j].di_size + FsBSIZE(dev) - 1)/FsBSIZE(dev);
	if(blks > 10) {
		blks += (blks-10+SNGLIND(dev)-1)/SNGLIND(dev) +
			(blks-10-SNGLIND(dev)+DBLIND(dev)-1)/DBLIND(dev) +
			(blks-10-SNGLIND(dev)-DBLIND(dev)+TRPLIND(dev)-1)/TRPLIND(dev);
	}
	if(FsBSIZE(dev) != BLOCK) {
		blks = (blks+BLOCK/FsBSIZE(dev))*FsBSIZE(dev)/BLOCK;
	}
	return(blks);
}

unsigned
hash(j)
register unsigned j;
{
	register unsigned start;
	register unsigned circle;
	circle = start = j % MAXUSERS;
	do
	{
		if ( userlist[circle].uid == j || userlist[circle].uid == UNUSED )
			return (circle);
		circle = (circle + 1) % MAXUSERS;
	} while ( circle != start);
	return (FAIL);
}

hashinit() {
	for(index=0; index < MAXUSERS ; index++)
	{
		userlist[index].uid = UNUSED;
		userlist[index].usage = 0;
		userlist[index].name[0] = '\0';
	}
}

setup(pfile)
char	*pfile;
{
	register struct passwd	*pw;

	if( !setpwent(pfile)) {
		fprintf(stderr, "%s: Cannot open %s\n", cmd, pfile);
		exit(5);
	}
	while ( (pw=getpwent()) != NULL )
	{
		if ( (index=hash(pw->pw_uid)) == FAIL )
		{
			fprintf(stderr,"diskacct: INCREASE SIZE OF MAXUSERS\n");
			return (FAIL);
		}
		if ( userlist[index].uid == UNUSED )
		{
			userlist[index].uid = pw->pw_uid;
			strncpy( userlist[index].name, pw->pw_name, MAXNAME);
		}
	}
}

todo(fname)
register char	*fname;
{
	register	i;

	for(i = 0; i < igncnt; i++) {
		if(strncmp(fname, ignlist[i], 6) == 0) return(FALSE);
	}
	return(TRUE);
}

char	*
skip(str)
register char	*str;
{
	while(*str) {
		if(*str == ' ' ||
		    *str == ',') {
			*str = '\0';
			str++;
			break;
		}
		str++;
	}
	return(str);
}

extern long atol();
extern char *fgets();

static FILE *pwf = NULL;
static char line[BUFSIZ+1];
static struct passwd passwd;

setpwent(pfile)
register char *pfile;
{
	if(pwf == NULL)
		pwf = fopen(pfile, "r");
	else
		rewind(pwf);
	return(pwf != NULL);
}

void
endpwent()
{
	if(pwf != NULL) {
		(void) fclose(pwf);
		pwf = NULL;
	}
}

static char *
pwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if(*p == '\n')
		*p = '\0';
	else if(*p)
		*p++ = '\0';
	return(p);
}

struct passwd *
getpwent()
{
	register char *p;
	long	x;

	if(pwf == NULL) {
		return(0);
	}
	p = fgets(line, BUFSIZ, pwf);
	if(p == NULL)
		return(0);
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	x = atol(p);	
	passwd.pw_uid = (x < 0 || x > MAXUID)? (MAXUID+1): x;
	p = pwskip(p);
	x = atol(p);
	passwd.pw_gid = (x < 0 || x > MAXUID)? (MAXUID+1): x;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	(void) pwskip(p);

	p = passwd.pw_passwd;
	while(*p && *p != ',')
		p++;
	if(*p)
		*p++ = '\0';
	passwd.pw_age = p;
	return(&passwd);
}
