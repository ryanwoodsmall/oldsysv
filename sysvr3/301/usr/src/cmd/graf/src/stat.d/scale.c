/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/scale.c	1.4"
/* <:t-5 d:> */
#define LOG(b,x)	(log(fabs((double)x))/log((double)b))

scale(min,max,n,smin,smax,intv) /* determine readable scale,
				after Lewart ACM alg 463 */
double min, max, *smin, *smax, *intv;
int n; /* approximate number of intervals */
{
	static double vint[] = {1, 2, 5, 10};
	static double sqr[] = {1.414, 3.162, 7.071, 1e38};
	double ai, b, m, log(), pow(), fabs();
	int lai, m1, i;

	ai = (max-min)/n;	/* approximate interval */
	if(ai == 0) lai = ai = 0;		/*indicates no input data(?)*/
	else lai = (ai<1 ? LOG(10,ai)-1 : LOG(10,ai));
	b = ai/pow(10.,(double)lai);

	for(i=0; b>sqr[i]; i++) ;  /* determine actual interval */
	*intv = vint[i]*pow(10.,(double)lai);

	if((m = min / *intv) < 0)	/* determine min and max */
		m1 = (int)m -1;
	else
		m1 = (int)m;
	if((m = max / *intv) < -1)
		m1 = (int)m;
	else
		m1 = (int)m+1;
	*smin = ((m= *intv*m1)<min ? m : min);
	*smax = ((m= *intv*m1)>max ? m : max);
}
