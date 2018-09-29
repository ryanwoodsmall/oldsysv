/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#
/* send4.c */




#define IDMOD send4

#define IDSTR "~|^`send4.c 1.5 1/15/76

#include "send.h"

static char Sccsid[] = "@(#)send4.c	1.3";

struct keywd *bkw;

iniky()
{
	register int t;
	register struct keywd *pk;
	t=sbrk(sizeof(*pk));
	ikw=pk=(struct keywd *)t;
	t += sizeof(*pk);
	bkw=pk->nx=(struct keywd *)t;
	pk->nk=pk->nr=0;
	pk->lk = (struct keywd *)pk->nk;
	pk->kp=pk->rp="";
};

gtky(ctx,n,s)
struct context *ctx;
char *s;
{
	register struct keywd *pk,*qk,*rk;
	rk=0;
	qk=ctx->kw1;
	for (pk=ikw;pk<qk;pk=pk->nx)
	if (pk->nk==n && mtch(n,pk->kp,s))
		rk=pk;
	return ((int)rk);
};

dfky(ctx,nks,ks,nrs,rs)
struct context *ctx;
char *ks,*rs;
{
	int tb;
	char *q,*px;
	register int t;
	register char *p;
	register struct keywd *pk;
	if ((t=nks)<=0)
		return;
	if (pk=(struct keywd *)gtky(ctx,t,ks))
	{
		if (pk->nr==(t=nrs) && mtch(t,pk->rp,rs))
		{
			if ((t>0 || pk->rp==rs) && pk>=ctx->kw0)
				return;
		};
	};
	t=(int)ctx->kw1;
	t += sizeof(*pk);
	q=(char *)t;
	t += nks;
	t += nrs;

	/* Force word alignment */

	t += (sizeof(int)) - 1;
	t &= ~((sizeof(int)) - 1);

	pk=(struct keywd *)t;
	if (pk >= bkw) /* leave at least one byte as slop for envir() */
	{
		tb = (t + DBR) & ~(DBR-1);
		if (brk(tb)==(-1))
		{
			prf("Core exceeded.\n"); 
			abt();
		};
		bkw=(struct keywd *)tb;
	};
	pk=ctx->kw1;
	ctx->kw1=(struct keywd *)t;
	pk->nx=(struct keywd *)t;
	t=ks[0]&0377;
	pk->lk=kwx[t];
	kwx[t]=pk;
	pk->nk=nks;
	pk->nr=nrs;
	pk->kp=q;
	px=(p=ks)+nks;
	while (p<px) *q++=(*p++);
	if (rs<=(char *)0377)
	{
		pk->rp=rs; 
		return;
	};
	pk->rp=q;
	px=(p=rs)+nrs;
	while (p<px) *q++=(*p++);
};

/*end*/
