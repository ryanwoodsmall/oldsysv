/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/sddrv.c	10.2"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/sbd.h"
#include "sys/csr.h"

extern struct r16 sbdrcsr;
sdinit()
{
	idinit();
	ifinit();
}
sdopen(dev,flag,otyp)
{
	if (dev & 0x80)
		ifopen(dev,flag,otyp);
	else
		idopen(dev,flag,otyp);
}
sdclose(dev,flag,otyp)
{
	if (dev & 0x80)
		ifclose(dev,flag,otyp);
	else
		idclose(dev,flag,otyp);
}
sdioctl(dev,cmd,arg,flag)
{
	if (dev & 0x80)
		ifioctl(dev,cmd,arg,flag);
	else
		idioctl(dev,cmd,arg,flag);
}
sdstrategy(bp)
struct buf *bp;
{
	if (bp->b_dev & 0x80)
		ifstrategy(bp);
	else
		idstrategy(bp);
}
sdprint(dev,str)
char *str;
{
	if (dev & 0x80)
		ifprint(dev, str);
	else
		idprint(dev, str);
}
sdread(dev)
{
	if (dev & 0x80)
		ifread(dev);
	else
		idread(dev);
}
sdwrite(dev)
{
	if (dev & 0x80)
		ifwrite(dev);
	else
		idwrite(dev);
}

extern void ifint();
extern idint();

sdint(dev)
{

	idint(dev);
	if (sbdrcsr.data & CSRDISK)
		ifint(dev);
}
