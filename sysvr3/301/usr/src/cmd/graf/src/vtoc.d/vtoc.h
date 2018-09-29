/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/vtoc.h	1.1"
/* <: t-5 d :> */
#include "ctype.h"
#include <stdio.h>
#include "setop.h"
#include "util.h"
#include "errpr.h"
#include "debug.h"
#include <sys/types.h>
/*
 *  vtoc.h defines the node structure of the tree
 *  and sets values of constants
 */

#define  ID 0
#define  LS 1
#define  TEXT 2
#define  MARK 3
#define NAMELEN	256
#define TXSZ 100
#define IDTXSZ 75
#define MAXKID MAXNODE-1
#define MAXNODE 500
#define GOLDRAT 1.62	/*  golden ratio of box length to box height */
#define XYRAT .571		/*  length to height ratio(XUNIT/YUNIT)  */
#define XUNIT(sz)	(11.2/14 * sz)		/*  char width plus hor space */
#define YUNIT(sz)	(1.3 * sz)		/*  line height plus vert space */
struct node {
	int seq;				/* sequence number of node */
	int level;			/* depth in tree */
	char *id;				/* id field */
	char weight;			/* line weight of box */
	char style;			/* line style of box */
	char *text;			/* text in box */
	char *mark;			/* mark field of box */
	int numkid;			/* number of children */
	struct node **kid;		/* pointer to an array of children */
	struct node *par;		/* pointer to parent */
	int alw;				/* allowance of node and all its children */
	int rx;				/* relative horizontal coordinate */
	double vx;			/* vertual horizontal coordinate */
	double vy;			/* vertual vertical coordinate */
};

#define MAXLENGTH 256
