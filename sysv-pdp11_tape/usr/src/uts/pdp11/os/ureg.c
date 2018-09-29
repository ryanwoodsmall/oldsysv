/* @(#)ureg.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/seg.h"

/*
 * Load the user hardware segmentation
 * registers from the software prototype.
 * The software registers must have
 * been setup prior by estabur.
 */
sureg()
{
	register physadr ump, rmp;
	register i;
	ushort taddr, daddr;
	struct text *tp;

	taddr = daddr = u.u_procp->p_addr;
	if ((tp=u.u_procp->p_textp) != NULL)
		taddr = tp->x_caddr;
	i = (cputype==40)?8:16;
	rmp = UISD;
	ump = (physadr)&u.u_uisd[0];
	do {
		if (ump->r[0]&ABS)
			rmp->r[16] = ump->r[16];
		else
			rmp->r[16] = ump->r[16] + (ump->r[0]&TX? taddr:daddr);
		(rmp++)->r[0] = (ump++)->r[0];
	} while (--i);
}

/*
 * Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 * The last argument determines whether the text
 * segment is read-write or read-only.
 */
estabur(nt, nd, ns, sep, xrw)
{
	register short a, *ap, *dp;
	int *limudp;
	char bitm;

	if (checkur(nt, nd, ns, sep))
		return(-1);
	a = 0;
	ap = &u.u_uisa[0];
	dp = &u.u_uisd[0];
	while(nt >= 128) {
		*dp++ = (127<<8) | xrw|TX;
		*ap++ = a;
		a += 128;
		nt -= 128;
	}
	if (nt) {
		*dp++ = ((nt-1)<<8) | xrw|TX;
		*ap++ = a;
	}
	if (sep)
	while(ap < &u.u_uisa[8]) {
		*ap++ = 0;
		*dp++ = 0;
	}
	a = USIZE;
	while(nd >= 128) {
		*dp++ = (127<<8) | RW;
		*ap++ = a;
		a += 128;
		nd -= 128;
	}
	if (nd) {
		*dp++ = ((nd-1)<<8) | RW;
		*ap++ = a;
		a += nd;
	}
	if (sep) {
		limudp = &u.u_uisa[16];
		bitm = u.u_mbitm >> ((ap - u.u_uisa) - 8);
	} else {
		limudp = &u.u_uisa[8];
		bitm = u.u_mbitm >> (ap - u.u_uisa);
	}
	while(ap < limudp) {
		if (bitm&01) {
			dp++;
			ap++;
		} else {
			*dp++ = 0;
			*ap++ = 0;
		}
		bitm >>= 1;
	}
	a += ns;
	while(ns >= 128) {
		a -= 128;
		ns -= 128;
		*--dp = (127<<8) | RW;
		*--ap = a;
	}
	if (ns) {
		*--dp = ((128-ns)<<8) | RW | ED;
		*--ap = a-128;
	}
	if (!sep) {
		ap = &u.u_uisa[0];
		dp = &u.u_uisa[8];
		while(ap < &u.u_uisa[8])
			*dp++ = *ap++;
		ap = &u.u_uisd[0];
		dp = &u.u_uisd[8];
		while(ap < &u.u_uisd[8])
			*dp++ = *ap++;
	}
	sureg();
	return(0);
}

#define	MAXC	1023
checkur(nt, nd, ns, sep)
{
	register int last, first;
	char bitm;

	last = 0;
	first = 8;
	for(bitm=u.u_mbitm; bitm; bitm <<= 1) {
		--first;
		if (bitm < 0 && last == 0)
			last = first;
	}
	if (sep) {
		if (cputype == 40)
			goto err;
		if (ctos(nt) > 8 || ctos(nd)+ctos(ns) > 8)
			goto err;
		if (ctos(nd) > first || 8-ctos(ns) <= last)
			goto err;
		if (nt > MAXC)
			goto err;
		if (nd + ns + USIZE > MAXC)
			goto err;
	} else {
		if (ctos(nt) + ctos(nd) > first || 8-ctos(ns) <= last)
			goto err;
		if (ctos(nt)+ctos(nd)+ctos(ns) > 8)
			goto err;
		if (nt + nd + ns + USIZE > MAXC)
			goto err;
	}
	if (nt+nd+ns+USIZE > maxmem)
		goto err;
	return(0);

err:
	u.u_error = ENOMEM;
	return(-1);
}
