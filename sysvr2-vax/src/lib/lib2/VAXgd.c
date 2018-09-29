static char	sccsid[] = "@(#)VAXgd.c	1.4";

/*
 * Common driver for RP04/RP06, and RM05/80 and RP07
 */

#include "stand.h"

int gd_addr = 0x20010000;
int gd_boot = 0;

struct	device {
	int	cs1;		/* Control and Status register 1 */
	int	ds;		/* Drive Status */
	int	er1;		/* Error register 1 */
	int	mr;		/* Maintenance */ 
	int	as;		/* Attention Summary */
	int	da;		/* Desired address register */
	int	dt;		/* Drive type */
	int	la;		/* Look ahead */
	int	sn;		/* Serial number */
	int	of;		/* Offset register */
	int	dc;		/* Desired Cylinder address register */
	int	cc;		/* Current Cylinder */
	int	er2;		/* Error register 2 */
	int	er3;		/* Error register 3 */
	int	ec1;		/* Burst error bit position */
	int	ec2;		/* Burst error bit pattern */
};

/*
 * VAX Massbus adapter registers
 */

struct mba_regs {
	int mba_csr,
	    mba_cr,
	    mba_sr,
	    mba_var,
	    mba_bcr;
};

union MBREGS {
	struct mba_regs Mbar;
	struct {
		char fill1[0x400];
		struct RxR {
			struct device Dev;
			int fill[16];
		} RXreg[8];
	} Unit;
	struct MAP {
		long fill3[0x200];
		long M_map[256];
	}Map;
} *Mptr;

struct dlimits {
	int dtype, nsect, ntrak;
	char *dname;
} d_limit[] = {
	{ 0x2010, 22, 19, "RP04"},
	{ 0x2011, 22, 19, "RP05"},
	{ 0x2012, 22, 19, "RP06"},
	{ 0x2016, 31, 14, "RM80"},
	{ 0x2017, 32, 19, "RM05"},
	{ 0x2022, 50, 32, "RP07"},
	{ 0, 0, 0, "???"}	/* leave last in case of funny type */
};
int dr_typ;

#define NTYPES 4

#define M_BASE 0x20000000

#define NSECT d_limit[dr_typ].nsect
#define NTRAC d_limit[dr_typ].ntrak

#define GO	01
#define DCLR	010
#define PRESET	020
#define WCOM	060
#define RCOM	070

#define VV	0100
#define MOL	010000

#define FMT22	010000
#define SSEI	01000

#define DCK	0100000
#define SSE	040

#define INIT	01

#define DTABT	0x1000
#define DTCMP	0x2000

static struct {
	int	cn;
	int	tn;
	int	sn;
	char	*ma;
	int	bc;
} _drive;

_gdstrategy (io, func)
register struct iob *io; {
	register i, unit;
	int errcnt = 0;
	daddr_t bn;
	register struct device *gdaddr;
	register *mbap, frame;

	unit = io->i_dp->dt_unit;
	bn = io->i_bn;

#ifdef DEBUG
	printf("\n\nDriver-unit %d iobuf:", unit);
	printf("\ni_bn : %ld (%lo)",io->i_bn,io->i_bn);
	printf("\ni_offset : %ld (%lo)",io->i_offset,io->i_offset);
	printf("\ni_ma : %d (%o)",io->i_ma,io->i_ma);
	printf("\ni_cc : %d (%o)",io->i_cc,io->i_cc);
	printf("\ni_flgs : %d (%o)",io->i_flgs,io->i_flgs);
#endif

	Mptr = (union MBREGS *) gd_addr;
	gdaddr = &Mptr->Unit.RXreg[unit].Dev;
	for(dr_typ = 0; NSECT != 0; dr_typ++)
		if((gdaddr->dt&0x71ff) == d_limit[dr_typ].dtype) break;
	if (NSECT == 0) {
		_prs("NO DEVICE--UNKNOWN DRIVE TYPE\n");
		errno = ENXIO;
		return(-1);
	}

	Mptr->Mbar.mba_cr = INIT;
	gdaddr->cs1 = DCLR | GO;

	if ((gdaddr->ds & VV) == 0) {
		gdaddr->cs1 = PRESET | GO;
		gdaddr->of = FMT22;
	} else gdaddr->of &= ~SSEI;


	_drive.cn = bn / (NSECT * NTRAC);
	_drive.sn = bn % (NSECT * NTRAC);
	_drive.tn = _drive.sn / NSECT;
	_drive.sn %= NSECT;

	unit = GO;
	if (func == READ)
		unit |= RCOM;
	else
		unit |= WCOM;

	if ((gdaddr->ds & MOL) == 0) {
		_prs (d_limit[dr_typ].dname);
		_prs (" unit not ready\n");
		while ((gdaddr->ds & MOL) == 0);
	}

	mbap = Mptr->Map.M_map;
	frame = ((int) io->i_ma) >> 9;
	for (i = ((io->i_cc + 511) >> 9) + 1; i > 0; i--)
		*mbap++ = 0x80000000 | frame++;
	*mbap = 0;	/* invalidate mba entry */

	_drive.ma = (char *) ( ((int) io->i_ma) & 0x1ff );
	_drive.bc = io->i_cc;

	do {
		gdaddr->dc = _drive.cn;
		gdaddr->da = (_drive.tn << 8) | _drive.sn;
		Mptr->Mbar.mba_var = (int) _drive.ma;
		Mptr->Mbar.mba_bcr = -_drive.bc;
		gdaddr->cs1 = unit;
		while ((Mptr->Mbar.mba_sr & DTCMP) == 0);

		if ( (Mptr->Mbar.mba_sr & DTABT) == 0 ) return( io->i_cc);

		if ( (gdaddr->er1 == DCK) )
		{	if ( _gdecc( io, gdaddr) < 0 ) return( io->i_cc);
		}
		else if ( gdaddr->er3 & SSE ) _gdsse( io, gdaddr);

		Mptr->Mbar.mba_cr = INIT;
		gdaddr->cs1 = DCLR | GO;
	} while (++errcnt < 10);

	errno = EIO;
	return (-1);
}

static
_gdecc (io, gdaddr)
register struct iob *io;
register struct device *gdaddr;
{
	register i, n, b;

	if (gdaddr->ec2 == 0)
		return (1);
	i = (gdaddr->ec1 & 0xffff) - 1;
	n = i & 017;
	i = (i & ~017) >> 3;
	b = (((Mptr->Mbar.mba_bcr & 0xffff) + _drive.bc - 1) >> 9) & 0177;
	i += b << 9;
	if (i < _drive.bc)
		*((int *) (&_drive.ma[i])) ^= gdaddr->ec2 << n;
	i += 2;
	if (i < _drive.bc)
		*((int *) (&_drive.ma[i])) ^= gdaddr->ec2 >> (16-n);
	if (Mptr->Mbar.mba_bcr == 0)
		return (-1);
	i = _drive.tn * NSECT + _drive.sn + b + 1;
	if (i >= NSECT * NTRAC) {
		_drive.cn += i / ( NSECT * NTRAC );
		i %= NSECT * NTRAC;
	}
	_drive.tn = i / NSECT;
	_drive.sn = i % NSECT;
	_drive.ma += b << 9;
	_drive.bc -= b << 9;
	return (0);
}

static
_gdsse( io, gdaddr)
register struct iob *io;
register struct device *gdaddr;
{
	register unsigned b;
	register i;

	gdaddr->cs1 = DCLR | GO;
	gdaddr->of |= SSEI; /* turn off sse interupts */
	b = (((Mptr->Mbar.mba_bcr & 0xffff) + _drive.bc) >> 9) & 0177;
	Mptr->Mbar.mba_sr = -1;	/* clear mb exception */

	i = _drive.tn * NSECT + _drive.sn + b;
	if (i >= NSECT * NTRAC)
	{	_drive.cn += i / ( NSECT * NTRAC );
		i %= NSECT * NTRAC;
	}
	_drive.tn = i / NSECT;
	_drive.sn = (i % NSECT) + 1; /* increment sector around sse */
	_drive.ma += b << 9;
	_drive.bc -= b << 9;
}
