/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libI77:wsfe.c	1.6"
/*write sequential formatted external*/
#include "fio.h"
#include "fmt.h"
extern int x_putc(),w_ed(),w_ned();
extern int xw_end(),xw_rev(),x_wSL();
extern int hiwater;
s_wsfe(a) cilist *a;	/*start*/
{	int n;
	if(!init) f_init();
	if(n=c_sfe(a)) return(n);
	reading=0;
	sequential=1;
	formatted=1;
	external=1;
	elist=a;
	hiwater = cursor=recpos=0;
	F77nonl = 0;
	scale=0;
	fmtbuf=a->cifmt;
	curunit = &units[a->ciunit];
	cf=curunit->ufd;
	if(pars_f(fmtbuf)<0) err(a->cierr,100,"startio");
	putn= x_putc;
	doed= w_ed;
	doned= w_ned;
	doend=xw_end;
	dorevert=xw_rev;
	donewrec=x_wSL;
	fmt_bg();
	cplus=0;
	cblank=curunit->ublnk;
	if(!curunit->uwrt) (void) nowwriting(curunit);
	return(0);
}
x_putc(c)
{
	/* this uses \n as an indicator of record-end */
	if(c == '\n' && recpos < hiwater)
		if(cf->_flag & _IOLBF || !curunit->useek)	/* cdm 5-17-84 */
			cf->_ptr += hiwater - recpos;		/* cdm 5-17-84 */
		else						/* cdm 5-17-84 */
			(void) fseek(cf, hiwater - recpos, 1);	/* cdm 5-17-84 */
	putc(c,cf);
	recpos++;
}
pr_put(c)
{	static flag new = 1;
	recpos++;
	if(c=='\n')
	{	new=1;
		putc(c,cf);
	}
	else if(new==1)
	{	new=0;
		if(c=='0') putc('\n',cf);
		else if(c=='1') putc('\f',cf);
	}
	else putc(c,cf);
}
x_wSL()
{
	(*putn)('\n');
	recpos=0;
	cursor = 0;
	hiwater = 0;
	return(1);
}
xw_end()
{
	if(F77nonl == 0)
		(*putn)('\n');
	hiwater = recpos = cursor = 0;
	return(0);
}
xw_rev()
{
	if(workdone) (*putn)('\n');
	hiwater = recpos = cursor = 0;
	return(workdone=0);
}
