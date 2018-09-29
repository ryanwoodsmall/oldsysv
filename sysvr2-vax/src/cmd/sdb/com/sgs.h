	/* @(#) sgs.h: 1.11 2/27/84 */

#if vax
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)VAXROMAGIC) || \
			  (((unsigned short)x)==(unsigned short)VAXWRMAGIC))

/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default : readonly sharable text segment
 *		AOUT2MAGIC :	     : writable text segment
 *		AOUT3MAGIC :	     : Paging magic #.
 */

#define AOUT1MAGIC	0410
#define AOUT2MAGIC	0407
#define	AOUT3MAGIC	0413	/* Paging aout header magic number. */

#endif
#if u3b || u3b5 || u3b2
#if u3b
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)N3BMAGIC) || \
			(((unsigned short)x)==(unsigned short)NTVMAGIC))
#else
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)FBOMAGIC) || \
			(((unsigned short)x)==(unsigned short)MTVMAGIC))
#endif

/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear on that header:
 *
 *		AOUT1MAGIC : default : combined text and data segments
 *		AOUT2MAGIC :         : separate text and data segments
 */

#define AOUT1MAGIC	0407
#define AOUT2MAGIC	0410
#define	AOUT3MAGIC	0413	/* Paging aout header magic number. */
#endif

#define	SGSNAME	""
#define SGS ""
#define RELEASE "Release 5.0 6/1/82"
