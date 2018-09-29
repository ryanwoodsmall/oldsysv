static char SCCSID[]="@(#)clip.c	1.1";
/* <: t-5 d :> */
#include "../../../../include/util.h"
#define LEFT 01
#define RIGHT 02
#define BOT 04
#define TOP 010


clip(x1,y1,x2,y2,lowx,lowy,highx,highy)
int *x1, *y1, *x2, *y2;
int lowx, lowy, highx, highy;
{
	int c1, c2, c;
	long xa= *x1, xb= *x2, ya= *y1, yb= *y2;
	int x, y;

	c1 = code((int)xa,(int)ya,lowx,lowy,highx,highy);
	c2 = code((int)xb,(int)yb,lowx,lowy,highx,highy);

	while(c1 || c2) {
		if(c1 && c2)return(FAIL);
		c = c1;
		if(c == 0) c = c2;

		if(c & LEFT) { /* push toward left edge */
			y = ya + (yb - ya)*(lowx - xa)/(xb - xa);
			x = lowx;
		} else if(c & RIGHT) { /*  push toward right edge */
			y = ya + (yb - ya)*(highx - xa)/(xb - xa);
			x = highx;
		} else if(c & BOT) { /*  push toward bottom edge */
			x = xa + (xb - xa)*(lowy - ya)/(yb - ya);
			y = lowy;
		} else if(c & TOP) { /*  push toward top edge */
			x = xa + (xb - xa)*(highy - ya) / (yb - ya);
			y = highy;
		}
		if(c == c1) {
			xa = x;
			ya = y;
			c1 = code(x,y,lowx,lowy,highx,highy);
		}
		else {
			xb = x;
			yb = y;
			c2 = code(x,y,lowx,lowy,highx,highy);
		}
	}
	*x1 = xa;
	*x2 = xb;
	*y1 = ya;
	*y2 = yb;

	return(SUCCESS);
}
static
code(x,y,lowx,lowy,highx,highy)
int x, y, lowx, lowy, highx, highy;
{

	if(x < lowx) return(LEFT);
	else if(x > highx) return(RIGHT);
	else if(y < lowy) return(BOT);
	else if(y > highy) return(TOP);
	else return(0);
}
