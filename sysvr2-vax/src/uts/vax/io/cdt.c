/* @(#)cdt.c	6.1 */
#define debugn(X,Y)	if(debug&X)printf(Y);
/* Console DEC Tape II driver 
 */

/* tunable parameters */
#define MAXINITS	2
#define TIMEOUT		600

/* System Constants */
#define CD0	11
#define CDTLAST	512	/* number of blocks on the tape */
#define PPB	4	/* packets per blocks (packets are 128 bytes) */
#define PSIZE	128	/* packet size, hardware */

/* Packet headers */
#define DATA	0x01
#define CMD	0x02
#define INIT	0x04
#define CONT	0x10
#define XOFF	0x13

/* CMD packet opcodes */
#define READ	0x02
#define WRITE	0x03

/* CDT internal states */
#define CIDLE	0
#define CWAIT	1
#define CCOUNT	2
#define CPACKET	3
#define CCSL	4
#define CCSH	5

/* register bit definitions */
#define READY	0x80
#define IE	0x40
#define BREAK	0x01

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/iobuf.h"
#include "sys/mtpr.h"
asm(" .set CSRS,28");
asm(" .set CSRD,29");
#include "sys/signal.h"
#include "sys/user.h"

int	debug = 0x0;

struct cdtpacket {
	unsigned char	flag;
	unsigned char	count;
	unsigned char	*data;
	unsigned short	cs;
} cdtrcv, cdtxmit;

int cdtsoft = 0, cdtopns = 0;

static unsigned cdttmp;

struct iobuf cdtbuf;
struct iobuf cdttab = tabinit(CD0,0);

cdtopen(dev)
{
	debugn(1, "o");
	cdtopns++;
	if (!(cdtbuf.b_flags & B_ONCE)) {
		cdtbuf.b_flags |= B_ONCE;
		spl6();
		cdtinit();
		spl0();
	}
}

cdtclose(dev)
{
	debugn(1, "c");
	if (!(--cdtopns)) cdtsoft = 0;
}

cdtstrategy(bp)
register struct buf *bp;
{
	debugn(1, "s");
	if (bp->b_blkno < 0 || bp->b_blkno >= CDTLAST) {
		if (bp->b_blkno == CDTLAST && bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else
		{	bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl6();
	if (cdtbuf.b_actf) cdtbuf.b_actl->av_forw = bp;
	else cdtbuf.b_actf = bp;
	cdtbuf.b_actl = bp;
	if (cdtbuf.b_active == 0) cdtstart();
	spl0();
}

cdtstart()
{
	register checksum, i;
	register struct buf *bp;
	static unsigned char cmd[10];

	debugn(1, "S");
	if ((bp = cdtbuf.b_actf) == NULL) return;
	cdtbuf.b_active++;
	bp->b_resid = bp->b_bcount;
	if (bp->b_flags & B_READ)
		cdtrcv.data = (unsigned char *) bp->b_un.b_addr;
	cdtxmit.flag = CMD;
	cdtxmit.count = 10;
	cdtxmit.data = cmd;
	checksum = *(unsigned short *)&cdtxmit.flag;
	cmd[0] = (bp->b_flags & B_READ)? READ : WRITE;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = 0;
	cmd[6] = bp->b_bcount & 0xff;
	cmd[7] = bp->b_bcount>>8;
	cmd[8] = bp->b_blkno & 0xff;
	cmd[9] = bp->b_blkno>>8;
	for (i = 0; i < 5; i++) {
		checksum += ((unsigned short *)cmd)[i];
		if (checksum > 0xffff)
			checksum = ++checksum & 0xffff;
	}
	cdtxmit.cs = checksum;
	cdtxint();
}

cdtxint()
{
	static state = CIDLE, xcount = 0;

	debugn(1, "x");
	if (!cdtbuf.b_active) {
		state = CIDLE;
		mtpr(CSTS, 0);
		return;
	}
	while(mfpr(CSTS) & READY)
		switch (state) {
		case CIDLE:
			state = CCOUNT;
			mtpr(CSTD, cdtxmit.flag);
			mtpr(CSTS, IE);
			break;

		case CCOUNT:
			xcount = cdtxmit.count;
			state = xcount? CPACKET : CCSL;
			mtpr(CSTD, cdtxmit.count);
			break;

		case CPACKET:
			if (!(--xcount)) state = CCSL;
			mtpr(CSTD, *(cdtxmit.data++));
			break;

		case CCSL:
			state = CCSH;
			mtpr(CSTD, cdtxmit.cs & 0xff);
			break;

		case CCSH:
			state = CIDLE;
			mtpr(CSTD, (cdtxmit.cs>>8) & 0xff);
			mtpr(CSTS, 0);
			return;
		}
}

cdtxpint()
{
	register checksum, i;
	register struct buf *bp;

	debugn(1, "X");
	bp = cdtbuf.b_actf;
	if ((bp == NULL) || (!cdtbuf.b_active) || (bp->b_flags & B_READ))
		return;
	if (cdtxmit.flag == CMD)
		cdtxmit.data = (unsigned char *) bp->b_un.b_addr;
	cdtxmit.flag = DATA;
	cdtxmit.count = (bp->b_resid < 128)? bp->b_resid : 128;
	bp->b_resid -= cdtxmit.count;
	if (cdtxmit.count&1) cdtxmit.data[cdtxmit.count++] = 0;

	checksum = *(unsigned short *) &cdtxmit.flag;
	for (i = 0; i < cdtxmit.count>>1; i++) {
		checksum += ((unsigned short *)cdtxmit.data)[i];
		if (checksum > 0xffff)
			checksum = ++checksum  & 0xffff;
	}
	cdtxmit.cs = checksum;
	cdtxint();
}

cdtrint()
{
	register struct cdtpacket *ptr;
	register unsigned chr, pcount, rcount;
	int i;
	static state = CIDLE, timer = 0;
	static struct cdtpacket ins[PPB+1];
	static unsigned char rcdata[128];

	debugn(1, "r");
	do {
		do	asm("mfpr $CSRS,_cdttmp");
		while (!(cdttmp & READY) || (timer++>TIMEOUT));
		asm("mfpr $CSRD,_cdttmp");
		chr = cdttmp & 0xff;
		timer = 0;
		switch(state) {
		case CIDLE:
			pcount = 0;
			ptr = ins;
		case CWAIT:
			switch(chr) {
			case DATA:
			case CMD:
				ptr[pcount].flag = chr;
				state = CCOUNT;
				break;

			case INIT:
				cdtinit();
				state = CIDLE;
				break;

			case CONT:
				if (cdtbuf.b_active &&
					!(cdtbuf.b_actf->b_flags & B_READ))
						cdtxpint();
				state = CIDLE;
				break;

			default:
				printf("\nCDT: bad packet flag %x.\n", chr);
				cdtinit();
				if (cdtbuf.b_actf &&
					(cdtbuf.b_actf->b_flags & B_READ))
				{	state = CWAIT;
					pcount = 0;
					cdtstart();
				}
				else	state = CIDLE;
				break;
			}
			break;

		case CCOUNT:
			rcount = ptr[pcount].count = chr;
			if (ptr[pcount].flag == CMD)
				ptr[pcount].data = rcdata;
			else if (!pcount)
				ptr[pcount].data = cdtrcv.data;
			state = CPACKET;
			if (rcount > 128)
			{
#define RMLERR	"\nCDT: receive message too long (%d[10] bytes).\n"
				printf(RMLERR, rcount);
				cdtinit();
				state = CIDLE;
			}
			break;

		case CPACKET:
			*(ptr[pcount].data++) = chr;
			if (!(--rcount)) state = CCSL;
			break;

		case CCSL:
			ptr[pcount].cs = chr;
			state = CCSH;
			break;

		case CCSH:
			((char *) &ptr[pcount].cs)[1] = chr;
			if (ptr[pcount].flag == DATA) {
				if (pcount++ > PPB) {
#define BSERR "\nCDT: blksize > %d[PPB]*%d[PSIZE].\n"
					printf(BSERR, PPB, PSIZE);
					cdtinit();
					state = CIDLE;
				} else {
					ptr[pcount].data = ptr[pcount-1].data;
					state = CWAIT;
				}
			} else {
				for (i = 0; i <= pcount; i++) {
					cdtrcv.flag = ptr[i].flag;
					cdtrcv.count = ptr[i].count;
					cdtrcv.data = ptr[i].data;
					cdtrcv.cs = ptr[i].cs;
					cdtrpint();
				}
				state = CIDLE;
			}
			break;
		}
	} while (state != CIDLE);
	if (!cdtbuf.b_active && cdtbuf.b_actf) cdtstart();
}

cdtrpint()
{
	register i, checksum;
	register struct buf *bp;
	static softerrs, cserr;

	debugn(1, "R");
	bp = cdtbuf.b_actf;
	cdtrcv.data -= cdtrcv.count;
	switch(cdtrcv.flag) {
	case DATA:
		if (!cdtbuf.b_active || !(bp->b_flags & B_READ))
			break;
		bp->b_resid -= cdtrcv.count;
		checksum = *(unsigned short *) &cdtrcv.flag;
		for (i = 0; i < cdtrcv.count>>1; i++)
		{	checksum += ((unsigned short *)cdtrcv.data)[i];
			if (checksum > 0xffff)
				checksum = ++checksum & 0xffff;
		}
		if (cdtrcv.cs != checksum) cserr++;
		cdtrcv.data += cdtrcv.count;
		return;

	case CMD:
		if (cdtrcv.data[1] > 1 || cserr)
		{	if (++cdtbuf.b_errcnt > 32)
			{	printf("\nCDT: hard err blk %d[10]\n",
					bp->b_blkno);
				bp->b_flags |= B_ERROR;
			}
			else if (cdtrcv.data[1] == 0xf7)
			{	printf("\nCDT: no tape.\n");
				bp->b_flags |= B_ERROR;
			}
			else if (cdtrcv.data[1] == 0xf5)
			{	printf("\nCDT: Write Locked.\n");
				bp->b_flags |= B_ERROR;
			}
			else if (cdtrcv.data[1] == 0xdf &&
					cdtbuf.b_errcnt > 1)
			{	printf("\nCDT: MOTOR FAILURE\n");
				bp->b_flags |= B_ERROR;
			}
			else cdtbuf.b_active = 0;
		}
		else if ((cdtsoft += cdtrcv.data[1]) == 17)
		{	printf("\nCDT: >16 soft errors.\n");
			cdtsoft++;
		}
		break;

	default:
		cdtbuf.b_active = 0;
		break;
	}
	cserr = 0;
	if (cdtrcv.flag == DATA) {
		printf("\nCDT: unexpected receive data.\n");
		cdtinit();
	}
	if (cdtbuf.b_active) {
		cdtbuf.b_active = 0;
		cdtbuf.b_errcnt = 0;
		cdtbuf.b_actf = bp->av_forw;
		iodone(bp);
	}
	cdtrcv.data = NULL;
}

cdtinit()
{
	register timer, count;

	debugn(1, "i");
	mtpr(CSTS, BREAK);
	cdtputc(0);
	cdtputc(0);
	cdtputc(INIT);
	cdtputc(INIT);
	if (cdtbuf.b_active) {
		cdtbuf.b_active = 0;
		cdtxint();
		++cdtbuf.b_errcnt;
	}
	for (count = 0; count < MAXINITS; count++)
		if (cdtgetc(TIMEOUT) == CONT)
		{	mtpr(CSRS, IE);
			return;
		}
	printf("\nCDT init failed\n");
	timeout(cdtinit, 0, 15*HZ);
}

cdtputc(chr)
{
	register timer;

	for (timer = 0; !(mfpr(CSTS) & READY); timer++)
		if (timer > TIMEOUT)
		{	printf("\nCDT: serial transmit timeout.\n");
			break;
		}
	mtpr(CSTD, chr);
}

cdtgetc(limit)
register limit;
{
	register timer;

	for (timer = 0; !(mfpr(CSRS) & READY); timer++)
		if (timer > limit)
		{	printf("\nCDT: serial receive timeout.\n");
			return(-1);
		}
	return(mfpr(CSRD) & 0xff);
}

cdtprint(dev, str)
char *str;
{
	printf("%s on CDT, minor %o[octal]\n", str, dev);
}
