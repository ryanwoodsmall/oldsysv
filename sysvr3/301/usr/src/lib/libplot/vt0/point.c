/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libplot:vt0/point.c	1.2"
extern vti;
extern xnow,ynow;
point(xi,yi){
	struct {char pad,c; int x,y;} p;
	p.c = 2;
	p.x = xnow = xsc(xi);
	p.y = ynow =  ysc(yi);
	write(vti,&p.c,5);
}
