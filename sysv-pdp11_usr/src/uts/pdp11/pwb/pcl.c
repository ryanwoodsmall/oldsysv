/*
 *	@(#)pcl.c	1.3
 *
 *	PCL-11 Multiplexing / Demultiplexing Driver
 *	Permits two-way communications between 16 machines per PCL.
 */

# include "sys/param.h"
# include "sys/sysmacros.h"
# include "sys/types.h"
# include "sys/errno.h"
# include "sys/file.h"
# include "sys/signal.h"
# include "sys/proc.h"
# include "sys/dir.h"
# include "sys/user.h"
# include "sys/pcl.h"
# include "sys/buf.h"
# ifdef pdp11
# include "sys/map.h"
# endif

extern	struct	pcl	pcl_pcl[];	/* communication structures */
extern	int	pcl_cnt;		/* number of channels */
extern int	pclc_cnt;		/* number of pcl's */
extern  struct  pclhw  *pcl_addr[];	/* addresses of pcl hardware */

struct	pcl *	pclsrch();

# define pclRintr	pclxintr	/* int vectors are reversed in hw */
# define pclXintr	pclrintr	/* int vectors are reversed in hw */

# ifdef pdp11
paddr_t	pcl_uba;		/* Unibus address for transfers */
# endif
struct	buf	pcl_buf;	/* buffer header for UBM */
struct	pcl *	pcl_ioq[2];	/* head of linked list of transmissions */
struct	pcl	pclctrl;	/* pcl control channel interface */
struct	pcldb	pcldb[PCLINDX];	/* buffer for debug info */
int		pclindx;	/* index of next debug record */
struct	pcl *	pclspare;	/* available slot for conversation */
int		pclbusy[2];	/* transmitter in use */
struct 	pcl *	psave[2];

/*
 *	open: first time only - call initialization routine.
 *	Find available communications structure	for this channel.
 *	Check that channel is not in use or waiting for close
 *	message from remote.
 */

pclopen(dev, flag)
	int	dev;
	int	flag;
{
	register  struct  pcl  *p;

	if(pcl_buf.b_flags == 0)
		pclinit();
	spl5();
	do {
		if(p = pclsrch(dev)) {
			if(p->pcl_flag & P_OPEN)
				u.u_error = EBUSY;
		} else if((p = pclspare) == NULL)
			u.u_error = ENOSPC;
		if(u.u_error) {
			spl0();
			return;
		}
	} while(pclwait(p));
	p->pcl_dev = dev;
	p->pcl_flag = P_OPEN | P_RETRY | (p->pcl_flag & P_ROPEN);
	p->pcl_pgrp = u.u_procp->p_pgrp;
	p->pcl_icnt = p->pcl_ioff = 0;
	p->pcl_hdr = PCLOPEN | pclchan(dev);
	p->pcl_ocnt = 10;
	pclqueuer(p);
	pclwait(p);
	if(p->pcl_flag & P_XERR) {
		u.u_error = EIO;
		p->pcl_flag = 0;
	}
#	ifdef	PCLNDELAY
	else
	if ( !(flag & FNDELAY) )
	{
		while ( !(p->pcl_flag & P_ROPEN) )
		{
			p->pcl_flag &= ~P_WASC;
			p->pcl_flag |= P_WOPEN;
			sleep((caddr_t)p+2, PCLRPRI);
		}

		p->pcl_hdr = PCLOC | pclchan(dev);
		p->pcl_ocnt = 10;
		pclqueuer(p);
		pclwait(p);
	}
#	endif	PCLNDELAY
	spl0();
}

/*
 *	close: transmit close message to remote.  If header is
 *	set, a transmission is pending and pclxintr will send close
 *	message on completion.
 */

pclclose(dev)
	int	dev;
{
	register  struct  pcl  *p;

	spl5();
	if((p = pclsrch(dev)) != NULL) {
		if ( p->pcl_flag & P_ROPEN )
		{
			if(p->pcl_hdr == 0) {
				p->pcl_hdr = PCLCLOSE | pclchan(dev);
				p->pcl_ocnt = 0;
				pclqueuer(p);
			}
		}
		p->pcl_flag = 0;
	}
	spl0();
}

/*
 *	read: locate corresponding communication structure.
 *	Return characters from buffer, then EOF indication
 *	else wait for next transmission.  Last done only if
 *	remote is still open.
 */

pclread(dev)
	int	dev;
{
	register  struct  pcl  *p;
	register  int  c;

	spl5();
	p = pclsrch(dev);
  retry:
	if(p->pcl_icnt) {
		c = min(u.u_count, p->pcl_icnt);
		move(p->pcl_ibuf + p->pcl_ioff, c, B_READ);
		p->pcl_icnt -= c;
		p->pcl_ioff += c;
	} else if(p->pcl_flag & P_REOF)
		p->pcl_flag &= ~P_REOF;
	else if(p->pcl_flag & P_RERR) {
		p->pcl_flag &= ~P_RERR;
		u.u_error = EIO;
	} else if(p->pcl_flag & P_ROPEN) {
		p->pcl_flag |= P_READ;
		sleep((caddr_t) p, PCLRPRI);
		goto retry;
	} else
		u.u_error = EBADF;
	spl0();
}

/*
 *	write: wait until header is available for use.
 *	Gather up user data into output buffer and arrange
 *	for transmission.  Wait for completion.
 */

pclwrite(dev)
	int	dev;
{
	register  struct  pcl  *p;

	spl5();
	p = pclsrch(dev);
	if((p->pcl_flag & P_ROPEN) == 0) {
		u.u_error = EBADF;
		spl0();
		return;
	}
	/* should add separate wait for xmit and busy */
	pclwait(p);
	if((p->pcl_flag & P_ROPEN) == 0) {
		u.u_error = EBADF;
		spl0();
		return;
	}
	if(u.u_count == 0) {
		p->pcl_hdr = PCLEOF | pclchan(p->pcl_dev);
		p->pcl_ocnt = 10;	/* send data to permit rejection */
		pclqueuer(p);
		pclwait(p);
	} else while(u.u_count && (p->pcl_flag & P_XERR) == 0) {
		p->pcl_ocnt = min(u.u_count, PCLBSZ);
		if(move(p->pcl_obuf, p->pcl_ocnt, B_WRITE))
			break;
		if(p->pcl_ocnt & 01) {
			p->pcl_ocnt++;
			p->pcl_hdr = PCLODATA | pclchan(p->pcl_dev);
		} else
			p->pcl_hdr = PCLEDATA | pclchan(p->pcl_dev);
		pclqueuer(p);
		pclwait(p);
	}
	if(p->pcl_flag & P_XERR) {
		p->pcl_flag &= ~P_XERR;
		u.u_error = EIO;
	}
	else
	if((p->pcl_flag & P_ROPEN) == 0) {
		u.u_error = EBADF;
	}
	pclwake(p, P_WRITE);
	spl0();
}

/*
 *	start: prepare for transmission.  Verify that channel is still open
 *	as this may be a retry, if so, change to a close.
 */

pclxstart(pcln)
int	pcln;	/* pcl # */
{
	register  struct  pcl  *p;
	register  struct  pclhw  *hw;
#	ifdef pdp11
	paddr_t	uba;
#	else
	register  int  uba;
	extern  _sdata;
#	endif

	if((p = pcl_ioq[pcln]) == NULL)
		return;
	if ( !(p->pcl_flag & P_OPEN) && pclhdr(p->pcl_hdr) != PCLCLOSE )
	{
		p->pcl_hdr = PCLCLOSE | pclchan(p->pcl_dev);
		p->pcl_ocnt = 0;
	}
	hw = pcl_addr[pcln];
	pclbusy[pcln] = B_BUSY;
	p->pcl_flag &= ~P_XERR;
	hw->pcl_tcr = TXINIT;
	hw->pcl_tcr |= IE;
#	ifdef pdp11
	uba = pcl_uba + p->pcl_obuf - pcl_buf.b_paddr;
#	else
	uba = (int) p->pcl_obuf - (int) (&_sdata);
#	endif
	hw->pcl_mmr |= MASTER;
	hw->pcl_tba = (short) uba;
	hw->pcl_tcr |= (short) ((uba & EABITS) >> EAOFF);
	hw->pcl_tbc = -(p->pcl_ocnt + sizeof pcl_pcl[0].pcl_hdr);
	hw->pcl_tdb = p->pcl_hdr;
	hw->pcl_tcr |= (pclmach(p->pcl_dev) << DSTOFF) |
	    SNDWD | STTXM | TXNPR | RIB;
	pcldebug(dbxdata, hw->pcl_tcr, hw->pcl_tsr, p->pcl_hdr);
}

/*
 *	start: prepare for reception.
 */

pclrstart(p)
	register  struct  pcl  *p;
{
	register  struct  pclhw  *hw;
#	ifdef pdp11
	paddr_t	uba;
#	else
	register  int  uba;
	extern  _sdata;
#	endif

	hw = pcl_addr[pclpcl(p->pcl_dev)];
	hw->pcl_rbc = -PCLBSZ;
#	ifdef pdp11
	uba = pcl_uba + p->pcl_ibuf - pcl_buf.b_paddr;
#	else
	uba = (int) p->pcl_ibuf - (int) (&_sdata);
#	endif
	hw->pcl_rba = (short) uba;
	hw->pcl_rcr = (short) (RCVDAT | RCNPR | IE |
	     ((uba & EABITS) >> EAOFF));
}

/*
 *	queuer: place transmission request at the end of the
 *	linked list of requests.  Start output if the list
 *	was previously empty. Always called at spl5.
 */

pclqueuer(p)
	register  struct  pcl  *p;
{
	register  struct  pcl  *q;
	register int		pcln;	/* pcl # */

	p->pcl_ioq = NULL;
	q = pcl_ioq[pcln = pclpcl(p->pcl_dev)];
	if(q == NULL) {
		pcl_ioq[pcln] = p;
		pclxstart(pcln);
	} else {
		for(; q->pcl_ioq; q = q->pcl_ioq);
		q->pcl_ioq = p;
	}
}

/*
 *	receiver interrupt: examine incoming header.
 *	Reject transmission if channel not open or input buffer
 *	is not available.  Some headers are not followed
 *	by data - sending a signal, sending status, closing
 *	a channel - and cannot be rejected.
 */

pclRintr(pcln)
	register int		pcln;	/* pcl # */
{
	register struct pcl *	p;
	register struct pclhw *	hw;
	register int		h;
	register int		rsr;
	static short		hdr[2];

	hw = pcl_addr[pcln];
	h = hdr[pcln];
	p = psave[pcln];
	rsr = hw->pcl_rsr;

	if(rsr & ERR)
		if(p == NULL)
			printf("pcl[%d] recv err\n", pcln);
		else {
			p->pcl_flag |= P_RERR;
			pclwake(p, P_READ);
			if ( pclhdr(h) == PCLOPEN )	/* if we are receiving an open */
				p->pcl_flag &= ~P_ROPEN;
		}
	else if(rsr & REJCOM)
		;
	else if(rsr & SUCTXF && p != NULL) {
		p->pcl_ioff = 0;
		if(pclhdr(h) == PCLEOF) {
			p->pcl_flag |= P_REOF;
			p->pcl_icnt = 0;
		} else if ( pclhdr(h) != PCLOPEN && pclhdr(h) != PCLOC ) {
			p->pcl_icnt = PCLBSZ + hw->pcl_rbc - (pclhdr(h) == PCLODATA ? 1 : 0);
		} else
			p->pcl_icnt = 0;
		pclwake(p, P_READ);

	} else if(rsr & DTORDY) {

		register int	dev;

		hdr[pcln] = h = hw->pcl_rdb;
		dev = ((((hw->pcl_rcr & SRCPCL) >> SRCOFF) - 1) << 3) | (pcln << 7) | pclchan(h);
		psave[pcln] = p = pclsrch(dev);

		switch( pclhdr(h) ) {

		case PCLEOF:
		case PCLEDATA:
		case PCLODATA:
			if(p == NULL || (p->pcl_flag & P_OPEN) == 0)
				break;
			if(p->pcl_flag & P_REOF || p->pcl_icnt)
				hw->pcl_rcr |= REJ;
			else
				pclrstart(p);
			pcldebug(dbrdata, hw->pcl_rcr, rsr, h);
			return;
		case PCLOPEN:	/* open initialise */
		case PCLOC:	/* open complete */
			if ( p )
			{
				if ( (p->pcl_flag & (P_WASC|P_OPEN)) == P_OPEN )
				{
					if ( pclhdr(h) == PCLOPEN )
					{
						if(p->pcl_flag & P_ROPEN) {
							p->pcl_flag |= P_WASC;
							p->pcl_flag &= ~P_ROPEN;
							pclwake(p, P_READ | P_WRITE | P_WOPEN);
							break;
						}
					}
					p->pcl_flag |= P_ROPEN;
					pclwake(p, P_WOPEN);
				}
				else
				{
					hw->pcl_rcr |= REJ;
					pcldebug(dbrecv, hw->pcl_rcr, rsr, h);
					return;
				}
			}
			else
			{
				if ( (psave[pcln] = p = pclspare) == NULL )
				{
					hw->pcl_rcr |= REJ;
					pcldebug(dbrecv, hw->pcl_rcr, rsr, h);
					return;
				}
				p->pcl_dev = dev;
			}
			pclrstart(p);
			pcldebug(dbrecv, hw->pcl_rcr, rsr, h);
			return;
		case PCLCTRL:
			psave[pcln] = &pclctrl;
			if((pclctrl.pcl_flag & P_OPEN) == 0)
				break;
			if(pclctrl.pcl_icnt)
				hw->pcl_rcr |= REJ;
			else {
				pclctrl.pcl_dev = dev;
				pclrstart(&pclctrl);
			}
			pcldebug(dbrecv, hw->pcl_rcr, rsr, h);
			return;
		case PCLSIGNAL:
			if(p && p->pcl_flag & P_OPEN)
				signal(p->pcl_pgrp, (h >> 4) & 037);
			break;
		case PCLCLOSE:
			if(p) {
				p->pcl_flag &= ~P_ROPEN;
				if(p->pcl_flag & P_OPEN)
				{
					p->pcl_flag |= P_WASC;
					pclwake(p, P_READ | P_WRITE | P_WOPEN);
				}
			}
			break;
		default:
			printf("pcl[%d] bad hdr: %o\n", pcln, h);
		}
	} else
		printf("pcl[%d] rsr err %o\n", pcln, rsr);

	psave[pcln] = NULL;
	pcldebug(dbrecv, hw->pcl_rcr, rsr, h);
	hw->pcl_rcr = RCINIT;
	hw->pcl_rcr |= IE | RCVWD;
}

/*
 *	transmitter interrupt: wakeup writer on successful transmission.
 *	Also, those waiting for write buffer.  Attempt retry
 *	on rejection if retry bit is set.
 */

pclXintr(pcln)
	register int		pcln;	/* pcl # */
{
	register struct pcl *	p;
	register int		tsr;
	register struct pclhw *	hw;
	register int		savhdr;

	hw = pcl_addr[pcln];
	tsr = hw->pcl_tsr;

	if(pclbusy[pcln] == 0)
	{
		printf("pcl[%d] xmit int %o\n", pcln, tsr);
		savhdr = 0;
	}
	else if(tsr & (ERR | SORE | SUCTXF | TBSBSY)) {
		p = pcl_ioq[pcln];
		pcl_ioq[pcln] = p->pcl_ioq;
		savhdr = p->pcl_hdr;
		if(tsr & MSTDWN)
			hw->pcl_mmr |= MASTER;
		if(tsr & SORE && p->pcl_flag & P_RETRY)
			timeout(pclqueuer, p, PCLDELAY);
		else if(p->pcl_flag & P_OPEN) {
			if(tsr & (SORE | ERR | TBSBSY))
				p->pcl_flag |= P_XERR;
			p->pcl_hdr = NULL;
			pclwake(p, P_WRITE);
		} else if(pclhdr(p->pcl_hdr) != PCLCLOSE) {
			p->pcl_hdr = PCLCLOSE | pclchan(p->pcl_dev);
			p->pcl_ocnt = 0;
			pclqueuer(p);
		} else
			p->pcl_hdr = NULL;
	} else {
		printf("pcl[%d] tsr err %o\n", pcln, tsr);
		savhdr = 0;
	}
	pclbusy[pcln] = 0;
	pcldebug(dbxmit, hw->pcl_tcr, tsr, savhdr);
	pclxstart(pcln);
}

/*
 *	search: locate a communication structure for a channel
 *	given the machine id and the logical channel.  Leave address
 *	of an available slot in pclspare.
 */

struct  pcl  *
pclsrch(dev)
	register  int  dev;
{
	register  struct  pcl  *p, *e;

	e = &pcl_pcl[pcl_cnt];
	pclspare = NULL;
	for(p = pcl_pcl; p != e; p++)
		if(p->pcl_flag & (P_OPEN | P_ROPEN)) {
			if(p->pcl_dev == dev)
				return(p);
		} else if(pclspare == NULL)
			pclspare = p;
	return(NULL);
}

/*
 *	wait: wait until output buffer and header are available
 *	for use in a transmission.
 *	Always called at spl5.
 */

pclwait(p)
	register  struct  pcl  *p;
{
	register  int  ret = 0;

	while(p->pcl_hdr) {
		ret++;
		p->pcl_flag |= P_WRITE;
		sleep((caddr_t) p + 1, PCLWPRI);
	}
	return(ret);
}

/*
 *	control: used to send control information across the
 *	link and synchronize the two sides.
 */

pclioctl(dev, cmd, arg, mode)
{
	register  struct  pcl  *p;
	struct	ctrlmsg {
		char	*addr;
		int	count;
	} cb;

	spl5();
	p = pclsrch(dev);
	if(p->pcl_flag & P_NOCTRL)
		u.u_error = ENOTTY;
	else switch(cmd) {
		case WAIT:
			while((p->pcl_flag & (P_ROPEN | P_WASC)) == 0) {
				p->pcl_flag |= P_WOPEN;
				sleep((caddr_t) p + 2, PCLRPRI);
			}
			if(p->pcl_flag & P_WASC)
				u.u_error = EBADF;
			else {
				pclwait(p);
				p->pcl_hdr = PCLOC | pclchan(dev);
				p->pcl_ocnt = 10;
				pclqueuer(p);
				pclwait(p);
			}
			break;
		case FLAG:
			p->pcl_flag &= ~(P_RETRY | P_NOCTRL);
			p->pcl_flag |= (arg & (P_RETRY | P_NOCTRL));
			break;
		case SIG:
			pclwait(p);
			p->pcl_hdr = PCLSIGNAL | pclchan(dev) |
			    ((arg & 037) << 4);
			p->pcl_ocnt = 0;
			pclqueuer(p);
			pclwait(p);
			break;
		case CTRL:
			if(p->pcl_flags & P_RSTR)
				u.u_error = EPERM;
			else if(copyin((char *) arg, (char *) &cb, sizeof cb))
				u.u_error = EFAULT;
			else if(cb.count == 0 || cb.count > PCLBSZ)
				u.u_error = EINVAL;
			else {
				pclwait(p);
#				ifdef pdp11
				if(copyio(p->pcl_obuf,cb.addr,cb.count,U_WUD)) {
#				else
				if(copyin(cb.addr, p->pcl_obuf, cb.count)) {
#				endif
					u.u_error = EFAULT;
					break;
				}
				p->pcl_ocnt = cb.count;
				p->pcl_hdr = PCLCTRL | pclchan(dev);
				pclqueuer(p);
				pclwait(p);
				if(p->pcl_flag & P_XERR) {
					p->pcl_flag &= ~P_XERR;
					u.u_error = EIO;
				}
				pclwake(p, P_WRITE);
			}
			break;
		case RSTR:
			p->pcl_flags |= P_RSTR;
			break;
		case GET:
			if ( copyin((caddr_t)arg, (caddr_t)&cb, sizeof cb) )
				u.u_error = EFAULT;
			else
			if ( cb.count <= 0 )
				u.u_error = EINVAL;
			else
			{
				if ( cb.count > sizeof (struct pcl) )
					cb.count = sizeof (struct pcl);
				if ( copyout((caddr_t)p, (caddr_t)cb.addr, cb.count) )
					u.u_error = EFAULT;
			}
			break;
		case GDEBUG:
			if ( copyin((caddr_t)arg, (caddr_t)&cb, sizeof cb) )
				u.u_error = EFAULT;
			else
			if ( cb.count <= 0 )
				u.u_error = EINVAL;
			else
			{
				if ( cb.count > sizeof pcldb )
					cb.count = sizeof pcldb;
				if ( copyout((caddr_t)pcldb, (caddr_t)cb.addr, cb.count) )
					u.u_error = EFAULT;
				else
					u.u_rval1 = pclindx;
			}
			break;
		default:
			u.u_error = EINVAL;
		}
	spl0();
}


/*
 *	wakeup: activate roadblocked process.
 */

pclwake(p, flag)
	register  struct  pcl  *p;
	register  int  flag;
{
	if(p->pcl_flag & P_READ & flag)
		wakeup((caddr_t) p);
	if(p->pcl_flag & P_WRITE & flag)
		wakeup((caddr_t) p + 1);
	if(p->pcl_flag & P_WOPEN & flag)
		wakeup((caddr_t) p + 2);
	p->pcl_flag &= ~flag;
}

/*
 *	debug stuff: circular array of values captured at appropriate times.
 *	Contains type of record, command register, status register,
 *	and pcl structure header.
 */

pcldebug(t, c, s, h)
	pcldb_t		t;
	int		c, s, h;
{
	static int	dbcount;

	pcldb[pclindx].pcl_record = (uchar)t;
	pcldb[pclindx].pcl_index = (uchar)(dbcount++);
	pcldb[pclindx].pcl_cmdreg = c;
	pcldb[pclindx].pcl_statreg = s;
	pcldb[pclindx].pcl_header = h;
	if(++pclindx == PCLINDX)
		pclindx = 0;
}

/*
 *	init: allocate physical memory for buffers.  Insure the
 *	existance of a bus master.  Initialize buffer addresses.
 */

# ifdef pdp11

pclinit()
{
	register  struct  pclhw  *hw;
	register  struct  pcl  *p;
	register  char  (*c)[PCLBSZ] = 0;
	int		pcln;		/* pcl # */
	paddr_t  pa;
	int	b;

	if(pclc_cnt < 1 || pclc_cnt > 2) {
		printf("Bad PCL count\n");
		u.u_error = EINVAL;
		return;
	}
	b = PCLBSZ * (2 * pcl_cnt + 1);
	if((pa = ctob((paddr_t) (unsigned) malloc(coremap, btoc(b)))) ==
	    (paddr_t) 0) {
		u.u_error = ENOMEM;
		return;
	}
	pcl_buf.b_paddr = pa;
	for(p = pcl_pcl; p != &pcl_pcl[pcl_cnt]; p++) {
		p->pcl_ibuf = pa + (paddr_t)(unsigned)c++;
		p->pcl_obuf = pa + (paddr_t)(unsigned)c++;
	}
	pclctrl.pcl_ibuf = pa + (paddr_t)(unsigned)c;
	pcl_buf.b_bcount = b;
	pcl_buf.b_flags = B_PHYS;
	pcl_uba = ubmaddr(&pcl_buf, ubmalloc(b));
	for(pcln = 0;pcln < pclc_cnt;pcln++) {
		hw = pcl_addr[pcln];
		hw->pcl_mmr |= MASTER;
		hw->pcl_rcr = RCINIT;
		hw->pcl_rcr |= IE | RCVWD;
	}
}

# else

pclinit()
{
	register struct pclhw	*hw;
	register int		pcln;	/* pcl # */

	if(pclc_cnt < 1 || pclc_cnt > 2) {
		printf("Bad PCL count\n");
		u.u_error = EINVAL;
		return;
	}
	pcl_buf.b_flags = B_PHYS;
	for(pcln = 0;pcln < pclc_cnt;pcln++) {
		hw = pcl_addr[pcln];
		hw->pcl_mmr |= MASTER;
		hw->pcl_rcr = RCINIT;
		hw->pcl_rcr |= IE | RCVWD;
	}
}

# endif

/*  	Clear routine:  Clear PCL in case of power fail.  */

pclclr()
{
	int	psw;
	register struct pcl	*p;
	register int	pcln;
	register struct pclhw	*hw;

printf ("pcl clear");
	if ( pcl_buf.b_flags == 0 ) return;
	psw = spl5();
	for ( pcln = 0; pcln < pclc_cnt; pcln++ ) {
		hw = pcl_addr[pcln];
		hw->pcl_mmr |= MASTER;
		hw->pcl_rcr = RCINIT;
		hw->pcl_rcr |= IE | RCVWD;
		pclbusy[pcln] = 0;
		pclxstart(pcln);
		if ( (p = psave[pcln]) != NULL ) {
			p->pcl_flag |= P_RERR;
			pclwake ( p, P_READ | P_WOPEN );
			psave[pcln] = NULL;
		}
	}
	splx(psw);
}


/*
 *	control open: call initialization routine.  Insure
 *	exclusive use.
 */

pclcopen()
{
	if(pcl_buf.b_flags == 0)
		pclinit();
	if(pclctrl.pcl_flag & P_OPEN)
		u.u_error = EBUSY;
	else
		pclctrl.pcl_flag = P_OPEN;
}

/*
 *	control close: unlock control channel.  Discard any
 *	un-read data.
 */

pclcclose()
{
	spl5();
	pclctrl.pcl_icnt = 0;
	pclctrl.pcl_flag = 0;
	spl0();
}

/*
 *	control read: wait for ioctl control message.
 */

pclcread()
{
	register  struct  pcl  *p;

	p = &pclctrl;
	spl5();
	while(p->pcl_icnt == 0) {
		p->pcl_flag |= P_READ;
		sleep((caddr_t) p, PCLRPRI);
	}
	move(p->pcl_ibuf, min(u.u_count, p->pcl_icnt), B_READ);
	p->pcl_flag &= ~P_READ;
	p->pcl_icnt = 0;
	spl0();
}
