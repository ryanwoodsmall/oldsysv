char drawxver[] = "@(#)draw.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

arc(dx1,dy1,dx2,dy2)
int	dx1 ;
int	dy1 ;
int	dx2 ;
int	dy2 ;
{
	float	xcen ;
	float	ycen ;
	int	xfrom ;
	int	yfrom ;
	int	xto ;
	int	yto ;
	float	r ;
	float	sqrt() ;

	xfrom = xpos ;
	yfrom = ypos ;
	xcen = xpos + dx1 ;
	ycen = ypos + dy1 ;
	xto = xpos + dx1 + dx2 ;
	yto = ypos + dy1 + dy2 ;
	r = sqrt((float)(dx1*dx1+dy1*dy1)) ;

	curve(xfrom,yfrom,xto,yto,xcen,ycen,r,r) ;

	xpos = xto ;
	ypos = yto ;

}

circle(d)
int	d ;
{
	float	rad ;

	rad = d * 0.5 ;
	curve(xpos,ypos,xpos,ypos,xpos+rad,(float)ypos,rad,rad) ;
	xpos += d ;
}

draw_line(xdelta,ydelta)
int	xdelta ;
int	ydelta ;
{
	int	x ;
	int	y ;
	int	xend ;
	int	yend ;
	int	xcur ;
	int	ycur ;
	int	dx ;
	int	dy ;
	int	dxneg ;
	int	dyneg ;
	int	dist ;
	int	tdist ;
	int	xdel ;
	int	ydel ;
	int	txdel ;
	int	tydel ;
	int	slope ;
	int	maxdel ;

					/* round to plot resolution */
	x = (xpos+P_RES/2) / P_RES * P_RES ;
	y = (ypos+P_RES/2) / P_RES * P_RES ;
	dx = (xpos+xdelta+P_RES/2) / P_RES * P_RES - x ;
	dy = (ypos+ydelta+P_RES/2) / P_RES * P_RES - y ;

	dxneg = (xdelta<0) ;
	dyneg = (ydelta<0) ;
	slope = dxneg ^ dyneg ;
	maxdel = (NVERTICES-1) * P_RES ;
	xcur = 0 ;
	ycur = 0 ;
	xend = abs(dx) ;
	yend = abs(dy) ;

	while ( xcur != xend  ||  ycur != yend )
	{
		dist = 100000 ;
		for ( tydel=0 ; tydel<=maxdel ; tydel+=P_RES )
			for ( txdel=0 ; txdel<=maxdel ; txdel+=P_RES )
			{
				if ( txdel == 0  &&  tydel == 0 )
					continue ;
				if ( xcur+txdel > xend  ||  ycur+tydel > yend )
					continue ;
				tdist = abs((ycur+tydel)*xend-(xcur+txdel)*yend) ;
				if ( tdist <= dist )
				{
					dist = tdist ;
					xdel = txdel ;
					ydel = tydel ;
				}
			}
		xcur += xdel ;
		ycur += ydel ;
		if ( dxneg )
			xdel = -xdel ;
		if ( dyneg )
			ydel = -ydel ;
		store_char
		(
			plotmap[min(y,y+ydel)/P_RES%NOFFSETS][slope][abs(xdel)/P_RES][abs(ydel)/P_RES],
			PLOTFLAG,
					/* index of size corresponding to unitwidth */
			1,
			abs(xdel),
			min(x,x+xdel),
			min(y,y+ydel)/(NOFFSETS*P_RES)*(NOFFSETS*P_RES)
		) ;
		x += xdel ;
		y += ydel ;
/*
		fprintf(stderr,"%5d%5d\n",x,y) ;
*/
	}
	xpos += xdelta ;
	ypos += ydelta ;
}
ellipse(d1,d2)
int	d1 ;
int	d2 ;
{
	float	a ;
	float	b ;

	a = d1 * 0.5 ;
	b = d2 * 0.5 ;
	curve(xpos,ypos,xpos,ypos,xpos+a,(float)ypos,a,b) ;
	xpos += d1 ;
}
