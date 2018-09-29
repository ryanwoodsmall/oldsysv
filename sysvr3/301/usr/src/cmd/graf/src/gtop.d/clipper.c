/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/clipper.c	1.2"
/********************************************************************/
/* G. B. Smith  112834                          Date:  07/06/84     */
/********************************************************************/
/*                                                                  */
/* name:  clipper                                                   */
/*                                                                  */
/* function: clips a line segment within a user specified window    */
/*           using the Cohen-Sutherland algorithm.                  */
/*           See pg 146, Fundamentals of Interactive Comp. Graphics */
/*                       Foley & Van Dam, 1982                      */
/*                                                                  */
/* input : vin  --  unclipped line segment.                         */
/*         xmin --  clipping window.                                */
/*         xmax --                                                  */
/*         ymin --                                                  */
/*         ymax --                                                  */
/*         vout --  clipped line segment.                           */
/*                                                                  */
/* output: NULL if "vin" does not intersect the window, else 1.     */
/*                                                                  */
/********************************************************************/

#include <stdio.h>

#define  TOP     0x8
#define  BOTTOM  0x4
#define  RIGHT   0x2
#define  LEFT    0x1

int clipper( vin, xmin, xmax, ymin, ymax, vout )

int vin[4], xmin, xmax, ymin, ymax, vout[4];

{
 int  i, p1_outcode, p2_outcode, outcode, p1, p2;

 for( i=0; i<=4; i++ )
 	vout[i]=vin[i];

 for(;;)
  {
    p1_outcode = foutcode( vout[0], vout[1], xmin, xmax, ymin, ymax );
    p2_outcode = foutcode( vout[2], vout[3], xmin, xmax, ymin, ymax );

    if( p1_outcode & p2_outcode ) return(0); /* Does not intersect window */

    if( !(p1_outcode | p2_outcode)) return(1); /* Line is inside of box */

/*** We must clip the output vector ****/

    if( p1_outcode==0 ) { p1=2; p2=0; outcode=p2_outcode; }
    else                { p1=0; p2=2; outcode=p1_outcode; }

    if( outcode&TOP )
     {
      vout[p1] = vout[p1] + 
              (vout[p2]-vout[p1])*(ymax-vout[p1+1])/(vout[p2+1]-vout[p1+1]);
      vout[p1+1] = ymax;
     }
    else
     if( outcode&BOTTOM )
     {
      vout[p1] = vout[p1] + 
              (vout[p2]-vout[p1])*(ymin-vout[p1+1])/(vout[p2+1]-vout[p1+1]);
      vout[p1+1] = ymin;
     }
    else
     if( outcode&RIGHT )
     {
      vout[p1+1] = vout[p1+1] + 
              (vout[p2+1]-vout[p1+1])*(xmax-vout[p1])/(vout[p2]-vout[p1]);
      vout[p1] = xmax;
     }
    else
     {
      vout[p1+1] = vout[p1+1] + 
              (vout[p2+1]-vout[p1+1])*(xmin-vout[p1])/(vout[p2]-vout[p1]);
      vout[p1] = xmin;
     }
   }
}

/********************************************************************/
/* G. B. Smith  112834                          Date:  07/06/84     */
/********************************************************************/
/*                                                                  */
/* name:  foutcode                                                  */
/*                                                                  */
/* function: computes the "outcodes" for the routine clipper.       */
/*           See pg 146, Fundamentals of Interactive Computer Graph.*/
/*                       Foley & Van Dam, 1982                      */ 
/*                                                                  */
/* input : x, y    --  integer coordinates.                         */
/*         xmin    --  box in space.  We want to determine where    */
/*         xmax    --  integer coordinates are wrt the box.         */
/*         ymin    --                                               */
/*         ymax    --                                               */
/*                                                                  */
/* output: outcode --  bit4 set == above the box                    */
/*                     bit3 set == below the box                    */
/*                     bit2 set == to right of box                  */
/*                     bit1 set == to left of box                   */
/*                                                                  */
/********************************************************************/

#define SIGN(X) ((X)>=0?0:1)

int foutcode( x, y, xmin, xmax, ymin, ymax )

int x, y, xmin, xmax, ymin, ymax;
{
 int bit1, bit2, bit3, bit4;

 bit4 = SIGN(ymax-y)<<3;
 bit3 = SIGN(y-ymin)<<2;
 bit2 = SIGN(xmax-x)<<1;
 bit1 = SIGN(x-xmin);

 return( bit4|bit3|bit2|bit1 );
}

