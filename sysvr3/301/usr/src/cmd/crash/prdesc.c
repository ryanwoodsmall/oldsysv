/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:prdesc.c	1.3"
#include "sys/param.h"
#include "a.out.h"
#include "sys/signal.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/var.h"

#include <crash.h>
#include <sys/mount.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/buf.h>
#include "sys/s5param.h"
#include <sys/filsys.h>
#include <sys/sema.h>
#include <sys/comm.h>

extern struct syment *Nrcvd;
extern struct syment *Nsndd;
extern struct syment *Rcvd;
extern struct syment *Sndd;
extern long prdes();
extern long lseek();

prrcvd()
{
	char *token();
	char *arg;
	int cnt,nrcvd;
	int bootstate;
	struct syment *boot;

	if((boot = symsrch("bootstate")) == NULL) {
		printf("bootstate not found in symbol table\n");
		return;
	}
	lseek(mem,prdes((long)boot->n_value,-1),0);
	if(read(mem,(char *)&bootstate, sizeof bootstate) != sizeof bootstate) {
		printf("read error on bootstate\n");
		return;
	}
	if(!bootstate) {
		printf("no network activity\n");
		while(token() != NULL);
		return;
	}
	if((Nrcvd = symsrch("nrcvd")) == NULL) {
		printf("cannot determine size of receive descriptor table\n");
		return;
	}
	if(!Rcvd)
		if((Rcvd = symsrch("rcvd")) == NULL){
			printf("receive descriptor table not found\n");
			return;
		}	
	
	lseek(mem,prdes((long)Nrcvd->n_value,-1),0);
	if(read(mem,(char *)&nrcvd, sizeof nrcvd) != sizeof nrcvd) {
		printf("read error on Nrcvd\n");
		return;
	}

        printf("SLOT ACNT QTYPE  PTRADR  QCNT RCNT CONN SDNACK STATE\n");

	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < nrcvd; cnt++)
			prcvdtab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < nrcvd)
			prcvdtab(cnt,1);
		else {
			printf("%d is out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}

int
prcvdtab(cnt,request)
int cnt,request;
{
	struct rcvd rcvdbuf;
	char type;



	lseek(mem,prdes((long)(Rcvd->n_value + cnt * sizeof rcvdbuf),-1),0);
	if(read(mem,&rcvdbuf,sizeof rcvdbuf) != sizeof rcvdbuf) {
		printf("read error on rcvdbuf\n");
		return;
	}

	if((rcvdbuf.rd_stat == RDUNUSED) && !request)
		return;

	printf ("%4d %4u %c ",
		cnt,
		rcvdbuf.rd_act_cnt,
		((rcvdbuf.rd_qtype & 0xf) == 1) ? 'G' : 'S');
		printf("%s%s%s",
			(rcvdbuf.rd_qtype & RDFILE) ? "f" : " ",
			(rcvdbuf.rd_qtype & RDINODE) ? "i" : " ",
			(rcvdbuf.rd_qtype & RDTEXT) ? "t" : " ");
	printf(" %8x %4u %4u %4u %6x",
		rcvdbuf.r1.rd_inodep,
		rcvdbuf.rd_qcnt,
		rcvdbuf.rd_refcnt,
		rcvdbuf.rd_connid,
		rcvdbuf.rd_sdnack);
		printf("%s%s",
			(rcvdbuf.rd_stat & RDUSED) ? " used" : "",
			(rcvdbuf.rd_stat & RDLINKDOWN) ? " ldown" : "");
	printf("\n");
}

int
prsndd()
{
	char *token();
	char *arg;
	int cnt,nsndd,bootstate;
	struct syment *boot;


	if((boot = symsrch("bootstate")) == NULL) {
		printf("bootstate not found in symbol table\n");
		return;
	}
	lseek(mem,prdes((long)boot->n_value,-1),0);
	if(read(mem,(char *)&bootstate, sizeof bootstate) != sizeof bootstate) {
		printf("read error on bootstate\n");
		return;
	}
	if(!bootstate) {
		printf("no network activity\n");
		while(token() != NULL);
		return;
	}
	if((Nsndd = symsrch("nsndd")) == NULL) {
		printf("cannot determine size of send descriptor table\n");
		return;
	}
	if(!Sndd)
		if((Sndd = symsrch("sndd")) == NULL){
			printf("send descriptor table not found\n");
			return;
		}	
	
	lseek(mem,prdes((long)Nsndd->n_value,-1),0);
	if(read(mem,(char *)&nsndd, sizeof nsndd) != sizeof nsndd) {
		printf("read error on Nsndd\n");
		return;
	}

		
        printf("SLOT RCNT SNDX MNDX CONN  SQUEUE  STATE\n");
	
	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < nsndd; cnt++)
			prsndtab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < nsndd)
			prsndtab(cnt,1);
		else {
			printf("%d is out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}

int
prsndtab(cnt,request)
int cnt,request;
{
	struct sndd snddbuf;

	lseek(mem,prdes((long)(Sndd->n_value + cnt * sizeof snddbuf),-1),0);
	if(read(mem,&snddbuf,sizeof snddbuf) != sizeof snddbuf) {
		printf("read error on snddbuf\n");
		return;
	}

	if((snddbuf.sd_stat == SDUNUSED) && !request)
		return;

	printf ("%4d %4u %4u %4u %4u %8x",
		cnt,
		snddbuf.sd_refcnt,
		snddbuf.sd_sindex,
		snddbuf.sd_mntindx,
		snddbuf.sd_connid,
		snddbuf.sd_queue);
	printf("%s%s",
		(snddbuf.sd_stat & SDUSED) ? " used" : "",
		(snddbuf.sd_stat & SDLINKDOWN) ? "ldown" : "");
	printf("\n");
}
