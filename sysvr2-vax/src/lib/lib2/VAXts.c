/*
 * TS11 tape driver
 */
static char	sccsid[] = "@(#)VAXts.c	1.1";

#include <stand.h>

#define	MAXBLKS	128

#define CPUTYPE	((mfpr(0x3e)>>24) & 0xff)
#define VAX780	1

#define TSADDR	0xf550
#define V780ADDR	0x20130000	/* 780 unibus addr space io pg */
#define V750ADDR	0xff0000	/* 750 unibus addr space io pg */

struct	device {
	ushort	tsdb;
	ushort	tssr;
};

/* status message */
struct	sts {
	ushort	s_sts;
	ushort	len;
	ushort	rbpcr;
	ushort	xs0;
	ushort	xs1;
	ushort	xs2;
	ushort	xs3;
};

/* Error codes in stat 0 */
#define	TMK	0100000
#define	RLS	040000
#define	ONL	0100
#define	WLE	04000

/* command message */
struct cmd {
	ushort	c_cmd;
	ushort	c_loba;
	ushort	c_hiba;
	ushort	c_size;
};

#define	ACK	0100000
#define	CVC	040000
#define	IE	0200
#define	RCOM	01

#define	SETCHR	04

#define	WCOM	05

#define	SFORW	010
#define SFTM	01010
#define	SREV	0410
#define	REW	02010

#define	WTM	011
#define ERASE	0411

#define	GSTAT	017

/* characteristics data */
struct charac {
	ushort	char_loba;
	ushort	char_hiba;
	ushort	char_size;
	ushort	char_mode;
};


/* Bits in (unibus) status register */
#define	SC	0100000
#define	SSR	0200
#define	OFL	0100
#define	NBA	02000


struct tsmesg {
	struct	cmd ts_cmd;
	struct	sts ts_sts;
	struct	charac ts_char;
	int	align;
};

static struct tsmesg dts;
static dtsinfo;

static daddr_t _eof;
static inited = 0;

static
_tsinit( tsaddr)
register struct device *tsaddr;
{
register struct tsmesg *tsm;
register int *ubap;
register i;

	inited = 1;
	if ( CPUTYPE == VAX780 ) ubap = (int *)0x20006800;
	else			 ubap = (int *)0xf30800;
/*
	tsaddr->tssr = 0;
	while ((tsaddr->tssr&SSR)==0)
		;
*/
	i = (int)&dts;
	i &= 0xffffff;
	dtsinfo = ( (i&0777) | (MAXBLKS&0777)<<9 );
	tsm = (struct tsmesg *)dtsinfo;
	dtsinfo = (dtsinfo | dtsinfo>>16) & 0xffff;
	i >>= 9;
	i |= 0x80000000;
	*(ubap+MAXBLKS) = i;
	*(ubap+MAXBLKS+1) = i+1;
	dts.ts_cmd.c_cmd = ACK | SETCHR;
	dts.ts_cmd.c_loba = (ushort) &tsm->ts_char;
	dts.ts_cmd.c_hiba = ((unsigned)&tsm->ts_char)>>16;
	dts.ts_cmd.c_size = sizeof(dts.ts_char);
	dts.ts_char.char_loba = (ushort) &tsm->ts_sts;
	dts.ts_char.char_hiba = ((unsigned)&tsm->ts_sts)>>16;
	dts.ts_char.char_size = sizeof(dts.ts_sts);
	dts.ts_char.char_mode = 0;
	tsaddr->tsdb = dtsinfo;
	while ((tsaddr->tssr & SSR) == 0)
		;
}

_tsclose (io)
register struct iob *io;
{	register flag, unit;
	register struct device *tsaddr;

	if ( CPUTYPE == VAX780 )
		tsaddr = (struct device *) (TSADDR | V780ADDR);
	else
		tsaddr = (struct device *) (TSADDR | V750ADDR);

	flag = io->i_flgs;
	unit = io->i_dp->dt_unit & 03;

	if (flag & F_WRITE) {
		tcom( WTM, tsaddr);
		tcom( WTM, tsaddr);
	}
	if ((io->i_dp->dt_unit & 04) == 0)
		tcom( REW, tsaddr);
	else if (flag & F_WRITE)
		tcom( SREV, tsaddr);
	else if (!_eof)
		tcom( SFTM, tsaddr);
	_eof = 0;
	inited = 0;
}

_tsstrategy( io, func)
register struct iob *io;
{
	register com, unit;
	int errcnt = 0;
	int dev, ret;
	register struct device *tsaddr;
	register i, *ubap, frame;

	dev = io->i_dp->dt_unit;
	if ( CPUTYPE == VAX780 )
		tsaddr = (struct device *) (TSADDR | V780ADDR);
	else
		tsaddr = (struct device *) (TSADDR | V750ADDR);
	unit = dev & 03;

	if (!inited) _tsinit( tsaddr);

	if (func == READ)
		com = RCOM;
	else if (func == WRITE)
		com = WCOM;
	else
		com = func;

	if ((tsaddr->tssr & SSR) == 0) {
		_prs ("TS11 unit not ready\n");
		while ((tsaddr->tssr & SSR) == 0);
	}

	ubap = (int *)((CPUTYPE == VAX780)? 0x20006800 : 0xf30800);
	frame = ((int) io->i_ma) >> 9;
	for (i = ((io->i_cc + 511) >> 9) + 1; i > 0; i--)
		*ubap++ = 0x80000000 | frame++;
	*ubap = 0;	/* invalidate mba entry */

	do {
		dts.ts_cmd.c_loba = ((int) io->i_ma) & 0x1ff;
		dts.ts_cmd.c_hiba = 0;
		dts.ts_cmd.c_size = io->i_cc;
		dts.ts_cmd.c_cmd = ACK|CVC|com;

		_eof = 0;

		tsaddr->tsdb = dtsinfo;
		if ( twait( tsaddr) )
			return( io->i_cc - dts.ts_sts.rbpcr);
		ret = tsaddr->tssr;
		if ( (ret & 016) == 012)
			/* recoverable error, tape unmoved */
			;
		else if ( (ret & 016) == 010 )
			/* recoverable error, tape moved */
			tcom( SREV, tsaddr);
		else errcnt = 10;
		if (com == WCOM) {
			while ((tsaddr->tssr & SSR) == 0);
			tcom( ERASE, tsaddr);
		}
	} while (++errcnt < 10);
	errno = EIO;
	return (-1);
}

static
tcom( com, tsaddr)
register struct device *tsaddr;
{

	dts.ts_cmd.c_cmd = ACK|CVC|com;
	dts.ts_cmd.c_loba = 1;
	tsaddr->tsdb = dtsinfo;
	twait( tsaddr);
}

static
twait( tsaddr)
register struct device *tsaddr;
{
register i;

	while ((tsaddr->tssr&SSR)==0)
		;
	i = tsaddr->tssr;
	if (i&SC) {
		if ( dts.ts_sts.xs0 & TMK ) {
			_eof++;
			return(1);
		}
		printf("Tape error, tssr %x(16).\n", i);
		return( 0);
	}
	return( 1);
}

static
mfpr( regno)
{
	asm(" mfpr	4(ap),r0");
}
