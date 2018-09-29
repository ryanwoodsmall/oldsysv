/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/bio.c	1.8"

#include "head.h"

/*
 * NAMES:  bread(), blseek(), binit()
 *
 * DESCRIPTION:
 *	 This is a buffered read package.
 *
 *       Bread may be called with a negative nbytes which causes it to
 *      read backwards.  In this case, buffer should point to the first
 *      byte following the buffer.  If only a partial read is possible
 *      (due to beginning of file), only the last bytes of the buffer
 *      will be filled.
 */


bread(brs, buff, nbytes)
struct brbuf *brs; char *buff; {
	register int k, nb;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("bread");
	}
	else if (debugflag == 2)
	{
		enter2("bread");
		arg("brs");
		printf("0x%x",brs);
		comma();
		arg("buff");
		printf("0x%x",buff);
		comma();
		arg("nbytes");
		printf("0x%x",nbytes);
		closeparen();
	}
#endif
	if (nbytes > 0) {
		for (nb=nbytes; nb>0; nb--) {
			if (brs->nr == 0) {
				brs->nr = read(brs->fd,brs->next=brs->b,BRSIZ);
				brs->nl = 0;
				if (brs->nr < 0)
				{
#if DEBUG
					if (debugflag == 1)
					{
						exit1();
					}
					else if (debugflag == 2)
					{
						exit2("bread");
						printf("0x%x",-1);
						endofline();
					}
#endif
					return(-1);
				}
				if (brs->nr == 0)
				{
#if DEBUG
					if (debugflag == 1)
					{
						exit1();
					}
					else if (debugflag == 2)
					{
						exit2("bread");
						printf("0x%x",nbytes-nb);
						endofline();
					}
#endif
					return(nbytes-nb);
				}
				}
			*buff++ = *brs->next++;
			brs->nr--;
			brs->nl++;
			}
		}
	else {
		nbytes = -nbytes;
		for (nb=nbytes; nb>0; nb--) {
			if (brs->nl == 0) {
				if ((k=tell(brs->fd)) >= BRSIZ + brs->nr) {
					lseek(brs->fd,(long)-(BRSIZ+brs->nr),1);
					brs->nl = read(brs->fd, brs->b, BRSIZ);
				} else {
					lseek(brs->fd, 0L, 0);
					k = k - brs->nr;
					if (k < 0) k = 0;
					brs->nl = read(brs->fd, brs->b, k);
				}
				if (brs->nl == 0) return(nbytes-nb);
				brs->next = brs->b + brs->nl;
				brs->nr = 0;
				}
			*--buff = *--brs->next;
			brs->nr++;
			brs->nl--;
			}
		}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("bread");
		printf("0x%x",nbytes);
		endofline();
	}
#endif
	return(nbytes);
	}

blseek(brs, offset, flag) 
struct brbuf *brs; long offset; {
	brs->nl = 0;
	brs->nr = 0;
	return(lseek(brs->fd,offset,flag));
	}

binit(brs)
struct brbuf *brs; {
	brs->nl = brs->nr = 0;
}

long
tell(fildes) {
	return(lseek(fildes, 0L, 1));
}
