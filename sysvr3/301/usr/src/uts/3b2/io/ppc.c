/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/ppc.c	10.9"
/*
 *
 * PPC Peripheral (3B2) PORTS Controller Driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/iu.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/firmware.h"
#include "sys/devcode.h"
#include "sys/cmn_err.h"
#include "sys/pump.h"
#include "sys/sysmacros.h"
#include "sys/cio_defs.h"
#include "sys/pp_dep.h"
#include "sys/queue.h"	
#include "sys/ppc.h"
#include "sys/lla_ppc.h"
#include "sys/ppc_lla.h"
#include "sys/inline.h"

int pp_bnbr;

/*
 * Stuff from self-config master file.
 */
extern char	ppisopen[];		/* Port is open */
/*
 * Xflag modes
 */
#define X_ITIME		0x004		/* enable ITIME and IFS */

/*
 * raw mode parameters (1200-19200 baud)
 */
#define ITIME	2	/* intercharacter timer: 25-50 msec */
#define IFS	64	/* input field size: 64 bytes */

#define SYSG_TIME 1500  	/* timeout for sysgen  change*/
#define CIO_TIME 1500	  	/* timeout for LLC(CIO) commands to complete*/
#define CL_TIME	500		/* Time to compelete output drain on port close*/
#define DRNTIM	500		/* Time for request entries to drain */
/*
 * intercharacter timer values
 * for low baud rates
 */
char pptime[] = {
	0,	/* 0 */
	18,	/* 50 */
	12,	/* 75 */
	9,	/* 110 */
	7,	/* 134.5 */
	6,	/* 150 */
	5,	/* 200 */
	3,	/* 300 */
};
#define PU_BLOCK	256	/* size of  kernel scratch memory for pump */
#define PU_LBLOCK	PU_BLOCK/4
long    scratch[PU_LBLOCK]; /* scratch area for ppc 
                        download used by the pump
			routines */

char	ppc_speeds[16] =
{
	0, 0, B75BPS, B110BPS, B134BPS, B150BPS,
	0, B300BPS, B600BPS, B1200BPS, B1800BPS,
	B2400BPS, B4800BPS, B9600BPS, B19200BPS, 0
};
SG_DBLK sg_dblk;

int pumpflag;

int to_drain = 0;		/* timeout on drain has occured */

/*
 * Minor device definition
 */
#define TP(b,p)	&pp_tty[b*5+p] 	/* tp from board and port*/

#define CB_PER_PPC	4	/* desired number of cblock per port*/
#define INIT_CB		3       /* Additional cblocks given when first
				   port a board is opened */

ppopen(dev, flag)
register dev_t dev;
{
	register struct tty *tp;
	register struct ppcboard *pb;
	register char *cptr;
	register dev_t device;

	device = minor(dev);
	ppisopen[device] = 1;

	/* check if this board is to be pumped */
	if (flag&O_PUMP)
	{
		if (u.u_uid != 0)
		{
			u.u_error = EPERM;
			return;
		}
		pumpflag = 1;
		return;
	}
	if (pb->b_state&SYSGFAIL)
	{
		u.u_error = ENXIO;
		return;
	}
	pb = &pp_board[ppcbid[device]];
	tp = &pp_tty[device]; /* get tty structure */
	
	/* if this port is not open initialize its parameters */
	if (!(tp->t_state&ISOPEN)) 
	{
		splpp();
		/* get entry for device enable*/
		while ((!(lla_xfree(ppcbid[device],ppcpid[device])))||(dstat[device]&CLDRAIN))
		{
			dstat[device] |= WENTRY;
			dstat[device] |= OPDRAIN;
			sleep(&tp->t_cc[1],TTOPRI);
			if (!(dstat[device]&OPDRAIN))
			{
				u.u_error = ENXIO;
				spl0();
				return;
			}
				
			
		}	
		dstat[device] &= ~OPDRAIN;
                spl0();


		splpp();
		ttinit(tp);
		ppc_conn(ppcbid[device],ppcpid[device]);
		spl0();
		ppparm(dev);
		if (u.u_error)
			return;
	}
	else
	{
		ppc_conn(ppcbid[device],ppcpid[device]);
	}
	
	
	splpp();
	/* If delay on open is set wait unit carrier is on */
	if (!(flag&FNDELAY) && !(tp->t_state&CARR_ON))
		while (!(tp->t_state&CARR_ON)) {
			tp->t_state |= WOPEN; /* waiting for open to
					       complete */
			sleep((caddr_t)&tp->t_canq, TTIPRI);
		}
	if ((!(dstat[device]&SUPBUF)) && (ppcpid[device] != CENTRONICS))
	{
        	if (pb->dcb <= INIT_CB)
			pb->dcb = INIT_CB; /* give more cblocks for the
				first port opened on a board */
        	pb->dcb += CB_PER_PPC ; /* get more cblocks for the new port */ 
		dstat[device] |= SUPBUF;
	}
	
	/* call line discipline open */
	(*linesw[tp->t_line].l_open)(tp);
	spl0();
}

ppstart()
{
	int i, j;
	char *ptr;

	pp_bnbr = devcheck(D_PORTS, pp_addr);

	pumpflag = 0;

	for (i = 0; i < pp_bnbr; i++)
	{
		csbit[i] = 1;
		for (ptr = (char *)&pp_board[i]; ptr < ((char *)&pp_board[i] + sizeof(pp_board[i])); ptr++)
		{
				*ptr = 0;
		}
		for (j = 0; j < 5; j++)
		{
			ppcbid[(i*5)+j] = i;
			ppcpid[(i*5)+j] = j;
		}
	}
}

nodrain(tp)
register struct tty *tp;
{
	register sx;

	sx = tp - pp_tty;

	to_drain++;		/* timeout has occured */

	dstat[sx] &= ~CLDRAIN;
	wakeup((caddr_t)&tp->t_oflag);
}
ppclose(dev)
register dev_t dev;
{
	register struct tty *tp;
	int tout; /*time out parameter */
	extern nodrain();
	extern void entlft();
	register dev_t device;


	device = minor(dev);
	ppisopen[device] = 0;

	/* check to see if this device was pump */

	if (pumpflag) {
		pumpflag = 0;
		return;
	}

	tp = &pp_tty[device];

	/* decrement counters for RECEIVE entry queue */
	splpp();
	if (ppcpid[device] != CENTRONICS)
	{
		if (tp->t_rbuf.c_ptr !=NULL)
		{
			pp_board[ppcbid[device]].qcb--;
			if (pp_board[ppcbid[device]].qcb < 0)
			{
				pp_board[ppcbid[device]].qcb=0;
			}
		}

		if (dstat[device]&SUPBUF)
		{
			pp_board[ppcbid[device]].dcb -= CB_PER_PPC; /* reduce cblocks */
			if (pp_board[ppcbid[device]].dcb <= INIT_CB)
				pp_board[ppcbid[device]].dcb -= INIT_CB;
			if (pp_board[ppcbid[device]].dcb <0)
			{
				pp_board[ppcbid[device]].dcb = 0;
			}

			dstat[device] &= ~SUPBUF;
		}
	}

	if (tp->t_state&CARR_ON)
	{
	     	while ((tp->t_outq.c_cc) || (tp->t_state&BUSY)) 
		{
			tp->t_state |= TTIOW;
			dstat[device] |= CLDRAIN;
			tout=timeout(nodrain,tp,CL_TIME);
			sleep((caddr_t)&tp->t_oflag, PZERO);
			if (!(dstat[device]&CLDRAIN))
			{
				break;
			}
			else
			{
				untimeout(tout);
				dstat[device] &= ~CLDRAIN;
			}
		}
		if (dstat[device]&OPDRAIN)
		{
			dstat[device] &= ~OPDRAIN;
			wakeup((caddr_t)&tp->t_cc[1]);
		}
	}
	if (tp->t_state&BUSY)
		tp->t_state &= ~BUSY;
		
	spl0();
	(*linesw[tp->t_line].l_close)(tp);
	ppdis(tp,dev);
}


void
entlft(tp)
register struct tty *tp;
{
	wakeup((caddr_t)&tp->t_oflag);
}


ppread(dev)
{
	register struct tty *tp;
	register dev_t device;

	device = minor(dev);
	tp = &pp_tty[device];
	(*linesw[tp->t_line].l_read)(tp);
}

ppwrite(dev)
{
	register struct tty *tp;
	register dev_t device;

	device = minor(dev);
	tp = &pp_tty[device];
	(*linesw[tp->t_line].l_write)(tp);
}

send_brk(tp,bid,pid,arg)
register struct tty *tp;
register short bid,pid;

{
	register sx;
	register index;

	ttywait(tp);
	index = tp - pp_tty;

	sx=splpp();
	while (!(lla_xfree(bid,pid)))
	{
		dstat[index] |= WENTRY;
		sleep((caddr_t)&tp->t_cc[1],TTOPRI);
	
	}
	dstat[index] |= WBREAK;
	ppc_break(bid,pid,arg);  /* This does not send a break 
		to peripheral, it only drains the output */
	while (dstat[index]&WBREAK)
	{
		sleep((caddr_t)&tp->t_cc[2],TTOPRI);
	}
	splx(sx);
}
ppioctl(dev, cmd, arg, mode)
{
	register struct tty *tp;
	register dev_t device;

	device = minor(dev);

	switch (cmd)
	{
		case TCSBRK:
			tp = &pp_tty[device]; /* get tty structure */
			send_brk(tp,ppcbid[device],ppcpid[device],arg);
			break;
		case PUMP:
			if (pumpflag != 1)
			{
				u.u_error = EPERM;
				return;
			}
			pmctl(dev, cmd, arg, mode);
			break;
		default:
			if (ttiocom(&pp_tty[device], cmd, arg, mode))
				ppparm(dev);
			break;
	}
}

/*  device interrupt handler */ 

ppint(bid)
register short bid;
{
	register struct tty *tp;
	register short pid;
	CENTRY cqe;
	register char *cc_ptr;
	register index;
	register struct ppcboard *pb;
		/*get completion queue entry */
        pb = &pp_board[bid];
	while (lla_cqueue(bid,&cqe) == PASS) {
	if ((bid >= pp_bnbr) || (pb->b_state&SYSGFAIL))
		return;
		tp = TP(bid, cqe.common.codes.bits.subdev);
		index = ((bid * 5) + cqe.common.codes.bits.subdev);
		/* determine the interrupt opcode */
		switch (cqe.common.codes.bytes.opcode) {
		case PPC_RECV:
			sysinfo.rcvint++;

			/* give returned cblock to t_buff */
			if
				((tp->t_rbuf.c_ptr = (caddr_t)cqe.common.addr) ==
					NULL)
					break;

			if (cqe.appl.pc[0]&(RC_FLU|RC_DSR))
			    /*c_count denotes number of unfilled characters
			      in cblock */
			    /* send an empty buffer to ttin */
			    tp->t_rbuf.c_count = tp->t_rbuf.c_size;
			    
			else
			    tp->t_rbuf.c_count = tp->t_rbuf.c_size - 
				 cqe.common.codes.bytes.bytcnt
				     - 1;
			pb->qcb--; /* return cblock */
			if (cqe.appl.pc[0]&RC_BRK)
			{
				(*linesw[tp->t_line].l_input)(tp, L_BREAK);
 				if (tp->t_state&CARR_ON)
 					tp->t_state |= BUSY;
 				ppc_device(bid, cqe.common.codes.bytes.subdev, DR_ABX);
				*tp->t_rbuf.c_ptr = 0; 
			    	tp->t_rbuf.c_count = tp->t_rbuf.c_size - 1;
			}
			(*linesw[tp->t_line].l_input)(tp, L_BUF);
			/* supply buffers */
			ppproc(tp,T_INPUT);
			break;
		case PPC_XMIT:
			sysinfo.xmtint++;

			if ((cc_ptr= (char *)cqe.common.addr)!=NULL)
				putcf(CMATCH((struct cblock *)cc_ptr));

			if (dstat[index]&WENTRY)
			{
				dstat[index] &= ~(SETOPT|WENTRY);
				wakeup((caddr_t)&tp->t_cc[1]);
			}

			tp->t_state &= ~BUSY;
			ppproc(tp,T_OUTPUT);
			
			break;
		case PPC_ASYNC:
			switch (cqe.appl.pc[0]) {
			case AC_BRK:
				sysinfo.rcvint++;
				(*linesw[tp->t_line].l_input)(tp, L_BREAK);
 				if (tp->t_state&CARR_ON)
 					tp->t_state |= BUSY;
 				ppc_device(bid, cqe.common.codes.bytes.subdev, DR_ABX);
				break;
			case AC_DIS:
				sysinfo.mdmint++;
				tp->t_state &= ~CARR_ON;
				signal(tp->t_pgrp, SIGHUP);
				tp->t_pgrp = 0;
				ttyflush(tp, (FREAD|FWRITE));
				break;
			case AC_CON:
				sysinfo.mdmint++;
				if ((tp->t_state & CARR_ON) == 0)
				{
					wakeup((caddr_t)&tp->t_canq);
					tp->t_state |= CARR_ON;
				}
				break;
			case AC_FLU:
				/*
				 * all output cblocks given to
				 * request queue have been taken
				 * by the ppc have been flushed
				 */
				if (dstat[index]&WENTRY)
				{
					dstat[index] &= ~(SETOPT|WENTRY);
					wakeup((caddr_t)&tp->t_cc[1]);
				}
				tp->t_state &= ~BUSY;
				ppproc(tp,T_OUTPUT); /* resume output
						processing */
				break;
			case AC_CLS:
			
				dstat[index] &= ~QENTDRN;
				wakeup((caddr_t)&tp->t_oflag);
				break;
			}
			break;
		case PPC_OPTIONS:
			/* return cblock to free list */
			if ((cc_ptr=(char *)cqe.common.addr) != NULL)
				putcf(CMATCH((struct cblock *)cc_ptr));
			if (dstat[index]&WENTRY)
			{
				dstat[index] &= ~(SETOPT|WENTRY);
				wakeup((caddr_t)&tp->t_cc[1]);
			}

			tp->t_state &= ~BUSY;
			ppproc(tp,T_OUTPUT);

			break;
		case PPC_DISC:		
		case PPC_CONN:
			if (dstat[index]&WENTRY)
			{
				dstat[index] &= ~(SETOPT|WENTRY);
				wakeup((caddr_t)&tp->t_cc[1]);
			}

			tp->t_state &= ~BUSY;
			ppproc(tp,T_OUTPUT);
			break;

		case PPC_DEVICE:
			break;
		case PPC_BRK:
			dstat[index] &= ~WBREAK;
			wakeup((caddr_t)&tp->t_cc[2]);
			break;
		case SYSGEN:

			pb->b_state |= ISSYSGEN;
			wakeup((caddr_t)pb);
			break;
		case NORMAL:
		case FAULT:
		case QFAULT:
			if (E_OPCODE(cqe)==QFAULT)
				cmn_err(CE_WARN,
				"PORTS: QFAULT - opcode= %d, board = %d, \n, subdev = %d, bytecnt = %d, buff address = %x, \n\n",
				E_APPL(cqe,0), bid,
			      	cqe.common.codes.bytes.subdev, E_BYTCNT(cqe),E_ADDR(cqe));
			
			if (E_OPCODE(cqe)==FAULT)
				cmn_err(CE_WARN,
				"PORTS: FAULT - opcode= %d, board = %d, \n, subdev = %d, bytecnt = %d, buff address = %x, \n\n", 
				E_APPL(cqe,0), bid,
			      	cqe.common.codes.bytes.subdev, E_BYTCNT(cqe),E_ADDR(cqe));
			if (!(pb->b_state&CIOTYPE))
				break;
			pb->b_state &= ~CIOTYPE;
			pb->retcode = cqe.common.codes.bytes.opcode;
			wakeup((caddr_t)&pb->qcb);
			break;
		default:
			cmn_err(CE_WARN, "PORTS: unknown completion code: %d\n",
			      cqe.common.codes.bytes.opcode);
			break;
		}
	}
}

ppproc(tp, cmd)
register struct tty *tp;
{
	register short bid,pid;
	register sx;
	register struct cblock *cb_ptr;
	register index;

	index = tp - pp_tty; /* find index of pp_tty[] */
	bid = index / 5;
	pid = index  - (bid*5);

	switch (cmd) {
	case T_WFLUSH:
		if (!(tp->t_state&CARR_ON))
		{
			sx = splpp();
			ppc_device(bid,pid,DR_ABX);
			splx(sx);
		}
		break;
	case T_RESUME:
		ppc_device(bid,pid,DR_RES);
	case T_OUTPUT:
		sx = splpp();

		if (tp->t_state&BUSY)
		{
			splx(sx);
			break;
		}

		if (!(lla_xfree(bid,pid)))
		{
			splx(sx);
			break;
		}

		if (!(CPRES&(*linesw[tp->t_line].l_output)(tp))) {
			splx(sx);
			break;
		}
		else
		{
			ppc_xmit(bid,pid,tp->t_tbuf.c_ptr, tp->t_tbuf.c_count - 1);
			tp->t_tbuf.c_ptr = NULL;
		}
 				
		tp->t_state |= BUSY;
		splx(sx);
		break;

	case T_RFLUSH:

		ppc_device(bid,pid,DR_ABR);
		if (!(tp->t_state&TBLOCK))
			break;
	case T_UNBLOCK:
		tp->t_state &= ~TBLOCK;
		ppc_device(bid,pid,DR_UNB);
		break;
	case T_INPUT:
		if (pid == CENTRONICS)
			break;
		sx=splpp();
		if (tp->t_rbuf.c_ptr != NULL)
		{
			register struct ppcboard *pb;
			register char *cptr;
			pb = &pp_board[bid];
	   		
			pb->qcb++; /* get cblock from ttopen or ttin*/
			if (pb->dcb >= pb->qcb)
	   		{
				ppc_recv(bid,tp->t_rbuf.c_ptr); 
				tp->t_rbuf.c_ptr=NULL;
	     			/* add more cblocks if you can get from
			     			the free list */
				while (pb->dcb > pb->qcb)
				{
		     			if ((cptr = getcf()->c_data)
					   == ((struct cblock *)NULL)->c_data)
			       			break;
		     		       	ppc_recv(bid,cptr);
		 			pb->qcb++;
				}
	    		}
	    		else /* too many cblocks */
	    		{
				/* return cblock to free list */
				putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
				tp->t_rbuf.c_ptr=NULL;
				lla_attn(bid);
				pb->qcb--;
	    		}
		}
		splx(sx);
		break;
	case T_SUSPEND:
		ppc_device(bid,pid,DR_SUS);
		break;
		case T_BREAK:
		
		/*send_brk(tp,bid,pid,0);*/
		break;
	case T_BLOCK:
		tp->t_state |= TBLOCK;
		
		ppc_device(bid,pid,DR_BLK);
		break;
		case T_PARM:
		ppparm(index);
		break;
	}
}
ppparm(dev)
register dev;
{
	register struct tty *tp;
	register  xflag;
	register Options *opt;
	struct cblock *cp;
	register short bid, pid;
	register dev_t device;
	register int	index;

	device = minor(dev);
	tp = TP(ppcbid[device],ppcpid[device]);
	index = tp - pp_tty; /* find index of pp_tty[] */
	bid = index / 5;
	pid = index  - (bid * 5);
	if ((tp->t_cflag&CBAUD) == 0) {
		tp->t_cflag |= HUPCL;
		ppc_device(bid, pid, DR_DIS);
		return;
	}
	splpp();
	while (!(lla_xfree(ppcbid[device],ppcpid[device])))
	{
		dstat[device] |= (SETOPT|WENTRY);
		sleep((caddr_t)&tp->t_cc[1],TTOPRI);
	}
	if ((cp = getcf()) == NULL) {
		u.u_error = EIO;
		spl0();
		return;
	}
	opt = (Options *) cp->c_data;
	opt->line = 0; /* line discipline 0 */
	opt->ld.zero.iflag = tp->t_iflag;
	opt->ld.zero.oflag = tp->t_oflag;
	opt->ld.zero.cflag = tp->t_cflag;
	/* convert baud rate to duart register specification */
	opt->ld.zero.cflag &= ~CBAUD; /* zero baud bits */
	opt->ld.zero.cflag |= ppc_speeds[tp->t_cflag&CBAUD]&0xF;
	if (((opt->ld.zero.cflag)&0xF) == 0) 
	{
		putcf(cp);
		u.u_error = EIO;
		spl0();
		return;
	}


	if (ppcpid[device] == CENTRONICS)
		opt->ld.zero.cflag &= ~CREAD;
	xflag = 0;

	xflag |= X_ITIME;
	if ((tp->t_cflag&CBAUD) <= B300)
		opt->ld.zero.itime = pptime[tp->t_cflag&CBAUD];
	else
		opt->ld.zero.itime = ITIME;

	opt->ld.zero.lflag = xflag;

	/* send options to the ppc */
	
	ppc_option(ppcbid[device],ppcpid[device],(char *)opt,sizeof(Options));
	spl0();
}

badboard(bid)
register bid; /* board number*/
{
	/* sysgen did not work */
	wakeup((caddr_t)&pp_board[bid]);
}

/* pprst is called by the pump routine to reset and sysgen the
   dumb firmware on the ports board */
int pprst(bid)
register int bid;
{
	register int errors;

	if ((pp_bnbr = devcheck(D_PORTS, pp_addr)) <= 0) {

		if (bid < pp_bnbr)
			pp_board[bid].b_state = SYSGFAIL;
		return(PU_DEVCH);
	}

	if ((bid + 1)> pp_bnbr) 
		return(PU_DEVCH);


	errors = 0;
	lla_reset(bid);

	splpp();
	pp_board[bid].b_state &= ~(SYSGFAIL|ISSYSGEN);
	spl0();
	if (ppsysgen(bid) != PASS) {
		cmn_err(CE_WARN, "PORTS: SYSGEN failure on board %d\n", bid);
		errors++;
	}

	return((errors == 0) ? PASS : FAIL);
}


int ppsysgen(bid)
register short bid; /* board number */

{
	extern badboard();
	register struct tty *tp;
	register struct ppcboard *brd_ptr; /* ptr to board structure */
	register tid; /* timer id */
	register short pid; /* port id number */
	/* initialize qentries  !!!!!! */
	brd_ptr = &pp_board[bid];

	splpp();
	brd_ptr->b_state  = 0;
	sg_dblk.request   = (long)&R_QUEUE(bid);
	sg_dblk.complt    = (long)&C_QUEUE(bid);
	sg_dblk.req_size  = RQSIZE;
	sg_dblk.comp_size = CQSIZE;
	sg_dblk.no_rque   = NUM_QUEUES;


	/* try to sysgen the board */
	if (lla_sysgen(bid,&sg_dblk) == PASS)
	{
		tid=timeout(badboard,bid,SYSG_TIME);
		sleep(brd_ptr,PZERO);
		if (brd_ptr->b_state&ISSYSGEN)
		{
			untimeout(tid);
		}
		else
		{
			brd_ptr->b_state = SYSGFAIL;
			spl0();
			return(FAIL);
		}
	}
	else
	{
		brd_ptr->b_state = SYSGFAIL;
		spl0();
		return(FAIL);
	}
	/* set ppc proc routine */
	for (pid=0; pid<=4 ;pid++)
	{
		tp = TP(bid,pid);
		tp->t_proc=ppproc;
		tp->t_state = EXTPROC;
	}
	spl0();
	return(PASS);
}
/*wake up after timeout on disconnect */


ppdis(tp,dev)
register struct tty *tp;
register int dev;

{
	int eflush;
	register dev_t device;
			
	device = minor(dev);

	while (!(lla_xfree(ppcbid[device],ppcpid[device])))
	{
		dstat[device] |= WENTRY;
		sleep(&tp->t_cc[1],TTOPRI);
			
	}	
	splpp();
	/* calculate the number of CBLOCKS to be returned to thesystem */
	if ((eflush=pp_board[ppcbid[device]].qcb - pp_board[ppcbid[device]].dcb) < 0)
		eflush=0;
	if (!ppisopen[device])
	{
		ttyflush(tp, (FREAD | FWRITE));
		tp->t_state &= ~CARR_ON;
		ppc_disc(ppcbid[device], ppcpid[device], (char) eflush,
		    (tp->t_cflag & HUPCL) ? (GR_DTR | GR_CREAD) : GR_CREAD);
	}
	spl0();
}
cio_time(tb)


register struct ppcboard *tb;
{
	wakeup((caddr_t)&tb->qcb);
}
ppdld(bid,mda,pda,mds)
short bid;  			/* board id */
char *mda;			/* mainstore data address */
char *pda;			/* ppc ram address */
short mds;			/* mainstore byte count */
{
	extern cio_time();
	register struct ppcboard *tb;
	register tid;		/* timeout id */
	tb = &pp_board[bid];
	if (tb->b_state&CIOTYPE)
		return(PU_OTHER); /* there is a CIO type command already in
				process */
	splpp();
	tb->b_state |= CIOTYPE;
	if (lla_dlm(bid,mda,pda,mds) == FAIL)
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return (PU_OTHER);
	}
	tid= timeout(cio_time,tb,CIO_TIME);
	sleep((caddr_t)&tb->qcb,PZERO);
	if (!(tb->b_state&CIOTYPE))
	{
		untimeout(tid);
		spl0();
	}
	else
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return(PU_TIMEOUT);
	}
	return (tb->retcode);
}
ppfcf(bid,pda)
short bid; 	 		/* board id */
char *pda;			/* ppc ram address */
{
	extern cio_time();
	register struct ppcboard *tb;
	register tid;		/* timeout id */
	tb = &pp_board[bid];
	if (tb->b_state&CIOTYPE)
		return(PU_OTHER); /* there is a CIO type command already in
				process */
	splpp();
	tb->b_state |= CIOTYPE;
	if (lla_fcf(bid,pda) == FAIL)
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return (PU_OTHER);
	}
	tid= timeout(cio_time,tb,CIO_TIME);
	sleep((caddr_t)&tb->qcb,PZERO);
	if (!(tb->b_state&CIOTYPE))
	{
		untimeout(tid);
		spl0();
	}
	else
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return(PU_TIMEOUT);
	}
	return (tb->retcode);
}
ppdos(bid)
short bid;  		/* board id */
{
	register struct ppcboard *tb;
	tb = &pp_board[bid];
	if (tb->b_state&CIOTYPE)
		return(-1); /* there is a CIO type command already in
				process */
	splpp();
	tb->b_state |= CIOTYPE;
	if (lla_dos(bid) == FAIL)
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return (-2);
	}
	while (tb->b_state&CIOTYPE)
		sleep((caddr_t)&tb->qcb,PZERO);
	spl0();
	return (tb->retcode);
}
ppdsd(bid,mda)
short bid;	  		/* board id */
char	     *mda;		/* mainstore data address*/
{
	register struct ppcboard *tb;
	tb = &pp_board[bid];
	if (tb->b_state&CIOTYPE)
		return(-1); /* there is a CIO type command already in
				process */
	splpp();
	tb->b_state |= CIOTYPE;
	if (lla_dsd(bid,mda) == FAIL)
	{
		tb->b_state &= ~ CIOTYPE;
		spl0();
		return (-2);
	}
	/* wait for completion report */
	while (tb->b_state&CIOTYPE)
		sleep((caddr_t)&tb->qcb,PZERO);
	spl0();
	return (tb->retcode);
}
pppump(pmpr)
register struct pump_st *pmpr;
{
	register slices; /* number of of 256 (PU_BLOCK) byte memory slices */
	register  char *usr_addr; /* user address pointer pointing to the
		user memort to be  moved to kernel space*/

	register i;
	register char *ppc_addr; /* ports address for download */
	long bsize; /* block size to be given to the PPC */
	long rem_size; /* (buffer size)% 256 */
	long rtcod;
	register dev_t device;

	device = minor(pmpr->dev);

	switch (pmpr->cmdcode)
	{
	case PU_DLD: /* download case */
		slices = pmpr->size/PU_BLOCK;
		if ((rem_size = pmpr->size%PU_BLOCK) != 0)
			slices++;
		if (ppcbid[device] >= pp_bnbr)
		{
			u.u_error = ENXIO;
			pmpr->retcode = PU_OTHER;
			return;
		}
		ppc_addr = (char *)pmpr->to_addr;
		usr_addr = (char *)pmpr->bufaddr;
		for (i = 1; i <= slices; i++)
		{
			if ((i == slices) && (rem_size != 0))
				bsize = rem_size;
			else
				bsize = PU_BLOCK;
			if (copyin(usr_addr,scratch,bsize))
			{
				u.u_error = EFAULT;
				pmpr->retcode = PU_OTHER;
				return;
			}
			if ((pmpr->retcode = 
				ppdld(ppcbid[device],scratch,ppc_addr,bsize))
					!= NORMAL)
			{
				return ;
			}
			
			usr_addr += PU_BLOCK;
			ppc_addr += PU_BLOCK;
		}
		break;

	case PU_RST:
		if ((rtcod = pprst(ppcbid[device])) == PASS)
			break;
		else
		{
			if (rtcod == PU_DEVCH)
				pmpr->retcode = PU_DEVCH;
			else
				pmpr->retcode = PU_OTHER;
			return;
		}

	case PU_FCF:
		if ((pmpr->retcode = ppfcf(ppcbid[device],pmpr->to_addr))
			!=  NORMAL)
			return;
		break;
	case  PU_SYSGEN:
		splpp();
		pp_board[ppcbid[device]].b_state &= ~(SYSGFAIL|ISSYSGEN);
		spl0();

		for (i = 1; i <= 4000; i++);
		if (ppsysgen(ppcbid[device]) == PASS)
		{
			pmpr->retcode = SYSGEN;
			return;
		}
		else
		{
			pmpr->retcode = PU_OTHER;
			return;
		}

	default:
		pmpr->retcode = PU_OTHER;
		cmn_err(CE_WARN, "PORTS: Unknown pump command: %d\n",
			pmpr->cmdcode);
		return;
	}
	pmpr->retcode = NORMAL;
	return;
}


ppclr()
{
	register int bid, pid;
	register struct tty *tp;

/*
 * The systm has detected a power failure, and is about to go down:
 *
 *	1. Send a special notice to the firmware
 *	2. Mark all boards as down, so as to fail any further attempts
 *		to reference them
 *	3. Wake up any processes sleeping very deeply
 */

	for (bid = 0; bid < pp_bnbr; bid++) {
		ppc_clr(bid);

		pp_board[bid].b_state = SYSGFAIL;

		wakeup(&pp_board[bid]);
		wakeup((caddr_t)&pp_board[bid].qcb);
	}

	return;
}

pmctl(dev, cmd, val, mode)
{
	struct pump_st pump;

	if (copyin((char *) val, (char *) &pump, sizeof(struct pump_st)))
	{
		u.u_error = EFAULT;
		return;
	}

	pppump(&pump);

	if (copyout((char *) &pump, (char *) val, sizeof(struct pump_st)))
	{
		u.u_error = EFAULT;
		return;
	}
	return;
}
