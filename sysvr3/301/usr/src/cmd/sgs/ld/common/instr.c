/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/instr.c	1.6"

/*
 * Define various statistic-collection variables used to monitor the
 * internal workings of ld
 *
 *
 * Statistics are collected if the compile time define PERF_DATA
 * is turned on.  Output of these variables
 * is controlled by the presence of the UN*X file "instr.data". If
 * this file exists and can be opened for "append", these variables
 * are output.  Output occurs only on complete, successful of ld.
 */

#if PERF_DATA
long	nwalks = 0L,		/* number of collision chain walks  */
	nfwalks = 0L,		/* number of unsuccessful walks	    */
	ncolisns = 0L,		/* number of hashtab[] collisions   */
	maxchain = 0L;		/* max length of a collision chain  */

long	ttime;			/* total elapsed time		    */
struct tbuffer {		/* process and child execution	    */
	long proc_user_time;	/*	times, as returned by the   */
	long proc_system_time;	/*	times() function	    */
	long child_user_time;
	long child_system_time;
	} ptimes;
#endif
