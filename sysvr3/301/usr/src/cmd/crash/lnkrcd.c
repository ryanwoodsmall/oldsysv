/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:lnkrcd.c	1.2"
#include "stdio.h"
#include "syms.h"
#include "sys/types.h"
#include "sys/macro.h"
#include "sys/param.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/stream.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/gdpstr.h"
#include "sys/pack.h"
#include "crash.h"

extern struct syment *Nlink;
extern struct syment *Lnkrcd;
extern long prdes();
extern long lseek();

int
prlnkrcd()
{
	char *token();
	char*arg;
	int cnt,nlnk;


	if((Nlink = symsrch("nlink")) == NULL) {
		printf("cannot determine size of link record table\n");
		return;
	}
	if(!Lnkrcd)
		if((Lnkrcd = symsrch("lnkrcd")) == NULL){
			printf("link record table not found\n");
			return;
		}	
	
	lseek(mem,prdes((long)Nlink->n_value,-1),0);
	if(read(mem,(char *)&nlnk, sizeof nlnk) != sizeof nlnk) {
		printf("read error on Nlink\n");
		return;
	}

	printf("SLOT CPSN EPSN  OQHEAD   RQHEAD  SACK RACK QCNT SEND L A S STAT DLNK  DSTADR\n");
	
	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < nlnk; cnt++)
			prlnktab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < nlnk)
			prlnktab(cnt,1);
		else {
			printf("%d is out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}

int
prlnktab(cnt,request)
int cnt,request;
{
	struct lnkrcd_st lnkbuf;
	int i;


	lseek(mem,prdes((long)(Lnkrcd->n_value + cnt * sizeof lnkbuf),-1),0);
	if(read(mem,&lnkbuf,sizeof lnkbuf) != sizeof lnkbuf) {
		printf("read error on lnkbuf\n");
		return;
	}

	if((lnkbuf.lr_state == VC_CLOSED) && !request)
		return;

	printf("%4d %4u %4u %8x %8x %4u %4u %4u %4u",
		cnt,
		(int) lnkbuf.lr_cpsn,
		(int) lnkbuf.lr_epsn,
		lnkbuf.lr_outputq.qc_head,
		lnkbuf.lr_retrnsq.qc_head,
		(int) lnkbuf.lr_sackno,
		(int) lnkbuf.lr_rackno,
		(int) lnkbuf.lr_qcnt,
		(int) lnkbuf.lr_sendcnt);
		printf(" %1x %1x %1x ",
		lnkbuf.lr_lnktout,
		lnkbuf.lr_acktout,
		lnkbuf.lr_sendack);
		printf("%s%s%s%s",
			(lnkbuf.lr_state & VC_OPEN) ? "o" : " ",
			(lnkbuf.lr_state & VC_PENDING) ? "p" : " ",
			(lnkbuf.lr_state & VC_LISTEN) ? "l" : " ",
			(lnkbuf.lr_state & VC_UP) ? "u" : " ");
		if(lnkbuf.lr_state == VC_UP) {
			printf(" %4u ",(int) lnkbuf.lr_dstlink);
			for(i = 0; i < NADDRLEN; i++)
				printf("%02x",lnkbuf.lr_dstnadr.address[i]);
		}
		else printf("   -     -  ");
		printf("\n");
}
