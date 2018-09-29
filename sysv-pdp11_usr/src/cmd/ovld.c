/*
 *  link editor
 *  modified to be a overlayed segmentation register loader by wnj 6/79
 *  further modified ilj and bso 2/81
 */

#include "signal.h"
#include "sys/types.h"
#include "sys/stat.h"

/*	Layout of a.out file :
 *
 *	header of 8 words	magic number 405, 407, 410, 411
 *				text size	)
 *				data size	) in bytes but even
 *				bss size	)
 *				symbol table size
 *				entry point
 *				{unused}
 *				flag set if no relocation
 *
 *
 *	header:		0
 *	text:		16
 *	data:		16+textsize
 *	relocation:	16+textsize+datasize
 *	symbol table:	16+2*(textsize+datasize) or 16+textsize+datasize
 *
 */
#define TRUE	1
#define FALSE	0

#define	ARCMAGIC 0177545
#define OMAGIC	0405
#define	FMAGIC	0407
#define	NMAGIC	0410
#define	IMAGIC	0411

#define	EXTERN	040
#define	UNDEF	00
#define	ABS	01
#define	TEXT	02
#define	DATA	03
#define	BSS	04
#define	COMM	05	/* internal use only */

#define	RABS	00
#define	RTEXT	02
#define	RDATA	04
#define	RBSS	06
#define	REXT	010

#define NOVLY	16
#define	RELFLG	01
#define	NROUT	256
#define	NSYM	1103
#define	NSYMPR	1000

char	premeof[] = "Premature EOF";
char	goodnm[] = "__.SYMDEF";

/* table of contents stuff */
#define TABSZ	700
struct tab
{
	char cname[8];
	long cloc;
} tab[TABSZ];
int tnum;

/* overlay management */
int	vindex;
struct overlay {
	int	argsav;
	int	symsav;
	struct liblist	*libsav;
	char	*vname;
	int	ctsav, cdsav, cbsav;
	int	offt, offd, offb, offs;
} vnodes[NOVLY];

/* input management */
struct page {
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[256];
} page[2];

struct {
	int	nuser;
	int	bno;
} fpage;

struct stream {
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	struct page	*pno;
};

struct stream text;
struct stream reloc;

struct {
	char	aname[14];
	long	atime;
	char	auid, agid;
	int	amode;
	long	asize;
} archdr;

struct {
	int	fmagic;
	int	tsize;
	int	dsize;
	int	bsize;
	int	ssize;
	int	entry;
	int	pad;
	int	relflg;
} filhdr;

/* one entry for each archive member referenced;
 * set in first pass; needs restoring for overlays
 */
struct liblist {
	long	loc;
};

struct liblist	liblist[NROUT];
struct liblist	*libp = liblist;

/* symbol management */
struct symbol {
	char	sname[8];
	char	stype;
	char	sovly;
	int	svalue;
	int	sovalue;
};

struct xsymbol {
	char	sname[8];
	char	stype;
	char	sovly;
	int	svalue;
};
struct local {
	int locindex;			/* index to symbol in file */
	struct symbol *locsymbol;	/* ptr to symbol table */
};

struct	xsymbol	cursym;			/* current symbol */
struct	symbol	symtab[NSYM];		/* actual symbols */
struct	symbol	**symhash[NSYM];	/* ptr to hash table entry */
struct	symbol	*lastsym;		/* last symbol entered */
int	symindex;			/* next available symbol table entry */
struct	symbol	*hshtab[NSYM+2];	/* hash table for symbols */
struct	local	local[NSYMPR];

/* internal symbols */
struct	symbol	*p_etext;
struct	symbol	*p_edata;
struct	symbol	*p_end;
struct	symbol	*p_ovflg;
struct	symbol	*entrypt;

int	trace;
/* flags */
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with 'L' */
int	Sflag;		/* discard all except locals and globals*/
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	nflag;		/* pure procedure */
int	Oflag;		/* set magic # to 0405 (overlay) */
int	dflag;		/* define common even with rflag */
int	iflag;		/* I/D space separated */
int	vflag;		/* overlays used */
int	Fflag;		/* bso: discard local symbols except ~* */

int	ofilfnd;
char	*ofilename = "l.out";
int	infil;
char	*filname;

/* cumulative sizes set in pass 1 */
int	tsize;
int	dsize;
int	bsize;
int	ssize;

/* symbol relocation; both passes */
int	ctrel;
int	cdrel;
int	cbrel;

int	errlev;
int	delarg	= 4;
char	tfname[] = "/tmp/ovldaXXXXX";

/* output management */
struct buf {
	int	fildes;
	int	nleft;
	int	*xnext;
	int	iobuf[256];
};
struct buf	toutb;
struct buf	doutb;
struct buf	troutb;
struct buf	droutb;
struct buf	soutb;

/* miscellaneous */
struct symbol	**lookup();
struct symbol	**slookup();
struct symbol	*lookloc();

/* added for text overlay register */

#define THUNKSIZ 12

int	torgwas;		/* Saves torigin while doing overlays */
int	tsizwas;		/* Saves tsize while doing overlays */
int	numov;			/* Total number of overlays */
int	curov;			/* Overlay being worked on just now */
int	inov;			/* 1 if working on an overlay */
int	ovsize[8];		/* The sizes of the overlays */

#define	max	ovsize[0]

struct buf	voutb;		/* Overlay text goes here */

struct	xsymbol ovhndlr =
	{ 	"ovhndlr", EXTERN+UNDEF, 0, 0 };
int	ovbase = 7 * 8192;		/* The base address of the overlays */

delexit()
{
	unlink("l.out");
	if (delarg==0)
		chmod(ofilename, 0777 & ~umask(0));
	exit(delarg);
}

main(argc, argv)
char **argv;
{
	register int c, i;
	int num;
	register char *ap, **p;
	int found;
	int vscan;
	char save;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, delexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, delexit);
	if (argc == 1)
		exit(4);
	p = argv+1;

	/* scan files once to find symdefs */
	for (c=1; c<argc; c++) {
		if (trace) printf("%s:\n", *p);
		filname = 0;
		ap = *p++;

		if (*ap == '-') {
			for (i=1; ap[i]; i++) {
				switch (ap[i]) {
				case 'o':
					if (++c >= argc)
						error(2, "Bad output file");
					ofilename = *p++;
					ofilfnd++;
					continue;

				case 'u':
				case 'e':
					if (++c >= argc)
						error(2, "Bad 'use' or 'entry'");
					enter(slookup(*p++));
					if (ap[i]=='e')
						entrypt = lastsym;
					continue;

				case 'v':
					if (++c >= argc)
						error(2, "-v: arg missing");
					vflag=TRUE;
					vscan = vindex;
					found=FALSE;
					while (--vscan>=0 && found==FALSE)
						found = eq(vnodes[vscan].vname, *p);
					if (found) {
						endload(c, argv);
						restore(vscan);
					} else
						record(c, *p);
					p++;
					continue;

				case 'D':
					if (++c >= argc)
						error(2, "-D: arg missing");
					num = atoi(*p++);
					if (dsize>num)
						error(2, "-D: too small");
					dsize = num;
					continue;

				case 'l':
					save = ap[--i];
					ap[i]='-';
					load1arg(&ap[i]);
					ap[i]=save;
					break;

				case 'x':
					xflag++;
					continue;

				case 'F':
					Fflag++;
					Xflag = 0;
					continue;

				case 'X':
					if( !Fflag )
						Xflag++;
					continue;

				case 'S':
					Sflag++;
					continue;

				case 'r':
					rflag++;
					arflag++;
					continue;

				case 's':
					sflag++;
					xflag++;
					continue;

				case 'n':
					nflag++;
					continue;

				case 'd':
					dflag++;
					continue;

				case 'i':
					iflag++;
					continue;

				case 'O':
					Oflag++;
					continue;

				case 't':
					trace++;
					continue;

				case 'Z':	/* overlayed objects follow */
					if (!inov) {
						tsizwas = tsize;
						if (numov == 0) {
							cursym = ovhndlr;
							enter(lookup());
						}
					} else {
						ovsize[curov] = tsize;
						if (trace)
							printf("overlay %d size %d\n", curov, ovsize[curov]);
					}
					tsize = 0;
					inov = 1;
					numov++;
					if (numov > 8)
						error(2, "Too many overlays, limit 8");
					curov++;
					continue;

				case 'L':	/* end of overlay objects */
					if (inov == 0)
						error(2, "-L: Not in overlay");
					ovsize[curov] = tsize;
					if (trace)
						printf("overlay %d size %d\n", curov, ovsize[curov]);
					curov = inov = 0;
					tsize = tsizwas;
					continue;

				default:
					error(2, "bad flag");
				} /*endsw*/
				break;
			} /*endfor*/
		} else
			load1arg(ap);
	}
	endload(argc, argv);
}

/* used after pass 1 */
int	nsym;
int	torigin;
int	dorigin;
int	borigin;

endload(argc, argv)
int argc;
char **argv;
{
	register int c, i;
	int dnum;
	register char *ap, **p;

	if (numov)
		rflag = 0;
	filname = 0;
	middle();
	setupout();
	p = argv+1;
	libp = liblist;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (trace) printf("%s:\n", ap);
		if (*ap == '-') {
			for (i=1; ap[i]; i++) {
				switch (ap[i]) {
				case 'D':
					for (dnum = atoi(*p); dorigin<dnum; dorigin += 2) {
						putw(0, &doutb);
						if (rflag)
							putw(0, &droutb);
					}
				case 'u':
				case 'e':
				case 'o':
				case 'v':
					++c;
					++p;

				default:
					continue;

				case 'l':
					ap[--i]='-';
					load2arg(&ap[i]);
					break;

				case 'Z':	/* overlayed objects follow */
					if (inov == 0)
						torgwas = torigin;
					else
						roundov();
					torigin = ovbase;
					inov = 1;
					curov++;
					continue;

				case 'L':	/* end of overlay objects */
					roundov();
					inov = 0;
					if (trace)
						printf("end overlay generation\n");
					torigin = torgwas;
					continue;
				} /*endsw*/
				break;
			} /*endfor*/
		} else
			load2arg(ap);
	}
	finishout();
}

roundov()
{

	while (torigin & 077) {
		putw(0, &voutb);
		torigin += sizeof (int);
	}
}

record(c, nam)
int c;
char *nam;
{
	register struct overlay *v;

	v = &vnodes[vindex++];
	v->argsav = c;
	v->symsav = symindex;
	v->libsav = libp;
	v->vname = nam;
	v->offt = tsize;
	v->offd = dsize;
	v->offb = bsize;
	v->offs = ssize;
	v->ctsav = ctrel;
	v->cdsav = cdrel;
	v->cbsav = cbrel;
}

restore(vscan)
int vscan;
{
	register struct overlay *v;
	register int saved;

	v = &vnodes[vscan];
	vindex = vscan+1;
	libp = v->libsav;
	ctrel = v->ctsav;
	cdrel = v->cdsav;
	cbrel = v->cbsav;
	tsize = v->offt;
	dsize = v->offd;
	bsize = v->offb;
	ssize = v->offs;
	saved = v->symsav;
	while (symindex>saved)
		*symhash[--symindex]=0;
}

/* scan file to find defined symbols */
load1arg(acp)
char *acp;
{
	register char *cp;
	long nloc;

	cp = acp;
	switch ( getfile(cp)) {
	case 0:
		load1(0, 0L);
		break;

		/* regular archive */
	case 1:
		nloc = 1;
		while ( step(nloc))
			nloc += (archdr.asize + sizeof(archdr) + 1) >> 1;
		break;

		/* table of contents */
	case 2:
		tnum = archdr.asize / sizeof(struct tab);
		if (tnum >= TABSZ) {
			error(2, "fast load buffer too small");
		}
		lseek(infil, (long)(sizeof(filhdr.fmagic)+sizeof(archdr)), 0);
		read(infil, (char *)tab, tnum * sizeof(struct tab));
		while (ldrand());
		libp->loc = -1;
		libp++;
		break;
		/* out of date table of contents */
	case 3:
		error(0, "out of date (warning)");
		for(nloc = 1+((archdr.asize+sizeof(archdr)+1) >> 1); step(nloc); nloc += (archdr.asize + sizeof(archdr) +
		    1) >> 1);
		break;
	}
	close(infil);
}

step(nloc)
long nloc;
{
	dseek(&text, nloc, sizeof archdr);
	if (text.size <= 0) {
		libp->loc = -1;
		libp++;
		return(0);
	}
	mget((int *)&archdr, sizeof archdr);
	if (load1(1, nloc + (sizeof archdr) / 2)) {
		libp->loc = nloc;
		libp++;
	}
	return(1);
}

ldrand()
{
	int i;
	struct symbol *sp, **pp;
	struct liblist *oldp = libp;
	for(i = 0; i<tnum; i++) {
		if ((pp = slookup(tab[i].cname)) == 0)
			continue;
		sp = *pp;
		if (sp->stype != EXTERN+UNDEF)
			continue;
		step(tab[i].cloc >> 1);
	}
	return(oldp != libp);
}

add(a,b,s)
int a, b;
char *s;
{
	long r;

	r = (long)(unsigned)a + (unsigned)b;
	if (r >= 0200000)
		error(1,s);
	return(r);
}

/* single file or archive member */
load1(libflg, loc)
long loc;
{
	register struct symbol *sp;
	int savindex;
	int ndef, nloc, type, mtype;

	readhdr(loc);
	ctrel = tsize;
	cdrel += dsize;
	cbrel += bsize;
	ndef = 0;
	nloc = sizeof cursym;
	savindex = symindex;
	if ((filhdr.relflg&RELFLG)==1) {
		error(1, "No relocation bits");
		return(0);
	}
	loc += (sizeof filhdr)/2 + filhdr.tsize + filhdr.dsize;
	dseek(&text, loc, filhdr.ssize);
	while (text.size > 0) {
		mget((int *)&cursym, sizeof cursym);
		type = cursym.stype;
		if (Sflag) {
			mtype = type&037;
			if (mtype==1 || mtype>4) {
				continue;
			}
		}
		if ((type&EXTERN)==0) {
			char c = cursym.sname[0];

			if( (!Xflag || c!='L') && (!Fflag || c == '~') )
				nloc += sizeof cursym;
			continue;
		}
		/* force overlay routines to use ovcsav and ovcret */
		if (inov && cursym.sname[0] == 'c') {
			if (!strcmp(cursym.sname, "csav"))
				strcpy(cursym.sname, "ovcsav");
			if (!strcmp(cursym.sname, "cret"))
				strcpy(cursym.sname, "ovcret");
		}
		symreloc();
		if (enter(lookup()))
			continue;
		if ((sp = lastsym)->stype != EXTERN+UNDEF)
			continue;
		if (cursym.stype == EXTERN+UNDEF) {
			if (cursym.svalue > sp->svalue)
				sp->svalue = cursym.svalue;
			continue;
		}
		if (sp->svalue != 0 && cursym.stype == EXTERN+TEXT)
			continue;
		ndef++;
		sp->stype = cursym.stype;
		sp->svalue = cursym.svalue;
		if ((sp->stype &~ EXTERN) == TEXT)
			sp->sovly = curov;
		if (trace)
			printf("found %8.8s in overlay %d at %d\n", sp->sname, sp->sovly, sp->svalue);
	}
	if (libflg==0 || ndef) {
		tsize = add(tsize,filhdr.tsize,"text overflow");
		dsize = add(dsize,filhdr.dsize,"data overflow");
		bsize = add(bsize,filhdr.bsize,"bss overflow");
		ssize = add(ssize,nloc,"symbol table overflow");
		return(1);
	}
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	while (symindex>savindex)
		*symhash[--symindex]=0;
	return(0);
}

middle()
{
	register struct symbol *sp, *symp;
	register t, csize;
	int nund, corigin;
	int ttsize;

	torigin=0;
	dorigin=0;
	borigin=0;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end   = *slookup("_end");
	p_ovflg = *slookup("_ovflg");
	ldrsym(p_ovflg, numov, EXTERN+DATA);
	/*
	 * If there are any undefined symbols, save the relocation bits.
	 */
	symp = &symtab[symindex];
	if (rflag==0) {
		for (sp = symtab; sp<symp; sp++)
			if(sp->stype==EXTERN+UNDEF && sp->svalue==0 && sp!=p_end && sp!=p_edata && sp!=p_etext && sp!=p_ovflg) {
				rflag++;
				dflag = 0;
				break;
			}
	}
	if (rflag)
		nflag = sflag = iflag = Oflag = 0;
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (dflag || rflag==0) {
		ldrsym(p_etext, tsize, EXTERN+TEXT);
		ldrsym(p_edata, dsize, EXTERN+DATA);
		ldrsym(p_end, bsize, EXTERN+BSS);
		for (sp = symtab; sp<symp; sp++)
			if (sp->stype==EXTERN+UNDEF && (t = sp->svalue)!=0) {
				t = (t+1) & ~01;
				sp->svalue = csize;
				sp->stype = EXTERN+COMM;
				csize = add(csize, t, "bss overflow");
			}
	}
	/*
	 *	for overlays:
	 *	- allocate space in root segment for overlay entry code.
	 *	- make symbol table refer this location.
	 */
	if (numov) {
		for (sp = symtab; sp < symp; sp++) {
			if (trace)
				printf("%8.8s stype %o svalue %o sovalue %o sovly %d\n", sp->sname, sp->stype, sp->svalue,
				    sp->sovalue, sp->sovly);
			if (sp->sovly && sp->stype == EXTERN+TEXT) {
				sp->sovalue = sp->svalue;
				sp->svalue = tsize;
				tsize += THUNKSIZ;
				if (trace)
					printf("relocating %s in overlay %d from %o to %o\n", sp->sname, sp->sovly, sp->sovalue,
					    sp->svalue);
			}
		}
	}
	/*
	 * Now set symbols to their final value
	 */
	if (nflag || iflag)
		tsize = (tsize + 077) & ~077;
	/*
	 *	for overlays:
	 *	- relocate into virtual location.
	 *	- adjust overlay sizes to be multiple of 64
	 *	- calculate highest virtual text size
	 */
	ttsize = tsize;
	if (numov) {
		register int i;

		for (sp = symtab; sp < symp; sp++)
			if (sp->sovly && sp->stype == EXTERN+TEXT) {
				sp->sovalue += ovbase;
				if (trace)
					printf("%.8s at 0%6o overlay %d\n", sp->sname, sp->sovalue, sp->sovly);
			}
		for (i = 1; i < 8; i++) {
			ovsize[i] = (ovsize[i] + 077) &~ 077;
			if (ovsize[i] > max)
				max = ovsize[i];
		}
		if (trace)
			printf("maximum overlay size is %d.\n", max);
		ttsize = ovbase + max;
		ttsize = (ttsize + 017777) &~ 017777;
		if (trace)
			printf("overlays end before %u.\n", ttsize);
	}
	dorigin = ttsize;
	if (nflag)
		dorigin = (ttsize+017777) & ~017777;
	if (iflag)
		dorigin = 0;
	corigin = dorigin + dsize;
	borigin = corigin + csize;
	nund = 0;
	for (sp = symtab; sp<symp; sp++) switch (sp->stype) {
	case EXTERN+UNDEF:
		errlev |= 01;
		if (arflag==0 && sp->svalue==0) {
			if (nund==0)
				printf("Undefined:\n");
			nund++;
			printf("%.8s\n", sp->sname);
		}
		continue;

	case EXTERN+ABS:
	default:
		continue;

	case EXTERN+TEXT:
		sp->svalue += torigin;
		continue;

	case EXTERN+DATA:
		sp->svalue += dorigin;
		continue;

	case EXTERN+BSS:
		sp->svalue += borigin;
		continue;

	case EXTERN+COMM:
		sp->stype = EXTERN+BSS;
		sp->svalue += corigin;
		continue;
	}
	if (sflag || xflag)
		ssize = 0;
	bsize = add(bsize, csize, "bss overflow");
	nsym = ssize / (sizeof cursym);
}

ldrsym(asp, val, type)
struct symbol *asp;
{
	register struct symbol *sp;

	if ((sp = asp) == 0)
		return;
	if (sp->stype != EXTERN+UNDEF || sp->svalue) {
		printf("%.8s: ", sp->sname);
		/* 		if (trace) */
		printf("svalue %o ", sp->svalue);
		error(1, "Multiply defined");
		return;
	}
	sp->stype = type;
	sp->svalue = val;
}

setupout()
{
	tcreat(&toutb, 0);
	mktemp(tfname);
	tcreat(&doutb, 1);
	if (sflag==0 || xflag==0)
		tcreat(&soutb, 1);
	if (rflag) {
		tcreat(&troutb, 1);
		tcreat(&droutb, 1);
	}
	if (numov)
		tcreat(&voutb, 1);	/* for overlays */
	filhdr.fmagic = (Oflag ? OMAGIC :( iflag ? IMAGIC : ( nflag ? NMAGIC : FMAGIC )));
	if (numov) {
		if (filhdr.fmagic == FMAGIC)
			error(2, "Can't register overlay 407 execs");
		filhdr.fmagic |= 020;
	}
	filhdr.tsize = tsize;
	filhdr.dsize = dsize;
	filhdr.bsize = bsize;
	filhdr.ssize = sflag? 0: (ssize + (sizeof cursym)*symindex);
	if (entrypt) {
		if (entrypt->stype!=EXTERN+TEXT)
			error(1, "Entry point not in text");
		else if (entrypt->sovly)
			error(1, "Entry point in overlay");
		else
			filhdr.entry = entrypt->svalue | 01;
	} else
		filhdr.entry=0;
	filhdr.pad = 0;
	filhdr.relflg = (rflag==0);
	mput(&toutb, (int *)&filhdr, sizeof filhdr);
	/*
	 *	write ovsize array to a.out if overlayed
	 */
	if (numov) {
		register int i;
		for (i = 0; i < 8; i++)
			putw(ovsize[i], &toutb);
	}
}

tcreat(buf, tempflg)
struct buf *buf;
{
	register int ufd;
	char *nam;
	nam = (tempflg ? tfname : ofilename);
	if ((ufd = creat(nam, 0666)) < 0)
		error(2, tempflg?"cannot create temp":"cannot create output");
	close(ufd);
	buf->fildes = open(nam, 2);
	if (tempflg)
		unlink(tfname);
	buf->nleft = sizeof(buf->iobuf)/sizeof(int);
	buf->xnext = buf->iobuf;
}

load2arg(acp)
char *acp;
{
	register char *cp;
	register struct liblist *lp;

	cp = acp;
	if (getfile(cp) == 0) {
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0L);
	} else {	/* scan archive members referenced */
		for (lp = libp; lp->loc != -1; lp++) {
			dseek(&text, lp->loc, sizeof archdr);
			mget((int *)&archdr, sizeof archdr);
			mkfsym(archdr.aname);
			load2(lp->loc + (sizeof archdr) / 2);
		}
		libp = ++lp;
	}
	close(infil);
}

load2(loc)
long loc;
{
	register struct symbol *sp;
	register struct local *lp;
	register int symno;
	int type, mtype;

	readhdr(loc);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;
	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	lp = local;
	symno = -1;
	loc += (sizeof filhdr)/2;
	dseek(&text, loc + filhdr.tsize + filhdr.dsize, filhdr.ssize);
	while (text.size > 0) {
		symno++;
		mget((int *)&cursym, sizeof cursym);
		/* force overlay routines to use ovcsav and ovcret */
		if (inov && cursym.sname[0] == 'c') {
			if (!strcmp(cursym.sname, "csav"))
				strcpy(cursym.sname, "ovcsav");
			if (!strcmp(cursym.sname, "cret"))
				strcpy(cursym.sname, "ovcret");
		}
		symreloc();
		type = cursym.stype;
		if (Sflag) {
			mtype = type&037;
			if (mtype==1 || mtype>4) continue;
		}
		if ((type&EXTERN) == 0) {
			if (!sflag&&!xflag&&(!Xflag||cursym.sname[0]!='L')&&(!Fflag||cursym.sname[0]=='~'))
				mput(&soutb, (int *)&cursym, sizeof cursym);
			continue;
		}
		if ((sp = *lookup()) == 0)
			error(2, "internal error: symbol not found");
		if (cursym.stype == EXTERN+UNDEF || cursym.stype == EXTERN+TEXT) {
			if (lp >= &local[NSYMPR])
				error(2, "Local symbol overflow");
			lp->locindex = symno;
			lp++->locsymbol = sp;
			continue;
		}
		if (cursym.stype!=sp->stype
		    || cursym.svalue!=sp->svalue && !sp->sovly
		    || sp->sovly && cursym.svalue!=sp->sovalue) {
			printf("%.8s: ", cursym.sname);
			/* 			if (trace) { */
			printf(" sovly %d sovalue %o ", sp->sovly, sp->sovalue);
			printf("new %o hav %o ", cursym.svalue, sp->svalue);
			/* 			} */
			error(1, "Multiply defined");
		}
	}
	dseek(&text, loc, filhdr.tsize);
	dseek(&reloc, loc + half(filhdr.tsize + filhdr.dsize), filhdr.tsize);
	load2td(lp, ctrel, inov ? &voutb : &toutb, &troutb, 1);
	dseek(&text, loc+half(filhdr.tsize), filhdr.dsize);
	dseek(&reloc, loc+filhdr.tsize+half(filhdr.dsize), filhdr.dsize);
	load2td(lp, cdrel, &doutb, &droutb, 0);
	torigin += filhdr.tsize;
	dorigin += filhdr.dsize;
	borigin += filhdr.bsize;
}

struct symbol *
loctsym(addr)
unsigned addr;
{
	register struct symbol *sp;
	for( sp = symtab ; sp < & symtab[NSYM]; sp++ ){
		if( sp->sovalue == addr && sp->sovly == curov )
			return(sp);
	}
	error(1, "Failed to find a symbol");
	return 0;
}

load2td(lp, creloc, b1, b2, txflg)
struct local *lp;
struct buf *b1, *b2;
{
	register r, t;
	register struct symbol *sp;
	unsigned instr;

	for (;;) {
		/*
		 * The pickup code is copied from "get" for speed.
		 */

		/* next text or data word */
		if (--text.size <= 0) {
			if (text.size < 0)
				break;
			text.size++;
			t = get(&text);
		} else if (--text.nibuf < 0) {
			text.nibuf++;
			text.size++;
			t = get(&text);
		} else
			t = *text.ptr++;

		/* next relocation word */
		if (--reloc.size <= 0) {
			if (reloc.size < 0)
				error(2, "Relocation error");
			reloc.size++;
			r = get(&reloc);
		} else if (--reloc.nibuf < 0) {
			reloc.nibuf++;
			reloc.size++;
			r = get(&reloc);
		} else
			r = *reloc.ptr++;

		if( r == 0 ) {
			/* must be an instruction */
			instr = t;
		} else switch(r&016) {

		case RTEXT:
			t += ctrel;
			if( inov && txflg ) {
				/*
				 * in overlay code must force value
				 * references to a symbol to reference
				 * the overlay entry address not
				 * the virtual address.
				 */
				switch( instr & 0177700 ) {
				case 012700:	/* mov $X, */
				case 022700:	/* cmp $X, */
					sp = loctsym(t);
					if( sp )
						t = sp->svalue;
				}
			}
			break;

		case RDATA:
			t += cdrel;
			break;

		case RBSS:
			t += cbrel;
			break;

		case REXT:
			sp = lookloc(lp, r);
			if (sp->stype==EXTERN+UNDEF) {
				r = (r&01) + ((nsym+(sp-symtab))<<4) + REXT;
				break;
			}
			t += sp->svalue;
			r = (r&01) + ((sp->stype-(EXTERN+ABS))<<1);
			break;
		}
		if (r&01)
			t -= creloc;
		putw(t, b1);
		if (rflag)
			putw(r, b2);
	}
}

finishout()
{
	register n, *p;
	register struct symbol *sp, *symp;

	/*	for overlays:
	 *	- emit overlay entry code, space has already be allocated.
	 */
	if (numov) {
		int aovno = adrof("__ovno");
		int aovhndlr = adrof("ovhndlr");
		symp = &symtab[symindex];
		for (sp = symtab; sp < symp; sp++)
			if (sp->sovly && (sp->stype & (EXTERN+TEXT))) {
				putw(012701, &toutb);	/* mov $N*2,r1 */
				putw(sp->sovly*2, &toutb);
				putw(012700, &toutb);	/* mov $E+4,r0 */
				putw(sp->sovalue+4, &toutb);
				putw(012707, &toutb);	/* mov *$ovhndlr,pc */
				putw(aovhndlr, &toutb);
				torigin += THUNKSIZ;
			}
	}
	if (nflag||iflag) {
		n = torigin;
		while (n&077) {
			n += 2;
			putw(0, &toutb);
			if (rflag)
				putw(0, &troutb);
		}
	}
	if (numov)
		copy(&voutb);	/* add overlays to output file */
	copy(&doutb);
	if (rflag) {
		copy(&troutb);
		copy(&droutb);
	}
	if (sflag==0) {
		if (xflag==0)
			copy(&soutb);
		for (p = (int *)symtab; p < (int *)&symtab[symindex];) {
/* this is bad machine dependent code... */
			putw(*p++, &toutb); putw(*p++, &toutb);  /* symbol */
			putw(*p++, &toutb); putw(*p++, &toutb);  /* name */
			putw(*p, &toutb);	/* type and ovly # */
			if((*p++ >> 8) & 0377)	/* if in ovly, use sovalue */
			{
				p++;
				putw(*p++, &toutb); 
			} else {		/* otherwise, use svalue */
				putw(*p++, &toutb);
				p++;
			}
		}
	}
	flush(&toutb);
	close(toutb.fildes);
	if (!ofilfnd) {
		unlink("a.out");
		link("l.out", "a.out");
		ofilename = "a.out";
	}
	delarg = errlev;
	delexit();
}

/*
 *	for overlays:
 *	- return address of symbol
 */
adrof(s)
char *s;
{
	register struct symbol **p = slookup(s);

	if (*p == 0) {
		printf("%.8s: ", s);
		error(1, "Undefined!");
		return (0);
	}
	return ((*p)->svalue);
}

copy(buf)
struct buf *buf;
{
	register f, *p, n;

	flush(buf);
	lseek(f = buf->fildes, (long)0, 0);
	while ((n = read(f, (char *)buf->iobuf, sizeof(buf->iobuf))) > 1) {
		n >>= 1;
		p = (int *)buf->iobuf;
		do
			putw(*p++, &toutb);
		while (--n);
	}
	close(f);
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cp8c(s, cursym.sname);
	cursym.stype = 037;
	cursym.svalue = torigin;
	mput(&soutb, (int *)&cursym, sizeof cursym);
}

mget(aloc, an)
int *aloc;
{
	register *loc, n;
	register *p;

	n = an;
	n >>= 1;
	loc = aloc;
	if ((text.nibuf -= n) >= 0) {
		if ((text.size -= n) > 0) {
			p = text.ptr;
			do
				*loc++ = *p++;
			while (--n);
			text.ptr = p;
			return;
		} else
			text.size += n;
	}
	text.nibuf += n;
	do {
		*loc++ = get(&text);
	}	while (--n);
}

mput(buf, aloc, an)
struct buf *buf;
int *aloc;
{
	register *loc;
	register n;

	loc = aloc;
	n = an>>1;
	do {
		putw(*loc++, buf);
	}	while (--n);
}

dseek(asp, aloc, s)
long aloc;
struct stream *asp;
{
	register struct stream *sp;
	register struct page *p;
	long b, o;
	int n;

	b = aloc >> 8;
	o = aloc & 0377;
	sp = asp;
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			lseek(infil, (aloc & ~0377L) << 1, 0);
			if ((n = read(infil, (char *)p->buff, 512)>>1) < 0)
				n = 0;
			p->nibuf = n;
		} 
		else
			error(2, "No pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	sp->ptr = p->buff + o;
	if (s != -1)
		sp->size = half(s);
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

half(i)
{
	return((i>>1)&077777);
}

get(asp)
struct stream *asp;
{
	register struct stream *sp;

	sp = asp;
	if (--sp->nibuf < 0) {
		dseek(sp, (long)(sp->bno + 1) << 8, -1);
		--sp->nibuf;
	}
	if (--sp->size <= 0) {
		if (sp->size < 0)
			error(2, premeof);
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = (struct page *)&fpage;
	}
	return(*sp->ptr++);
}

getfile(acp)
char *acp;
{
	register char *cp;
	register int c;
	struct stat x;

	cp = acp;
	infil = -1;
	archdr.aname[0] = '\0';
	filname = cp;
	if (cp[0]=='-' && cp[1]=='l') {
		if(cp[2] == '\0')
			cp = "-la";
		filname = "/usr/lib/libxxxxxxxxxxxxxxx";
		for(c=0; cp[c+2]; c++)
			filname[c+12] = cp[c+2];
		filname[c+12] = '.';
		filname[c+13] = 'a';
		filname[c+14] = '\0';
		if ((infil = open(filname+4, 0)) >= 0) {
			filname += 4;
		}
	}
	if (infil == -1 && (infil = open(filname, 0)) < 0)
		error(2, "cannot open");
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = (struct page *)&fpage;
	fpage.nuser = 2;
	dseek(&text, 0L, 2);
	if (text.size <= 0)
		error(2, premeof);
	if(get(&text) != ARCMAGIC)
		return(0);	/* regualr file */
	dseek(&text, 1L, sizeof archdr);	/* word addressing */
	if(text.size <= 0)
		return(1);	/* regular archive */
	mget((int *)&archdr, sizeof archdr);
	if(strncmp(archdr.aname, goodnm, 14) != 0)
		return(1);	/* regular archive */
	else {
		fstat(infil, &x);
		if(x.st_mtime > archdr.atime)
		{
			return(3);
		}
		else return(2);
	}
}

struct symbol **lookup()
{
	int i;
	int clash;
	register struct symbol **hp;
	register char *cp, *cp1;

	i = 0;
	for (cp = cursym.sname; cp < &cursym.sname[8];)
		i = (i<<1) + *cp++;
	for (hp = &hshtab[(i&077777)%NSYM+2]; *hp!=0;) {
		cp1 = (*hp)->sname;
		clash=FALSE;
		for (cp = cursym.sname; cp < &cursym.sname[8];)
			if (*cp++ != *cp1++) {
				clash=TRUE;
				break;
			}
		if (clash) {
			if (++hp >= &hshtab[NSYM+2])
				hp = hshtab;
		} else
			break;
	}
	return(hp);
}

struct symbol **slookup(s)
char *s;
{
	cp8c(s, cursym.sname);
	cursym.stype = EXTERN+UNDEF;
	cursym.svalue = 0;
	return(lookup());
}

enter(hp)
struct symbol **hp;
{
	register struct symbol *sp;

	if (*hp==0) {
		if (symindex>=NSYM)
			error(2, "Symbol table overflow");
		symhash[symindex] = hp;
		*hp = lastsym = sp = &symtab[symindex++];
		cp8c(cursym.sname, sp->sname);
		sp->stype = cursym.stype;
		sp->svalue = cursym.svalue;
		if (sp->stype == EXTERN+TEXT) {
			sp->sovly = curov;
			if (trace)
				printf("found %8.8s in overlay %d at %d\n", sp->sname, sp->sovly, sp->svalue);
		}
		return(1);
	} else {
		lastsym = *hp;
		return(0);
	}
}

symreloc()
{
	switch (cursym.stype) {

	case TEXT:
	case EXTERN+TEXT:
		cursym.svalue += ctrel;
		return;

	case DATA:
	case EXTERN+DATA:
		cursym.svalue += cdrel;
		return;

	case BSS:
	case EXTERN+BSS:
		cursym.svalue += cbrel;
		return;

	case EXTERN+UNDEF:
		return;
	}
	if (cursym.stype&EXTERN)
		cursym.stype = EXTERN+ABS;
}

error(n, s)
char *s;
{
	if (errlev==0)
		printf("ovld:");
	if (filname) {
		printf("%s", filname);
		if (archdr.aname[0])
			printf("(%.14s)", archdr.aname);
		printf(": ");
	}
	printf("%s\n", s);
	if (n > 1)
		delexit();
	errlev = n;
}

struct symbol *
lookloc(alp, r)
struct local *alp;
{
	register struct local *clp, *lp;
	register sn;

	lp = alp;
	sn = (r>>4) & 07777;
	for (clp = local; clp<lp; clp++)
		if (clp->locindex == sn)
			return(clp->locsymbol);
	error(2, "Local symbol botch");
}

readhdr(loc)
long loc;
{
	register st, sd;

	dseek(&text, loc, sizeof filhdr);
	mget((int *)&filhdr, sizeof filhdr);
	if (filhdr.fmagic != FMAGIC)
		error(2, "Bad format");
	st = (filhdr.tsize+01) & ~01;
	filhdr.tsize = st;
	cdrel = -st;
	sd = (filhdr.dsize+01) & ~01;
	cbrel = - (st+sd);
	filhdr.bsize = (filhdr.bsize+01) & ~01;
}

cp8c(from, to)
char *from, *to;
{
	register char *f, *t, *te;

	f = from;
	t = to;
	te = t+8;
	while ((*t++ = *f++) && t<te);
	while (t<te)
		*t++ = 0;
}

eq(s1, s2)
char *s1;
char *s2;
{
	while (*s1==*s2++)
		if ((*s1++)==0)
			return(TRUE);
	return(FALSE);
}

putw(w, b)
register struct buf *b;
{
	*(b->xnext)++ = w;
	if (--b->nleft <= 0)
		flush(b);
}

flush(b)
register struct buf *b;
{
	register n;

	if ((n = (char *)b->xnext - (char *)b->iobuf) > 0)
		if (write(b->fildes, (char *)b->iobuf, n) != n)
			error(2, "output error");
	b->xnext = b->iobuf;
	b->nleft = sizeof(b->iobuf)/sizeof(int);
}
