/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:srmount.c	1.3"
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
#include <sys/buf.h>
#include "sys/s5param.h"
#include <sys/filsys.h>


extern struct syment *Srmount, *Nsrmount, *Inode;
extern long prdes();
extern long lseek();

prsrmount() {
	char *token();
	char *arg;
	int cnt,nsrmount;


	if((Nsrmount = symsrch("nsrmount")) == NULL) {
		printf("cannot determine size of server mount table\n");
		return;
	}
	if(!Srmount)
		if((Srmount = symsrch("srmount")) == NULL){
			printf("server mount table not found\n");
			return;
		}	
	
	lseek(mem,prdes((long)Nsrmount->n_value,-1),0);
	if(read(mem,(char *)&nsrmount, sizeof nsrmount) != sizeof nsrmount) {
		printf("read error on Nsrmount\n");
		return;
	}


	printf("SLOT SYSID RINO MNDX RCNT DOTDOT FLAGS RD\n");

	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < nsrmount ; cnt++)
			prsrmntab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < nsrmount)
			prsrmntab(cnt,1);
		else {
			printf("%d is out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}

int
prsrmntab(cnt,request)
int cnt,request;
{
	struct srmount srmntbuf;


	lseek(mem,prdes((long)(Srmount->n_value + cnt * sizeof srmntbuf),-1),0);
	if(read(mem,&srmntbuf,sizeof srmntbuf) != sizeof srmntbuf) {
		printf("read error on srmntbuf\n");
		return;
	}


	if((srmntbuf.sr_flags == MFREE) && !request)
		return;
	printf ("%4d %5u %4u %4u %4u %6u",
		cnt,
		srmntbuf.sr_sysid,
		((unsigned) srmntbuf.sr_rootinode - (unsigned) Inode->n_value) /		 sizeof(struct inode),
		srmntbuf.sr_mntindx,
		srmntbuf.sr_refcnt,
		srmntbuf.sr_dotdot);
	printf(" %s%s",
		(srmntbuf.sr_flags & MINUSE) ? "inuse" : "",
		(srmntbuf.sr_flags & MINTER) ? "inter" : "");
	printf(" %s\n",(srmntbuf.sr_flags & MRDONLY) ? "ro" : "rw");
}
