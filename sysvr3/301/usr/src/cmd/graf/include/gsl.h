/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:include/gsl.h	1.3"
#define XYMIN      (-32767)
#define XYMID      0
#define XYMAX      32767
#define XDIM       13107
#define YDIM       13107
#define FCH        '\037'

#define INVISIBLE	0
#define NARROW     1
#define MEDIUM     2
#define BOLD       4

#define SOLID      0
#define DOTTED     1
#define DOTDASH    2
#define DASHED     3
#define LONGDASH   4

#define BLACK	0
#define BLUE	1
#define GREEN	2
#define RED	4
#define FILL	0770	/* was 255 */

#define DFLTCOLOR      BLACK
#define DFLTFONT       16	/* NARROW, SOLID */
#define X0Y0       (XYMIN +1)

struct gslparm {
	FILE *fp;
	double x0,y0,x,y,xs,ys,ux,uy;
};
struct area{
	int lx,ly,hx,hy;
};
