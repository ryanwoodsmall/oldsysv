/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:region.c	1.1"
/* crash/region.c */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "syms.h"
#include <crash.h>

extern struct syment *Inode, *Region;
extern long lseek();
extern long prdes();

prregion()
{
	/* prregion collects any entry arguments and calls
	 * prregtab to print each region table entry.
	 */

	register char *arg;
	register int cnt;
	char *token();

	if((arg = token()) == NULL)
		for(cnt = 0; cnt < v.v_region; cnt++)
			prregtab(cnt, 0);
	else do {
		prregtab(atoi(arg), 1);
	} while(arg = token());
}

prregtab(c, all)
{
	/* prregtab locates and reads each region table entry
	 * into a buffer and prints it.  It calls prlist to
	 * print the list of pointers to page tables.
	 */

	struct region rbuf;
	char *typ;
	long forw,back;
	unsigned nopg;

	if(c <= -1)
		return;
	if(c >= v.v_region) {
		printf("%d out of range\n",c);
		return;
	}
	lseek(mem, prdes((long)(Region->n_value+c*sizeof rbuf),-1),0);
	if(read(mem,&rbuf,sizeof(rbuf)) != sizeof rbuf) {
		printf("read error on region table\n");
		return;
	}

	if(all == 0 && rbuf.r_refcnt == 0)
		return;

	printf("SLOT NSEG PGSZ #VALID REFCNT WCNT   FSIZE   TYPE    FLAGS\n");
	printf("%4d %4d %4d   %4d   %4d %4d %8x ",
		c,rbuf.r_listsz,rbuf.r_pgsz, rbuf.r_nvalid,rbuf.r_refcnt,
		rbuf.r_waitcnt,rbuf.r_filesz);
	switch(rbuf.r_type) {
		case RT_UNUSED:  typ = "unusd"; break;
		case RT_PRIVATE:  typ = "priv"; break;
		case RT_STEXT:  typ = "stext"; break;
		case RT_SHMEM:  typ = "shmem"; break;
		default:  typ = "?"; break;
	}
	printf(" %5s  ",typ);
	printf("%s%s%s%s%s%s%s\n",
		rbuf.r_flags & RG_NOSWAP ? " noswap" : "",
		rbuf.r_flags & RG_NOFREE ? " nofree" : "",
		rbuf.r_flags & RG_DONE ? " done" : "",
		rbuf.r_flags & RG_NOSHARE ? " noshare" : "",
		rbuf.r_flags & RG_LOCK ? " lock" : "",
		rbuf.r_flags & RG_WANTED ? " want" : "",
		rbuf.r_flags & RG_WAITING ? " wait" : "");
	
	printf("\tFORW: ");
	if((forw = ((long)rbuf.r_forw-Region->n_value)/sizeof(struct region))
		>= v.v_region)
		printf("region[]");
	else printf("region[%d]",forw);
	printf("\tBACK: ");
	if((back = ((long)rbuf.r_back-Region->n_value)/sizeof(struct region))
		>= v.v_region)
		printf("region[]");
	else printf("region[%d]",back);
	printf("\tIPTR: ");
	if(rbuf.r_iptr)
		printf("inode[%d]\n",rbuf.r_iptr-(struct inode *)Inode->n_value);
	else
		printf("inode[]\n");
	if(rbuf.r_pgsz) {
		nopg = ((rbuf.r_pgsz + (NPGPT - 1))/NPGPT);
		prlist(rbuf.r_list,nopg);
	}
}



prlist(addr,nopg)
long addr;
unsigned nopg;
{
	/* prlist prints the list of pointers to page tables */

	int list, i;

	printf("\tLIST: %8x\n\t", addr);
	lseek(mem,prdes(addr,-1),0);
	for(i = 0; i < nopg; i++) {
		if(read(mem,(char *)&list,sizeof list) != sizeof list) {
			printf("read error on rlist\n");
			return;
		}
		if(list == NULL)
			break;
		printf("%8x", list);
		if(((i+1) % 4) == 0)
			printf("\n\t");
	}
	printf("\n");
}

