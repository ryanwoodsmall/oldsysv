/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/gate.h	10.1"
/*
 *  Gate table descriptions
 */

typedef struct gate_l2 * gate_l1;	/* level 1 gate table entry */

struct gate_l2				/* level 2 gate table entry */
	{
	psw_t psw;
	int (*pc)();
	};
