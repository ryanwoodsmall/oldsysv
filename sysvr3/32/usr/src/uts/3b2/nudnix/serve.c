/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/serve.c	10.45.10.18"
/*
 *	JAMES - remote file server for unix
 *
 *	JAMES is a kernel daemon process that handles requests
 *	for file activity from remote unix systems.
 *
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/inode.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/file.h"
#include "sys/fstyp.h"
#include "sys/fcntl.h"
#include "sys/nami.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/stat.h"
#include "sys/sysinfo.h"
#include "sys/idtab.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/rdebug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/recover.h"
#include "sys/buf.h"
#include "sys/rbuf.h"
#include "sys/hetero.h"
#include "sys/statfs.h"

#define TERMSIG	(1L << (SIGTERM-1))
#define USR1SIG	(1L << (SIGUSR1-1))

int currserv;		/* current number of servers running */

extern	rcvd_t	cr_rcvd();
extern	struct rd_user *cr_rduser();
extern	struct rd_user *alloc_rduser();
extern	int	msgflag;
extern	rcvd_t	rd_recover;
extern	time_t	lbolt;
void		rm_msgs_list(), chkrsig();


serve ()
{
	rcvd_t	de_queue();
	rcvd_t	make_gift (), setrsig ();
	rcvd_t	gift;		/* what the requestor wanted	*/
	rcvd_t	queue;		/* receive descriptor msg arrived on	*/
	register struct	request	*msg_in;
	register struct	response *msg_out;
	register struct sndd *sdp;	/* reply path back to requestor	*/
	register struct gdp *gdpp;
	int	insize,outsize;	/* size of out/incoming msg		*/
	int	ret_val;	/* general ret val		*/
	int	ocount;		/* save incoming u.u_count	*/
	off_t	oldoffset;	/* save incoming u.u_offset	*/
	int	i;		/* temporary variable		*/
	int	bcount;		/* Read block count */
	int	s;
	mblk_t	*bp,*nbp,*sbp,*chkrdq();
	queue_t	*qp;
	struct message *msig;
	struct	rd_user *rduptr;
	int	mandatory;	/* return mandatory lock indicator */
	int	mand_chg;   /* status has changed to mandatory lock
					since open/creat */

	extern	rcvd_t	nsqueue;
	extern	struct proc *s_active;	/* server active list */
	extern	lock_t	procslock;
	extern rcvd_t srmnt();

	/* ignore all signals except SIGKILL and SIGTERM */
	for (i=0; i<NSIG; i++)
		u.u_signal[i] = SIG_IGN;
	u.u_signal[SIGKILL - 1] = 0;
	u.u_signal[SIGTERM - 1] = 0;
	u.u_signal[SIGUSR1 - 1] = 0;
#ifdef DEBUG
	clrilocks();
#endif
	if ((sdp = cr_sndd ()) == NULL) {
		if (s_active)
			psignal(s_active, SIGUSR1);
		u.u_gift = NULL;
		idleserver++;
		serve_exit();
	}
	bcopy ("server", u.u_psargs, 7);

	while (TRUE)  {
		ASSERT(noilocks() == 0);
		bcopy ("server", u.u_comm, 7);
		sdp->sd_queue = NULL;
		sdp->sd_stat = (SDUSED | SDSERVE);
		sdp->sd_srvproc =  NULL;
		u.u_gift	= sdp;	/* for copyin (write, ioctl) */
		u.u_gift->sd_temp = NULL;
		u.u_procp->p_sig &= ~(USR1SIG | TERMSIG);
		if (u.u_procp->p_cursig == SIGUSR1
		  || u.u_procp->p_cursig == SIGTERM)
			u.u_procp->p_cursig = 0;
		idleserver++;
		if((queue = de_queue ((rcvd_t) ANY, &bp, sdp, &insize))==NULL) {
			ASSERT(noilocks() == 0);
			serve_exit();
		}
		u.u_procp->p_rlink = s_active;
		s_active = u.u_procp;
		msig = (struct message *)bp->b_rptr;
		u.u_procp->p_sig = 0;
		u.u_procp->p_cursig = 0;
		u.u_error = 0;
		idleserver--;
		++currserv;
	psmsg:
		mand_chg = 0;
		mandatory = 0;
		msg_in = (struct request *) PTOMSG(msig);

		bcopy ("SERVER", u.u_comm, 7);
		sysinfo.serve++;
		gift = NULL;  ret_val = 0;
		/*
		 *  set up u area for syscall just like in real syscall ()
		 */
		u.u_procp->p_epid = msg_in->rq_pid;
		/* set sysid to index of stream where message came from */
		qp = (queue_t *)msig->m_queue;
		gdpp = GDP(qp);
		u.u_procp->p_sysid = gdpp->sysid;
		u.u_uid = gluid(gdpp, msg_in->rq_uid);
		u.u_gid = glgid(gdpp, msg_in->rq_gid);

		u.u_mntindx	= msg_in->rq_mntindx;  /* for nami stuff */
		u.u_dirp	= msg_in->rq_data;  /* pathname in data part */
		u.u_ap		= u.u_arg;
		u.u_rval1	= 0;
		u.u_gift->sd_copycnt = 0;
		u.u_gift->sd_temp = (int)bp;
		u.u_copymsg	= NULL;
		u.u_copybp	= NULL;
		u.u_syscall	= msg_in->rq_opcode;
		u.u_limit	= msg_in->rq_ulimit;
		u.u_rflags	= 0;

		/* fail request if resource is in funny state - e.g., in fumount */
		if (u.u_syscall != DUSRMOUNT) {
			if  (srmount[u.u_mntindx].sr_flags & MINTER) {
				freemsg (bp); 
				u.u_gift->sd_temp = NULL;
				rmactive (u.u_procp);
				DUPRINT1(DB_RECOVER,"request for MINTER resource \n");
				continue;
			}
			/* following two sd fields used by recovery/fumount */
			sdp->sd_mntindx = u.u_mntindx;
			sdp->sd_srvproc = u.u_procp;
		}

		dinfo.isyscall++;	/* incoming system calls */
		sysinfo.syscall++;	/* total system calls */
		DUPRINT3(DB_SYSCALL,"server: opcode %d (%s)\n",
			u.u_syscall, sysname(u.u_syscall));
		if (currserv > (maxserve-1))
			u.u_procp->p_sig |= USR1SIG;
		else if (idleserver == 0) {
			s = spl5();
			msgflag |= MORE_SERVE;
			splx(s);
			wakeup(&rd_recover->rd_qslp);
		}
 		switch (u.u_syscall)  {
		case DUACCESS:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_fmode;
			set_dir (msg_in, queue);
			u.u_ruid = u.u_uid;
			u.u_rgid = u.u_gid;
			saccess ();
			break;
		case DUCHDIR:
		{
			register struct inode *ip;
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			ip = u.u_cdir;
			/*  bump ip's ref count (need to lock & unlock?)  */
			plock(ip);
			ip->i_count++;
			prele(ip);
			chdir ();
			if (!u.u_error) { 
			    if(gift = make_gift(u.u_cdir, FILE_QSIZE, sdp))  
			 	srmount[u.u_mntindx].sr_refcnt++;
			    else {
				plock (u.u_cdir);
				iput(u.u_cdir);
			    }
		 	    u.u_cdir = NULL;
			} else  {  /* locked above, release if failed  */
				plock (ip);
				ip->i_count--;
				prele (ip);
			}
			break;
		}
		case DUCHMOD:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_fmode;
			set_dir (msg_in, queue);
			chmod ();
			break;
		case DUIUPDATE:
		{
			register struct inode *ip;
			ip = queue->rd_inode;
			freemsg (bp);
			u.u_gift->sd_temp = NULL;
			plock (ip);
			ip->i_flag |= ISYN;
			FS_IUPDAT (ip, &time, &time);
			prele (ip);
			break;
		}
	 	case DUUPDATE:
		{
			register struct mount *mp = queue->rd_inode->i_mntdev;

			freemsg (bp);
			u.u_gift->sd_temp = NULL;
			(*fstypsw[mp->m_fstyp].fs_update)(mp) ;
			break;
		}
		case DUCHOWN:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = (int) gluid(gdpp,
						msg_in->rq_newuid);
			u.u_arg[2] = (int) glgid(gdpp,
						msg_in->rq_newgid);
			set_dir (msg_in, queue);
			chown ();
			break;
		case DUCHROOT:
		{
			register struct inode *ip;
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			if (ip = u.u_rdir) {
				plock(ip);
				ip->i_count++;
				prele(ip);
			}
				/*** need lock & unlock for AP ***/
			chroot ();
			if (!u.u_error) {
			    if(gift = make_gift(u.u_rdir, FILE_QSIZE, sdp))
			 	srmount[u.u_mntindx].sr_refcnt++;
			    else {
				plock (u.u_rdir);
				iput(u.u_rdir);
			    }
			    u.u_rdir = NULL;
			}  else  if (ip)  {
				plock (ip);
				ip->i_count--;
				prele(ip);
			}
			break;
		}
		case DUIPUT:
		{
			register struct srmnt *smp;
			register struct inode *ip;

			dinfo.isyscall--;	
			sysinfo.syscall--;
			ip = queue->rd_inode;
			freemsg (bp);
			u.u_gift->sd_temp = NULL;
			plock(ip);
			if(--srmount[u.u_mntindx].sr_refcnt==0) {
				 printf ("serve DUIPUT: free srmount entry\n");
			         srmount[u.u_mntindx].sr_flags = MFREE;
			}
			del_rcvd(queue,sdp->sd_queue);
			iput(ip);
			break;
		}
		case DUCLOSE:
		{
			register struct inode *ip;
			long count;
			long foffset;

			count = msg_in->rq_count;
			foffset = msg_in->rq_foffset;
			queue = setrsig (msig, queue);
			ip = queue->rd_inode;
			u.u_fmode = msg_in->rq_mode;
			freemsg (bp);
			u.u_gift->sd_temp = NULL;
			plock (ip);
			if ( ip->i_count == 1 && 
			     queue->rd_user_list != NULL &&
			     queue->rd_user_list->ru_fcount > 1) {
				ip->i_count++;
				if (setjmp (u.u_qsav))
					u.u_error = EINTR;
				else
					FS_CLOSEI (ip,(char)u.u_fmode,
					    (short)count,foffset);
				ip->i_count--;
			}
			else {
				if (setjmp (u.u_qsav))
					u.u_error = EINTR;
				else
					FS_CLOSEI (ip,(char)u.u_fmode,
					    (short)count,foffset);
			}
			chkrsig ();
			if (count == 1)
				del_rduser (queue);
			if (srmount[u.u_mntindx].sr_flags & MCACHE)
				ret_val = ip->i_vcode;
			prele (ip);
			break;
		}
		case DUCOREDUMP:
		{
			register struct inode *ip;
			struct argnamei nmarg;
			extern spath();
			extern struct inode *coreip();

			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_cmask = msg_in->rq_cmask;
			set_dir (msg_in, queue);
			u.u_error = 0;
			u.u_dirp = "core";
			u.u_syscall = DUCOREDUMP;
			nmarg.cmd = NI_CREAT;
			nmarg.mode = ((IREAD|IWRITE)>>6)|
				     ((IREAD|IWRITE)>>3)|(IREAD|IWRITE);
			nmarg.ftype = 0;
			ip = namei(spath, &nmarg);
			if (!u.u_error) 
				if(gift = make_gift(ip, FILE_QSIZE, sdp)) {
					srmount[u.u_mntindx].sr_refcnt++;
					prele (ip);
				} else 
					iput(ip);
			break;
		}

		case DUCREAT:
			queue = setrsig (msig, queue);
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_fmode;
			u.u_cmask = msg_in->rq_cmask;
			set_dir (msg_in, queue);
			if (setjmp (u.u_qsav))
				u.u_error = EINTR;
			else
				creat ();
			chkrsig ();
			if (!u.u_error) {
			 	register struct inode *ip;

				ip = (u.u_ofile[0])->f_inode;
				/* don't allow streams devices */
				if ((ip->i_ftype == IFCHR) && (ip->i_sptr != NULL)) {
						closef(u.u_ofile[0]);
						u.u_error = ENOSTR;
						break;
				}

				if(gift = make_gift(ip, FILE_QSIZE, sdp)) {
					srmount[u.u_mntindx].sr_refcnt++;
					unfalloc (u.u_ofile[0]);
				} else {
					DUPRINT3 (DB_SERVE,"serve: creat make_gift fails, ip %x, fp %x\n", ip, u.u_ofile[0]);
					closef(u.u_ofile[0]);
				}
				u.u_ofile[0] = NULL;
				if ( !(FS_ACCESS(ip, IMNDLCK) ))
					mandatory = 1;
			}
			break;

		case DUEXEC:
		case DUEXECE:
		{
			extern	struct inode *eval2_hdr ();
			struct inode *ip;
			struct response *msg;
			int len;
			dinfo.isysexec++;	/* incoming exec's */
			sysinfo.sysexec++;	/* total exec's */
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			if (ip = eval2_hdr())  {
				if ((!u.u_error) &&
			    	    (gift = make_gift(ip, FILE_QSIZE, sdp)))  {
					gift->rd_qtype |= RDTEXT;
					while ((nbp = alocbuf (sizeof(struct response) - DATASIZE + PSCOMSIZ, BPRI_MED)) == NULL);
					msg = (struct response *)PTOMSG(nbp->b_rptr);
					msg->rp_type = RESP_MSG;
					msg->rp_mode = 0;
					if (!FS_ACCESS(ip, ISUID))
						msg->rp_mode |= ISUID;
					if (!FS_ACCESS(ip, ISGID))
						msg->rp_mode |= ISGID;
					if (!FS_ACCESS(ip, ISVTX))
						msg->rp_mode |= ISVTX;
					u.u_copymsg = msg;
					len = strlen(u.u_dent.d_name);
					u.u_msgend = msg->rp_data + len + 1;
					u.u_copymsg->rp_bp = (long)nbp;
					bcopy((caddr_t) u.u_dent.d_name,
						(caddr_t) msg->rp_data,len+1);
					srmount[u.u_mntindx].sr_refcnt++;
					prele(ip);
				} else 
					iput (ip);
			}
			break;
		}
		case DUFCNTL:
			ret_val = rfs_fcntl(bp, msig, msg_in, queue, &mand_chg);
			break;
		case DUFSTAT:
		{
			long lrq_bufptr;

			lrq_bufptr = msg_in->rq_bufptr;
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			plock(queue->rd_inode);
			stat1 (queue->rd_inode, lrq_bufptr);
			prele(queue->rd_inode);
			if (!u.u_error)  {
			   register struct mount *mp;
			   mp = queue->rd_inode->i_mntdev;
			   ((struct stat *)u.u_copymsg->rp_data)->st_dev = mp - mount;

			    adjust_time((struct stat *)u.u_copymsg->rp_data, qp);
			    stat_rmap(gdpp, (struct stat *)u.u_copymsg->rp_data);
			}
			break;
		}
		case DUFSTATFS:
		{
			long lrq_bufptr;
			long lrq_fstyp;
			long lrq_len;

			lrq_bufptr = msg_in->rq_bufptr;
			lrq_fstyp = msg_in->rq_fstyp;
			lrq_len = msg_in->rq_len;

			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			plock(queue->rd_inode);
			statfs1(queue->rd_inode, lrq_bufptr, lrq_len, lrq_fstyp);
			prele(queue->rd_inode);
			break;
		}
		case DUGETDENTS:
		{
			long lrq_base;
			long lrq_count;

			lrq_base = msg_in->rq_base;
			lrq_count = msg_in->rq_count;
			u.u_offset = msg_in->rq_offset;
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			ret_val = FS_GETDENTS (queue->rd_inode,
					    lrq_base,lrq_count);
			if(ret_val > 0) {
				if(u.u_copymsg)
					u.u_copymsg->rp_offset = u.u_offset;
			}
			break;
		}
		case DUIOCTL:
		{
			long lrq_cmd;
			long lrq_fflag;
			long lrq_ioarg;

			lrq_cmd = msg_in->rq_cmd;
			lrq_fflag = msg_in->rq_fflag;
			lrq_ioarg = msg_in->rq_ioarg;
			queue = setrsig (msig, queue);
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			u.u_rflags |= U_RCOPY;
			if(setjmp(u.u_qsav))
				u.u_error = EINTR;
			else
				FS_IOCTL (queue->rd_inode, lrq_cmd,
					  lrq_ioarg, lrq_fflag);
			u.u_rflags &= ~U_RCOPY;
			chkrsig ();
			break;
		}
		case DULBMOUNT:
		{
			register struct inode *ip;
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_newmntindx;
			set_dir (msg_in, queue);
			if ((ip = namei (upath, 0)) != NULL) {
				iput(ip);
				u.u_error = EMULTIHOP;
			}
			break;
		}
			
		case DULINK:
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			link ();
			if (!u.u_error) {
			    if(gift = make_gift(u.u_pdir, FILE_QSIZE, sdp))  {
				prele (u.u_pdir);
				srmount[u.u_mntindx].sr_refcnt++;
		    	    } else
				iput(u.u_pdir);
			    u.u_pdir = NULL;
			}
			break;
		case DULINK1:
		{
			struct argnamei nmarg;
			register struct inode *ip;
			register tmp;

			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			if ((tmp = msg_in->rq_link) != 0) {
				ip = rcvd[tmp].rd_inode;
				if (ip->i_ftype == IFDIR && !suser())
					break;
			}
			else
				ip = NULL;
			nmarg.cmd = NI_LINK;
			nmarg.idev = (ip == NULL) ? -1 : ip->i_dev;
			nmarg.ino = (ip == NULL) ? -1 : ip->i_number;
			(void)namei(upath, &nmarg);
			break;
		}
		case DUMKDIR:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_fmode;
			u.u_cmask = msg_in->rq_cmask;
			set_dir (msg_in, queue);
			mkdir ();
			break;
		case DUMKNOD:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_fmode;
			u.u_arg[2] = (dev_t) msg_in->rq_dev;
			u.u_cmask = msg_in->rq_cmask;
			set_dir (msg_in, queue);
			mknod ();
			break;
		case DUOPEN:
			queue = setrsig (msig, queue);
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_mode;
			u.u_arg[2] = msg_in->rq_crtmode;
			u.u_cmask = msg_in->rq_cmask;
			set_dir (msg_in, queue);
			if (setjmp(u.u_qsav))
				u.u_error = EINTR;
			else
				open ();
			DUPRINT3(DB_SIGNAL, "serve: after open sig %x, cursig %d\n", 
			   u.u_procp->p_sig, u.u_procp->p_cursig);	
			chkrsig ();
			if (!u.u_error) {
				register struct inode *ip;

				ip = (u.u_ofile[0])->f_inode;
				/* don't allow a remote streams device */
				if ((ip->i_ftype == IFCHR) && (ip->i_sptr != NULL)) {
						closef(u.u_ofile[0]);
						u.u_error = ENOSTR;
						break;
				}

				if(gift = make_gift (ip, FILE_QSIZE, sdp)) {
					srmount[u.u_mntindx].sr_refcnt++;
					unfalloc (u.u_ofile[0]);
				} else {
					DUPRINT3 (DB_SERVE,"serve: open make_gift fails, ip %x, fp %x\n", ip, u.u_ofile[0]);
					closef(u.u_ofile[0]);
				}
				if ( !(FS_ACCESS(ip, IMNDLCK) ))
					mandatory = 1;
				u.u_ofile[0] = NULL;
			}
			break;
		case DUREAD:
		{
			register struct inode *ip;
			register int type;
			dinfo.isysread++;	/* incoming read's */
			sysinfo.sysread++;
			queue = setrsig (msig, queue);
			u.u_base = (caddr_t) msg_in->rq_base;
			u.u_offset = msg_in->rq_offset;
			u.u_count = msg_in->rq_count;
			u.u_fmode = msg_in->rq_fmode;
			u.u_segflg = 0;
			ip = queue->rd_inode;
			type = ip->i_ftype;
			oldoffset = u.u_offset;
			ocount = u.u_count;
			bcount = u.u_ior;
			if ( !FS_ACCESS(ip,IMNDLCK) && msig->m_stat & VER1 
					&& !(msg_in->rq_flags & RQ_MNDLCK)) {
				mand_chg = 1;
				break;
			}
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			if (type == IFREG || type == IFDIR || type== IFIFO)
				plock (ip);
			if (setjmp(u.u_qsav)) 
				u.u_error = EINTR;
			else {
				FS_READI (ip);
				if (type == IFREG || type == IFDIR || type== IFIFO)
					prele (ip);
			}
			chkrsig ();
			srmount[u.u_mntindx].sr_bcount += (u.u_ior - bcount);
			ret_val = u.u_count;
			dinfo.ireadch += ocount - u.u_count;	/* incoming ch's read */
			sysinfo.readch += ocount - u.u_count;
			break;
		}
		case DUREADI:
			dinfo.isyscall--;	
			sysinfo.syscall--;
			u.u_base = (caddr_t) msg_in->rq_base;
			u.u_offset = msg_in->rq_offset;
			u.u_count = msg_in->rq_count;
			u.u_fmode = msg_in->rq_fmode;
			u.u_segflg = 0;
			if ( !FS_ACCESS(queue->rd_inode, IMNDLCK) 
					&& msig->m_stat & VER1 
					&& !(msg_in->rq_flags & RQ_MNDLCK)) {
				mand_chg = 1;
				break;
			}
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			bcount = u.u_ior;
			plock (queue->rd_inode);
			FS_READI (queue->rd_inode);
			prele (queue->rd_inode);
			srmount[u.u_mntindx].sr_bcount += (u.u_ior - bcount);
			ret_val = u.u_count;
			break;
		case DURSIGNAL:
		{
			register struct proc *l;
			register int s;
			dinfo.isyscall--;
			sysinfo.syscall--;
			for (l = s_active; l != NULL; l = l->p_rlink) {
				if (l == u.u_procp)
					continue;
				DUPRINT3(DB_SIGNAL,"DURSIGNAL: l->p_sysid = %d  u.u_procp->p_sysid %d\n",l->p_sysid,u.u_procp->p_sysid);
				if ((l->p_epid == (short)msg_in->rq_pid)  && 
				    (l->p_sysid == u.u_procp->p_sysid)) { 
					psignal(l, SIGTERM);
					break;
				}
			}
			if (l == NULL) {
				s=splrf();
				sdp->sd_srvproc = NULL;
				queue =  inxtord(msig->m_dest);
				if ((sbp = chkrdq(queue,msg_in->rq_pid,
				     msg_in->rq_sysid)) == NULL) {
					splx(s);	
				} else {
					splx(s);
					freemsg(bp);
					u.u_gift->sd_temp = NULL;
					bp = sbp;
					msig = (struct message *)bp->b_rptr;
					msig->m_stat |= SIGNAL;
					set_sndd(sdp,(queue_t *)msig->m_queue,msig->m_gindex,msig->m_gconnid);
					goto psmsg;
				}
			}	
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			rmactive(u.u_procp);
			continue;
		}
		case DURMDIR:
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			rmdir ();
			break;
		case DURMOUNT:	
			/* nami part of rmount */
			u.u_arg[1] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			rmount ();
			break;
		case DUSRMOUNT:
			/* Server side of remote mount */
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = (int) msg_in->rq_flag;
			if(gdpp->mntcnt == 0) {
				hibyte(gdpp->sysid) =
					lobyte(loword(msg_in->rq_sysid));
				u.u_procp->p_sysid = gdpp->sysid;
				gdpp->time = msg_in->rq_synctime - time;
			}
			gift = srmnt (gdpp);
			if (u.u_error == 0) {
				gdpp->mntcnt++;
				gift->rd_refcnt++;
			}
			break;
		case DUSRUMOUNT:
		{
			register struct srmnt *smp;
			register struct inode *ip;

			smp = &srmount[u.u_mntindx];
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
			ip = smp->sr_rootinode;
			plock (ip);
			if (srumount (smp, ip)) {
				del_rcvd (queue, sdp->sd_queue);
				gdpp->mntcnt--;
				iput (ip);
			} else
				prele (ip);
			break;
		}
		case DUSEEK:
			ret_val = queue->rd_inode->i_size;
			break;
		case DUSTAT:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_bufptr;
			set_dir (msg_in, queue);
			stat ();
			if (!u.u_error)  {
				register struct mount *mp;
				register dev_t dev;
			 	 dev = ((struct stat *)u.u_copymsg->rp_data)->st_dev;
			   	for(mp = mount; mp < (struct mount *)v.ve_mount; mp++) 
					if((mp->m_flags & MINUSE) && dev == mp->m_dev)
							break;
			   	((struct stat *)u.u_copymsg->rp_data)->st_dev = mp - mount;

			    adjust_time((struct stat *)u.u_copymsg->rp_data, qp);
			    stat_rmap(gdpp, (struct stat *)u.u_copymsg->rp_data);
			}
			break;
		case DUSTATFS:
			u.u_arg[0] = (int) msg_in->rq_data;
			u.u_arg[1] = msg_in->rq_bufptr;
			u.u_arg[2] = sizeof (struct statfs);
			u.u_arg[3] = msg_in->rq_fstyp;
			set_dir(msg_in, queue);
			statfs();
			break;
		case DUUNLINK:
			u.u_arg[0] = (int) msg_in->rq_data;
			set_dir (msg_in, queue);
			unlink ();
			break;
		case DUUTIME:
			{
			time_t stv[2];

			u.u_arg[0] = (int) msg_in->rq_data;	/*  pathname */
			u.u_arg[1] = (int) msg_in->rq_bufptr;   /*  times */
			if (msg_in->rq_bufptr != NULL) {
				u.u_copybp = NULL;	/* can't presend data */
				u.u_segflg = 0;
				if (copyin((caddr_t)msg_in->rq_bufptr, 
					(caddr_t)stv, sizeof(stv))) {
					u.u_error = EFAULT;
					break;
				} else {
					if (gdpp->hetero != NO_CONV) {
						fcanon("ll", (char *)stv, (char *)stv);
					}
					stv[0] -= GDP(qp)->time;
					stv[1] -= GDP(qp)->time;
					u.u_arg[1] = (int) stv;
				}
			}
			set_dir (msg_in, queue);
			u.u_segflg = 1;
			utime ();
			u.u_segflg = 0;
			break;
			}
		case DUUTSSYS:
		{
			register struct inode *ip;
			register struct mount *mp;
			extern struct mount mount[];
			if( msg_in->rq_dev < 0 || msg_in->rq_dev >= (long)v.v_mount){
				 u.u_error = EINVAL;
				break;
			}
			mp = &mount[msg_in->rq_dev];
			if(!(mp->m_flags & MINUSE)) { 
				u.u_error = EINVAL;
				break;
			}
			if((ip = iget(mp, mp->m_mount->i_number)) == NULL) {
				u.u_error = ENONET;
				break;
			}
			u.u_arg[0] = (int) msg_in->rq_bufptr;
			u.u_arg[1] = ip->i_dev;
			u.u_arg[2] = 2;		/* ustat only */
			iput(ip);
			utssys ();
			break;
		}

		case DUWRITE:
			ret_val = rfs_write(bp, msig, msg_in, queue, &mand_chg);
			break;
		case DUWRITEI:
			ret_val=rfs_writei(bp, msig, msg_in, queue, &mand_chg);
			break;
		default:
		   	DUPRINT2(DB_SERVE, "Server: Illegal msg %x\n", 
				u.u_syscall);
			u.u_error = EREMOTE;
			break;
		}
sret:
		rmactive(u.u_procp);
		if (u.u_ofile[0] != NULL) {
			register struct inode *ip;

			ip = (u.u_ofile[0])->f_inode;
			if (gift = make_gift(ip, FILE_QSIZE, sdp)) {
				srmount[msg_in->rq_mntindx].sr_refcnt++;
				unfalloc(u.u_ofile[0]);
			} else {
				DUPRINT3 (DB_SERVE,"serve: ofile make_gift failed, ip %x, fp %x\n", ip, u.u_ofile[0]);
				closef(u.u_ofile[0]);
			}
			u.u_ofile[0] = NULL;
		}
		if (u.u_gift->sd_temp != NULL) {
			freemsg(bp);
			u.u_gift->sd_temp = NULL;
		}
		/*
		 * if msg needs to be sent, calculate size.
		 * else allocate a msg to send.
		 */

		if (msg_out = u.u_copymsg){
			outsize = u.u_msgend - (char *) msg_out;
			msg_out->rp_subyte = 0;
		}
		else  {
			while ((nbp = alocbuf (sizeof (struct response), BPRI_MED)) == NULL);
			msg_out = (struct response *)PTOMSG(nbp->b_rptr);
			msg_out->rp_type = RESP_MSG;
			msg_out->rp_bp = (long)nbp;
			msg_out->rp_subyte = 1;	     /* set to indicate no data */
			outsize = sizeof (struct response) - DATASIZE;
		}
		if (mandatory == 1 || mand_chg == 1) 
			msg_out->rp_cache |= RP_MNDLCK;
		else
			msg_out->rp_cache &= ~RP_MNDLCK;
		/* Send NACK if file status has changed to mandatory lock
		 * after open. */
		if ( mand_chg == 1) {
			msg_out->rp_type = NACK_MSG;
			u.u_error = ENOMEM;
		}

		/* special handling for client caching */
		if (u.u_error == 0) {
			switch (u.u_syscall) {
			case DUSRMOUNT:
				if (srmount[u.u_mntindx].sr_flags & MCACHE)
					ret_val |= MCACHE;
				break;
			case DUREAD: 
				/* check if cache can be reenabled. */
				msg_out->rp_cache = 0;
				if (srmount[u.u_mntindx].sr_flags & MCACHE) {
					int s;
					rduptr = alloc_rduser(queue->rd_inode->i_rcvd);
					s = spl6();
					/* Count number of read messages
					 * occurring while in disabled state.
					 * DATASIZE is size of each server
					 * rcopyout to client */
					if (rduptr->ru_cflag & CACHE_DISABLE){
						/* Compute number of rcopyout
						 * messages to client */
						rcinfo.dis_bread = (((ocount - 1
						    + oldoffset)/DATASIZE) -
					           (oldoffset/DATASIZE)) + 1;
						/* Add one for initial request
						 * message from client */
						rcinfo.dis_bread++;
					}
					if (rduptr->ru_cflag & CACHE_DISABLE &&
				    	   ((lbolt - queue->rd_mtime >= rcache_time)
					   || (lbolt < queue->rd_mtime))) {
						rduptr->ru_cflag &= ~CACHE_DISABLE;
						rduptr->ru_cflag |= CACHE_REENABLE;	
					}
					splx(s);
					if (rduptr->ru_cflag & CACHE_REENABLE &&
					   !FS_ACCESS(queue->rd_inode, IMNDLCK))
						/* turn off cache if file
						   becomes mandatory locked */
						rduptr->ru_cflag = CACHE_OFF;
					if (rduptr->ru_cflag & CACHE_REENABLE) {
						rduptr->ru_cflag &= ~CACHE_REENABLE;
						rduptr->ru_cflag |= CACHE_ENABLE;	
						msg_out->rp_cache = rduptr->ru_cflag;
					}
				}
				msg_out->rp_isize = queue->rd_inode->i_size;
				break;
			case DUOPEN:
			case DUCREAT:
				/* check if cache is allowed */
				msg_out->rp_cache = 0;
				if (srmount[u.u_mntindx].sr_flags & MCACHE) {
					int s;
					ret_val = gift->rd_inode->i_vcode;
					rduptr = alloc_rduser(gift);
					s = spl6();
					if (rduptr->ru_cflag & CACHE_DISABLE &&
				    	   ((lbolt - queue->rd_mtime >= rcache_time)
					   || (lbolt < queue->rd_mtime))) {
						rduptr->ru_cflag &= ~CACHE_DISABLE;
						rduptr->ru_cflag |= CACHE_REENABLE;	
					}
					splx(s);
					if (rduptr->ru_cflag & CACHE_REENABLE &&
					   !FS_ACCESS(queue->rd_inode, IMNDLCK))
						/* turn off cache if file
						   becomes mandatory locked */
						rduptr->ru_cflag = CACHE_OFF;
					if (rduptr->ru_cflag & CACHE_REENABLE) {
						rduptr->ru_cflag &= ~ CACHE_REENABLE;
						rduptr->ru_cflag |= CACHE_ENABLE;
					}
					if (rduptr->ru_cflag & CACHE_ENABLE)
						msg_out->rp_cache = rduptr->ru_cflag;
				}
				break;
			case DUWRITEI:
			case DUREADI:
				/* return i_size */
				msg_out->rp_isize = queue->rd_inode->i_size;
				break;
			case DUFCNTL:
				if (ret_val == F_FREESP) 
					msg_out->rp_isize = queue->rd_inode->i_size;
				break;
			}
		}

		msg_out->rp_opcode = u.u_syscall;
 		if (u.u_error == EDOTDOT ) {
			msg_out->rp_opcode = DUDOTDOT;
			u.u_error = 0;
			DUPRINT2(DB_SYSCALL,"serve:u.u_arg[0] %s\n",u.u_arg[0]);
			DUPRINT2(DB_SYSCALL,"serve: DOTDOT path=%s\n", msg_out->rp_data);
		}

		if (gift) {
			register struct inode *ip = gift->rd_inode;

			msg_out->rp_ftype = ip->i_ftype;
			msg_out->rp_size = ip->i_size;
			msg_out->rp_nlink = ip->i_nlink;
			msg_out->rp_uid = ip->i_uid;
			msg_out->rp_gid = ip->i_gid;
			msg_out->rp_fhandle = (long)ip;
		}
		msg_out->rp_mntindx	= u.u_mntindx;
		msg_out->rp_rval	= ret_val;
		msg_out->rp_errno	= u.u_error;
		/* return any remote signal */
		msg_out->rp_sig		= u.u_procp->p_sig;
		if (u.u_procp->p_cursig)
			msg_out->rp_sig	|= (1L << (u.u_procp->p_cursig - 1));
		msg_out->rp_sysid	= u.u_procp->p_sysid;

		if (sndmsg (sdp, (mblk_t *)msg_out->rp_bp, 
				   outsize, gift) == FAILURE) {
			register int mntindx;
			DUPRINT1 (DB_SERVE, "server sndmsg failed \n");
			/* When sndmsg fails because link is down,
			 * recovery will clean up.  Otherwise, we've
			 * done stuff on the server that we should undo.
			 */

			/* This code assumes that sndmsg fails only because
			 * link is down.  A server that was working in
			 * disconnected resource is done.  Reduce the
			 * count of such servers, and wakeup recovery
			 * if this is the last of them.
			 */
			mntindx = sdp->sd_mntindx;
			if (--srmount[mntindx].sr_slpcnt == 0) {
				wakeup (&srmount[mntindx]);
			}
		}
	}
}

/*
 * pathname evaluation starts at either the
 * current directory or the root directory,
 * depending on whether the filename starts
 * with a slash (/).
 */
set_dir (msg_in, queue)
register struct	request	*msg_in;
register rcvd_t	queue;
{
	u.u_cdir = queue->rd_inode;
	if (msg_in->rq_rrdir) 
		u.u_rdir = rcvd[msg_in->rq_rrdir].rd_inode;
	else
		u.u_rdir = (struct inode *) NULL;
}

/*
 * 	Make a gift to give to a client.
 */
rcvd_t
make_gift (inodep, qsize, out_port)
register struct inode *inodep;
register char qsize;
register sndd_t out_port;	/* who the gift is for */
{
	register rcvd_t gift;

	/* if sd went bad, forget it */
	if (out_port->sd_stat & SDLINKDOWN) {
		u.u_error = ENOLINK;
		return (NULL);
	}

	gift = inodep->i_rcvd;
	if (gift == NULL)  {
		if ((gift = cr_rcvd (qsize, GENERAL)) == NULL) {
			u.u_error = ENOMEM;
			return (NULL);
		}
		gift->rd_inode = inodep;
		inodep->i_rcvd = gift;
	} else 
		gift->rd_refcnt++;

	/* keep track of who we gave it to */
	if ((cr_rduser (gift, out_port->sd_queue)) == NULL) {
		del_rcvd (gift, NULL);
		u.u_error = ENOSPC;
		return (NULL);
	}
	return (gift);
}


rmactive(proc)
struct proc *proc;
/* remove me from the list of active servers */
{
	register struct proc *current;
	extern int currserv;

	currserv--;
	u.u_procp->p_sig &= ~USR1SIG;
	if (u.u_procp->p_cursig == SIGUSR1)
		u.u_procp->p_cursig = 0;
	if ((current = s_active) == proc)
		s_active = s_active->p_rlink;
	else {
		while (current && current->p_rlink != proc) 
			current = current->p_rlink;
		if (current)
			current->p_rlink = current->p_rlink->p_rlink;
	}
}

struct inode *
eval2_hdr()
{
	register struct inode *ip;

	if ((ip = namei(upath, 0)) == NULL)
		return (NULL);
	if (FS_ACCESS(ip, IOBJEXEC)) {
		u.u_error = EACCES;
		iput(ip);
		return(NULL);
	}
	if ((ip->i_flag& ITEXT) == 0 && ip->i_count != 1) {
		register struct file *fp;
		for (fp = file; fp < (struct file *)v.ve_file; fp++)
			if (fp->f_count && fp->f_inode == ip &&
			   (fp->f_flag & FWRITE)) {
				u.u_error = ETXTBSY;
				iput(ip);
				return(NULL);
			}
		}
	ip->i_flag |= ITEXT;
	return(ip);
}

/*This routine is called to search a receive descriptor queue for pid
 *and sysid.  The message which matches will be removed from the queue
*/

#define	rsmsg	((struct request *)PTOMSG(current->b_rptr))
mblk_t *
chkrdq(que,pid,sysid)
rcvd_t que;
long pid, sysid;
{
	mblk_t *current, *deque();
	register int i;
	
	DUPRINT3(DB_SIGNAL, "chkrdq: que %x pid %x\n", que, pid);
	for (i = 0; i < que->rd_qcnt; i++) {  
		current = deque(&que->rd_rcvdq);
		DUPRINT5(DB_SIGNAL, "chkrdq: que %x pid %x rsmsg %x msgpid %x\n"
			, que, pid, rsmsg, rsmsg->rq_pid);
		if ((rsmsg->rq_pid == pid) && (rsmsg->rq_sysid == sysid)) {
			que->rd_qcnt--;
			if (rcvdemp(que))
				rm_msgs_list(que);
			return (current);
		}
		enque(&que->rd_rcvdq, current);
	}
	return(NULL);	/*signal msg not on queue*/
}			

#ifndef u3b

struct inode *
eval_hdr()
{
	cmn_err(CE_WARN, "Call to eval_hdr()");
	return(0);
}

#endif


/*
 *	All dying servers come through here.
 *	A server dies when it is awakened from
 *	de_queue with nothing on its queue.
 */

serve_exit()
{
	register int i;
	extern struct inode *rfs_cdir;
	extern struct proc *s_zombie;
	extern int nzombcnt, nservers;
	extern rcvd_t rd_recover;

	DUPRINT1 (DB_SERVE, "server_exit:\n");
	u.u_procp->p_epid = NULL; 
	u.u_rdir = NULL;
	u.u_cdir = rfs_cdir;
	for (i = 0; i < NOFILE; i++) {
		u.u_ofile[i] = NULL;
		u.u_pofile[i] = 0;
	}
	if (u.u_gift)
		free_sndd (u.u_gift);
	--nservers;
	--idleserver;
	u.u_procp->p_rlink = s_zombie;
	s_zombie = u.u_procp;
	if(++nzombcnt >= 1)	/* clean up zombie servers */
		wakeup(&rd_recover->rd_qslp);
	exit();
}

/*
 *	Don't allow last server to sleep - return ENOMEM.
 */

static void
chkrsig ()
{
	if ((u.u_procp->p_sig & USR1SIG) || u.u_procp->p_cursig == SIGUSR1)
		if (!(u.u_procp->p_sig & TERMSIG)
		  && u.u_procp->p_cursig != SIGTERM)
			if (u.u_error == EINTR)
				u.u_error = ENOMEM;
	u.u_procp->p_sig &= ~TERMSIG; /* server needs sig reset */
	if (u.u_procp->p_cursig == SIGTERM)
		u.u_procp->p_cursig = 0;
}

struct rcvd *
setrsig (m, q)
register struct message *m;
register struct rcvd *q;
{
	if (m->m_stat & SIGNAL) {
		u.u_procp->p_sig |= TERMSIG;
		return (inxtord(m->m_dest));
	}
	return (q);
}



adjust_time (statbuf, qp)
struct stat *statbuf;
queue_t *qp;
{
	register gdp_time; 

	gdp_time = GDP(qp)->time;
	statbuf->st_atime += gdp_time;
	statbuf->st_ctime += gdp_time;
	statbuf->st_mtime += gdp_time;
}

/* This is the rfs_fcntl routine called from the DUFCNTL case statement of 
 * serve.
 */
int
rfs_fcntl(bp, msig, msg_in, queue, mand_chgp)
mblk_t *bp;
struct message *msig;
struct request *msg_in;
rcvd_t queue;
int *mand_chgp;
{
	int cmd, arg, flag, offset;
	register struct inode *ip;
	struct request stkmsg;

	queue = setrsig (msig, queue);
	ip = queue->rd_inode;
	cmd = msg_in->rq_cmd;
	arg = msg_in->rq_fcntl;
	flag = msg_in->rq_fflag;
	offset = msg_in->rq_foffset;

	if ( !FS_ACCESS(ip,IMNDLCK) && msig->m_stat & VER1 
			&& !(msg_in->rq_flags & RQ_MNDLCK)
			&& cmd == F_FREESP) {
		*mand_chgp = 1;
		return (0);;
	}
	if (flag & FRCACH) {	/* is lock cached */
		switch (cmd) {
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:
		case F_CHKFL:
		case F_FREESP:
			if (msg_in->rq_prewrite > 0){
				stkmsg.rq_prewrite = msg_in->rq_prewrite;
				stkmsg.rq_sofar = 0;
				bcopy ( msg_in->rq_data, stkmsg.rq_data, msg_in->rq_prewrite);
				freemsg(bp);
				u.u_gift->sd_temp = NULL;
				u.u_copybp = (mblk_t  *)&stkmsg;
			}
			break;
		}
		flag &= ~FRCACH;
	}
	else {
		freemsg(bp);
		u.u_gift->sd_temp = NULL;
	}	
	if(setjmp(u.u_qsav)) 
		u.u_error = EINTR;
	else if (cmd == F_SETFL)
		FS_FCNTL(ip, F_CHKFL, arg, flag, offset);
	else if (cmd == F_FREESP) {
		plock(ip);
		FREESP(ip, arg, flag, offset);
		prele(ip);
	} else
		FS_FCNTL(ip, cmd, arg, flag, offset); 
	chkrsig ();
	return(cmd);
}

/* The rfs_write function is called from the DUWRITE case statement of 
 * serve.c .  
 */
int
rfs_write(bp, msig, msg_in, queue, mand_chgp)
mblk_t *bp;
struct message *msig;
struct request *msg_in;
rcvd_t queue;
int *mand_chgp;
{
	int offset, rqoffset;
	int ocount;
	register int type;
	register struct inode *ip;
	mblk_t *nbp;
	struct request stkmsg;

	dinfo.isyswrite++;	/* incoming write's */
	sysinfo.syswrite++;
	queue = setrsig (msig, queue);
	ip = queue->rd_inode;
	type = ip->i_ftype;
	u.u_base = (caddr_t) msg_in->rq_base;
	u.u_count = msg_in->rq_count;
	u.u_fmode = msg_in->rq_fmode;
	rqoffset = msg_in->rq_offset;
	if ( !FS_ACCESS(ip,IMNDLCK) && msig->m_stat & VER1 
			&& !(msg_in->rq_flags & RQ_MNDLCK)) {
		*mand_chgp = 1;
		return(0);
	}
	if (type == IFCHR || ip->i_flag & ILOCK ||
	    (queue->rd_user_list->ru_next !=NULL &&
	     type == IFREG && rcacheinit)
	     && msg_in->rq_prewrite > 0){
		stkmsg.rq_prewrite = msg_in->rq_prewrite;
		stkmsg.rq_sofar = 0;
		bcopy ( msg_in->rq_data, stkmsg.rq_data, msg_in->rq_prewrite);
		freemsg(bp);
		u.u_gift->sd_temp = NULL;
		u.u_copybp = (mblk_t *)&stkmsg;
	}
	else if (msg_in->rq_prewrite > 0) {
		msg_in->rq_sofar = 0;
		u.u_copybp = (mblk_t *)msg_in;
	}
	if (type == IFREG || type == IFDIR || type== IFIFO){
		plock (ip);
	} 
	if(u.u_fmode & FAPPEND) {
		u.u_offset = ip->i_size;
		offset = u.u_offset;
	} else
		u.u_offset = rqoffset;
	u.u_segflg = 0;
	ocount = u.u_count;
	if(setjmp(u.u_qsav)) 
		u.u_error = EINTR;
	else {
		WRITEI(ip);
		if (type == IFREG || type == IFDIR || type== IFIFO)
			prele (ip);
	}
	/* free before allocating another */
	if (u.u_gift->sd_temp) {  /* safety valve */
		freemsg((mblk_t *)u.u_gift->sd_temp);
		u.u_gift->sd_temp = NULL;
		u.u_copybp = NULL;
	}
	if(!u.u_copymsg) {
		while ((nbp = alocbuf(sizeof (struct response)-DATASIZE, BPRI_MED)) == NULL);
		u.u_copymsg = (struct response *)PTOMSG(nbp->b_rptr);
		u.u_copymsg->rp_type = RESP_MSG;
		u.u_msgend = u.u_copymsg->rp_data;
		u.u_copymsg->rp_bp = (long)nbp;
	}
	if (u.u_fmode & FAPPEND)
		u.u_copymsg->rp_isize = offset;
	else
		u.u_copymsg->rp_isize = ip->i_size;
	srmount[u.u_mntindx].sr_bcount += (ocount - u.u_count)/1024;
	dinfo.iwritech += ocount - u.u_count;	/* incoming ch's written */
	sysinfo.writech += ocount - u.u_count;
	chkrsig ();
	return (u.u_count);
}
/*
 *The rfs_writei() function is called from the DUWRITEI case statement of 
 *serve.c
 */
rfs_writei(bp, msig, msg_in, queue, mand_chgp)
mblk_t *bp;
struct message *msig;
struct request *msg_in;
rcvd_t queue;
int *mand_chgp;
{	
	register struct inode *ip;
	int ocount;
	struct request stkmsg;

	dinfo.isyscall--;
	sysinfo.syscall--;
	u.u_base = (caddr_t) msg_in->rq_base;
	u.u_count = msg_in->rq_count;
	u.u_fmode = msg_in->rq_fmode;
	ip = queue->rd_inode;
	if ( !FS_ACCESS(ip,IMNDLCK) && msig->m_stat & VER1 
			&& !(msg_in->rq_flags & RQ_MNDLCK)) {
		*mand_chgp = 1;
		return(0);
	}
	if ((u.u_offset = msg_in->rq_offset) < 0)
		u.u_offset = queue->rd_inode->i_size;
	u.u_segflg = 0;
	ocount = u.u_count;
	if (ip->i_ftype == IFCHR || ip->i_flag & ILOCK ||
	    (queue->rd_user_list->ru_next !=NULL &&
	     ip->i_ftype == IFREG && rcacheinit)
	     && msg_in->rq_prewrite > 0){
		stkmsg.rq_prewrite = msg_in->rq_prewrite;
		stkmsg.rq_sofar = 0;
		bcopy ( msg_in->rq_data, stkmsg.rq_data, msg_in->rq_prewrite);
		freemsg(bp);
		u.u_gift->sd_temp = NULL;
		u.u_copybp = (mblk_t *)&stkmsg;
	}
	else if (msg_in->rq_prewrite > 0) {
		msg_in->rq_sofar = 0;
		u.u_copybp = (mblk_t *)msg_in;
	}
	plock(ip);
	WRITEI(ip);
	prele(ip);
	srmount[u.u_mntindx].sr_bcount += (ocount - u.u_count)/1024;
	return(u.u_count);
}
