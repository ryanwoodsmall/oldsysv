/* @(#)dn.c	1.2 */
/*
 * DN-11 ACU interface
 * with shared acu capability
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"

struct device {
	short	dn_reg[4];
};

struct device *dn_addr[];
int	dn_cnt;

#define	PWI	0100000
#define	ACR	040000
#define	DLO	010000
#define	DONE	0200
#define	IENABLE	0100
#define	DSS	040
#define	PND	020
#define	MENABLE	04
#define	DPR	02
#define	CRQ	01

#define	DNPRI	(PZERO+5)
#define	dnaddr(X)	((X>>2)&03)	/* which DN11-AA */
#define	dnreg(X)	(X&03)		/* which DN11-DA */
#define	dnacu(X)	((X>>4)&017)	/* which acu (if shared) */

dnopen(dev)
register dev;
{
	register struct device *dp;
	register short *ep;

	if (dnaddr(dev) >= dn_cnt ||
		(dp = dn_addr[dnaddr(dev)])->dn_reg[dnreg(dev)]&PWI) {
		u.u_error = ENXIO;
		return;
	}
	ep = &(dp->dn_reg[dnreg(dev)]);
	if (dnacu(dev)) {
		if (*ep & DLO) {
			delay(100);
			if (*ep & DLO) {
				u.u_error = EBUSY;
				return;
			}
		}
	} else if (*ep & (DLO|CRQ)) {
		u.u_error = EBUSY;
		return;
	}
	dp->dn_reg[0] |= MENABLE;
	if (dnacu(dev)) {
		delay(6);
		*ep = IENABLE|MENABLE|CRQ;
		spl4();
		if ((*ep&PND) == 0)
			sleep(ep, DNPRI);
		*ep = (dnacu(dev)<<8)|IENABLE|MENABLE|DPR|CRQ;
		sleep(ep, DNPRI);
		if (*ep&PWI)
			u.u_error = ENXIO;
		else if (*ep&DSS)
			u.u_error = EIO;
		else if (*ep&DLO)
			u.u_error = EBUSY;
		if (u.u_error)
			*ep = MENABLE;
		spl0();
	} else
		*ep = IENABLE|MENABLE|CRQ;
}

dnclose(dev)
{
	dn_addr[dnaddr(dev)]->dn_reg[dnreg(dev)] = MENABLE;
}

dnwrite(dev)
{
	register c;
	register short *dp;

	dp = &(dn_addr[dnaddr(dev)]->dn_reg[dnreg(dev)]);
	while ((*dp & (PWI|ACR|DSS)) == 0) {
		spl4();
		if ((*dp&PND) == 0 || u.u_count == 0 || (c=cpass()) < 0)
			sleep(dp, DNPRI);
		else switch(c) {
		case '-':
			delay(4*HZ);
			break;
		case 'f':
			*dp &= ~CRQ;
			delay(1*HZ);
			*dp |= CRQ;
			break;
		case '*':
		case ':':
			c = 012;
			goto dial;
		case '#':
		case ';':
			c = 013;
			goto dial;
		case 'e':
		case '<':
			c = 014;
			goto dial;
		case 'w':
		case '=':
			c = 015;
			goto dial;
		default:
			if (c<'0' || c>'9')
				continue;
		dial:
			*dp = (c<<8)|IENABLE|MENABLE|DPR|CRQ;
			sleep(dp, DNPRI);
		}
		spl0();
	}
	if (*dp&(PWI|ACR))
		u.u_error = EIO;
}

dnintr(dev)
{
	register short *dp, *ep;

	dp = &(dn_addr[dev]->dn_reg[0]);
	*dp &= ~MENABLE;
	for (ep=dp; ep<dp+4; ep++)
		if (*ep&DONE) {
			*ep &= ~DONE;
			wakeup(ep);
		}
	*dp |= MENABLE;
}
