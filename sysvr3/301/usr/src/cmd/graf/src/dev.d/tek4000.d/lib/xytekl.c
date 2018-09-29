/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/xytekl.c	1.5"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"

#if u370 | u3b | u3b5 | u3b2

union addrbits{
	int exword;
	struct pos{
		unsigned int dummy:16;
		unsigned int fill:9;
		unsigned int tagb7:1;
		unsigned int tagb6:1;
		unsigned int margb5:1;
		unsigned int y2b4:1;
		unsigned int y1b3:1;
		unsigned int x2b2:1;
		unsigned int x1b1:1;
	}bt;
};

#else

union addrbits{
	int exword;
	struct pos{
		unsigned int x1b1:1;
		unsigned int x2b2:1;
		unsigned int y1b3:1;
		unsigned int y2b4:1;
		unsigned int margb5:1;
		unsigned int tagb6:1;
		unsigned int tagb7:1;
		unsigned int fill:9;
	}bt;
};

#endif

xytekl(sx,sy,tkptr,exbyte)
int sx,sy,*exbyte;
struct tekaddr *tkptr;
{	union addrbits ab;

	sx *= 4; sy *= 4;  /*  4096 space  */
	ab.bt.fill=0;
	ab.bt.tagb7=ab.bt.tagb6=1;
	if(sx>=512*4) ab.bt.margb5=1;
	else ab.bt.margb5=0;
	tkptr->yh = ((sy >> 7) & 037) + 040;
	tkptr->yl = ((sy >> 2) & 037) + 0140;
	ab.bt.y2b4=((sy >> 1) & 01);
	ab.bt.y1b3=(sy & 01);
	tkptr->xh = ((sx >> 7) & 037) + 040;
	tkptr->xl = ((sx >> 2) & 037) + 0100;
	ab.bt.x2b2=((sx >> 1) & 01);
	ab.bt.x1b1=(sx & 01);
	*exbyte = ab.exword;
	return;
}
