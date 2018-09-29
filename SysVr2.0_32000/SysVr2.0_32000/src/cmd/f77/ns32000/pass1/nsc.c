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
/*	@(#)vax.c	1.8	*/
#include "defs"

#ifdef SDB
#	include <a.out.h>
extern int types2[];
#endif

#include "pccdefs"

/*
	VAX-11/780 - SPECIFIC ROUTINES
*/


int maxregvar = MAXREGVAR;
int regnum[] = { 7, 6, 5, 4 };
static char *regmask[] = { "", "r7", "r6,r7", "r5,r6,r7", "r4,r5,r6,r7" };



ftnint intcon[14] =
	{ 2, 2, 2, 2,
	  15, 31, 24, 56,
	  -128, -128, 127, 127,
	  32767, 2147483647 };

#if HERE == VAX
	/* then put in constants in octal */
long realcon[6][2] =
	{
		{ 0200, 0 },
		{ 0200, 0 },
		{ 037777677777, 0 },
		{ 037777677777, 037777777777 },
		{ 032200, 0 },
		{ 022200, 0 }
	};
#else
double realcon[6] =
	{
	2.9387358771e-39,
	2.938735877055718800e-39,
	1.7014117332e+38,
	1.701411834604692250e+38,
	5.960464e-8,
	1.38777878078144567e-17,
	};
#endif




prsave()
{
int proflab;
if(profileflag)
	{
	proflab = newlabel();
	fprintf(asmfile, ".L%d:\t.space\t4\n", proflab);
	p2pi("\taddr\t.L%d,r0", proflab);
	p2pass("\tjsr\tmcount");
	}
}



goret(type)
int type;
{
/* entry points change fp, must change it back
p2pass("\tsprd\tfp, r1\n");
p2pi("\taddd\t$.FP%d, r1\n", procno);
p2pass("\tlprd\tfp, r1\n");
*/
p2pi("\texit\t[%s]", regmask[highregvar]);
p2pass("\tret\t0");
p2pij("\t.set\t.FSP%d, %d", procno, (autoleng * sizeof(char) + 3) & ~3);
}




/*
 * move argument slot arg1 (relative to ap)
 * to slot arg2 (relative to ARGREG)
 */

mvarg(type, arg1, arg2)
int type, arg1, arg2;
{
p2pij("\tmovd\t%d(fp),%d(fp)", arg1+ARGOFFSET, arg2+argloc);
}




prlabel(fp, k)
FILEP fp;
int k;
{
fprintf(fp, ".L%d:\n", k);
}



prconi(fp, type, n)
FILEP fp;
int type;
ftnint n;
{
fprintf(fp, "\t%s\t%ld\n", (type==TYSHORT ? ".word" : ".double"), n);
}


prnicon(fp, type, num, n)
register FILEP fp;
int type;
register int n;
ftnint num;
{
char buf[20];

sprintf(buf, "%ld", num);
fprintf(fp, "\t%s\t%s", (type==TYSHORT ? ".word" : ".double"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}



prcona(fp, a)
FILEP fp;
ftnint a;
{
fprintf(fp, "\t.double\t.L%ld\n", a);
}



prnacon(fp, a, n)
register FILEP fp;
ftnint a;
register int n;
{
char buf[20];

sprintf(buf, ".L%ld", a);
fprintf(fp, "\t.double\t%s", buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

#ifndef ns32000
prconr(fp, type, x)
FILEP fp;
int type;
float x;
{
fprintf(fp, "\t%s\t0f%e\n", (type==TYREAL ? ".float" : ".long"), x);
}

prnrcon(fp, type, x, n)
register FILEP fp;
int type;
register int n;
float x;
{
char buf[30];

sprintf(buf, "0f%e", x);
fprintf(fp, "\t%s\t%s", (type==TYREAL ? ".float" : ".long"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

prncxcon(fp, type, real, imag, n)
register FILEP fp;
int type;
register int n;
double real, imag;
{
char buf[50];

sprintf(buf, "0f%e,0f%e", real, imag);
fprintf(fp, "\t%s\t%s", (type == TYREAL ? ".float" : ".long"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}
#endif



#ifdef ns32000
prconr(fp, type, x)
FILEP fp;
int type;
double x;
{
/* non-portable cheat to preserve bit patterns */
union { double xd; float xf[2]; long int xl[2]; } cheat;
if(type == TYREAL) {
	cheat.xf[0] = x;
	fprintf(fp, "\t.double\t0x%lx\n", cheat.xl[0]);
	}
else {
	cheat.xd = x;
	fprintf(fp, "\t.double\t0x%lx,0x%lx\n", cheat.xl[0], cheat.xl[1]);
	}
}

prnrcon(fp, type, x, n)
register FILEP fp;
int type;
double x;
register int n;
{
char buf[50];
union {double xd; float xf[2]; long int xl[2]; } cheat;

if (type == TYREAL)
{
	cheat.xf[0] = x;
	sprintf(buf, "0x%lx", cheat.xl[0]);
}
else
{
	cheat.xd = x;
	sprintf(buf, "0x%lx,0x%lx", cheat.xl[0], cheat.xl[1]);
}
fprintf(fp, "\t.double\t%s", buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

prncxcon(fp, type, real, imag, n)
register FILEP fp;
int type;
register int n;
double real, imag;
{
char buf[60];
union {double xd; float xf[2]; long xl[2];} cheat1, cheat2;

if (type == TYREAL)
{
	cheat1.xf[0] = real;
	cheat2.xf[0] = imag;
	sprintf(buf, "0x%lx,0x%lx", cheat1.xl[0], cheat2.xl[0]);
}
else
{
	cheat1.xd = real;
	cheat2.xd = imag;
	sprintf(buf, "0x%lx,0x%lx,0x%lx,0x%lx", cheat1.xl[0], cheat1.xl[1],
	  cheat2.xl[0], cheat2.xl[1]);
}
fprintf(fp, "\t.double\t%s", buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}
#endif



praddr(fp, stg, varno, offset)
FILE *fp;
int stg, varno;
ftnint offset;
{
char *memname();

if(stg == STGNULL)
	fprintf(fp, "\t.double\t0\n");
else
	{
	fprintf(fp, "\t.double\t%s", memname(stg,varno));
	if(offset)
		fprintf(fp, "+%ld", offset);
	fprintf(fp, "\n");
	}
}




preven(k)
int k;
{
fprintf(asmfile, "\t.align\t%d\n", k);
}



vaxgoto(index, nlab, labs)
expptr index;
register int nlab;
struct Labelblock *labs[];
{
register int i;
register int swlab;
register int dlab;

putforce(TYINT, index);
fprintf(asmfile,".L%d:\t.double\t%d, 1\n", i = newlabel(), nlab);
p2pi("\tcheckd\tr0, .L%d, r0", i);
p2pi("\tbfs\t.L%d", dlab = newlabel());
p2pij(".L%d:\tcased\t.L%d[r0:d]", swlab = newlabel(), i = newlabel());
p2pi(".L%d:", i);
for(i = 0; i< nlab ; ++i)
	if( labs[i] )
		p2pij("\t.double\t.L%d-.L%d", labs[i]->labelno, swlab);
p2pi(".L%d:", dlab);
}


prarif(p, neg, zer, pos)
expptr p;
int neg, zer, pos;
{
int t;

putforce(t = p->headblock.vtype, p);
if (ISINT(t))
{
	p2pass("\tcmpqd\t0, r0");
        p2pi("\tbgt\t.L%d", neg);
        p2pi("\tbeq\t.L%d", zer);
        p2pi("\tbr\t.L%d", pos);
}
else
{
	p2pass("cmpl\tf0, $0");
	p2pi("\tblt\t.L%d", neg);
	p2pi("\tbeq\t.L%d", zer);
	p2pi("\tbr\t.L%d", pos);
}
}




char *memname(stg, mem)
int stg, mem;
{
static char s[20];

switch(stg)
	{
	case STGCOMMON:
	case STGEXT:
		sprintf(s, "_%s", varstr(XL, extsymtab[mem].extname) );
		break;

	case STGBSS:
	case STGINIT:
		sprintf(s, "v.%d", mem);
		break;

	case STGCONST:
		sprintf(s, ".L%d", mem);
		break;

	case STGEQUIV:
		sprintf(s, "q.%d", mem+eqvstart);
		break;

	default:
		badstg("memname", stg);
	}
return(s);
}




prlocvar(s, len, align)
char *s;
ftnint len;
int align;
{
fprintf(asmfile, "\t.bss\t%s,%ld,%d\n", s, len, align);
}



prext(name, leng, init)
char *name;
ftnint leng;
int init;
{
if(leng == 0)
	fprintf(asmfile, "\t.globl\t_%s\n", name);
else
	fprintf(asmfile, "\t.comm\t_%s,%ld\n", name, leng);
}





prendproc()
{
}




prtail()
{
}





prolog(ep, argvec)
struct Entrypoint *ep;
Addrp  argvec;
{
int i, argslot;
int size;
register chainp p;
register Namep q;
register struct Dimblock *dp;
expptr tp, mkaddcon();

if(procclass == CLBLOCK)
	return;
p2pi(".L%d:", ep->backlabel);
p2pij("\tenter\t[%s], .F%d", regmask[highregvar], procno);
if(argvec)
	{
	argloc = argvec->memoffset->constblock.const.ci + SZINT;
	if(proctype == TYCHAR)
		{
		mvarg(TYADDR, 0, chslot);
		mvarg(TYLENG, SZADDR, chlgslot);
		argslot = SZADDR + SZLENG;
		}
	else if( ISCOMPLEX(proctype) )
		{
		mvarg(TYADDR, 0, cxslot);
		argslot = SZADDR;
		}
	else
		argslot = 0;

	for(p = ep->arglist ; p ; p =p->nextp)
		{
		q = (Namep) (p->datap);
		mvarg(TYADDR, argslot, q->vardesc.varno);
		argslot += SZADDR;
		}
	for(p = ep->arglist ; p ; p = p->nextp)
		{
		q = (Namep) (p->datap);
		if(q->vtype==TYCHAR && q->vclass!=CLPROC)
			{
			if(q->vleng && ! ISCONST(q->vleng) )
				mvarg(TYLENG, argslot,
					q->vleng->addrblock.memno);
			argslot += SZLENG;
			}
		}

/* When generating code for procedures with more than one entry point,
 * the arguments are restacked. Two additional lines of code are needed.
 * line 1: Set ap past argument list + 1 slot over.
 * line 2: Place # of arguments in extra slot pointed to by ap.
 *      low address                      high address
 *      ---------------------------------------------
 *      |        | # args |  arg   |  arg   |       |
 *      ---------------------------------------------
 *                   ap                         fp

	p2pass("\tsprd\tfp, r0\n");
	p2pi("\tsubd\t$.FP%d, r0\n", procno);
	p2pass("\tlprd\tfp, r0\n");
	p2pij("\t.set\t.FP%d, %d", procno, ????);
*/
	}

for(p = ep->arglist ; p ; p = p->nextp)
	{
	q = (Namep) (p->datap);
	if(dp = q->vdim)
		{
		for(i = 0 ; i < dp->ndim ; ++i)
			if(dp->dims[i].dimexpr)
				puteq( fixtype(cpexpr(dp->dims[i].dimsize)),
					fixtype(cpexpr(dp->dims[i].dimexpr)));
		size = typesize[ q->vtype ];
		if(q->vtype == TYCHAR)
			if( ISICON(q->vleng) )
				size *= q->vleng->constblock.const.ci;
			else
				size = -1;

		/* on VAX, get more efficient subscripting if subscripts
		   have zero-base, so fudge the argument pointers for arrays.
		   Not done if array bounds are being checked.
		*/
		if(dp->basexpr)
			puteq( 	cpexpr(fixtype(dp->baseoffset)),
				cpexpr(fixtype(dp->basexpr)));

		if(! checksubs)
			{
			if(dp->basexpr)
				{
				if(size > 0)
					tp = (expptr) ICON(size);
				else
					tp = (expptr) cpexpr(q->vleng);
				putforce(TYINT,
					fixtype( mkexpr(OPSTAR, tp,
						cpexpr(dp->baseoffset)) ));
				p2pi("\tsubd\tr0,%d(fp)",
					p->datap->nameblock.vardesc.varno +
						ARGOFFSET);
				}
			else if(dp->baseoffset->constblock.const.ci != 0)
				{
				char buff[25];
				if(size > 0)
					{
					sprintf(buff, "\tsubd\t$%ld,%d(fp)",
						dp->baseoffset->constblock.const.ci * size,
						p->datap->nameblock.vardesc.varno +
							ARGOFFSET);
					}
				else	{
					putforce(TYINT, mkexpr(OPSTAR, cpexpr(dp->baseoffset),
						cpexpr(q->vleng) ));
					sprintf(buff, "\tsubd\tr0,%d(fp)",
						p->datap->nameblock.vardesc.varno +
							ARGOFFSET);
					}
				p2pass(buff);
				}
			}
		}
	}

if(typeaddr)
	puteq( cpexpr(typeaddr), mkaddcon(ep->typelabel) );
/* replace to avoid long jump problem
putgoto(ep->entrylabel);
*/
}




prhead(fp)
FILEP fp;
{
#if (!defined(FONEPASS)) && (FAMILY == PCC)
	p2triple(P2LBRACKET, ARGREG-highregvar, procno);
	p2word(0L);
	p2flush();
#endif
}



prdbginfo()
{
}

#ifdef SDB
prstab(name, val, stg, type)
char *name, *val;
int stg, type;
{
	char buf[BUFSIZ];

	if ((type & 0x0f) == T_ENUM)	/* got to be complex */
		sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t010;\t.tag\tComplex;\t.size\t8;\t.endef",
		    name, val, stg);
	else if ((type & 0x0f) == T_UCHAR)	/* got to be double complex */
		sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t010;\t.tag\tDcomplex;\t.size\t16;\t.endef",
		    name, val, stg);
	else
	   sprintf(buf,"\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t0x%x;\t.endef"
		, name, val, stg, type);
	p2pass(buf);
}


prarstab(name, val, stg, type, np)
char *name, *val;
int stg, type;
register Namep np;
{
	register int i, ts;
	char buf[BUFSIZ];

	sprintf(buf, "\t.def\t%s;\t.val\t%s-0x%x;\t.scl\t%d;\t.type\t",
	    name, val, np->vdim->baseoffset->constblock.const.ci *
	    (ts = typesize[(int) np->vtype]), stg);
	if ((i = type & 0x0f) == T_ENUM )
		strcat(buf, "010;\t.tag\tComplex;");
	else if (i == T_UCHAR)
		strcat(buf, "010;\t.tag\tDcomplex;");
	else
		sprintf(buf + strlen(buf), "0x%x;", type);
	sprintf(buf + strlen(buf), "\t.size\t0x%x;\t.dim\t",
	    np->vdim->nelt->constblock.const.ci * ts);
	for (i = 0; i != np->vdim->ndim; i++)
		sprintf(buf + strlen(buf), "%d,",
		    np->vdim->dims[i].dimsize->constblock.const.ci);
	sprintf(buf + strlen(buf) - 1, ";\t.endef");
	p2pass(buf);
}


prststab(name, val, stg, type, tag, size)
char *name, *val, *tag;
int stg, type, size;
{
	char buf[BUFSIZ];

	sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t%d;\t.tag\t%s;\t.size\t%d;\t.endef",
	    name, val, stg, type, tag, size);
	p2pass(buf);
}



stabtype(p)
register Namep p;
{
int d[7];		/* Type Word |d6|d5|d4|d3|d2|d1|typ|		 */
register int *pd;	/* Pointer to d. d fields = 2 bits, typ = 4 bits */
register int type;
register int i;

d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = 0;
pd = &d[1];

if (p->vtype == TYCOMPLEX)
	d[0] = T_ENUM;
else if (p->vtype == TYDCOMPLEX)
	d[0] = T_UCHAR;
else
	d[0] = types2[p->vtype];

/* For each dimension of an array, fill a d slot with 3 (array).
 * If array is argument, fill 1 less slot. Later on ptr value will be added.
 */
if (p->vdim) {
	i = (p->vstg == STGARG) ? 2 : 1;
	for (; i <= p->vdim->ndim; i++, pd++)
		*pd = 3;	/* 3 = array	*/
	}

if(p->vstg == STGARG)
	*pd++ = 1;		/* 1 = pointer	*/
if (p->vclass == CLPROC)
	*pd++ = 2;		/* 2 = function	*/

for (type = 0, pd--; pd > &d[0]; pd--)
	type = (type << 2) | *pd;
type = (type << 4) | d[0];
return(type);
}
#endif
