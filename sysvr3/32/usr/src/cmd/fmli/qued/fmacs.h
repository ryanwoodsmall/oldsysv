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

#ident	"@(#)vm.qued:src/qued/fmacs.h	1.3"

/* defines for Substreams */
#define SINGLE	0
#define MULTI	1

/* Miscellaneous defines and macros */
#define Flags		(Cfld->flags)
#define Fieldattr	(Cfld->fieldattr)
#define Currtype	(Cfld->currtype)
#define Scrollbuf	(Cfld->scrollbuf)
#define Buffoffset	(Cfld->buffoffset)
#define Buffsize	(Cfld->buffsize)
#define Bufflast	(Cfld->bufflast)
#define Value		(Cfld->value)
#define Valptr		(Cfld->valptr)

#define Fbufsize	((Cfld->rows * Cfld->cols) + Cfld->rows + 1)
#define Lastcol		(Cfld->cols - 1)
#define Lastrow		(Cfld->rows - 1)

/* field character operation macros */
#define	freadchar(r,c)	wreadchar(r+Cfld->frow,c+Cfld->fcol)
#define fputchar(x)	wputchar(x, Fieldattr);

/*
 * Defines for scrolling fields 
 */
extern int Pagelines;

#define LINESINFIELD	(Cfld->rows)
#define LINESIZE	(Cfld->cols + 1)
#define PAGEBYTES	(Pagelines * LINESIZE)
#define FIELDBYTES	(LINESINFIELD * LINESIZE)
#define LASTPAGE	(LINESINFIELD - Pagelines)

#define UP	0
#define DOWN	1
#define LEFT	2
#define RIGHT	3
