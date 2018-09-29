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
/* @(#)sysmacros.h	1.2 */
/*
 * Some macros for units conversion
 */
/* Core clicks to segments and vice versa */
#define ctos(x) ((x+(NCPS-1))/NCPS)
#define	stoc(x) ((x)*NCPS)
#define ptos(X)   ((X+(NCPS-1))/NCPS)	/* pages to segments */
#define	ptots(X)  ((X)/NCPS)		/* pages to truncated segments */
#define	stopg(x) ((x)*NCPS)		/* segments to pages */
#define	stob(x) ((x) << 16)		/* segments to bytes */
#define btos(x) ((x+(NBPS-1)) >> 16)		/* bytes to segments */
#define btots(x) ((x) >> 16)		/* bytes to truncated segments */

/* bytes to disk blocks */
#define btod(x)	((x+(BSIZE-1)) >> BSHIFT)
#define dtob(x)	((x) << BSHIFT)
#define dtop(X)	(((X) + (NDPP-1)) >> DPPSHFT)
#define ptod(X)	((X) << DPPSHFT)

/* clicks to bytes */
#ifdef BPCSHIFT
#define	ctob(x)	((x)<<BPCSHIFT)
#define	ptob(x)	((x)<<BPPSHIFT)
#else
#define	ctob(x)	((x)*NBPC)
#endif

/* bytes to clicks */
#ifdef BPCSHIFT
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))>>BPCSHIFT)
#define	btop(x)	(((unsigned)(x)+(NBPP-1))>>BPPSHIFT)
#define	btotp(x)	((unsigned)(x)>>BPPSHIFT)
#else
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))/NBPC)
#define	btoct(x)	((unsigned)(x)/NBPC)
#endif

/* inumber to disk address */
#ifdef INOSHIFT
#define	itod(x)	(daddr_t)(((unsigned)x+(2*INOPB-1))>>INOSHIFT)
#else
#define	itod(x)	(daddr_t)(((unsigned)x+(2*INOPB-1))/INOPB)
#endif

/* inumber to disk offset */
#ifdef INOSHIFT
#define	itoo(x)	(int)(((unsigned)x+(2*INOPB-1))&(INOPB-1))
#else
#define	itoo(x)	(int)(((unsigned)x+(2*INOPB-1))%INOPB)
#endif

/* major part of a device */
#define	major(x)	(int)((unsigned)x>>8)
#define	bmajor(x)	(int)(((unsigned)x>>8)&037)
#define	brdev(x)	(x&0x1fff)

/* minor part of a device */
#define	minor(x)	(int)(x&0377)

/* make a device number */
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))

/*
 * misc macros
 */

#define clratb(v)	lmr(PTB1,ptob(u.u_procp->p_addr))
			/* NOTE: clratb(SYSATB) will not work. use lmr */
#define SYSATB	0
#define USRATB	1

#define	critical(ps)	((ps & PS_IPL) != 0)
#define decay(a,b,c) a=((int)((a)*((c)-1)+(b)))/(c);
#define poff(X)   ((uint)(X) & POFFMASK)		/* page offset */
#define soff(X)   ((uint)(X) & SOFFMASK)		/* segment offset */
#define snum(X)	(((uint)(X)>>16) & 0x3fff)		/* segment number */
#define mkpte(x,y)	(ptob(y)|x)

/*
 * Check page bounds
 */
#define pfdrnge(pfd, s)	if ((pfd-pfdat) < firstfree || (pfd-pfdat) >= maxfree){ \
	printf("bad range %d %d %d\n", pfd - pfdat, firstfree, maxfree);\
	panic(s);}
/*
 * Calculate user priority (Performance)
 */
#define PMAX	127
#define calcppri(p,a) { \
		a = ((p->p_cpu + p->p_frate)>>1) + \
			p->p_nice + (PUSER-NZERO); \
		if (a > PMAX) a = PMAX; \
		p->p_pri = a; \
		curpri = p->p_pri; \
	}

/*
 * Increment page use count
 */
#define	MAXMUSE		32000	/* Maximum share count on a page	*/
#define pfdinc(pfd, s)		if (pfd->pf_use++ == MAXMUSE) \
				panic(s)
