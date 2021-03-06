static char SCCSID[]="@(#)scale.c	1.1";
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
	lai = (ai<1 ? LOG(10,ai)-1 : LOG(10,ai));
	b = ai/pow(10.,(double)lai);

	for(i=0; b>sqr[i]; i++) ;  /* determine actual interval */
	*intv = vint[i]*pow(10.,(double)lai);

	m1 = ((m=min/ *intv)<0 ? (int)m-1 : (int)m); /* determine min and max */
	*smin = ((m= *intv*m1)<min ? m : min);
	m1 = ((m=max/ *intv)<-1 ? (int)m : (int)m+1);
	*smax = ((m= *intv*m1)>max ? m : max);
}
