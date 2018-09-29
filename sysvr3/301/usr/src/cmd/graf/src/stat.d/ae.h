/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/ae.h	1.3"
#define OPR 0177
#define OPN ~0377
#define IDENT 0400
#define CONST 01400
#define UNKNOWN 0200
struct operand {
	FILE *fd;
	char name[14];
	double val;
};
struct p_entry {
	int opr;
	struct operand *opn;
};
