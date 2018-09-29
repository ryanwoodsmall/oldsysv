/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/lla_ppc.h	10.1"
/*
 * The following are general-purpose macros for accessing the
 * various queues, elements, and fields of the PPC CIO interface
 */

#define R_QUEUE(b)	(pp_board[b].rqueue)
#define C_QUEUE(b)	(pp_board[b].cqueue)

#define E_BYTCNT(e)	(e).common.codes.bytes.bytcnt
#define E_CMDSTAT(e)	(e).common.codes.bits.cmd_stat
#define E_SEQBIT(e)	(e).common.codes.bits.seqbit
#define E_SUBDEV(e)	(e).common.codes.bits.subdev
#define E_OPCODE(e)	(e).common.codes.bytes.opcode
#define E_ADDR(e)	(e).common.addr
#define E_APPL(e,i)	(e).appl.pc[i]

#define EP_BYTCNT(e)	(e)->common.codes.bytes.bytcnt
#define EP_CMDSTAT(e)	(e)->common.codes.bits.cmd_stat
#define EP_SEQBIT(e)	(e)->common.codes.bits.seqbit
#define EP_SUBDEV(e)	(e)->common.codes.bits.subdev
#define EP_OPCODE(e)	(e)->common.codes.bytes.opcode
#define EP_ADDR(e)	(e)->common.addr
#define EP_APPL(e,i)	(e)->appl.pc[i]

#define Q_PTRS(q)	(q).p_queues.all
#define Q_LOAD(q)	(q).p_queues.bit16.load
#define Q_ULOAD(q)	(q).p_queues.bit16.unload

#define RQPTRS(b,i)	Q_PTRS(R_QUEUE(b).queue[i])
#define RQLOAD(b,i)	Q_LOAD(R_QUEUE(b).queue[i])
#define RQULOAD(b,i)	Q_ULOAD(R_QUEUE(b).queue[i])

#define CQPTRS(b)	Q_PTRS(C_QUEUE(b).queue)
#define CQLOAD(b)	Q_LOAD(C_QUEUE(b).queue)
#define CQULOAD(b)	Q_ULOAD(C_QUEUE(b).queue)

#define RQEXPRESS(b)	R_QUEUE(b).express
#define CQEXPRESS(b)	C_QUEUE(b).express

#define RQENTRY(b,i,j)	R_QUEUE(b).queue[i].entry[j]
#define CQENTRY(b,j)	C_QUEUE(b).queue.entry[j]

/*
 * Several of the LLA functions are implemented as mappings into
 * common code
 */

#define TRUE	1
#define FALSE	0

/*
 * The following are internal macros used only by the ppc lla
 * functions
 */

#define SG_PTR	((long *) DBLK_PTR)
