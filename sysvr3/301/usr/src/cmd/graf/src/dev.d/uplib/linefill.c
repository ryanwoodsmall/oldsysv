/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/linefill.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
#include "fill.h"


struct points {
		int x1,y1;
	};

struct intpoints {
		double xx,yy;
	};
extern struct device dv;

linefill(cdptr)
struct command *cdptr;
{

/*
 *  linefill takes a command structure describing a lines primitive
 *  and fills the area described by the lines.  The slope of the fill
 *  line is determined by the color of the border line. linefill
 *  calculates the limits for the fill lines and contains the basic loop
 *  to fill the figure.
 */

	struct points *ptar;
	struct intpoints intar[MAXPOINTS];
	double slope,offset,lim,angle,incr,tmp,spacing;
	int n,ptarlen,col;
	struct area xylim;

	spacing = (dv.v.hx-dv.v.lx)/SPACING;

	ptarlen = (cdptr->cnt - 2)/2;
	ptar = (struct points *) cdptr->aptr;

	col=cdptr->color;

	if (cantfill(ptar,ptarlen)) return;

	maxminxy(ptar,ptarlen,&xylim);

	switch (col)  {
		case BLACK:	angle = BCKANG; break;
		case RED:		angle = REDANG; break;
		case GREEN:	angle = GRNANG; break;
		case BLUE:	angle = BLUANG; break;
		default:		angle = BCKANG; 
	}

	slope = tan(angle);		/*  angle cannot be PI/2  */
	if (slope < 0)  {
		incr = -spacing/cos(angle);
		offset = xylim.ly - slope * xylim.lx;
		lim = xylim.hy - slope * xylim.hx;
	}
	else  {
		incr = spacing/cos(angle);
		offset = xylim.ly - slope * xylim.hx;
		lim = xylim.hy - slope * xylim.lx;
	}

	incr = floor(incr);
	tmp = fmod(offset,incr);
	if (tmp < 0) offset -= tmp;
	else offset += incr-tmp;

	for (; offset < lim; offset += incr)  {
		n = slintcept(ptar,ptarlen,slope,offset,intar);
		drawfill(intar,n,col);
	}
}




#define IN  1
#define OUT 0
drawfill(intar,intarlen,color)
struct intpoints *intar;
int intarlen,color;
{
/*
 *  drawfill takes the array of intersection points and draws those
 *  lines that are inside the figure
 */
	int p1,p2;
	double max=XYMAX;
	short state=OUT;

	p1 = nextpoint(intar,intarlen,&max);

	for( p2=0; p1 != -1 && p2 != -1; )  {
		p2 = nextpoint(intar,intarlen,&max);
		if (p2 != -1)  {
			state = state ? OUT : IN;	/* passed boundary, state changes  */
			if (state && p1!=p2)  {
				clipseg( (int) intar[p1].xx, (int) intar[p1].yy,
					(int) intar[p2].xx, (int) intar[p2].yy,
					NARROW,SOLID,color);
			}
			p1 = p2;
		}
	}
}




int slintcept(ptar,ptarlen,slope,offset,intar)
struct points *ptar;
struct intpoints *intar;
double slope,offset;
int ptarlen;
{
/*
 * slintcept takes the points in ptar and calculates the intersection
 * points between those lines and the line defined by the given slope
 * and intercept.  The interception points are stored in intar.
 * slintcept also calls checkend to determine if the intersection
 * point is the endpoint of a line segment.
 */
	int n,i;
	double slope2,offset2,tmp,x1,y1,x2,y2;

	for( n=i=0; i < ptarlen; i++ )  {
		x1 = ptar[i].x1, y1 = ptar[i].y1;
		if (ptarlen == i+1)
			if (ptar[i].x1!=ptar[0].x1 || ptar[i].y1!=ptar[0].y1)
				x2=ptar[0].x1, y2=ptar[0].y1;
			else break;
		else x2=ptar[i+1].x1, y2=ptar[i+1].y1;

		if (x1 == x2 && y1 == y2);
		else if (x1 == x2)  {
			intar[n].xx = x1;
			intar[n].yy = (x1 * slope + offset);
			if (BETWEEN(intar[n].yy,y1,y2))  n++;
			else if (checkend(ptar,ptarlen,i,&(intar[n]),slope))  n++;
		}
		else  {
			slope2 = (y1-y2)/(x1-x2);
			offset2 = y1 - (x1 * slope2);
			if (slope != slope2)  {
				intar[n].xx = (offset-offset2)/(slope2-slope);
				intar[n].yy = (offset + slope * intar[n].xx);
				if (BETWEEN(intar[n].xx,x1,x2))  {
					if (BETWEEN(intar[n].yy,y1,y2)
						|| y1 == y2)  n++;
				}
				else if (checkend(ptar,ptarlen,i,&(intar[n]),slope))  n++;
			}
			else if (offset == offset2)  {
				if (thrubound(ptar,ptarlen,i,slope,offset))  {
					intar[n].xx = x1;
					intar[n].yy = y1;
					n++;
				}
			}
		}
	}


	return(n);
}


int checkend(ptar,ptarlen,index,intpt,slope)
struct points *ptar;
struct intpoints *intpt;
int ptarlen, index;
double slope;
{
/* 
 *  checkend determines if the intersection point is an endpoint.
 *  If it is, checkend compares the angles of the lines
 *  connected to that point with the fill angle to determine 
 *  whether the fill line passes a boundary.
 */
	int pt0,pt1,pt2;
	double dis1,dis2,x0x1,y0y1,x2x1,y2y1,angle1,angle2,fillangle;

	if (EQDOUB(ptar[index].x1,intpt->xx)
		&& EQDOUB(ptar[index].y1,intpt->yy))
		pt1 = index;
	else if (EQDOUB(ptar[index+1].x1,intpt->xx)
			&& EQDOUB(ptar[index+1].y1,intpt->yy))
		pt1 = index+1;
	else return(0);

	intpt->xx = ROUND(intpt->xx); intpt->yy = ROUND(intpt->yy);

	pt0 = pt2 = pt1;
	for (pt0=pt0 ? pt0-1 : ptarlen-1; ptar[pt0].x1 == ptar[pt1].x1
							&& ptar[pt0].y1 == ptar[pt1].y1;
							pt0=pt0 ? pt0-1 : ptarlen-1);

	for (pt2=pt2!=ptarlen-1 ? pt2+1 : 0; ptar[pt2].x1 == ptar[pt1].x1
							&& ptar[pt2].y1 == ptar[pt1].y1;
							pt2=pt2!=ptarlen-1 ? pt2+1 : 0);

	fillangle = atan(slope);

	x0x1 = ptar[pt0].x1 - ptar[pt1].x1;
	y0y1 = ptar[pt0].y1 - ptar[pt1].y1;
	dis1 = sqrt((x0x1*x0x1) + (y0y1*y0y1));

	x2x1 = ptar[pt2].x1 - ptar[pt1].x1;
	y2y1 = ptar[pt2].y1 - ptar[pt1].y1;
	dis2 = sqrt((x2x1*x2x1) + (y2y1*y2y1));

	angle1 = atan2(y0y1/dis1,x0x1/dis1);
	angle2 = atan2(y2y1/dis2,x2x1/dis2);

	if (BETWEEN(fillangle,angle1,angle2)) return(1);
	else return(0);
}


thrubound(ptar,ptarlen,index,slope,offset)
struct points *ptar;
int ptarlen,index;
double slope,offset;
{
/*
 *  thrubound is called when a fill line overlays a border
 *  line. It checks whether the fill line passes through a
 *  boundary or just skims the border of the figure.
 */

	int p1,p2,n;

	p1=p2= -1;
	for(n=index+2; n < ptarlen; n++)
		if (ptar[n].y1 != offset+slope*ptar[n].x1)  {
			p1=n; break;
		}
	if (p1 < 0)
		for (n=0; n < index; n++)
			if (ptar[n].y1 != offset+slope*ptar[n].x1)  {
				p1=n; break;
			}

	for(n=index-1; n >= 0; n--)
		if (ptar[n].y1 != offset+slope*ptar[n].x1)  {
			p2=n; break;
		}
	if (p2 < 0)
		for (n=ptarlen-1; n > index+1; n--)
			if (ptar[n].y1 != offset+slope*ptar[n].x1)  {
				p2=n; break;
			}

	if (ptar[p1].y1 < offset+slope*ptar[p1].x1)
		if (ptar[p2].y1 > offset+slope*ptar[p2].x1)
			return(1);
		else return(0);
	else if (ptar[p2].y1 < offset+slope*ptar[p2].x1)
			return(1);
		else return(0);
}


int cantfill(ptar,ptarlen)
struct points *ptar;
int ptarlen;
{
/*
 *  cantfill tests that no line overlays another.
 */
	double slope,slope1,offset,x0,y0,x1,y1,x2,y2;
	int vert,i,p0,p1,p2,p3,open;
	struct points *pa;

	if (ptar[0].x1 != ptar[ptarlen-1].x1
		|| ptar[0].y1 != ptar[ptarlen-1].y1)  {
		open = 1;
		x0 = ptar[0].x1, y0 = ptar[0].y1;
	}
	else open = 0;

	for ( vert=0; ptarlen > 2; vert=0,ptar++,ptarlen--)  {
		if (ptar[0].x1 == ptar[1].x1) vert = 1;
		else  slope = (ptar[0].y1-ptar[1].y1) /
					(double) (ptar[0].x1-ptar[1].x1);
		if (vert)  offset = ptar[0].x1;
		else offset = ptar[0].y1 - slope * ptar[0].x1;
		for (pa=ptar+1,i=ptarlen-2; i >= 0; i--,pa++)  {

			x1 = pa[0].x1, y1 = pa[0].y1;
			if (i == 0)
				if (open)
					x2 = x0, y2 = y0;
				else break;
			else x2 = pa[1].x1, y2 = pa[1].y1;
			if(x1 == x2) slope1 = 0.0;
			else slope1=(y1-y2)/(x1-x2);

			if ( (vert && x1==x2 && offset==x1)
				|| (!vert && x1!=x2
					&& slope==slope1
					&& offset==y1-x1*slope1) )  {
				p0=ptar[0].x1+ptar[0].y1, p1=ptar[1].x1+ptar[1].y1;
				p2=x1+y1, p3=x2+y2;
				if (MIN(p2,p3) < MAX(p0,p1) && MAX(p2,p3) > MIN(p0,p1) )
					return(1);
			}
		}
	}

	return(0);
}




int nextpoint(intar,intarlen,max)
struct intpoints intar[];
int intarlen;
double *max;
{
/*
 *  nextpoint gets the rightmost point less than max.
 */
	double nmax = XYMIN;
	int n,i;

	for (i=0,n=(-1); i < intarlen; i++)
		if (intar[i].xx >= nmax && intar[i].xx < *max)  {
			n=i;
			nmax = intar[i].xx;
		}

	*max = nmax;
	return(n);
}




maxminxy(ptar,ptarlen,arptr)
struct points *ptar;
struct area *arptr;
int ptarlen;
{
	arptr->lx = arptr->hx = ptar->x1;
	arptr->ly = arptr->hy = ptar->y1;
	for (ptar++,ptarlen--; ptarlen > 0; ptar++,ptarlen--)  {
		if (ptar->x1 > arptr->hx)  arptr->hx = ptar->x1;
		else if (ptar->x1 < arptr->lx) arptr->lx = ptar->x1;
		if (ptar->y1 > arptr->hy)  arptr->hy = ptar->y1;
		else if (ptar->y1 < arptr->ly)  arptr->ly = ptar->y1;
	}
}
