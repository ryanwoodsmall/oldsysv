/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:cirmgr.c	1.2"
#include "stdio.h"
#include "sys/param.h"
#include "a.out.h"
#include "sys/signal.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/sbd.h"

#include "crash.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/gdpstr.h"
#include "sys/cirmgr.h"

extern struct syment *Protocol;
extern struct syment *Gdp;
extern long prdes();
extern long lseek();

int
prproto()
{
	char *token();
	char *arg;
	int cnt;

	if(!Protocol)
		if((Protocol = symsrch("protocol")) == NULL){
			printf("protocol table not found\n");
			return;
		}	

	if(!Gdp)
		if((Gdp = symsrch("gdp")) == NULL){
			printf("gdp table not found\n");
			return;
		}	

	printf("SLOT PMAJ\n");

	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < MAXPROTO; cnt++)
			prprototab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < MAXPROTO)
			prprototab(cnt,1);
		else {
			printf("%d out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}
int
prprototab(cnt,request)
int cnt,request;
{
	struct protocol protobuf;
	struct gdp *ptr;
	struct gdp gdpbuf;
	int slot,i;

	lseek(mem,prdes((long)(Protocol->n_value + cnt * sizeof protobuf),-1),0);
	if(read(mem,&protobuf,sizeof protobuf) != sizeof protobuf) {
		printf("read error on protobuf\n");
		return;
	}
	if ((protobuf.pmajor == 0) && !request)
		return;
	printf ("%4u %4u\n",
		cnt,
		protobuf.pmajor);
	if((ptr = protobuf.head) == NULL)
		return;

	printf("SLOT STATE  PC PSEM IOSL SYSID PMNR  SQUEUE  PTYP PADDRESS\n");
	for(;;) {	
		slot = ((long)ptr - Gdp->n_value) / sizeof(struct gdp);
		if((slot < 0) && (cnt >= MAXGDP)) {
			printf("%x out of range\n",(long)ptr);
			return;
		}
		lseek(mem,prdes((long)(Gdp->n_value + slot * sizeof gdpbuf),-1),0);
		if(read(mem,&gdpbuf,sizeof gdpbuf) != sizeof gdpbuf) {
			printf("read error on gdpbuf\n");
			return;
		}
		if (gdpbuf.queue == NULL) {
			printf("%d gdp not in use\n",slot);
			return;
		}
		printf ("%4d %s%s%s%s%s%s",
			slot,
			(gdpbuf.state & GDPLISTEN) ? "l" : " ",
			(gdpbuf.state & GDPIOFAIL) ? "f" : " ",
			(gdpbuf.state & GDPIOSUCC) ? "s" : " ",
			(gdpbuf.state & IOWAITSLP) ? "i" : " ",
			(gdpbuf.state & GDPDISCONN) ? "d" : " ",
			(gdpbuf.state & GDPCONNECT) ? "c" : " ");
		printf(" %2x %4u %4u %5u %4u %8x %4u ",
			gdpbuf.Pconnection,
			gdpbuf.pconnect,
			gdpbuf.pioctl,
			gdpbuf.sysid,
			gdpbuf.pminor,
			gdpbuf.queue,
			gdpbuf.address.protocol);
		for(i = 0; i < NADDRLEN; i++)
			printf("%02x",gdpbuf.address.praddr.address[i]);
		printf("\n");
		if(ptr == protobuf.tail)
			break;
		ptr = gdpbuf.next;
	}
}


int
prgdp(addr,request)
long addr;
int request;
{
	char *token();
	char *arg;
	long addr,cnt;

	if(!Gdp)
		if((Gdp = symsrch("gdp")) == NULL){
			printf("gdp table not found\n");
			return;
		}	

	printf("SLOT STAT PC PSEM IOSL SYSID PMNR  SQUEUE  PTYP PADDRESS\n");

	if ((arg = token()) == NULL) {
		printf("address expected\n");
		return;
	}

	addr = hextoi(arg);
	cnt = (addr - Gdp->n_value) / sizeof(struct gdp);
	if((cnt >= 0) && (cnt < MAXGDP))
		prgdptab(cnt,request);
	else {
		printf("%x out of range\n",addr);
		while ((arg = token()) != NULL);
		return;
	}
}
int
prgdptab(cnt,request)
int cnt,request;
{
	struct gdp gdpbuf;
	int i;

	lseek(mem,prdes((long)(Gdp->n_value + cnt * sizeof gdpbuf),-1),0);
	if(read(mem,&gdpbuf,sizeof gdpbuf) != sizeof gdpbuf) {
		printf("read error on gdpbuf\n");
		return;
	}

	if ((gdpbuf.queue == NULL) && !request)
		return;
	printf ("%4d %s%s%s%s%s%s",
		cnt,
		(gdpbuf.state & GDPLISTEN) ? "l" : " ",
		(gdpbuf.state & GDPIOFAIL) ? "f" : " ",
		(gdpbuf.state & GDPIOSUCC) ? "s" : " ",
		(gdpbuf.state & IOWAITSLP) ? "i" : " ",
		(gdpbuf.state & GDPDISCONN) ? "d" : " ",
		(gdpbuf.state & GDPCONNECT) ? "c" : " ");
	printf(" %2x %4u %4u %5u %4u %8x %4u ",
		gdpbuf.Pconnection,
		gdpbuf.pconnect,
		gdpbuf.pioctl,
		gdpbuf.sysid,
		gdpbuf.pminor,
		gdpbuf.queue,
		gdpbuf.address.protocol);
	for(i = 0; i < NADDRLEN; i++)
		printf("%02x",gdpbuf.address.praddr.address[i]);
	printf("\n");
}
