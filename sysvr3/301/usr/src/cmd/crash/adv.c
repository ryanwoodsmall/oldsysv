/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:adv.c	1.3"
#include "stdio.h"
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

#include "crash.h"
#include "macros.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/s5param.h"
#include "sys/filsys.h"
#include "sys/adv.h"



extern struct syment *Nadv;
extern struct syment *Advbuf;
extern long prdes();
extern long lseek();

int
pradv()
{
	char *token();
	char *arg;
	int cnt,nadv;

	if((Nadv = symsrch("nadvertise")) == NULL) {
		printf("cannot determine size of advertise table\n");
		return;
	}
	if(!Advbuf)
		if((Advbuf = symsrch("advertise")) == NULL){
			printf("advertise table not found\n");
			return;
		}	
	lseek(mem,prdes((long)Nadv->n_value,-1),0);
	if(read(mem,(char *)&nadv, sizeof nadv) != sizeof nadv) {
		printf("read error on Nadv\n");
		return;
	}

	printf("SLOT CNT       NAME       QUEUE    FLAGS\n");

	if ((arg = token()) == NULL) 
		for (cnt = 0; cnt < nadv; cnt++)
			pradvtab(cnt,0);
	else do
		if ((cnt = atoi(arg)) < nadv)
			pradvtab(cnt,1);
		else {
			printf("%d out of range\n",cnt);
			return;
		}
	while ((arg = token()) != NULL);
}
int
pradvtab(cnt,request)
int cnt,request;
{
	struct advertise advbuf;

	lseek(mem,prdes((long)(Advbuf->n_value + cnt * sizeof advbuf),-1),0);
	if(read(mem,&advbuf,sizeof advbuf) != sizeof advbuf) {
		printf("read error on advbuf\n");
		return;
	}

	if ((advbuf.a_flags == MFREE) && !request)
		return;
	printf("%4d %4u %-14s %8x %s%s\n",
		cnt,
		advbuf.a_count,
		advbuf.a_name,
		advbuf.a_queue,
		advbuf.a_flags & MINUSE? " inuse" : "",
		advbuf.a_flags & MRDONLY? " rdonly" : "");
}
