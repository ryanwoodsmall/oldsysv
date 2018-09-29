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
/*	@(#)acctdef.h	1.6 of 3/3/83	*/
/*
 *	defines, typedefs, etc. used by acct programs
 */


/*
 *	acct only typedefs
 */
typedef	unsigned short	uid_t;

#if u3b || u3b5
#define HZ	100
#else
#define HZ	60
#endif

#define LSZ	12	/* sizeof line name */
#define NSZ	8	/* sizeof login name */
#define P	0	/* prime time */
#define NP	1	/* nonprime time */

/*
 *	limits which may have to be increased if systems get larger
 */
#define A_SSIZE	1000	/* max number of sessions in 1 acct run */
#define A_TSIZE	100	/* max number of line names in 1 acct run */
#define A_USIZE	500	/* max number of distinct login names in 1 acct run */

#define EQN(s1, s2)	(strncmp(s1, s2, sizeof(s1)) == 0)
#define CPYN(s1, s2)	strncpy(s1, s2, sizeof(s1))

#define SECSINDAY	86400L
#define SECS(tics)	((double) tics)/HZ
#define MINS(secs)	((double) secs)/60
#define MINT(tics)	((double) tics)/(60*HZ)

#ifdef pdp11
#define KCORE(clicks)	((double) clicks/16)
#endif
#if vax || ns32000
#define KCORE(clicks)	((double) clicks/2)
#endif
#if u3b || u3b5
#define KCORE(clicks)	((double) clicks*2)
#endif
