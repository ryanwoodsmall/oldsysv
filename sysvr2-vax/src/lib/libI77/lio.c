/*	@(#)lio.c	1.5	*/
#include "fio.h"
#include "fmt.h"
#include "lio.h"
extern int l_write();
int t_putc();
s_wsle(a) cilist *a;
{
	int n;
	if(!init) f_init();
	if(n=c_le(a)) return(n);
	reading=0;
	external=1;
	formatted=1;
	putn = t_putc;
	lioproc = l_write;
	if(!curunit->uwrt)
		return(nowwriting(curunit));
	else	return(0);
}
e_wsle()
{
	t_putc('\n');
	recpos=0;
	return(0);
}
t_putc(c)
{
	recpos++;
	putc(c,cf);
	return(0);
}
lwrt_I(n) ftnint n;
{
	char buf[LINTW],*p;
	(void) sprintf(buf," %ld",(long)n);
	if(recpos+strlen(buf)>=LINE)
	{	t_putc('\n');
		recpos=0;
	}
	for(p=buf;*p;t_putc(*p++));
	if( recpos < LINE ) t_putc(' ');	/* cdm 3-27-84 */
}
lwrt_L(n, len) ftnint n; ftnlen len;
{
	if(recpos+LLOGW>=LINE)
	{	t_putc('\n');
		recpos=0;
	}
	(void) wrt_L((uint *)&n,LLOGW, len);
	if( recpos < LINE ) t_putc(' ');	/* cdm 3-27-84 */
}
lwrt_A(p,len) char *p; ftnlen len;
{
	int i;
	if( recpos == LINE )			/* cdm 3-27-84 */
	{	t_putc('\n');			/* cdm 3-27-84 */
		recpos = 0;			/* cdm 3-27-84 */
	}
	if( recpos == 0 ) t_putc(' ');		/* cdm 3-27-84 */

	for( i = 0; i < len; ++i )		/* cdm 3-27-84 */
	{	t_putc(*p++);			/* cdm 3-27-84 */
		if( recpos == LINE )		/* cdm 3-27-84 */
		{	t_putc('\n');		/* cdm 3-27-84 */
			recpos = 0;		/* cdm 3-27-84 */
			t_putc(' ');		/* cdm 3-27-84 */
		}				/* cdm 3-27-84 */
	}					/* cdm 3-27-84 */
}
lwrt_F(n) double n;
{
	double absn;

	absn = n;
	if (absn < 0)
		absn = -absn;
	if (LLOW <= absn && absn < LHIGH)
	{
		if(recpos+LFW>=LINE)
		{
			t_putc('\n');
			recpos=0;
		}
		scale=0;
		(void) wrt_F((ufloat *)&n,LFW,LFD,(ftnlen)sizeof(n));
	}
	else
	{
		if(recpos+LEW>=LINE)
		{	t_putc('\n');
			recpos=0;
		}
		scale = 1;
		(void) wrt_E((ufloat *)&n,LEW,LED,LEE,(ftnlen)sizeof(n));
	}
	if( recpos < LINE ) t_putc(' ');	/* cdm 3-27-84 */
}
lwrt_C(a,b) double a,b;
{
	if(recpos+2*LFW+3>=LINE)
	{	t_putc('\n');
		recpos=0;
	}
	t_putc(' ');
	t_putc('(');
	lwrt_F(a);
	t_putc(',');
	lwrt_F(b);
	t_putc(')');
	if( recpos < LINE ) t_putc(' ');	/* cdm 3-27-84 */
}
l_write(number,ptr,len,type) ftnint *number,type; flex *ptr; ftnlen len;
{
	int i;
	ftnint x;
	double y,z;
	float *xx;
	double *yy;
	for(i=0;i< *number; i++)
	{
		switch((int)type)
		{
		default: fatal(204,"unknown type in lio");
		case TYSHORT: x=ptr->flshort;
			goto xint;
		case TYLONG: x=ptr->flint;
		xint: lwrt_I(x);
			break;
		case TYREAL: y=ptr->flreal;
			goto xfloat;
		case TYDREAL: y=ptr->fldouble;
		xfloat: lwrt_F(y);
			break;
		case TYCOMPLEX: xx= &(ptr->flreal);
			y = *xx++;
			z = *xx;
			goto xcomplex;
		case TYDCOMPLEX: yy = &(ptr->fldouble);
			y= *yy++;
			z = *yy;
		xcomplex: lwrt_C(y,z);
			break;
		case TYLOGICAL: lwrt_L(ptr->flint, len);
			break;
		case TYCHAR: lwrt_A((char *)ptr,len);
			break;
		}
		ptr = (flex *)((char *)ptr + len);
	}
	return(0);
}
