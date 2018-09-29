char curxver[] = "@(#)curve.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

# define max(a,b)	((a)>(b)?(a):(b))

curve(x1,y1,xe,ye,xc,yc,a,b)
int	x1 ;
int	y1 ;
int	xe ;
int	ye ;
float	xc ;
float	yc ;
float	a ;
float	b ;
{
	int	x2 ;
	int	y2 ;
	float	x3 ;
	float	y3 ;
	int	xi ;
	int	yi ;
	int	prev_x ;
	int	prev_y ;
	int	done ;
	float	xf1 ;
	float	yf1 ;
	float	xf2 ;
	float	yf2 ;
	float	d ;
	float	d1 ;
	float	d2 ;
	float	d3 ;
	float	c1 ;
	float	c2 ;
	float	c3 ;
	float	max_error ;
	float	prev_error ;
	float	sqrt() ;
	float	fabs() ;
	float	cosang() ;

	fprintf(stderr,"(%d,%d) (%d,%d) (%.2f,%.2f) %.2f %.2f\n",
	x1,y1,xe,ye,xc,yc,a,b) ;
	x1 = (x1+P_RES/2) / P_RES * P_RES ;
	y1 = (y1+P_RES/2) / P_RES * P_RES ;
	xe = (xe+P_RES/2) / P_RES * P_RES ;
	ye = (ye+P_RES/2) / P_RES * P_RES ;

	for ( done=0 ; !done ; )
	{

		prev_error = 1000000000 ;

		for ( xi=0 ; xi<10 ; xi++ )
		for ( yi=0 ; yi<10 ; yi++ )
		{
			if ( xi == 0  &&  yi == 0 )
				continue ;
			if ( x1 > xc )
				y2 = y1 - yi * P_RES ;
			else
				y2 = y1 + yi * P_RES ;
			if ( y1 > yc )
				x2 = x1 + xi * P_RES ;
			else
				x2 = x1 - xi * P_RES ;

			x3 = (x1+x2) * 0.5 ;
			y3 = (y1+y2) * 0.5 ;
			d1 = (x1-xc)*(x1-xc)/(a*a) + (y1-yc)*(y1-yc)/(b*b) ;
			d2 = (x2-xc)*(x2-xc)/(a*a) + (y2-yc)*(y2-yc)/(b*b) ;
			d3 = (x3-xc)*(x3-xc)/(a*a) + (y3-yc)*(y3-yc)/(b*b) ;

			max_error = max(fabs(d1-1),fabs(d2-1)) ;
			max_error = max(max_error,fabs(d3-1)) ;

			if ( max_error <= prev_error )
/*			(
				max_error < prev_error  ||
				max_error == prev_error  &&
				(x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) <
				(prev_x-x1)*(prev_x-x1) + (prev_y-y1)*(prev_y-y1)
			)
*/
			{
				prev_x = x2 ;
				prev_y = y2 ;
				prev_error = max_error ;
			}
		}

		c1 = cosang(x1-xc,y1-yc,xe-xc,ye-yc) ;
		c2 = cosang(xe-xc,ye-yc,prev_x-xc,prev_y-yc) ;
		c3 = cosang(x1-xc,y1-yc,prev_x-xc,prev_y-yc) ;

		if ( c1 >= c3  &&  c2 > c3 )
			done = 1 ;

		if ( done )
		{
			prev_x = xe ;
			prev_y = ye ;
		}

/*		fprintf(stderr,"%d %d %f\n",prev_x,prev_y,prev_error) ;
*/
		store_char
		(
			plotmap
				[0]
				[(prev_x>x1)^(prev_y>y1)]
				[abs(prev_x-x1)/P_RES]
				[abs(prev_y-y1)/P_RES],
			PLOTFLAG,
			1,
			abs(prev_x-x1),
			min(x1,prev_x),
			min(y1,prev_y)
		) ; 

		x1 = prev_x ;
		y1 = prev_y ;
	}

}

float
cosang(dx1,dy1,dx2,dy2)
float	dx1 ;
float	dy1 ;
float	dx2 ;
float	dy2 ;
{
	float	result ;

	result = dx1*dx2 + dy1*dy2 ;
	result /= sqrt(dx1*dx1+dy1*dy1) ;
	result /= sqrt(dx2*dx2+dy2*dy2) ;
	return(result) ;
}
arctest(x1,y1,xc,yc,xe,ye)
int	x1 ;
int	y1 ;
int	xc ;
int	yc ;
int	xe ;
int	ye ;
{
	int	x2 ;
	int	y2 ;
	int	xi ;
	int	yi ;
	int	prev_x ;
	int	prev_y ;
	int	done ;
	int	deriv1 ;
	int	deriv2 ;
	float	rad ;
	float	d1 ;
	float	d2 ;
	float	d3 ;
	float	c1 ;
	float	c2 ;
	float	c3 ;
	float	max_error ;
	float	prev_error ;
	float	sqrt() ;
	float	fabs() ;
	float	cosang() ;

	rad = (x1-xc)*(x1-xc) + (y1-yc)*(y1-yc) ;
	rad = sqrt(rad) ;

	x1 = (x1+P_RES/2) / P_RES * P_RES ;
	y1 = (y1+P_RES/2) / P_RES * P_RES ;
	xe = (xe+P_RES/2) / P_RES * P_RES ;
	ye = (ye+P_RES/2) / P_RES * P_RES ;

	for ( done=0 ; !done ;  )
	{

		prev_error = 1000000000 ;

		for ( xi=0 ; xi<10 ; xi++ )
		for ( yi=0 ; yi<10 ; yi++ )
		{
			if ( xi == 0  &&  yi == 0 )
				continue ;
			if ( x1 > xc )
				y2 = y1 - yi * P_RES ;
			else
				y2 = y1 + yi * P_RES ;
			if ( y1 > yc )
				x2 = x1 + xi * P_RES ;
			else
				x2 = x1 - xi * P_RES ;

			d1 = sqrt((float) (x1-xc)*(x1-xc) + (y1-yc)*(y1-yc) ) ;
			d2 = sqrt((float) (x2-xc)*(x2-xc) + (y2-yc)*(y2-yc) ) ;
			d3 = fabs((float) (y2-y1)*xc + (x1-x2)*yc + x2*y1 - x1*y2 ) ;
			d3 /= sqrt((float) (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ) ;

			max_error = max(fabs(d1-rad),fabs(d2-rad)) ;

			deriv1 = (x1-xc)*(x2-x1) + (y1-yc)*(y2-y1) ;
			deriv2 = (x2-xc)*(x2-x1) + (y2-yc)*(y2-y1) ;
			if ( deriv1<0 && deriv2>0  ||  deriv1>0 && deriv2<0 )
				max_error = max(max_error,fabs(d3-rad)) ;

			if ( max_error <= prev_error )
			{
				prev_x = x2 ;
				prev_y = y2 ;
				prev_error = max_error ;
			}
		}

		c1 = cosang((float)x1-xc,(float)y1-yc,(float)xe-xc,(float)ye-yc) ;
		c2 = cosang((float)xe-xc,(float)ye-yc,(float)prev_x-xc,(float)prev_y-yc) ;
		c3 = cosang((float)x1-xc,(float)y1-yc,(float)prev_x-xc,(float)prev_y-yc) ;

		if ( c1 >= c3  &&  c2 > c3 )
			done = 1 ;

		if ( done )
		{
			prev_x = xe ;
			prev_y = ye ;
		}

		fprintf(stderr,"%d %d\n",prev_x,prev_y) ;
		store_char
		(
			plotmap
				[0]
				[(prev_x>x1)^(prev_y>y1)]
				[abs(prev_x-x1)/P_RES]
				[abs(prev_y-y1)/P_RES],
			PLOTFLAG,
			1,
			abs(prev_x-x1),
			min(x1,prev_x),
			min(y1,prev_y)
		) ; 

		x1 = prev_x ;
		y1 = prev_y ;
	}

}
int
between(a,b,c)
int	a ;
int	b ;
int	c ;
{
	if ( a <= b  &&  b <= c )
		return(1) ;
	if ( c <= b  &&  b <= a )
		return(1) ;
	return(0) ;
}
