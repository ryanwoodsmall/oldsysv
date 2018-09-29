/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/expand.h	1.4"
/*
 */

#define L_SDI	0	/* a long sdi */
#define S_SDI	1	/* a short sdi */
#define U_SDI	2	/* an sdi of unknown size */

/* structure for a sdi selection set entry */

typedef struct {
	unsigned short sdicnt;	/* this is the Nth sdi */
	short itype;	/* type of this sdi */
	long minaddr;	/* minimum address for this sdi */
	short maxaddr;	/* difference between max. addr. and min. addr. */
	symbol *labptr;	/* ptr to label appearing in the operand */
	long constant;	/* value of constant appearing in operand */
	short sectnum;	/* number of section where sdi appears */
} ssentry;

/* structure that contains the span range for each sdi */

typedef struct {
	long lbound;	/* lower bound of sdi range */
	long ubound;	/* upper bound of sdi range */
} rangetag;
