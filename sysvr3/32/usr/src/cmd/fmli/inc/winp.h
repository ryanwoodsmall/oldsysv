/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)vm.inc:src/inc/winp.h	1.2"

#define MAXSUBS	5

/*	For possible "undo" operation enhancement 
typedef struct _lastop {
	token tok;
	int oprow;
	int opcol;
	int count;
	char *str;
} undo;
*/

typedef struct _inp {
	int frow;		/* first row of field within window */
	int fcol;		/* first column of field within window */
	int rows;		/* number of rows in field */
	int cols;		/* number of cols in field */
	int currow;		/* current row within field */
	int curcol;		/* current column within field */
	int flags;		/* see field flags below */
	int fieldattr;		/* highlight attribute (i.e., underlined) */
	int currtype;		/* used to indicate proper (sub)stream */ 
	char *scrollbuf;	/* buffer of scrolled lines */
	unsigned buffoffset;	/* offset of visible window in scroll buffer */
	unsigned buffsize;	/* total size of the scroll buffer */
	unsigned bufflast;	/* last valid char/line in scroll buffer */
	char *value;		/* present field value (since last sync)    */
	char *valptr;		/* current offset into the value string;    * 
				 * used primarily for scrolling fields to   *
				 * point to that part of the value string   *
				 * that has not been part of the visible    *
				 * scroll window, (i.e., not yet part of    *
				 * scrollbuf)				    */
} ifield;

/* field flags */
#define	I_NOEDIT	0x0001
#define I_CHANGED	0x0002
#define I_STRIPLEAD	0x0008
#define I_INSERT	0x0010
#define I_FULLWIN	0x0020
#define I_WRAP		0x0040
#define I_BLANK		0x0080
#define I_FANCY		0x0100
#define I_SCROLL	0x0200
#define I_FILL		0x0400
#define I_INVISIBLE	0x0800
#define	I_CHANGEABLE	~(I_CHANGED)

extern ifield *Cfld;	/* current field */
