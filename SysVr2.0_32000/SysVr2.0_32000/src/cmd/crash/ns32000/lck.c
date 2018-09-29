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
/* @(#)lck.c	1.2 */

#ifdef u3b2 || u3b
#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#ifdef u3b2
#include <sys/sbd.h>
#include <sys/immu.h>
#endif
#include <sys/var.h>
#include <a.out.h>
#endif
#include "crash.h"
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/flock.h>

#if	vax || pdp11 || ns32000
#	define MAXPRNDX		(int)v.ve_proc
#	define MAXINONDX	(int)v.ve_inode
#	define VIRT_PTR(ptr)	(ptr & VIRT_MEM)
#	define FLCKINFO_SYM	"_flckinfo"
#	define PROC_SYM		"_proc"
#	define INODE_SYM	"_inode"
#	define FLINOTAB_SYM	"_flinotab"
#	define FLOX_SYM		"_flox"
#	define FIDS_SYM		"_fids"
#	define SLEEPLCKS_SYM	"_sleeplcks"
#	define FRLOCK_SYM	"_frlock"
#	define FRFID_SYM	"_frfid"
#else
#	define FLCKINFO_SYM	"flckinfo"
#	define PROC_SYM		"proc"
#	define INODE_SYM	"inode"
#	define FLINOTAB_SYM	"flinotab"
#	define FLOX_SYM		"flox"
#	define FIDS_SYM		"fids"
#	define SLEEPLCKS_SYM	"sleeplcks"
#	define FRLOCK_SYM	"frlock"
#	define FRFID_SYM	"frfid"
#endif
#ifdef	u3b2
#	define MAXINONDX ((int)(v.ve_inode - Inode->n_value)/sizeof(struct inode))
#	define VIRT_PTR(ptr)	prdes(ptr, -1)
#	define MEM_FILE		mem
#endif
#ifdef	u3b5
#	define MAXPRNDX		v.v_proc
#	define MAXINONDX	v.v_inode
#	define VIRT_PTR(ptr)	prdes(ptr, -1)
#	define MEM_FILE		mem
#endif
#ifdef	u3b
#	define MAXPRNDX		v.v_proc
#	define MAXINONDX	v.v_inode
#	define VIRT_PTR(ptr)	ptr
#	define MEM_FILE		kmem
#endif

struct	tbl_info {
	long	base,
		size,
		limit_ptr,
		mem_base;
	};
struct	tbl_info proc_tbl,
		 inode_tbl,
		 flino_tbl,
		 flox_tbl;

tbl_cleanup()
{
	if (proc_tbl.base != NULL) free((char *)proc_tbl.base);
	if (inode_tbl.base != NULL) free((char *)inode_tbl.base);
	if (flino_tbl.base != NULL) free((char *)flino_tbl.base);
	if (flox_tbl.base != NULL) free((char *)flox_tbl.base);
}

lck_sigint()
{
	tbl_cleanup();
	sigint();
}

long read_mem(name, buffer, len)
char	name[];
long	buffer;
int	len;
{
struct	syment *symsrch();
struct	syment *sym_ptr;
long	addr;

	if ((sym_ptr = symsrch(name)) == 0) {
		printf("error - Unable to locate %s.\n",name);
		return(NULL);
		}
#if vax || pdp11 || ns32000
	if (readmem((char *)buffer, addr=(long)SYM_VALUE(sym_ptr), len) != len){
#else
	if ( (lseek(MEM_FILE, addr = VIRT_PTR(sym_ptr->n_value), 0) < 0) ||
	     (read(MEM_FILE, (char *)buffer, len) != len)) {
#endif
		printf("error - Unable to read %s\n",name);
		return(NULL);
		}
	return(addr);
}

long read_tbl(name, tbl, elm_cnt, elm_size)
char	name[];
struct	tbl_info *tbl;
int	elm_cnt;
int	elm_size;
{
char	*malloc();

	tbl->size = elm_size * elm_cnt ;
	if ((tbl->base = (long)malloc(tbl->size)) == NULL) {
		printf("error - Not enough memory to read %s table.\n", name);
		return(NULL);
		}
	tbl->limit_ptr = tbl->base + tbl->size - elm_size ;
	return( tbl->mem_base = read_mem(name, tbl->base, tbl->size) );
} 

long ptr_conv(tbl, ptr)
struct	tbl_info tbl;
long	ptr;
{
	if (ptr != NULL) {
		ptr = tbl.base + (VIRT_PTR(ptr) - tbl.mem_base) ;
		if ((ptr < tbl.base)||(ptr > tbl.limit_ptr)) {
			printf("error - next link %x outside of table.\n", ptr);
			return(NULL);
			}
		}
	return(ptr);
}

find_ino(inode_tbl, dev, number)
struct	inode *inode_tbl;
dev_t	dev;
ino_t	number;
{
int	slot;

	for (slot = 0; slot < MAXINONDX; slot++)
		if ((inode_tbl[slot].i_dev == dev)&&(inode_tbl[slot].i_number == number))
			break;
	if (slot >= MAXINONDX) {
		printf("error : _flinotab fl_dev %d fl_number %d not found in inode table.\n", dev,number);
		slot = -1 ;
		}
	return(slot);
}

find_proc(proc_tbl,pid)
struct	proc *proc_tbl;
int	pid;
{
int	slot;

	for (slot = 0; slot < MAXPRNDX; slot++)
		if (proc_tbl[slot].p_pid == pid)
			break;
	if (slot >= MAXPRNDX) {
		printf("error : _flox process id %d not found in proc table.\n",pid);
		slot = -1 ;
		}
	return(slot);
}

prlcks()
{
struct	flckinfo info;
struct	filock	*lptr;
struct	flino	*fptr;
struct	flino	sleep_fid;
int	ino_slot;
int	act_fls,
	free_fls;
int	act_lcks,
	free_lcks,
	sleep_lcks;

	act_fls = free_fls = act_lcks = free_lcks = sleep_lcks =  0;
	proc_tbl.base = inode_tbl.base = flino_tbl.base = flox_tbl.base = NULL ;

	printf("                 INODE   TYPE      START        LEN      PROC      BLOCKING\n");
	printf("                 SLOT                                    SLOT      PROC (SLOT)\n");

	signal(SIGINT, lck_sigint);

	if (read_mem(FLCKINFO_SYM, &info, sizeof(struct flckinfo)) == NULL) return ;

	if ((read_tbl(PROC_SYM, &proc_tbl, MAXPRNDX, sizeof(struct proc)) == NULL)     ||
	    (read_tbl(INODE_SYM, &inode_tbl, MAXINONDX, sizeof(struct inode)) == NULL) ||
	    (read_tbl(FLINOTAB_SYM, &flino_tbl, info.fils, sizeof(struct flino)) == NULL)    ||
	    (read_tbl(FLOX_SYM, &flox_tbl, info.recs, sizeof(struct filock)) == NULL))     {
		tbl_cleanup();
		return(0);
		}

	printf("Active   Locks :\n");
	if (read_mem(FIDS_SYM, (long)(&fptr), sizeof fptr) != NULL) {
		fptr = (struct flino *)ptr_conv(flino_tbl, fptr);
		while (fptr != NULL) {
			++act_fls;
			ino_slot = find_ino(inode_tbl.base, fptr->fl_dev, fptr->fl_number);
			lptr = (struct filock *)ptr_conv(flox_tbl, fptr->fl_flck);
			while (lptr != NULL) {
				++act_lcks;
				printf("%21d      %c     %7ld    %7ld      %4d      -\n",
					ino_slot,
					(lptr->set.l_type == F_RDLCK) ? 'R' : 
					(lptr->set.l_type == F_WRLCK) ? 'W' : '?' ,
					lptr->set.l_start, lptr->set.l_len,
					find_proc(proc_tbl.base,lptr->set.l_pid));
				lptr = (struct filock *)ptr_conv(flox_tbl, lptr->next);
				}
			fptr = (struct flino *)ptr_conv(flino_tbl, fptr->next);
			}
		}

	printf("Sleeping Locks :\n");
	if (read_mem(SLEEPLCKS_SYM, &sleep_fid, sizeof(struct flino)) != NULL) {
		lptr = (struct filock *)ptr_conv(flox_tbl, ((struct flino *)flino_tbl.base)->fl_flck);
		lptr = (struct filock *)ptr_conv(flox_tbl, sleep_fid.fl_flck);
		while (lptr != NULL) {
			++sleep_lcks;
			printf("                    -      %c     %7ld    %7ld      %4d      %-d\n",
				(lptr->set.l_type == F_RDLCK) ? 'R' : 
				(lptr->set.l_type == F_WRLCK) ? 'W' : '?' ,
				lptr->set.l_start, lptr->set.l_len ,
				find_proc(proc_tbl.base,lptr->set.l_pid) ,
				find_proc(proc_tbl.base,lptr->stat.blkpid) );
			lptr =	(struct filock *)ptr_conv(flox_tbl, lptr->next);
			}
		}

	printf("Traversing frlock list.\n");
	if (read_mem(FRLOCK_SYM, (long)(&lptr), sizeof lptr) != NULL) {
		lptr = (struct filock *)ptr_conv(flox_tbl, lptr);
		while (lptr != NULL) {
			++free_lcks;
			lptr = (struct filock *)ptr_conv(flox_tbl, lptr->next);
			}
		}

	printf("Traversing frfid  list.\n");
	if (read_mem(FRFID_SYM, (long)(&fptr), sizeof fptr) != NULL) {
		fptr = (struct flino *)ptr_conv(flino_tbl, fptr);
		while (fptr != NULL) {
			++free_fls;
			fptr = (struct flino *)ptr_conv(flino_tbl, fptr->next);
			}
		}

	printf("\nConfigured : file headers %4d \n",info.fils);
	printf("             record locks %4d \n",info.recs);
	printf("Actual     : file headers %4d  (active %4d   free %4d)\n",
		act_fls+free_fls, act_fls, free_fls);
	printf("             record locks %4d  (active %4d   free %4d   sleeping %4d)\n",
		act_lcks+free_lcks+sleep_lcks, act_lcks,
		free_lcks, sleep_lcks);

	tbl_cleanup();
	while(token() != NULL); /* remove extra arguments. */
	signal(SIGINT, sigint);
	return(0);
}    
