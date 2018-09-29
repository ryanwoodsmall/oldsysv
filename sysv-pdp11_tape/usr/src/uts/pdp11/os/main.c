/* @(#)main.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/filsys.h"
#include "sys/mount.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/tty.h"
#include "sys/map.h"
#include "sys/var.h"

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

struct inode *rootdir;
unsigned maxmem, physmem;

main(firstaddr)
{
	register int (**initptr)();
	extern int (*init_tbl[])();
	extern icode[], szicode;

	startup(firstaddr);
	/*
	 * set up system process
	 */

	proc[0].p_addr = firstaddr;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	proc[0].p_nice = NZERO;
	u.u_procp = &proc[0];
	u.u_cmask = CMASK;
	u.u_limit = CDLIMIT;


	/*
	 * initialize system tables
	 */

	for (initptr= &init_tbl[0]; *initptr; initptr++)
		(**initptr)();
	maxmem -= x25init();
	maxmem -= msginit();
	seminit();

	printf("avail mem = %D bytes\n", ctob((long)maxmem));
	if(MAXMEM < maxmem)
		maxmem = MAXMEM;
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;
	u.u_rdir = NULL;
	u.u_start = time;

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if (newproc()) {
		expand(USIZE + btoc(szicode));
		estabur((unsigned)0, btoc(szicode), (unsigned)0, 0, RO);
		copyout((caddr_t)icode, (caddr_t)0, szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
	sched();
}

/*
 * iinit is called once (from main) very early in initialization.
 * It reads the root's super block and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super block.
 * Usually because of an IO error.
 */
iinit()
{
	register struct buf *bp;
	register struct filsys *fp;


	(*bdevsw[major(rootdev)].d_open)(minor(rootdev), 1);
	(*bdevsw[major(pipedev)].d_open)(minor(pipedev), 1);
	(*bdevsw[major(swapdev)].d_open)(minor(swapdev), 1);
	bp = bread(rootdev, SUPERB);
	if (u.u_error)
		panic("iinit");
	mount[0].m_bufp = getablk(1);
	fp = (struct filsys *)paddr(mount[0].m_bufp);
	copyio(paddr(bp), fp, sizeof(struct filsys), U_RKD);
	brelse(bp);
	mount[0].m_flags = MINUSE;
	mount[0].m_dev = rootdev;
	fp->s_flock = 0;
	fp->s_ilock = 0;
	fp->s_ronly = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;

	clkset(fp->s_time);
}

/*
 * Initialize clist by freeing all character blocks.
 */
struct chead cfreelist;
cinit()
{
	register n;
	register struct cblock *cp;

	for(n = 0, cp = &cfree[0]; n < v.v_clist; n++, cp++) {
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CLSIZE;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register struct buf *dp;
	register unsigned i;
	paddr_t nbase;

	dp = &bfreelist;
	dp->b_forw = dp->b_back =
	    dp->av_forw = dp->av_back = dp;
	nbase = bufbase;
	for (i=0, bp=buf; i<v.v_buf+v.v_sabuf; i++, bp++) {
		bp->b_dev = NODEV;
		if (i<v.v_sabuf)
			bp->b_paddr = (paddr_t)sabuf[i];
		else {
			bp->b_paddr = nbase;
			nbase += BSIZE;
		}
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_bcount = BSIZE;
		brelse(bp);
	}
	pfreelist.av_forw = bp = pbuf;
	for (; bp < &pbuf[v.v_pbuf-1]; bp++)
		bp->av_forw = bp+1;
	bp->av_forw = NULL;
	for (i=0; i < v.v_hbuf; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];
}
