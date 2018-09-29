/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:kfp.c	1.2"
/* crash/kfp.c */
/*
 *		Copyright 1984 AT&T
 */
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
#include "sys/region.h"
#include "sys/proc.h"

#include	<crash.h>
#include "sys/nvram.h"

#define	PSW	ubp->u_pcb.psw
#define PCB     ubp->u_pcb
#define FP 1

extern	struct	user *ubp;
extern long prdes();
extern long Kfp;
extern struct xtra_nvr xtranvram;
extern long lseek();

/*
 * This function will print the stack frame pointer for the current
 * process.  If a value is passed, the stack frame pointer is assumed
 * at that address and printed.
 */

prkfp(fp)
long fp;
{
	int panicstr;
	long pcbaddr;
	struct pcb pcbuf;
	
	if(fp == CURRPROC)
	{
		if (getuarea(CURRPROC) == -1)
		{
			printf("bad read of ublock\n");
			return;
		}
	
		lseek(mem, prdes((long)(Panic->n_value),-1),0);

		if((read(mem,(char *)&panicstr,sizeof panicstr) == sizeof panicstr)
			&& (panicstr != 0)) {
			pcbaddr = xtranvram.systate.pcbp;
			lseek(mem,prdes(pcbaddr,CURRPROC),0);
			if(read(mem,(char *)&pcbuf,sizeof pcbuf)
				!= sizeof pcbuf) {
				printf("read error on pcbuf\n");
				return;
			}
			if(pcbuf.psw.CM == PS_USER) {
				printf("process in user mode, no valid Kfp for kernel stack\n");
				return;
			}
			Kfp = pcbuf.regsave[K_FP];
		}
		else {
			printf("Kfp not available for current process on running system\n");
			return;
		}
	}
	else Kfp = fp;
	printf("kfp: %8.8x\n", Kfp);
}
