/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/fm_mn_par.h	1.5"

extern	char *fld_eval();

#define	sing_eval(a,b)		fld_eval(&(a)->single, b, (a)->seqno)
#define	multi_eval(a,b,c)	fld_eval((a)->multi + b, c, (a)->seqno)

#define KEYWORDSIZE	14

#define VAL_CALC	-1
#define CMD		32

/* Possible return types for attribute. */

#define RET_INT		0x1
#define RET_STR		0x2
#define RET_BOOL	0x3
#define RET_LIST	0x4
#define RET_ARGS	0x5
#define RET_PATH	0x40
#define EVAL_ONCE	0x80
#define EVAL_ALWAYS	0x100
#define EVAL_SOMETIMES	0x200
#define FREEIT		0x400
#define MAKE_COPY	0x800
#define MENU_MARKED	0x1000
#define MENU_CHECKED	0x2000
#define RETS		7

#define INLINE		1

struct attribute {
	char *testring;
	int flags;
	char *def;
	char *cur;
	unsigned int seqno;
};

struct fld {
	struct attribute **attrs;
};

struct fm_mn {
	unsigned int seqno;
	struct fld single;
	struct fld *multi;
};
