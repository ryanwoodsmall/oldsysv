/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/lla.h	10.1"
#define RULOAD(QUE)	R_ADDR.queue[QUE].p_queues.bit16.unload
#define RLOAD(QUE)	R_ADDR.queue[QUE].p_queues.bit16.load
#define CULOAD	C_ADDR.queue.p_queues.bit16.unload
#define CLOAD	C_ADDR.queue.p_queues.bit16.load

#define RQEMPTY(QUE)	(RLOAD(QUE) == RULOAD(QUE))

#define RQFULL(QUE)	(((RULOAD(QUE)/sizeof(RENTRY) + RQSIZE - RLOAD(QUE)/sizeof(RENTRY)) % RQSIZE) ==  1)

#define CQEMPTY	(CULOAD == CLOAD)

#define NEW_RLPTR(QUE)	((1 + (RLOAD(QUE)/sizeof(RENTRY))) % RQSIZE)

#define NEW_CUPTR	((1 + CULOAD/sizeof(CENTRY)) % CQSIZE)
