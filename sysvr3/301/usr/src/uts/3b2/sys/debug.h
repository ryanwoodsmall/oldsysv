/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/debug.h	10.4"

#define	YES 1
#define	NO  0

#if DEBUG == YES
#define ASSERT(EX) if (!(EX))assfail("EX", __FILE__, __LINE__)
#else
#define ASSERT(x)
#endif

#ifdef MONITOR
#define MONITOR(id, w1, w2, w3, w4) monitor(id, w1, w2, w3, w4)
#else
#define MONITOR(id, w1, w2, w3, w4)
#endif

