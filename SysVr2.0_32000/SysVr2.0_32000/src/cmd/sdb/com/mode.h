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
	/* @(#) mode.h: 1.6 5/17/83 */

/*
 *	UNIX debugger
 */

#define MAXCOM	128
#define MAXARG	32
#define LINSIZ	256

#define TYPE	typedef
#define REG	register

TYPE	unsigned long	ADDR;
TYPE	unsigned long	POS;
TYPE	int	INT;
TYPE	int	VOID;
TYPE	long	L_INT;
/*	TYPE	float		REAL;		not used */
/*	TYPE	double		L_REAL;		not used */
TYPE	char		BOOL;
TYPE	char		CHAR;
TYPE	char		*STRING;
TYPE	char		MSG[];
TYPE	struct map	MAP;
TYPE	MAP		*MAPPTR;
TYPE	struct bkpt	BKPT;
TYPE	BKPT		*BKPTR;

/* file address maps */
struct map {
	ADDR	b1;	/* beginning (internal) address */
	ADDR	e1;	/* ending (internal) address	*/
	ADDR	f1;	/* file address corresponding to b1 */
	ADDR	b2;	/* beginning (internal) address) */
	ADDR	e2;	/* ending (internal) address	*/
	ADDR	f2;	/* file address corresponding to b2 */
	INT	ufd;	/* UNIX file descriptor		*/
};
struct bkpt {
	ADDR	loc;	/* location of breakpoint	*/
	ADDR	ins;	/* instruction at breakpoint	*/
	INT	count;	/* not used ?? 		*/
	INT	initcnt;/* not used ?? 		*/
	INT	flag;	/* non-zero if breakpoint inserted */
	CHAR	comm[MAXCOM];	/* ASCII user command at breakpoint */
	BKPT	*nxtbkpt;	/* next breakpoint in sdb table	*/
};

TYPE	struct reglist	REGLIST;
TYPE	REGLIST		*REGPTR;
struct reglist {
	STRING	rname;	/* name of register (AP, R0, etc.)	*/
	INT	roffs;	/* reg offset (ints) from R0 in user area */
};

#ifdef ns32000
#define	EOR	'\n'
#endif
