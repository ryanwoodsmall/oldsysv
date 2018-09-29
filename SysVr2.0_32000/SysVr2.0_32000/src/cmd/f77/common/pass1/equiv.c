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
/*	@(#)equiv.c	1.5	*/
#include "defs"

#ifdef SDB
#	include <a.out.h>
#endif

/* ROUTINES RELATED TO EQUIVALENCE CLASS PROCESSING */

/* called at end of declarations section to process chains
   created by EQUIVALENCE statements
 */
doequiv()
{
register int i;
int inequiv, comno, ovarno;
ftnint comoffset, offset, leng;
register struct Equivblock *p;
register struct Eqvchain *q;
struct Primblock *itemp;
register Namep np;
expptr offp, suboffset();
int ns, nsubs();
chainp cp;
char *memname();
#ifdef SDB
#	if TARGET != PDP11
char eqvbuf[20];
char esvbuf[20];
char buff[BUFSIZ];
static int eqvnum = 0;
#	endif
#endif

for(i = 0 ; i < nequiv ; ++i)
	{
	p = &eqvclass[i];
	p->eqvbottom = p->eqvtop = 0;
	comno = -1;

	for(q = p->equivs ; q ; q = q->eqvnextp)
		{
		offset = 0;
		itemp = q->eqvitem.eqvlhs;
		vardcl(np = itemp->namep);
		if(itemp->argsp || itemp->fcharp)
			{
			if(np->vdim!=NULL && np->vdim->ndim>1 &&
			   nsubs(itemp->argsp)==1 )
				{
				if(! ftn66flag)
					warn("1-dim subscript in EQUIVALENCE");
				cp = NULL;
				ns = np->vdim->ndim;
				while(--ns > 0)
					cp = mkchain( ICON(1), cp);
				itemp->argsp->listp->nextp = cp;
				}

			offp = suboffset(itemp);
			if(ISICON(offp))
				offset = offp->constblock.const.ci;
			else	{
				dclerr("nonconstant subscript in equivalence ",
					np);
				np = NULL;
				}
			frexpr(offp);
			}
		frexpr(itemp);

		if(np && (leng = iarrlen(np))<0)
			{
			dclerr("adjustable in equivalence", np);
			np = NULL;
			}

		if(np) switch(np->vstg)
			{
			case STGUNKNOWN:
			case STGBSS:
			case STGEQUIV:
				break;

			case STGCOMMON:
				comno = np->vardesc.varno;
				comoffset = np->voffset + offset;
				break;

			case STGARG:
				dclerr("equivalence variable name same as subroutine or function argument", np);
				np = NULL;
				break;
			default:
				if (np->vclass == CLMAIN ||
				    np->vclass == CLENTRY ||
				    np->vclass == CLPROC)
					dclerr("equivalence variable name same as procedure or entry", np);
				else
					dclerr("bad storage class in equivalence", np);
				np = NULL;
				break;
			}

		if(np)
			{
			q->eqvoffset = offset;
			p->eqvbottom = lmin(p->eqvbottom, -offset);
			p->eqvtop = lmax(p->eqvtop, leng-offset);
			}
		q->eqvitem.eqvname = np;
		}

	if(comno >= 0)
		eqvcommon(p, comno, comoffset);
	else  for(q = p->equivs ; q ; q = q->eqvnextp)
		{
		if(np = q->eqvitem.eqvname)
			{
			inequiv = NO;
			if(np->vstg==STGEQUIV)
				if( (ovarno = np->vardesc.varno) == i)
					{
					if(np->voffset + q->eqvoffset != 0)
						dclerr("inconsistent equivalence", np);
					}
				else	{
					offset = np->voffset;
					inequiv = YES;
					}

			np->vstg = STGEQUIV;
			np->vardesc.varno = i;
			np->voffset = - q->eqvoffset;

			if(inequiv)
				eqveqv(i, ovarno, q->eqvoffset + offset);
			}
		}
	}

for(i = 0 ; i < nequiv ; ++i)
	{
	p = & eqvclass[i];
	if(p->eqvbottom!=0 || p->eqvtop!=0)	/* a live chain */
		{
#ifdef SDB
		if(sdbflag)
#	if TARGET == PDP11
			prstab(CNULL, N_BCOMM, 0, 0);
#	else
			{
			sprintf(eqvbuf, "EQV%d", eqvnum);
			sprintf(esvbuf, "ESV%d", eqvnum++);
			sprintf(buff,"\t.def\t%s;\t.scl\t10;\t.type\t010;\t.size\t%s;\t.endef",
				eqvbuf,esvbuf);
			p2pass(buff);
			}
#	endif
#endif
		for(q = p->equivs ; q && (np = q->eqvitem.eqvname) != NULL;
		    q = q->eqvnextp)
			{
			np->voffset -= p->eqvbottom;
			if(np->voffset % typealign[np->vtype] != 0)
				dclerr("bad alignment forced by equivalence", np);
#ifdef SDB
			if(sdbflag)
#if	TARGET == PDP11
				{
				prstssym(np);
				prstleng(np, iarrlen(np));
				}
#	else
				{
				prstab(nounder(XL, np->varname),
				   convic(np->voffset), 8, stabtype(np));
				}
#	endif
#endif
			}
		p->eqvtop -= p->eqvbottom;
		p->eqvbottom = 0;
#ifdef SDB
			if(sdbflag)
#	if TARGET == PDP11
				prstab(CNULL, N_ECOML, 0, memname(STGEQUIV,i) );
#	else
			{
			sprintf(buff,
			    "\t.def\t.eos;\t.val\t%d;\t.scl\t102;\t.tag\t%s;\t.endef",
			    p->eqvtop, eqvbuf);
			p2pass(buff);
			prststab(memname(STGEQUIV, i), "0", C_STAT, -1, eqvbuf,
			    p->eqvtop);
			sprintf(buff,"\t.set\t%s,%d",esvbuf,p->eqvtop);
			p2pass(buff);
			}
#	endif
#endif
		}
	freqchain(p);
	}
}





/* put equivalence chain p at common block comno + comoffset */

LOCAL eqvcommon(p, comno, comoffset)
struct Equivblock *p;
int comno;
ftnint comoffset;
{
int ovarno;
ftnint k, offq;
register Namep np;
register struct Eqvchain *q;
#ifdef SDB
char eqcbuf[20];
char escbuf[20];
char buff[BUFSIZ];
static int eqcnum = 0;
#endif

if(comoffset + p->eqvbottom < 0)
	{
	errstr("attempt to extend common %s backward",
		nounder(XL, extsymtab[comno].extname) );
	freqchain(p);
	return;
	}

if( (k = comoffset + p->eqvtop) > extsymtab[comno].extleng)
	extsymtab[comno].extleng = k;

#ifdef SDB
if(sdbflag)
#	if TARGET == PDP11
	prstab( varstr(XL,extsymtab[comno].extname), N_BCOMM,0,0);
#	else
	{
	sprintf(eqcbuf,"EQC%d",eqcnum);
	sprintf(escbuf,"ESC%d",eqcnum++);
	sprintf(buff,"\t.def\t%s;\t.scl\t10;\t.type\t010;\t.size\t%s;\t.endef",
		eqcbuf,escbuf);
	p2pass(buff);
	}
#	endif
#endif

for(q = p->equivs ; q ; q = q->eqvnextp)
	if(np = q->eqvitem.eqvname)
		{
		switch(np->vstg)
			{
			case STGUNKNOWN:
			case STGBSS:
				np->vstg = STGCOMMON;
				np->vardesc.varno = comno;
				np->voffset = comoffset - q->eqvoffset;
#ifdef SDB
				if(sdbflag)
#	if TARGET == PDP11
					{
					prstssym(np);
					prstleng(np, iarrlen(np));
					}
#	else
					{
					prstab(nounder(XL, np->varname),
					   convic(np->voffset), 8,
					   stabtype(np));
					}
#	endif
#endif
				break;

			case STGEQUIV:
				ovarno = np->vardesc.varno;
				offq = comoffset - q->eqvoffset - np->voffset;
				np->vstg = STGCOMMON;
				np->vardesc.varno = comno;
				np->voffset = comoffset - q->eqvoffset;
				if(ovarno != (p - eqvclass))
					eqvcommon(&eqvclass[ovarno], comno, offq);
				break;

			case STGCOMMON:
				if(comno != np->vardesc.varno ||
				   comoffset != np->voffset+q->eqvoffset)
					dclerr("inconsistent common usage", np);
				break;


			default:
				badstg("eqvcommon", np->vstg);
			}
		}

#ifdef SDB
if(sdbflag)
#	if TARGET == PDP11
	prstab( varstr(XL,extsymtab[comno].extname), N_ECOMM,0,0);
#	else
	{
	sprintf(buff, "\t.def\t.eos;\t.val\t%d;\t.scl\t102;\t.tag\t%s;\t.endef",
	    p->eqvtop, eqcbuf);
	p2pass(buff);
	prststab(!strncmp(extsymtab[comno].extname,"_BLNK__",7)?"_BLNK_":nounder(XL,extsymtab[comno].extname),
	    "0", C_STAT, -1, eqcbuf, p->eqvtop); 
	sprintf(buff,"\t.set\t%s,%d",escbuf,p->eqvtop);
	p2pass(buff);
	}
#	endif
#endif

freqchain(p);
p->eqvbottom = p->eqvtop = 0;
}


/* put all items on ovarno chain on front of nvarno chain
 * adjust offsets of ovarno elements and top and bottom of nvarno chain
 */

LOCAL eqveqv(nvarno, ovarno, delta)
int ovarno, nvarno;
ftnint delta;
{
register struct Equivblock *p0, *p;
register Namep np;
struct Eqvchain *q, *q1;

p0 = eqvclass + nvarno;
p = eqvclass + ovarno;
p0->eqvbottom = lmin(p0->eqvbottom, p->eqvbottom - delta);
p0->eqvtop = lmax(p0->eqvtop, p->eqvtop - delta);
p->eqvbottom = p->eqvtop = 0;

for(q = p->equivs ; q ; q = q1)
	{
	q1 = q->eqvnextp;
	if( (np = q->eqvitem.eqvname) && np->vardesc.varno==ovarno)
		{
		q->eqvnextp = p0->equivs;
		p0->equivs = q;
		q->eqvoffset -= delta;
		np->vardesc.varno = nvarno;
		np->voffset -= delta;
		}
	else	ckfree( (ptr) q);
	}
p->equivs = NULL;
}




LOCAL freqchain(p)
register struct Equivblock *p;
{
register struct Eqvchain *q, *oq;

for(q = p->equivs ; q ; q = oq)
	{
	oq = q->eqvnextp;
	ckfree( (ptr) q);
	}
p->equivs = NULL;
}





LOCAL nsubs(p)
register struct Listblock *p;
{
register int n;
register chainp q;

n = 0;
if(p)
	for(q = p->listp ; q ; q = q->nextp)
		++n;

return(n);
}
