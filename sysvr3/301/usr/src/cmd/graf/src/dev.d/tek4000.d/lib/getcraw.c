/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/getcraw.c	1.3"
#include <stdio.h>
#include "tek.h"
extern struct tekterm tm;
getcraw() /* getchar, raw mode */
{
	char ch;

	ioctl(0,TCSETA,&tm.raw);
	fread(&ch,1,1,stdin);
	ioctl(0,TCSETA,&tm.cook);
	return(ch&0177);
}

putcraw(ch,x,y) /* put ch at screen coords x,y */
char ch;
int x,y;
{
	int ox, oy;

	getacur(&ox,&oy);
	locacur((x>1?x-2:x),(y>2?y-3:y));
	putchar(ch);
	locacur(ox,oy);
}
putdraw(i,x,y) /* put number i at screen x,y */
int i, x, y;
{
	int ox, oy;

	getacur(&ox,&oy);
	locacur((x>1?x-2:x),(y>2?y-3:y));
	printf("%d",i);
	locacur(ox,oy);
}
