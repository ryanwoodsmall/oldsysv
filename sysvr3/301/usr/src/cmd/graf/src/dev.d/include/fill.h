/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/include/fill.h	1.4"
extern double fabs(),floor(),ceil(),tan(),cos();
extern double fmod(),sqrt(),atan2(),atan();
#define EQDOUB(x,y) (fabs(x-y) < 0.000001 ? 1 : 0)
#define BETWEEN(x,y,z) (x<MAX(y,z) && x>MIN(y,z))
#define INARC(x,y,z) (SUB(x,y) <= SUB(z,y) ? 1 : 0)
#define PI 3.141592653589793238
#define SUB(x,y) ((x-y)>=0 ? x-y : x-y+(2*PI))
#define ROUND(x) (x < 0 ? floor(x-.5) : floor(x+.5))
#define MAXPOINTS 100
#define BCKANG (0.78895)
#define REDANG (2.36690)
#define GRNANG (0.0)
#define BLUANG (1.57079)
#define SPACING (130.)
