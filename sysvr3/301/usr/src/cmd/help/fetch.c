/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)help:fetch.c	1.3"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "switch.h"
#include "fetch.h"
#define EQ(x, y)	(strcmp(x, y) == 0)
#define SET_SIGNAL(x)	if(signal(x, SIG_IGN) != SIG_IGN) signal(x, sigdone)
#define	SKIP	1
#define	IODD	2
#define	OODD	4
#define	HEAD	8

/*  Global declarations  */
struct stat stbuf;

/*  Input/output copy of archive file member header  */
struct arf_hdr arfiobuf;

/*  Computable copy of archive file member header  */
struct {
	char	arf_name[16];
	long	arf_date;
	short	arf_uid;
	short	arf_gid;
	long	arf_mode;
	long	arf_size;
} arfbuf;

/*  Input/output copy of archive header  */
struct ar_hdr ariobuf;

/*  Computable copy of archive header  */
struct {
	char	ar_name[16];
	long	ar_date;
	long	ar_syms;
} arbuf;

char	*Cmd;
char	**namv;
int	namc;
char	*arnam;
char	*tfnam;
char	*tf1nam;
char	*tf2nam;
char	*file;
char	name[16];
int	af;
int	tf;
int	tf1;
int	tf2;

int	bastate;
char	buf[BUFSIZ];

/* Declare all archive functions */
void	delete();
void	extract();
void	fetch();
void	list();
void	replace();
void	init();
int	getaf();
void	noar();
void	sigdone();
void	done();
void	cleanup();
void	install();
void	movefil();
int	morefil();
int	stats();
void	copyfil();
int	getdir();
int	match();
int	bamatch();
char	*trim();
void	wrerr();
int	sputl();
long	sgetl();
char	*tempnam();
char	*ctime();
long	time();

/*  Main function  */
main(argc, argv)
char	*argv[];
int	argc;
{
	/*  Get command name  */
	Cmd = trim(argv[0]);

	/*  Set signals  */
	SET_SIGNAL(SIGHUP);
	SET_SIGNAL(SIGINT);
	SET_SIGNAL(SIGQUIT);

	/*  Check argument count  */
	if (argc < 2) {
		fprintf(stderr, "usage: %s archive files ...\n", Cmd);
		done(1);
	}

	/*  Get name of archive  */
	arnam = argv[1];
	namv = argv + 2;
	namc = argc - 2;

	/*  Check function based on command name  */
	SWITCH(Cmd)
	CASE("replace")
		replace();
	CASE("delete")
		delete();
	CASE("list")
		list();
	CASE("fetch")
		fetch();
	CASE("extract")
		extract();
	DEFAULT
		fprintf(stderr, "Bad command name:  %s\n", Cmd);
		done(1);
	ENDSW
	done(0);
}

void
replace()
{
	register f;

	init();
	getaf();
	while (!getdir()) {
		bamatch();
		if (namc == 0 || match()) {
			f = stats();
			if (f < 0) {
				if (namc)
					fprintf(stderr, "%s: cannot open %s\n", Cmd, file);
				goto cp;
			}
			copyfil(af, -1, IODD + SKIP);
			movefil(f);
			continue;
		}
cp:
		copyfil(af, tf, IODD + OODD + HEAD);
	}
	cleanup();
}

void
delete()
{
	init();
	if (getaf())
		noar();
	while (!getdir()) {
		if (match()) {
			copyfil(af, -1, IODD + SKIP);
			continue;
		}
		copyfil(af, tf, IODD + OODD + HEAD);
	}
	install();
}

void
fetch()
{
	if (getaf())
		noar();
	while (!getdir()) {
		if (namc == 0 || match()) {
			copyfil(af, 1, IODD);
			continue;
		}
		copyfil(af, -1, IODD + SKIP);
	}
}

void
list()
{
	if (getaf())
		noar();
	while (!getdir()) {
		if (namc == 0 || match()) {
			fprintf(stdout, "%s\n", trim(file));
		}
		copyfil(af, -1, IODD + SKIP);
	}
}

void
extract()
{
	register f;

	if (getaf())
		noar();
	while (!getdir()) {
		if (namc == 0 || match()) {
			f = creat(file, arfbuf.arf_mode & 0777);
			if (f < 0) {
				fprintf(stderr, "%s:  %s cannot create\n", Cmd, file);
				goto sk;
			}
			chown(file, arfbuf.arf_uid, arfbuf.arf_gid);
			copyfil(af, f, IODD);
			close(f);
			continue;
		}
sk:
		copyfil(af, -1, IODD + SKIP);
		if (namc > 0  &&  !morefil())
			return;
	}
}

void
init()
{
	tfnam = tempnam("/tmp", "ar");
	close(creat(tfnam, 0600));
	tf = open(tfnam, 2);
	if (tf < 0) {
		fprintf(stderr, "%s: cannot create temp file\n", Cmd);
		done(1);
	}
	strncpy(ariobuf.ar_magic, ARMAG, sizeof(ariobuf.ar_magic));
	strncpy(ariobuf.ar_name, trim(arnam), sizeof(ariobuf.ar_name));
	sputl(time(NULL), ariobuf.ar_date);
	sputl(arbuf.ar_syms, ariobuf.ar_syms);
	if (write(tf, &ariobuf, sizeof(ariobuf)) != sizeof(ariobuf))
		wrerr();
}

int
getaf()
{
	af = open(arnam, 0);
	if (af < 0)
		return(1);
	if (read(af, &ariobuf, sizeof(ariobuf)) != sizeof(ariobuf) || 
	    strncmp(ariobuf.ar_magic, ARMAG, SARMAG)) {
		fprintf(stderr, "%s: %s not in archive format\n", Cmd, arnam);
		done(1);
	}
	strncpy(arbuf.ar_name, ariobuf.ar_name, sizeof(arbuf.ar_name));
	arbuf.ar_date = sgetl(ariobuf.ar_date);
	arbuf.ar_syms = sgetl(ariobuf.ar_syms);
	return(0);
}

void
noar()
{
	fprintf(stderr, "%s: %s does not exist\n", Cmd, arnam);
	done(1);
}

void
sigdone()
{
	done(100);
}

void
done(c)
int	c;
{
	if (tfnam)
		unlink(tfnam);
	if (tf1nam)
		unlink(tf1nam);
	if (tf2nam)
		unlink(tf2nam);
	exit(c);
}

void
cleanup()
{
	register i, f;

	for (i = 0; i < namc; i++) {
		file = namv[i];
		if (file == 0)
			continue;
		namv[i] = 0;
		f = stats();
		if (f < 0) {
			fprintf(stderr, "%s: %s cannot open\n", Cmd, file);
			continue;
		}
		movefil(f);
	}
	install();
}

void
install()
{
	register int	i;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	close(af);
	af = creat(arnam, 0666);
	if (af < 0) {
		fprintf(stderr, "%s: cannot create %s\n", Cmd, arnam);
		done(1);
	}
	if (tfnam) {
		lseek(tf, 0l, 0);
		while ((i = read(tf, buf, BUFSIZ)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
	if (tf2nam) {
		lseek(tf2, 0l, 0);
		while ((i = read(tf2, buf, BUFSIZ)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
	if (tf1nam) {
		lseek(tf1, 0l, 0);
		while ((i = read(tf1, buf, BUFSIZ)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
}

void
movefil(f)
int	f;
{
	register char	*cp;
	register i;

	/* insert the file 'file' into the temporary file */
	cp = trim(file);
	for (i = 0; i < 16; i++)
		if (arfbuf.arf_name[i] = *cp)
			cp++;
	arfbuf.arf_size = stbuf.st_size;
	arfbuf.arf_date = stbuf.st_mtime;
	arfbuf.arf_uid = stbuf.st_uid;
	arfbuf.arf_gid = stbuf.st_gid;
	arfbuf.arf_mode = stbuf.st_mode;
	copyfil(f, tf, OODD + HEAD);
	close(f);
}

int
morefil()
{
	register i, n;

	for (n = i = 0; i < namc; i++)
		if (namv[i])
			n++;
	return(n);
}

int
stats()
{
	register f;

	f = open(file, 0);
	if (f < 0)
		return(f);
	if (fstat(f, &stbuf) < 0) {
		close(f);
		return(-1);
	}
	return(f);
}

void
copyfil(fin, fout, flag)
int	fin, fout, flag;
{
	/* copy next file size given in arfbuf */
	register i, o;
	int	pe;

	if (flag & HEAD) {
		strncpy(arfiobuf.arf_name, arfbuf.arf_name, sizeof(arfiobuf.arf_name));
		sputl(arfbuf.arf_date, arfiobuf.arf_date);
		sputl((long)arfbuf.arf_uid, arfiobuf.arf_uid);
		sputl((long)arfbuf.arf_gid, arfiobuf.arf_gid);
		sputl(arfbuf.arf_mode, arfiobuf.arf_mode);
		sputl(arfbuf.arf_size, arfiobuf.arf_size);
		if (write(fout, &arfiobuf, sizeof(arfiobuf)) != sizeof(arfiobuf))
			wrerr();
	}
	pe = 0;
	while (arfbuf.arf_size > 0) {
		i = o = BUFSIZ;
		if (arfbuf.arf_size < i) {
			i = o = arfbuf.arf_size;
			if (i & 1) {
				if (flag & IODD)
					i++;
				if (flag & OODD)
					o++;
			}
		}
		if (read(fin, buf, i) != i)
			pe++;
		if ((flag & SKIP) == 0)
			if (write(fout, buf, o) != o)
				wrerr();
		arfbuf.arf_size -= BUFSIZ;
	}
	if (pe) {
		fprintf(stderr, "%s: phase error on %s\n", Cmd, file);
		done(1);
	}
}

int
getdir()
{
	register i;

	i = read(af, &arfiobuf, sizeof(arfiobuf));
	if (i != sizeof(arfiobuf)) {
		if (tf1nam) {
			i = tf;
			tf = tf1;
			tf1 = i;
		}
		return(1);
	}
	strncpy(name, arfiobuf.arf_name, sizeof(arfiobuf.arf_name));
	file = name;
	strncpy(arfbuf.arf_name, name, sizeof(arfbuf.arf_name));
	arfbuf.arf_date = sgetl(arfiobuf.arf_date);
	arfbuf.arf_uid = sgetl(arfiobuf.arf_uid);
	arfbuf.arf_gid = sgetl(arfiobuf.arf_gid);
	arfbuf.arf_mode = sgetl(arfiobuf.arf_mode);
	arfbuf.arf_size = sgetl(arfiobuf.arf_size);
	return(0);
}

int
match()
{
	register i;

	for (i = 0; i < namc; i++) {
		if (namv[i] == 0)
			continue;
		if (strcmp(trim(namv[i]), file) == 0) {
			file = namv[i];
			namv[i] = 0;
			return(1);
		}
	}
	return(0);
}

int
bamatch()
{
	register f;

	switch (bastate) {
	case 1:
		bastate = 2;
	case 2:
		bastate = 0;
		tf1nam = tempnam("/tmp", "ar");
		close(creat(tf1nam, 0600));
		f = open(tf1nam, 2);
		if (f < 0) {
			fprintf(stderr, "%s: cannot create second temp\n", Cmd);
			done(1);
		}
		tf1 = tf;
		tf = f;
	}
}

char	*trim(s)
char	*s;
{
	register char	*p1, *p2;

	for (p1 = s; *p1; p1++)
		;
	while (p1 > s) {
		if (*--p1 != '/')
			break;
		*p1 = 0;
	}
	p2 = s;
	for (p1 = s; *p1; p1++)
		if (*p1 == '/')
			p2 = p1 + 1;
	return(p2);
}

void
wrerr()
{
	fprintf(stderr, "%s:  write error\n", Cmd);
	done(1);
}
