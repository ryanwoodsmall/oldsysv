/*	@(#)ar.c	2.1	*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ar.h>
#include <signal.h>
struct	stat	stbuf;
struct	ar_hdr	arbuf;
union {long l; short s[2]; char c[4]; } U;
struct	{
	char	p_name[14];
	unsigned short p_date1;
	unsigned short p_date2;
	char	p_uid;
	char	p_gid;
	unsigned short p_mode;
	unsigned short p_siz1;
	unsigned short p_siz2;
	} pbuf;

struct {
	char v_name[14];
	unsigned short v_pad1;
	unsigned short v_date1;
	unsigned short v_date2;
	char	v_uid;
	char	v_gid;
	unsigned short v_pad2;
	unsigned short v_mode;
	unsigned short v_pad3;
	unsigned short v_siz1;
	unsigned short v_siz2;
	} vbuf;

#define	SKIP	1
#define	IODD	2
#define	OODD	4
#define	HEAD	8
#define ARMAGIC	0177545
#define PDP11	1
#define VAX11	0

char	*man	=	{ "mrxtdpq" };
char	*opt	=	{ "cuvnbail" };

int	signum[] = {SIGHUP, SIGINT, SIGQUIT, 0};
int	sigdone();
long	lseek();
int	rcmd();
int	dcmd();
int	xcmd();
int	tcmd();
int	pcmd();
int	mcmd();
int	qcmd();
int	(*comfun)();
char	flg[26];
char	**namv;
int	namc;
char	*arnam;
char	*ponam;
char	*tmp0nam		=	{ "/tmp/vXXXXX" };
char	*tmp1nam	=	{ "/tmp/v1XXXXX" };
char	*tmp2nam	=	{ "/tmp/v2XXXXX" };
char	*tfnam;
char	*tf1nam;
char	*tf2nam;
char	*file;
char	name[16];
int	artype;
int	af;
int	tf;
int	tf1;
int	tf2;
int	qf;
int	bastate;
char	buf[512];

char	*trim();
char	*mktemp();
char	*ctime();

main(argc, argv)
char *argv[];
{
	register i;
	register char *cp;

	for(i=0; signum[i]; i++)
		if(signal(signum[i], SIG_IGN) != SIG_IGN)
			signal(signum[i], sigdone);
	if(argc < 3)
		usage();
	cp = argv[1];
	U.l = 1L;
	if(U.c[0] == 0)
		artype = PDP11;
	for(cp = argv[1]; *cp; cp++)
	switch(*cp) {
	case 'l':
	case 'v':
	case 'u':
	case 'n':
	case 'a':
	case 'b':
	case 'c':
	case 'i':
		flg[*cp - 'a']++;
		continue;

	case 'r':
		setcom(rcmd);
		continue;

	case 'd':
		setcom(dcmd);
		continue;

	case 'x':
		setcom(xcmd);
		continue;

	case 't':
		setcom(tcmd);
		continue;

	case 'p':
		setcom(pcmd);
		continue;

	case 'm':
		setcom(mcmd);
		continue;

	case 'q':
		setcom(qcmd);
		continue;

	default:
		fprintf(stderr, "ar: bad option `%c'\n", *cp);
		usage();
	}
	if(flg['l'-'a']) {
		tmp0nam = "vXXXXX";
		tmp1nam = "v1XXXXX";
		tmp2nam = "v2XXXXX";
		}
	if(flg['i'-'a'])
		flg['b'-'a']++;
	if(flg['a'-'a'] || flg['b'-'a']) {
		bastate = 1;
		ponam = trim(argv[2]);
		argv++;
		argc--;
		if(argc < 3)
			usage();
	}
	arnam = argv[2];
	namv = argv+3;
	namc = argc-3;
	if(comfun == 0) {
		if(flg['u'-'a'] == 0) {
			fprintf(stderr, "ar: one of [%s] must be specified\n", man);
			usage();
		}
		setcom(rcmd);
	}
	(*comfun)();
	done(notfound());
}

setcom(fun)
int (*fun)();
{

	if(comfun != 0) {
		fprintf(stderr, "ar: only one of [%s] allowed\n", man);
		usage();
	}
	comfun = fun;
}

rcmd()
{
	register f;

	getaf();
	putmagic();
	while(!getdir()) {
		bamatch();
		if(namc == 0 || match()) {
			f = stats();
			if(f < 0) {
				if(namc)
					fprintf(stderr, "ar: cannot open %s\n", file);
				goto cp;
			}
			if(flg['u'-'a'])
				if(stbuf.st_mtime <= arbuf.ar_date) {
					close(f);
					goto cp;
				}
			mesg('r');
			copyfil(af, -1, IODD+SKIP);
			movefil(f);
			continue;
		}
	cp:
		mesg('c');
		copyfil(af, tf, IODD+OODD+HEAD);
	}
	cleanup();
}

dcmd()
{

	if(getaf())
		noar();
	putmagic();
	while(!getdir()) {
		if(match()) {
			mesg('d');
			copyfil(af, -1, IODD+SKIP);
			continue;
		}
		mesg('c');
		copyfil(af, tf, IODD+OODD+HEAD);
	}
	install();
}

xcmd()
{
	register f;

	if(getaf())
		noar();
	while(!getdir()) {
		if(namc == 0 || match()) {
			f = creat(file, arbuf.ar_mode & 0777);
			if(f < 0) {
				fprintf(stderr, "ar: %s cannot create\n", file);
				goto sk;
			}
			mesg('x');
			copyfil(af, f, IODD);
			close(f);
			continue;
		}
	sk:
		mesg('c');
		copyfil(af, -1, IODD+SKIP);
		if (namc > 0  &&  !morefil())
			done(0);
	}
}

pcmd()
{

	if(getaf())
		noar();
	while(!getdir()) {
		if(namc == 0 || match()) {
			if(flg['v'-'a']) {
				printf("\n<%s>\n\n", file);
				fflush(stdout);
			}
			copyfil(af, 1, IODD);
			continue;
		}
		copyfil(af, -1, IODD+SKIP);
	}
}

mcmd()
{

	if(getaf())
		noar();
	putmagic();
	tf2nam = mktemp(tmp2nam);
	close(creat(tf2nam, 0600));
	tf2 = open(tf2nam, 2);
	if(tf2 < 0) {
		fprintf(stderr, "ar: cannot create third temp\n");
		done(2);
	}
	while(!getdir()) {
		bamatch();
		if(match()) {
			mesg('m');
			copyfil(af, tf2, IODD+OODD+HEAD);
			continue;
		}
		mesg('c');
		copyfil(af, tf, IODD+OODD+HEAD);
	}
	install();
}

tcmd()
{

	if(getaf())
		noar();
	while(!getdir()) {
		if(namc == 0 || match()) {
			if(flg['v'-'a'])
				longt();
			printf("%s\n", trim(file));
		}
		copyfil(af, -1, IODD+SKIP);
	}
}

qcmd()
{
	register i, f;

	if (flg['a'-'a'] || flg['b'-'a']) {
		fprintf(stderr, "ar: abi not allowed with q\n");
		usage();
	}
	getqf();
	for(i=0; signum[i]; i++)
		signal(signum[i], SIG_IGN);
	lseek(qf, 0L, 2);
	for(i=0; i<namc; i++) {
		file = namv[i];
		if(file == 0)
			continue;
		namv[i] = 0;
		mesg('q');
		f = stats();
		if(f < 0) {
			fprintf(stderr, "ar: %s cannot open\n", file);
			continue;
		}
		tf = qf;
		movefil(f);
		qf = tf;
	}
}


getmagic(fd)
{
	short mbuf;

	/* PDP-11 Magic number is 0177545
	   VAX   Magic number is 0177545 0000000
	*/
	artype = VAX11;
	if(read(fd,(char *)&mbuf,sizeof(short)) != sizeof(short) || (unsigned short) mbuf != ARMAGIC){
		fprintf(stderr, "ar: %s not in archive format\n", arnam);
		done(2);
	}
	read(fd,(char *)&mbuf,sizeof(short));
	if(mbuf != 0){
		artype = PDP11;
		lseek(fd, -2L, 1);	/* back over good data */
	}
}

putmagic(fd)
{
	short mbuf;

	tfnam = mktemp(tmp0nam);
	close(creat(tfnam, 0600));
	tf = open(tfnam, 2);
	if(tf < 0) {
		fprintf(stderr, "ar: cannot create temp file\n");
		done(2);
	}
	mbuf = ARMAGIC;
	if(write(tf,(char *)&mbuf,sizeof(short)) != sizeof(short))
		wrerr();
	mbuf = 0;
	if(artype == VAX11)
		if(write(tf,(char *)&mbuf,sizeof(short)) != sizeof(short))
			wrerr();
	return(0);
}

getaf()
{
	short mbuf;

	af = open(arnam, 0);
	if(af < 0)
		return(1);
	getmagic(af);
	return(0);
}

getqf()
{
	short mbuf;
	int sbuf;

	if ((qf = open(arnam, 2)) < 0) {
		if(!flg['c'-'a'])
			fprintf(stderr, "ar: creating %s\n", arnam);
		close(creat(arnam, 0666));
		if ((qf = open(arnam, 2)) < 0) {
			fprintf(stderr, "ar: cannot create %s\n", arnam);
			done(2);
		}
		sbuf = ARMAG;
		if (write(qf, (char *)&sbuf, sizeof(int)) != sizeof(int))
			wrerr();
		lseek(qf, 0L, 0);
		getmagic(qf);
	}
	else {
		getmagic(qf);
	}
}

usage()
{
	fprintf(stderr,"usage: ar [%s][%s] archive files ...\n", man, opt);
	done(2);
}

noar()
{

	fprintf(stderr, "ar: %s does not exist\n", arnam);
	done(2);
}

sigdone()
{
	done(100);
}

done(c)
{

	if(tfnam)
		unlink(tfnam);
	if(tf1nam)
		unlink(tf1nam);
	if(tf2nam)
		unlink(tf2nam);
	exit(c);
}

notfound()
{
	register i, n;

	n = 0;
	for(i=0; i<namc; i++)
		if(namv[i]) {
			fprintf(stderr, "ar: %s not found\n", namv[i]);
			n++;
		}
	return(n);
}

morefil()
{
	register i, n;

	n = 0;
	for(i=0; i<namc; i++)
		if(namv[i])
			n++;
	return(n);
}

cleanup()
{
	register i, f;

	for(i=0; i<namc; i++) {
		file = namv[i];
		if(file == 0)
			continue;
		namv[i] = 0;
		mesg('a');
		f = stats();
		if(f < 0) {
			fprintf(stderr, "ar: %s cannot open\n", file);
			continue;
		}
		movefil(f);
	}
	install();
}

install()
{
	register i;

	for(i=0; signum[i]; i++)
		signal(signum[i], SIG_IGN);
	if(af < 0)
		if(!flg['c'-'a'])
			fprintf(stderr, "ar: creating %s\n", arnam);
	close(af);
	af = creat(arnam, 0666);
	if(af < 0) {
		fprintf(stderr, "ar: cannot create %s\n", arnam);
		done(2);
	}
	if(tfnam) {
		lseek(tf, 0L, 0);
		while((i = read(tf, buf, 512)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
	if(tf2nam) {
		lseek(tf2, 0L, 0);
		while((i = read(tf2, buf, 512)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
	if(tf1nam) {
		lseek(tf1, 0L, 0);
		while((i = read(tf1, buf, 512)) > 0)
			if (write(af, buf, i) != i)
				wrerr();
	}
}

/*
 * insert the file 'file'
 * into the temporary file
 */
movefil(f)
{
	register char *cp;
	register i;

	cp = trim(file);
	for(i=0; i<14; i++)
		if(arbuf.ar_name[i] = *cp)
			cp++;
	arbuf.ar_size = stbuf.st_size;
	arbuf.ar_date = stbuf.st_mtime;
	arbuf.ar_uid = stbuf.st_uid;
	arbuf.ar_gid = stbuf.st_gid;
	arbuf.ar_mode = stbuf.st_mode;
	copyfil(f, tf, OODD+HEAD);
	close(f);
}

stats()
{
	register f;

	f = open(file, 0);
	if(f < 0)
		return(f);
	if(fstat(f, &stbuf) < 0) {
		close(f);
		return(-1);
	}
	return(f);
}

/*
 * copy next file
 * size given in arbuf
 */
copyfil(fi, fo, flag)
{
	register i, o;
	int pe;

	if(flag & HEAD)
		puthead(fo);
	pe = 0;
	while(arbuf.ar_size > 0) {
		i = o = 512;
		if(arbuf.ar_size < i) {
			i = o = arbuf.ar_size;
			if(i&1) {
				if(flag & IODD)
					i++;
				if(flag & OODD)
					o++;
			}
		}
		if(read(fi, buf, i) != i)
			pe++;
		if((flag & SKIP) == 0)
			if (write(fo, buf, o) != o)
				wrerr();
		arbuf.ar_size -= 512;
	}
	if(pe)
		phserr();
}

getdir()
{
	register i;

	i = gethead(af);
	if(i != sizeof arbuf) {
		if(tf1nam) {
			i = tf;
			tf = tf1;
			tf1 = i;
		}
		return(1);
	}
	for(i=0; i<14; i++)
		name[i] = arbuf.ar_name[i];
	file = name;
	return(0);
}

gethead(fd)
{
	register i;

	U.l = 1L;
	switch (artype) {

	case VAX11:
		if(U.c[0])
			/* VAX ar on VAX */
			return(read(fd,(char *)&arbuf,sizeof arbuf));
		else{
			/* VAX ar on PDP-11 */
			if((i = read(fd, (char *)&vbuf, sizeof(vbuf))) != sizeof(vbuf))
				return(i);
			for(i=0; i<14; i++)
				arbuf.ar_name[i] = vbuf.v_name[i];
			arbuf.ar_size = (long)vbuf.v_siz2<<16 | vbuf.v_siz1;
			arbuf.ar_uid = vbuf.v_uid;
			arbuf.ar_gid = vbuf.v_gid;
			arbuf.ar_mode = vbuf.v_mode;
			arbuf.ar_date = (long)vbuf.v_date2<<16 | vbuf.v_date1;
		}
		break;

	case PDP11:
		if(U.c[0]) {
			/* PDP-11 ar on VAX */
			if((i = read(fd, (char *)&pbuf, sizeof(pbuf))) != sizeof(pbuf))
				return(i);
			for(i=0; i<14; i++)
				arbuf.ar_name[i] = pbuf.p_name[i];
			arbuf.ar_size = (long)pbuf.p_siz1<<16 | pbuf.p_siz2;
			arbuf.ar_uid = pbuf.p_uid;
			arbuf.ar_gid = pbuf.p_gid;
			arbuf.ar_mode = pbuf.p_mode;
			arbuf.ar_date = pbuf.p_date1<<16 | pbuf.p_date2;
		}else

			/* PDP-11 ar on PDP-11 */
			return(read(fd, (char *)&arbuf, sizeof(arbuf)));
	}
	return(sizeof(arbuf));
}

puthead(fd)
{
	register i;

	U.l = 1L;
	switch(artype){

	case VAX11:
		if(U.c[0]){
			/*VAX ar on VAX */
			if(write(fd, (char *)&arbuf, sizeof arbuf) != sizeof arbuf)
				wrerr();
		} else {
			/* VAX ar on PDP-11 */
			for(i=0; i<14; i++)
				vbuf.v_name[i] = arbuf.ar_name[i];
			vbuf.v_siz1 = (short) (arbuf.ar_size & 0177777);
			vbuf.v_siz2 = (short) (arbuf.ar_size >> 16);
			vbuf.v_uid = arbuf.ar_uid;
			vbuf.v_gid = arbuf.ar_gid;
			vbuf.v_mode = (short) arbuf.ar_mode;
			vbuf.v_date1 = (short) (arbuf.ar_date & 0177777);
			vbuf.v_date2 = (short) (arbuf.ar_date >> 16);

			if(write(fd, (char *)&vbuf, sizeof vbuf) != sizeof vbuf)
				wrerr();
		}
		break;

	case PDP11:
		if(U.c[0]) {
			/* PDP-11 ar on VAX */
			for(i=0; i<14; i++)
				pbuf.p_name[i] = arbuf.ar_name[i];
			pbuf.p_siz2 = (short) (arbuf.ar_size & 0177777);
			pbuf.p_siz1 = (short) (arbuf.ar_size >> 16);
			pbuf.p_uid = arbuf.ar_uid;
			pbuf.p_gid = arbuf.ar_gid;
			pbuf.p_mode = (short) arbuf.ar_mode;
			pbuf.p_date2 = (short) (arbuf.ar_date & 0177777);
			pbuf.p_date1 = (short) (arbuf.ar_date >> 16);

			if(write(fd, (char*)&pbuf, sizeof pbuf) != sizeof pbuf)
				wrerr();
		} else
			/* PDP-11 ar on PDP-11 */
			if(write(fd, (char *)&arbuf, sizeof arbuf) != sizeof arbuf)
				wrerr();
	}
	return(0);
}

match()
{
	register i;

	for(i=0; i<namc; i++) {
		if(namv[i] == 0)
			continue;
		if(strcmp(trim(namv[i]), file) == 0) {
			file = namv[i];
			namv[i] = 0;
			return(1);
		}
	}
	return(0);
}

bamatch()
{
	register f;

	switch(bastate) {

	case 1:
		if(strcmp(file, ponam) != 0)
			return;
		bastate = 2;
		if(flg['a'-'a'])
			return;

	case 2:
		bastate = 0;
		tf1nam = mktemp(tmp1nam);
		close(creat(tf1nam, 0600));
		f = open(tf1nam, 2);
		if(f < 0) {
			fprintf(stderr, "ar: cannot create second temp\n");
			return;
		}
		tf1 = tf;
		tf = f;
	}
}

phserr()
{

	fprintf(stderr, "ar: phase error on %s\n", file);
}

mesg(c)
{

	if(flg['v'-'a'])
		if(c != 'c' || flg['v'-'a'] > 1)
			printf("%c - %s\n", c, file);
}

char *
trim(s)
char *s;
{
	register char *p1, *p2;

	for(p1 = s; *p1; p1++)
		;
	while(p1 > s) {
		if(*--p1 != '/')
			break;
		*p1 = 0;
	}
	p2 = s;
	for(p1 = s; *p1; p1++)
		if(*p1 == '/')
			p2 = p1+1;
	return(p2);
}

#define	IFMT	060000
#define	ISARG	01000
#define	LARGE	010000
#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000

longt()
{
	register char *cp;

	pmode();
	printf("%3d/%1d", arbuf.ar_uid, arbuf.ar_gid);
	printf("%7ld", arbuf.ar_size);
	cp = ctime(&arbuf.ar_date);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode()
{
	register int **mp;

	for (mp = &m[0]; mp < &m[9];)
		select(*mp++);
}

select(pairp)
int *pairp;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (arbuf.ar_mode&*ap++)==0)
		ap++;
	putchar(*ap);
}

wrerr()
{
	perror("ar write error");
	done(2);
}
