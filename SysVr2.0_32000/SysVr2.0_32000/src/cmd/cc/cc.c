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
/*	@(#)cc.c	1.5	*/
/*
 * cc command -- calls the appropriate passes of the
 *		c compiler, assembler, and loader.
 */

char	release[] = "@(#) C rel 6.0";

# include <stdio.h>
# include <ctype.h>
# include <signal.h>

#ifndef CSW
#include "*****ERROR: CSW undefined*****"
#endif

#define	DMR	0
#define	SCJ1	1
#define	SCJ2	2
#define	JFR	3
#define	SCPTR	sizeof(char *)

#define	Xc0	0
#define	Xc1	1
#define	Xc2	2
#define	Xcp	3
#define	Xas	4
#define	Xld	5
#define	NPASS	6

#if CSW == DMR
#define	NAMEc0 "c0"
#define	NAMEc1 "c1"
#endif
#if CSW == SCJ1
#define	NAMEc0 "comp"
#define	NAMEc1 0
#endif
#if CSW == SCJ2
#define	NAMEc0 "front"
#define	NAMEc1 "back"
#endif
#if CSW == JFR
#define	NAMEc0 "ccom"
#define	NAMEc1 0
#endif
#define	NAMEc2 "c2"
#define	NAMEcp "cpp"
#define	NAMEas "as"
#define	NAMEld "ld"

#define	TEMPDIR	""
char	*tmp1 = NULL;
char	*tmp2 = NULL;
char	*tmp3 = NULL;
char	*tmp4 = NULL;
char	*tmp5 = NULL;
char	*outfile;
char	**clist;
char	**list[NPASS];
int	nlist[NPASS];
char	**av;
int	pflag;
int	sflag;
int	cflag;
int	eflag;
int	gflag;
int	exflag;
int	oflag;
int	proflag;
int	noflflag;
char	fchar;
char	bflag;
char	debug = 0;

#ifndef CCNAME
#define	CCNAME	"cc"
#endif
char	*chpass ;
char	*npassname ;
char	rpassname[] = "/lib//";
char	alpassname[] = "/bin//";
char	minusl[4] = "-l\000";	/*template for "-lXc", "-lc", etc.*/
char	*nameps[NPASS] = {NAMEc0, NAMEc1, NAMEc2, NAMEcp, NAMEas, NAMEld};
char	*passes[NPASS];
char	altpass[NPASS];
char	*pref = "crt0.o";
extern	int optind;
extern	int opterr;
extern	char *optarg;
extern	int optopt;
#if vax || ns32000
#define OPTSTR	"So:OpcfEPD:I:U:Ct:B:#gZ:W:0:1:2:A:L:d:l:V"
#else
#define OPTSTR	"So:OpcfEPD:I:U:Ct:B:#gZ:W:0:1:2:A:L:d:l:V:"
#endif

char	*copy(), *setsuf(), *stralloc();
char	*strcat(), *strncat(), *strtok();
char	*strncpy(), *strcpy(), *strrchr();
char	*mktemp(), *malloc(), *tempnam();

main(argc, argv)
char *argv[]; 
{
	char *t;
	char *assource;
	int nc, nargs, nxo;
	char *f20;
	int i, j, c, na, ic;
	int idexit();

	opterr = 0;
	i = nc = nxo = 0;
	f20 = (char *)NULL;
	nargs = argc + 1;
	j = SCPTR * nargs - 1;		/*stralloc allocates extra byte.*/
	for (i=NPASS; i-- > 0; ) {
		nlist[i] = 0;
		list[i] = (char **) stralloc(j);
	}
	clist = (char **)stralloc(j);
	av = (char **)stralloc(j + 5 * SCPTR);
	setbuf(stdout, (char *)NULL);
	if((t = strrchr(argv[0], '/')) == NULL)
		fchar = *argv[0];
	else
		fchar = t[1];
	if(fchar == CCNAME[0])
		fchar = 0;
	while (optind<argc)
	  switch (c = getopt(argc, argv, OPTSTR)) {

	case 'S':
		sflag++;
		cflag++;
		continue;

	case 'o':
		outfile = optarg;
		if ((c=getsuf(outfile))=='c'||c=='o')
			error("'-o %s': Illegal name for output of ld", outfile);
		continue;

	case 'O':
		oflag++;
		continue;

	case 'p':
		proflag++;
		continue;

	case 'c':
		cflag++;
		continue;

	case 'f':
#if pdp11
		noflflag++;
#else
		fprintf(stderr, "'-f' option ignored on this processor.\n");
#endif
		continue;

	case 'E':
		exflag++;
	case 'P':
		pflag++;
		cflag++;
	case 'D':
	case 'I':
	case 'U':
	case 'C':
		if (nlist[Xcp] >= nargs) {
			error("Too many EPDIUC options", (char *)NULL);
			continue;
		}
		list[Xcp][nlist[Xcp]] = stralloc(strlen(optarg)+2);
		list[Xcp][nlist[Xcp]][0] = '-';
		list[Xcp][nlist[Xcp]][1] = c;
		list[Xcp][nlist[Xcp]][2] = '\0';
		strcat(list[Xcp][nlist[Xcp]], optarg);
		nlist[Xcp]++;
		continue;

	case 't':
		if (chpass)
			error("-t overwrites earlier option", (char *)NULL);
		chpass = optarg;
		if (chpass[0]==0)
			chpass = "012p";
		continue;

	case 'B':
		if (npassname)
			error("-B overwrites earlier option", (char *)NULL);
		npassname = optarg;
		if (npassname[0]==0){
			npassname = "/lib/o";
			bflag = 0;
		}else
			bflag = 1;
		continue;

	case '#':
		debug = 1;
		continue;

	case 'g':
#if CSW == DMR
		fprintf(stderr, "'-g' option not supported on the pdp11 cc compiler\n");
#else
		gflag++;
#endif
		continue;

	case 'Z':
		if(optarg[1] != 0){
			error("'-Z' needs one-letter subargument: -Z%s", optarg);
			continue;
		}
		(pref = "crt2.o")[3] = optarg[0];
		(f20 = "-l2")[2] = optarg[0];;
		continue;

	case 'W':
		if((optarg[1] != ',')
			|| ((t = strtok(optarg, ",")) != optarg)){
			error("Invalid subargument: -W%s", optarg);
			continue;
		}
		if((i = getXpass((ic = *t), "-W")) == -1)
			continue;
		while((t = strtok(NULL, ",")) != NULL) {
			if(nlist[i] >= nargs){
				error("Too many arguments for pass -W%c", ic);
				continue;
			}
			list[i][nlist[i]++] = t;
		}
		continue;

	case '0':
	case '1':
	case '2':
	case 'A':
	case 'L':
	case 'd':
		error("The cc option '-%c%s' is no longer supported.",c,optarg);
		error("Please use '-Wc,arg'", (char *)NULL);
		continue;

	case 'l':
	case 'V':
		t = stralloc(strlen(optarg)+2);
		t[0] = '-';
		t[1] = c;
		t[2] = '\0';
		strcat(t, optarg);
		goto checknl;
	case '?':
		t = stralloc(2);
		t[0] = '-';
		t[1] = optopt;
		t[2] = '\0';
checknl:
		if(nlist[Xld] >= nargs){
			free(t);
			error("Too many ld options", (char *)NULL);
			continue;
		}
		list[Xld][nlist[Xld]++] = t;
		continue;

	case EOF:
		t = argv[optind];
		optind++;
		if((c=getsuf(t))=='c' || c=='s'|| exflag) {
			clist[nc++] = t;
			t = setsuf(t, 'o');
		}
		if (nodup(list[Xld], t)) {
			if(nlist[Xld] >= nargs){
				error("Too many ld arguments", (char *)NULL);
				continue;
			}
			list[Xld][nlist[Xld]++] = t;
			if (getsuf(t)=='o')
				nxo++;
		}
	}
	if (gflag)
		oflag = 0;
	if(fchar) {
		(list[Xcp][nlist[Xcp]] = "-I/usr/Xinclude")[7] = fchar;
		nlist[Xcp]++;
	}
	if (npassname && chpass ==0)
		chpass = "012p";
	if (chpass && npassname==0)
		npassname = "/lib/n";
	if (chpass)
		for (t=chpass; *t; t++) {
			if((i = getXpass(*t, "-t")) == -1)
				continue;
			mkpname (i, npassname);
			altpass[i] = !bflag;
			continue;
		}
	rpassname[sizeof(rpassname)-2] = fchar;
	alpassname[sizeof(alpassname)-2] = fchar;
	for (i = 0; i < NPASS; i++)
		if(passes[i] == 0){
			mkpname (i, i < Xas ? rpassname : alpassname);
			altpass[i] = 1;
		}
	if (noflflag)
		pref = proflag ? "fmcrt0.o" : "fcrt0.o";
	else if (proflag)
		pref = "mcrt0.o";
	if(nc==0)
		goto nocom;
	if (pflag==0) {
		tmp1 = tempnam(TEMPDIR, "ctm1");
		tmp2 = tempnam(TEMPDIR, "ctm2");
		tmp3 = tempnam(TEMPDIR, "ctm3");
		tmp4 = tempnam(TEMPDIR, "ctm4");
		if (oflag)
			tmp5 = tempnam(TEMPDIR, "ctm5");
		if (!(tmp1 && tmp2 && tmp3 && tmp4 && (tmp5 || !oflag)) ||
			creat(tmp1, 0666) < 0)
			error("cc: cannot create temporaries: %s", tmp1);
	}
	if(eflag)
		dexit();
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	for (i=0; i<nc; i++) {
		if (nc>1)
			printf("%s:\n", clist[i]);
		if (getsuf(clist[i])=='s'&&!pflag) {
			assource = clist[i];
			goto assemble;
		} 
		else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		av[0] = nameps[Xcp];
		av[1] = clist[i];
		av[2] = exflag ? "-" : tmp4;
		na = 3;
		for(j=0; j < nlist[Xcp]; j++)
			av[na++] = list[Xcp][j];
		av[na]=0;
		if (callsys(passes[Xcp], av, &altpass[Xcp])) {
			cflag++; 
			eflag++;
			continue;
		}
		if (pflag)
			continue;
		na = 0;
		av[na++] = nameps[Xc0];
		for (j=0; j < nlist[Xc0]; j++)
			av[na++] = list[Xc0][j];
		av[na++] = tmp4;
		if (nameps[Xc1] == 0){
			if (sflag)
				assource = tmp3 = setsuf(clist[i], 's');
			av[na++] = tmp3;
			if (oflag)
				av[na-1] = tmp5;
		}else
			av[na++] = tmp1;
#if CSW == DMR
		av[na++] = tmp2;
#endif
		if (proflag) {
#if CSW == DMR
			av[na++] = "-P";
#else
			av[na++] = "-XP";
#endif
		} 
		if (gflag)
			av[na++] = "-Xg";
#if CSW == SCJ1
		if(sflag)
			av[na++] = "-l";
#endif
		av[na] = 0;
		if (callsys(passes[Xc0], av, &altpass[Xc0])) {
			cflag++;
			eflag++;
			continue;
		}
		if(nameps[Xc1] != (char *)0){
			na = 0;
			av[na++] = nameps[Xc1];
			for (j=0; j < nlist[Xc1]; j++)
				av[na++] = list[Xc1][j];
			av[na++] = tmp1;
#if CSW == DMR
			av[na++] = tmp2;
#endif
			if (sflag)
				assource = tmp3 = setsuf(clist[i], 's');
			av[na++] = tmp3;
			if (oflag)
				av[na-1] = tmp5;
#if CSW == SCJ2
			if(sflag)
				av[na++] = "-l";
#endif
			av[na] = 0;
			if(callsys(passes[Xc1], av, &altpass[Xc1])) {
				cflag++;
				eflag++;
				continue;
			}
		}
		if (oflag) {
			na = 0;
			av[na++] = nameps[Xc2];
			for (j=0; j < nlist[Xc2]; j++)
				av[na++] = list[Xc2][j];
			av[na++] = tmp5;
			av[na++] = tmp3;
			av[na] = 0;
			if (callsys(passes[Xc2], av, &altpass[Xc2])) {
				unlink(tmp3);
				assource = tmp5;
			} 
			else
				unlink(tmp5);
		}
		if (sflag)
			continue;
assemble:
		na = 0;
		av[na++] = nameps[Xas];
		for (j=0; j < nlist[Xas]; j++)
			av[na++] = list[Xas][j];
#if pdp11
		av[na++] = "-u";
#endif
		av[na++] = "-o";
		av[na++] = setsuf(clist[i], 'o');
		av[na++] = assource;
		av[na] = 0;
		cunlink(tmp1);
		cunlink(tmp2);
		cunlink(tmp4);
		if (callsys(passes[Xas], av, &altpass[Xas]) > 0) {
		  /*this test used to be "> 1" for old pdp11 as*/
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nlist[Xld]!=0) {
		na = i = 0;
		av[na++] = nameps[Xld];
		j = sizeof("/lib//") - 1;
		av[na++] = t = stralloc(j + strlen(pref));
		strcpy(t, "/lib//");
		strcat(t, pref);
		if(fchar){
			t[j-1] = fchar;
			if(access(t, 04) != 0)
				t[j-1] = '/';
		}
		if (outfile) {
			av[na++] = "-o";
			av[na++] = outfile;
		}
#if CSW == JFR
		if ( proflag )
			av[na++] = "-L/lib/libp";
#endif
		while(i<nlist[Xld])
			av[na++] = list[Xld][i++];
		if(fchar)
			minusl[2] = fchar;	/*set up for "-lnc", etc.*/
		if (gflag)
			av[na++] = strcat( strcpy( "-lXg", minusl), "g");
		if(f20 != NULL)
			av[na++] = f20;
		else
			av[na++] = strcat( strcpy( "-lXc", minusl), "c");
		av[na] = 0;
		eflag |= callsys(passes[Xld], av, &altpass[Xld]);
		if (nc==1 && nxo==1 && eflag==0)
			cunlink(setsuf(clist[0], 'o'));
	}
	dexit();
}

idexit()
{
	eflag += 100;
	dexit();
}

dexit()
{
	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
	}
	exit(eflag);
}

/*VARARGS*/
error(s, a, b)
char *s, *a, *b;
{
	fprintf(stderr, s, a, b);
	putc('\n', stderr);
	cflag++;
	eflag++;
}


getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while(t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c<=14 && c>2 && *s++=='.')
		return(*s);
	return(0);
}

char *
setsuf(as, ch)
char *as;
{
	register char *s, *s1;

	s = s1 = copy(as);
	while(*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return(s1);
}

callsys(f, v, alt)
char f[], *v[]; 
char *alt;
{
	int t, status;
	char **p;

	if (debug) {
		printf("callsys %s:", f);
		for (p=v; *p != NULL; p++)
			printf(" '%s'", *p);
		printf(" .\n");
		return(0);
	}
	if ((t=fork())==0) {
		execv(f, v);
		if(*alt != 0){
			alt = stralloc(4 + strlen(f));
			strcpy(alt, "/usr");
			strcat(alt, f);
			execv(alt, v);
		}
		error("Can't find %s, %s", f, alt);
		exit(300);
	} else
		if (t == -1) {
			error("Can't fork; Try again");
			return(1);
		}
	while(t!=wait(&status))
		;
	if (t = status&0377) {
		if (t!=SIGINT)
			error("Fatal error in %s", f);
		eflag += 200;
		dexit();
	}
	return((status>>8) & 0377);
}

char *
copy(s)
register char *s;
{
	register char *ns;

	ns = stralloc(strlen(s));
	return(strcpy(ns, s));
}

char *
stralloc(n)
int	n;
{
	char *malloc();
	register char *s;

	s = malloc(n+1);
	if (s==NULL) {
		error("out of space", (char *)NULL);
		dexit();
	}
	return(s);
}


nodup(l, os)
char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != 'o')
		return(1);
	while(t = *l++) {
		while(c = *s++)
			if (c != *t++)
				break;
		if (*t=='\0' && c=='\0')
			return(0);
		s = os;
	}
	return(1);
}

cunlink(f)
char *f;
{
	if (f==NULL)
		return;
	unlink(f);
}


mkpname(n, pname)
int	n;
char	*pname;
{
	if (nameps[n] == 0)
		return;
	passes[n] = stralloc(strlen(pname) + strlen(nameps[n]));
	strcpy(passes[n], pname);
	strcat(passes[n], nameps[n]);
}


getXpass(c, opt)
char	c, *opt;
{
	switch (c) {
	default:
		error("Unrecognized pass name: '%s%c'", opt, c);
		return(-1);
	case '0':
	case '1':
	case '2':
		return(c - '0');
	case 'p':
		return(Xcp);
	case 'a':
		return(Xas);
	case 'l':
		return(Xld);
	}
}
