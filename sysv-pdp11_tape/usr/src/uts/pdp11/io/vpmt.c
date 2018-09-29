/*	vpmt.c 1.10 of 3/16/82
	@(#)vpmt.c	1.10
 */



#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/dir.h"
#ifdef	u3b
#include "sys/istk.h"
#endif
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/map.h"
#include "sys/csi.h"
#include "sys/csihdw.h"
#include "sys/vpmt.h"
#include "sys/file.h"


#define BUFSIZE	512		/*Buffer size*/
#define VPMPRI	(PZERO+5)

/*vpm state bits*/

#define TOPEN	01	/* Device open */
#define BDEVSET	02	/* Connected to VPM protocol module */
#define VPMERR	04	/* Error state set */
#define XOPEN	010	/* Exclusive open */
#define CTLOPEN	020	/* Open for bdevset only */
#define WRPT	040	/* interpretrer report waiting */
#define WSTART  0100    /* waiting foor startup report */
#define VPMBAD  0200	/* vpminit fatal error */
#define VPMFLAG 0400   /* xmtq empty */

extern int vpm_cnt;
extern struct vpmt vpmt[];
extern struct csibd vpmtbd[];
extern struct vpminfo vpminfo;
extern int vpmbsz;

extern struct csibd *csideq();
extern struct csibd *csibget();

struct csibuf vpmtbmt;

vpmopen(dev, flag)
{
	register struct vpmt *vp;
	register int ec;
	register int k;
	register struct csibd *bdp;
	struct csiflags *flagp;
	extern vpmtrint();
	struct csiflags csiflag;
	int i;




	if (dev >= vpm_cnt) {
		u.u_error = ENXIO;
		return;
	}
	vp = &vpmt[dev];
	flagp = &csiflag;
	flagp->options = 0;
	flagp->intopts = 0;
	flagp->window = 0;
	flagp->timer0 = 0;
	flagp->timer1 = 0;
	if (vp->vt_state & VPMBAD) {
		u.u_error = ENOMEM;
		return;
	}
	if (flag&FNDELAY) {
		vp->vt_state |= CTLOPEN;
		csisave(vp->vt_idev,'o',vp->vt_state,0);
		return;
	}
	if ((vp->vt_state&BDEVSET) == 0) {
		u.u_error = EUNATCH;
		csisave( vp->vt_idev,'k', vp->vt_state, (int)vp->vt_idev);
		return;
	}
	if ((vp->vt_state&TOPEN) == 0) {
		flagp->intopts=vp->vt_intopts;
		flagp->options=vp->vt_options;
		vp->vt_state |= WSTART;
		if(ec=csistart(vp->vt_idev,flagp)) {
			csisave(vp->vt_idev,'p',ec,0);
			if(ec==STERR)
				u.u_error=EIO;
			else
				u.u_error=ENOCSI;
			return;
		}
		csisave(vp->vt_idev,'s',flagp->options,flagp->intopts);
		csiseterrs(vp->vt_idev,vp->vt_errset,NOERRS);
		spl5();
		while(WSTART&vp->vt_state)
			if( sleep((caddr_t)&vp->vt_state,PCATCH|VPMPRI) )
				vp->vt_state &= ~WSTART;
		spl0();
		if((TOPEN&vp->vt_state)==0) {
			u.u_error = EIO;
			csistop(vp->vt_idev);
			return;
		}
		if((flag&FREAD)&&(flag&FWRITE))
			vp->vt_state |= XOPEN;
		for (k = 0; k < vpminfo.ebq; k++) {
			if ((bdp = csibget(&vpmtbmt,BUFSIZE,NVSLP)) == NULL){
				csistop(vp->vt_idev);
				u.u_error = ENOMEM;
				vp->vt_state &= BDEVSET;
				return;
			}
			csiemptq(vp->vt_idev, bdp);
		}
	} else {
		if ((vp->vt_state&XOPEN) || ((flag&FREAD) && (flag&FWRITE))) {
			u.u_error = EACCES;
			return;
		}
	}
	csisave( vp->vt_idev,'o', vp->vt_state, 0);
}
vpmclose(dev)
{
	register struct vpmt *vp;
	register struct csibd *bdp;
	extern vpmwake();
	int i;


	vp = &vpmt[dev];
	if((vp->vt_state&CTLOPEN)&&((vp->vt_state&TOPEN)==0)) {
		vp->vt_state &= BDEVSET;
		csisave(vp->vt_idev,'c',vp->vt_state,0);
		return;
	}
	vp->vt_state |= VPMFLAG;
	timeout(vpmwake,dev,5*HZ);
	spl5();
	while ( (csixmtq(vp->vt_idev,NULL) != 0) && (vp->vt_state&VPMFLAG) )
		if(sleep((caddr_t)&vp->vt_state,PCATCH|VPMPRI))
			vp->vt_state &= ~VPMFLAG;
	spl0();
	vp->vt_state &= ~VPMFLAG;
	csistop(vp->vt_idev);
	csisave(vp->vt_idev,'Z',vp->vt_state,0);
	for(i=0;i<8;i++)
		vp->vt_errset[i]=0;
	while (bdp = csideq(&vp->vt_rcvq))
		csibrtn(&vpmtbmt,bdp);
	vp->vt_state &= BDEVSET;
	vp->vt_rbc = 0;
	vp->vt_bos = 0;
	csisave(vp->vt_idev,'c',vp->vt_state,0);
}
vpmwrite(dev)
{
	register struct vpmt *vp;
	register struct csibd *bdp;
	register ct;


	vp = &vpmt[dev];
	if (vp->vt_state&VPMERR) {
		u.u_error = EL2HLT;
		return;
	}
	if ((vp->vt_state&CTLOPEN)&&((vp->vt_state&TOPEN) == 0)) {
		u.u_error = EACCES;
		return;
	}
	if ((vp->vt_state&BDEVSET) == 0) {
		u.u_error = EUNATCH;
		return;
	}
	while (u.u_count) {
		spl5();
		while (csixmtq(vp->vt_idev,NULL) >= vpminfo.xbq &&
		    (vp->vt_state&VPMERR) == 0)
			sleep((caddr_t)&vp->vt_state, VPMPRI);
		spl0();
		if (vp->vt_state&VPMERR) {
			u.u_error = EL2HLT;
			return;
		}
		ct = min((unsigned)u.u_count, BUFSIZE);
		if ((bdp=csibget(&vpmtbmt,ct,NVSLP)) == NULL) {
			u.u_error = ENOMEM;
			return;
		}
		if (csicopy(bdp,0,ct,B_WRITE) < 0) {
			csibrtn(&vpmtbmt,bdp);
			return;
		}
		bdp->d_ct = ct;
		csisave( vp->vt_idev,'w', ct, (int)u.u_count);
		csixmtq(vp->vt_idev, bdp);
	}
}
vpmread(dev)
{
	register struct vpmt *vp;
	register struct csibd *bdp;
	register cnt;


	vp = &vpmt[dev];
	if (vp->vt_state&VPMERR) {
		u.u_error = EL2HLT;
		return;
	}
	if ((vp->vt_state&CTLOPEN)&&((vp->vt_state&TOPEN) == 0)) {
		u.u_error = EACCES;
		return;
	}
	if ((vp->vt_state&BDEVSET) == 0) {
		u.u_error = EUNATCH;
		return;
	}
	if (vp->vt_rbc == 0) {
		spl5();
		while ((vp->vt_rbc = csideq(&vp->vt_rcvq)) == 0
		    && (vp->vt_state&VPMERR) == 0)
			sleep((caddr_t)&vp->vt_rcvq, VPMPRI);
		spl0();
		if (vp->vt_state&VPMERR) {
			u.u_error = EL2HLT;
			return;
		}
	}
	bdp = vp->vt_rbc;
	cnt = bdp->d_ct - vp->vt_bos;
	cnt = min((unsigned)cnt, u.u_count);
	if (csicopy(bdp,vp->vt_bos,cnt,B_READ) < 0) {
		u.u_error = EFAULT;
		return;
	}
	csisave( vp->vt_idev,'r', cnt, vp->vt_bos);
	if ((vp->vt_bos += cnt) == (bdp->d_ct)) {
		vp->vt_rbc = 0;
		bdp->d_ct = BUFSIZE;
		vp->vt_bos = 0;
		csiemptq(vp->vt_idev,bdp);
	}
}
vpmioctl(dev, cmd, arg1, mode)
{
	register struct vpmt *vp;
	char cmdar[4];
	int i;
	extern vpmtrint();
	short temperr[8];
	int *csidev;


	vp = &vpmt[dev];
	if ((vp->vt_state&BDEVSET) == 0 && cmd != VPMSDEV) {
		u.u_error = EUNATCH;
		return;
	}
	if ((vp->vt_state&VPMERR)&&(cmd != VPMSTAT)) {
		u.u_error = EL2HLT;
		return;
	}
	switch (cmd) {
	case VPMCMD:
		copyin(arg1, cmdar, sizeof(cmdar));
		csiscmd(vp->vt_idev,cmdar,4);
		return;
	case VPMERRS:
		if((i=csigeterrs(vp->vt_idev,temperr,NOERRS)) < 0) {
			u.u_error=EIO;
			return;
		}
		if(csiseterrs(vp->vt_idev,vp->vt_errset,NOERRS) < 0) {
			u.u_error=EIO;
			return;
		}
		if (copyout((caddr_t)temperr, arg1, i* (sizeof(short))) < 0)
			u.u_error = EFAULT;
		return;

	case VPMERRSET:
		if(copyin(arg1,(caddr_t)vp->vt_errset ,NOERRS*(sizeof(short))) < 0) {
			u.u_error = EFAULT;
			return;
		}
		if(csiseterrs(vp->vt_idev,vp->vt_errset,NOERRS) != NOERRS) {
			u.u_error = EIO;
			return;
		}
		return;
	case VPMRPT:
		u.u_rval1=0;
		if(vp->vt_state&WRPT) {
			copyout((caddr_t)vp->vt_rpt,arg1,4);
			vp->vt_state &= ~WRPT;
			u.u_rval1=1;
		}
		return;
	case VPMSDEV:
		if(vp->vt_state&TOPEN) {
			u.u_error = EINVAL;
			return;
		}
		if(vp->vt_state&BDEVSET) {
			u.u_error = EBUSY;
			return;
		}
		if ((vp->vt_idev=csiattach(arg1,vpmtrint,dev)) < 0) {
			u.u_error = EACCES;
			return;
		}
		vp->vt_options = 0;
		vp->vt_intopts = 0;
		vp->vt_state |= BDEVSET;
		return;
	case VPMDETACH:
		if(vp->vt_state&TOPEN) {
			u.u_error = EINVAL;
			return;
		}
		if((vp->vt_state&BDEVSET)==0) {
			u.u_error = EUNATCH;
			return;
		}
		if(csidetach(vp->vt_idev) < 0 ) {
			u.u_error=EACCES;
			return;
		}
		vp->vt_state &= ~BDEVSET;
		return;
	case VPMOPTS:
		if(vp->vt_state&TOPEN) {
			u.u_error = EINVAL;
			return;
		}
		vp->vt_options=(char)arg1;
		return;
	case VPMPCDOPTS:
		if(vp->vt_state&TOPEN) {
			u.u_error = EINVAL;
			return;
		}
		vp->vt_intopts=(char)arg1;
		return;
	case VPMSTAT:
		if ((vp->vt_state&BDEVSET) == 0) {
			u.u_error = EUNATCH;
			return;
		}
		i = vp->vt_idev;
		copyout(&i,(int *)arg1,sizeof(int));
		return;
	default:
		u.u_error = EINVAL;
		return;
	}
}
vpmtrint(idev,linkno, type, arg)
union csirpt *arg;
{
	register struct vpmt *vp;
	int temp1,temp2;
	int i;



	vp = &vpmt[linkno];
	switch (type) {
	case CSISTART:
		if(CSISOLP->retcode == 0)
			vp->vt_state |= TOPEN;
		vp->vt_state &= ~WSTART;
		wakeup((caddr_t)&vp->vt_state);
		csisave(vp->vt_idev,'B',vp->vt_state,CSISOLP->retcode);
		break;
	case CSIRXBUF:
		csibrtn(&vpmtbmt,CSISOLP->bdp);
		if(CSISOLP->retcode==CSIXMT) wakeup((caddr_t)&vp->vt_state);
		break;
	case CSIRRBUF:
		if(CSISOLP->retcode==CSIRCV) {
			csienq(CSISOLP->bdp,&vp->vt_rcvq);
			wakeup((caddr_t)&vp->vt_rcvq);
		} else 
			csibrtn(&vpmtbmt,CSISOLP->bdp);
		break;
	case CSISTOP:
		csisave(vp->vt_idev,'H',vp->vt_state,CSISOLP->retcode);
	case CSITERM:
		if (type==CSITERM)
			csisave(vp->vt_idev,'E',CSIUNSOLP->code,CSIUNSOLP->excode);
		vp->vt_state &= ~WSTART;
		wakeup((caddr_t)&vp->vt_state);
		if ((vp->vt_state&TOPEN) == 0)
			return;
		vp->vt_state |= VPMERR;
		wakeup((caddr_t)&vp->vt_rcvq);
		break;
	case CSIERR:
		break;
	case CSISRPT:
		for(i=0;i<CMDSZ;i++)
			vp->vt_rpt[i]=CSITRP->info[i];
		vp->vt_state |= WRPT;
		temp1=((short)CSITRP->info[0])|((short)CSITRP->info[1]<<8);
		temp2=((short)CSITRP->info[2])|((short)CSITRP->info[3]<<8);
		csisave(vp->vt_idev,'P',temp1,temp2);
		break;
	case CSITRACE:
		if(CSITRP->scloc==0) {
			temp1=(int)CSITRP->info[0] | ((int)CSITRP->info[1]<<8);
			temp2=(int)CSITRP->info[2] | ((int)CSITRP->info[3]<<8);
			csisave(vp->vt_idev,'S',temp1,temp2);
		} else {
			temp1=(int)CSITRP->info[0] | ((int)CSITRP->info[1]<<8);
			csisave(vp->vt_idev,'T',CSITRP->scloc,temp1);
		}
		break;
	case CSICLEANED:
		csisave(vp->vt_idev,'C',0,0);
		break;
	}
}
vpminit()
{
	register struct csibd *bdp;
	register struct vpmt *vp;

	if (csialloc(&vpmtbmt, vpmbsz) <=0 ) {
		printf("Can't allocate VPM space\n");
		for(vp = &vpmt[0]; vp < &vpmt[vpm_cnt]; vp++)
			vp->vt_state |= VPMBAD;
	}
	for(bdp = &vpmtbd[0]; bdp < &vpmtbd[vpminfo.bufdes]; bdp++)
		csibdlnk(&vpmtbmt,bdp);
}
vpmwake(dev)
{
	register struct vpmt *vp;

	vp = &vpmt[dev];
	if(vp->vt_state&VPMFLAG) {
		wakeup((caddr_t)&vp->vt_state);
		vp->vt_state &= ~VPMFLAG;
		csisave(vp->vt_idev,'Y',vp->vt_state,0);
	}
}
