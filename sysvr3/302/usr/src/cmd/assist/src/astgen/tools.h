/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:tools.h	1.2"

#define CTRL(q) ('q' & 037)
#define CTRL(l) ('l' & 037)
#define CTRL(k) ('k' & 037)
#define CTRL(b) ('b' & 037)
#define CTRL(t) ('t' & 037)
#define CTRL(e) ('e' & 037)
#define CTRL(d) ('d' & 037)
#define CTRL(u) ('u' & 037)
#define CTRL(r) ('r' & 037)
#define	MAXLINE	1000
#define	MAXWORD	80
#define	ESC	'\033'
#define	BS	'\010'

struct labels {
	int xlab; /*x coord of label*/
	int ylab; /*y coord of label*/
	int xval; /*x coord of user input area*/
	char *name; /*string to present*/
};
