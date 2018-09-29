/* @(#)sysmacros.h	1.3 */
/*
 * Some macros for units conversion
 */
/* Core clicks to segments and vice versa */
#define ctos(x) ((x+(NCPS-1))/NCPS)
#define	stoc(x) ((x)*NCPS)

/* Core clicks to disk blocks */
#define	ctod(x) ((x+(NCPD-1))/NCPD)

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

/* major part of a device */
#define	major(x)	(int)((unsigned)x>>8)
#define	bmajor(x)	(int)(((unsigned)x>>8)&037)
#define	brdev(x)	(x&0x1fff)

/* minor part of a device */
#define	minor(x)	(int)(x&0377)

/* make a device number */
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))
