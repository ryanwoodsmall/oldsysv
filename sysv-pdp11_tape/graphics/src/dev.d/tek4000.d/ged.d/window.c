static char SCCSID[]="@(#)window.c	1.1";
#include <stdio.h>
#include "ged.h"

window(pts,pi,ar)
int pts[], pi;
struct area *ar;
{
	int i;

	for( i=2, ar->lx=ar->hx=pts[0], ar->ly=ar->hy=pts[1]; i<pi*2; ) {
		ar->lx = MIN(ar->lx,pts[i]);
		ar->hx = MAX(ar->hx,pts[i]); i++;
		ar->ly = MIN(ar->ly,pts[i]);
		ar->hy = MAX(ar->hy,pts[i]); i++;
	}
	if( pi<2 || (ar->lx==ar->hx && ar->ly==ar->hy) ) return(FAIL);
	else return(SUCCESS);
}
