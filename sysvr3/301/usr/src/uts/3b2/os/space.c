/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/space.c	10.7"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/sema.h"
#include "sys/acct.h"

struct	acct	acctbuf;
struct	inode	*acctp;

#include "sys/buf.h"

struct	buf	bfreelist;	/* Head of the free list of buffers */
struct	pfree	pfreelist;	/* Head of physio buffer headers    */
int		pfreecnt;	/* Count of free physio buffers.    */

#include "sys/inode.h"

struct inode	*rootdir;	/* Inode for root directory. */


int	nptalloced;	/* Total number of page tables allocated.	*/
int	nptfree;	/* Number of free page tables.			*/

#include "sys/region.h"

reg_t	ractive;
reg_t	rfree;
reg_t	sysreg;

#include "sys/proc.h"

proc_t	*runq;		/* Head of linked list of running procs. */
proc_t	*curproc;	/* The currently running proc.		 */
int	curpri;		/* Priority of currently running proc.	 */

#include "sys/pfdat.h"

struct pfdat	phead;		/* Head of free page list. */
struct pfdat	*pfdat;		/* Page frame database. */
struct pfdat	**phash;	/* Page hash access to pfdat. */
struct pfdat	ptfree;		/* Head of page table free list. */
int		phashmask;	/* Page hash mask. */

#include "sys/sysinfo.h"

struct sysinfo sysinfo;
struct	syswait	syswait;
struct	syserr	syserr;
struct	dinfo	dinfo;		/* DU perf stat's */
struct  minfo minfo;

#include	"sys/swap.h"
#include	"sys/mount.h"

int	remote_acct;		/* REMOTE flag for acctp */
int	bootstate;		/* DU boot flag */
short	dufstyp;		/* DU file system switch index */

swpt_t	swaptab[MSFILES];
int	nextswap;


/*	Each process has 3 pregions (text, data, and stack) plus
 *	enough for the maximum number of shared memory segments.
 *	We also need one extra null pregion to indicate the end
 *	of the list.  The maximum number of shared memory segments
 *	will be added in reginit.
 */

int	pregpp = 3 + 1;


/*	The following describe the physical memory configuration.
**
**		maxclick - The largest physical click number.
**			   ctob(maxclick) is the largest physical
**			   address configured plus 1.
**
**		physmem	 - The amount of physical memory configured
**			   in clicks.  ctob(maxclick) is the amount
**			   of physical memory in bytes.
**		kpbase	 - The physical address of the start of
**			   the allocatable memory area.  That is,
**			   after all kernel tables are allocated.
**			   Pfdat table entries are allocated for
**			   physical memory starting at this address.
**			   It is always on a page boundary.
*/

int	maxclick;
int	physmem;
int	kpbase;


/*	The following are concerned with the kernel virtual
**	address space.
**
**		kptbl	- The address of the kernel page table.
**			  It is dynamically allocated in startup.c
*/

pde_t	*kptbl;

/*	The following space is concerned with the configuration of
**	NOFILES.  We have to do it this hard way because the code
**	which actually does this is in the assembly language file
**	misc.s and can't include header files or conveniently
**	print things.
**
**	The variable nofiles_cfg is assigned a value here to force
**	it to be in the data region rather than bss.  The value
**	will be overwritten by the code at vstart_s in misc.s.
**	However, this occurs before bss is cleared so that if
**	this variable were in bss, the value written by vstart_s
**	would be cleared in mlsetup when bss is cleared.
*/

int	nofiles_min = NOFILES_MIN;
int	nofiles_max = NOFILES_MAX;
int	nofiles_cfg = 1;		/* Originally configured value. */
