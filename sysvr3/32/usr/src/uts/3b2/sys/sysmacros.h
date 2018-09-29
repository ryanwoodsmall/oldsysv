/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sysmacros.h	10.5.4.4"
/*
 * Some macros for units conversion
 */

/* Core clicks to segments and vice versa */

#define ctos(x)		(((x) + (NCPS-1)) >> CPSSHIFT)
#define	ctost(x)	((x) >> CPSSHIFT)
#define	stoc(x)		((x) * NCPS)

/* byte address to segment and vice versa  */
#define sgnum(x)	(((unsigned)(x) >> 17) & 0x1fff)
#define stob(x)		((((unsigned)(x)) & 0x1fff) << 17)
#define	btos(x)		((unsigned)(x) >> 17)
#define secnum(x)	((unsigned)(x) >> 30)

/* Core clicks to immu max offset and vice versa */
#define ctomo(x)	((x) * 256 -1)
#define motoc(x)	((((x)+1)*8) >> BPCSHIFT)
#define motob(x)	(8*((x)+1)-1)

/* Core clicks to disk blocks */
#define	ctod(x) ((x)*NDPC)

/* inumber to disk address */
#ifdef INOSHIFT
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)
#else
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))/INOPB)
#endif

/* inumber to disk offset */
#ifdef INOSHIFT
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))
#else
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))%INOPB)
#endif

/* clicks to bytes */
#ifdef BPCSHIFT
#define	ctob(x)	((x)<<BPCSHIFT)
#else
#define	ctob(x)	((x)*NBPC)
#endif

/* bytes to clicks */
#ifdef BPCSHIFT
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))>>BPCSHIFT)
#define	btoct(x)	((unsigned)(x)>>BPCSHIFT)
#else
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))/NBPC)
#define	btoct(x)	((unsigned)(x)/NBPC)
#endif

#ifdef INKERNEL

/* major part of a device internal to the kernel */

extern char MAJOR[128];
#define	major(x)	(int)(MAJOR[(unsigned)((x)>>8)&0x7F])
#define	bmajor(x)	(int)(MAJOR[(unsigned)((x)>>8)&0x7F])

/* minor part of a device internal to the kernel */
extern char MINOR[128];
#define	minor(x)	(int)(MINOR[(unsigned)((x)>>8)&0x7F]+((x)&0xFF))

#else

/* major part of a device external from the kernel */
#define	major(x)	(int)(((unsigned)x>>8)&0x7F)

/* minor part of a device external from the kernel */
#define	minor(x)	(int)(x&0xFF)
#endif	/* INKERNEL */

/* make a device number */
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))

/*
 *   emajor() allows kernel/driver code to print external major numbers
 *   eminor() allows kernel/driver code to print external minor numbers
 */

#define emajor(x)	(int)(((unsigned)(x)>>8)&0x7F)
#define eminor(x)	(int)((x)&0xFF)

/*	Calculate user process priority.
*/

#define calcppri(p)	((p->p_cpu) >> 1) +  p->p_nice + (PUSER - NZERO)

/*
 *  Evaluate to true if the process is a server - Distributed UNIX
 */
#define	server()	(u.u_procp->p_sysid != 0)

/*
 * Defined for RFS client caching
 */

extern int rcacheinit;			/* RFS client caching flag */
extern unsigned long rfs_vcode;		/* version code for RFS caching */

#define CLOSEI(ip, flag, count, offset) \
{ \
	if (rcacheinit && (ip)->i_ftype == IFREG && (count) == 1 \
		&& !server() && (flag) & FWRITE) { \
		(ip)->i_wcnt--; \
		if ((ip)->i_flag & IWROTE && (ip)->i_wcnt == 0) { \
			(ip)->i_flag &= ~IWROTE; \
			if ((ip)->i_rcvd) \
				enable_cache((ip)->i_rcvd); \
		} \
	} \
	FS_CLOSEI(ip, flag, count, offset); \
}

#define OPENI(ip, mode) \
{ \
	if (rcacheinit && (ip)->i_ftype == IFREG && !server() && (mode) & FWRITE) \
		(ip)->i_wcnt++; \
	FS_OPENI(ip, mode); \
}

#define WRITEI(ip) \
{ \
	if (rcacheinit && (ip)->i_ftype == IFREG) { \
		if (!server()) \
			(ip)->i_flag |= IWROTE; \
		(ip)->i_vcode = ++rfs_vcode; \
		if ((ip)->i_rcvd) \
			disable_cache(ip); \
	} \
	FS_WRITEI(ip); \
}

#define ITRUNC(ip) \
{ \
	if (rcacheinit && (ip)->i_ftype == IFREG) { \
		if (!server()) \
			(ip)->i_flag |= IWROTE; \
		(ip)->i_vcode = ++rfs_vcode; \
		if ((ip)->i_rcvd) \
			disable_cache(ip); \
	} \
	FS_ITRUNC(ip); \
}

#define IPUT(ip) \
{ \
	if (rcacheinit && (ip)->i_nlink <= 0 && (ip)->i_count == 1) { \
		(ip)->i_vcode = ++rfs_vcode; \
	} \
	FS_IPUT(ip); \
}

#define FREESP(ip, arg, flag, offset) \
{ \
	if (rcacheinit && (ip)->i_ftype == IFREG) { \
		if (!server()) \
			(ip)->i_flag |= IWROTE; \
		(ip)->i_vcode = ++rfs_vcode; \
		if ((ip)->i_rcvd) \
			disable_cache(ip); \
	} \
	FS_FCNTL(ip, F_FREESP, arg, flag, offset); \
}


/* machine dependent operations - defined for RFS and STREAMS */

#ifdef	pdp11
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+1) & ~1)
#define LALIGN(p)		(char *)(((int)p+1) & ~3)
#endif
#ifdef	vax
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+3) & ~3)
#define	LALIGN(p)		(char *)(((int)p+3) & ~3)
#endif
#ifdef	u3b2
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+3) & ~3)
#define	LALIGN(p)		(char *)(((int)p+3) & ~3)
#endif

#define SNEXT(p)		(char *)((int)p + sizeof (short))
#define INEXT(p)		(char *)((int)p + sizeof (int))
#define LNEXT(p)		(char *)((int)p + sizeof (long))

