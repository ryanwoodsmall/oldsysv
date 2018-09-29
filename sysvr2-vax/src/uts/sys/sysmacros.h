/* @(#)sysmacros.h	6.2 */
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

/* Core clicks to disk blocks */
#define	ctod(x) ((x+(NCPD-1))/NCPD)

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

#define invsatb(r, v)	mtpr(TBIS, v)
#define SYSATB		0
#define USRATB		1

#define clratb(v)	mtpr(TBIA, 0)
#define vtop(a, b) (pte_t *)((pte_t *)baseseg[a] + (b))
#define ublkptaddr(x)	vtop(x->p_ptaddr, x->p_ptsize*NPGPT - USIZE)
#define	critical(ps)	((ps & PS_IPL) != 0)
#define decay(a,b,c) a=((int)((a)*((c)-1)+(b)))/(c);
#define poff(X)   ((uint)(X) & POFFMASK)		/* page offset */
#define soff(X)   ((uint)(X) & SOFFMASK)		/* segment offset */
#define snum(X)	(((uint)(X)>>16) & 0x3fff)		/* segment number */
#define mkpte(x, y)	(x | y)

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
#define calcppri(p) { \
		register int a; \
 \
		a = ((p->p_cpu + p->p_frate)>>1) + \
			p->p_nice + (PUSER-NZERO); \
		if (a > PMAX) a = PMAX; \
		p->p_pri = a; \
		curpri = p->p_pri; \
	}
#define resetpri(p, o)	calcppri(p)

/*
 * Increment page use count
 */
#define	MAXMUSE		32000	/* Maximum share count on a page	*/
#define pfdinc(pfd, s)		if (pfd->pf_use++ == MAXMUSE) \
				panic(s)
