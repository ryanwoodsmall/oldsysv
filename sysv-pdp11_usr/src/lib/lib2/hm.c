static char	sccsid[] = "@(#)hm.c	1.1";

/*
 * RM05 disk driver
 */

#include <stand.h>

struct	device {
	int	hmcs1;		/* Control and Status register 1 */
	int	hmds;		/* Drive Status */
	int	hmer1;		/* Error register 1 */
	int	hmmr;		/* Maintenance */ 
	int	hmas;		/* Attention Summary */
	int	hmda;		/* Desired address register */
	int	hmdt;		/* Drive type */
	int	hmla;		/* Look ahead */
	int	hmsn;		/* Serial number */
	int	hmof;		/* Offset register */
	int	hmdc;		/* Desired Cylinder address register */
	int	hmcc;		/* Current Cylinder */
	int	hmer2;		/* Error register 2 */
	int	hmer3;		/* Error register 3 */
	int	hmec1;		/* Burst error bit position */
	int	hmec2;		/* Burst error bit pattern */
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

#define MBA_ERB 0x400
#define MBA_MAP 0x800

#define MBA0	((struct mba_regs *) 0x20014000)
#define HPADDR	((struct device *) 0x20014400)
#define NSECT	32
#define NTRAC	19

#define GO	01
#define DCLR	010
#define PRESET	020
#define WCOM	060
#define RCOM	070

#define VV	0100
#define MOL	010000

#define FMT22	010000

#define DCK	0100000

#define INIT	01

#define DTABT	0x1000
#define DTCMP	0x2000

static struct {
	int	cn;
	int	tn;
	int	sn;
} _rm05;

_hmstrategy (io, func)
register struct iob *io; {
	register i, unit;
	int errcnt = 0;
	daddr_t bn;
	register struct device *hmaddr;
	register *mbap, frame;

	unit = io->i_dp->dt_unit;

	if ((unit & 04) == 0)
		bn = io->i_bn;
	else {
		unit &= 03;
		bn = io->i_bn;
		bn -= io->i_dp->dt_boff;
		i = unit + 1;
		unit = bn % i;
		bn /= i;
		bn += io->i_dp->dt_boff;
	}

	hmaddr = (struct device *) (((int *) HPADDR) + 32*unit);

	MBA0->mba_cr = INIT;
	hmaddr->hmcs1 = DCLR | GO;

	if ((hmaddr->hmds & VV) == 0) {
		hmaddr->hmcs1 = PRESET | GO;
		hmaddr->hmof = FMT22;
	}

	_rm05.cn = bn / (NSECT * NTRAC);
	_rm05.sn = bn % (NSECT * NTRAC);
	_rm05.tn = _rm05.sn / NSECT;
	_rm05.sn %= NSECT;

	unit = GO;
	if (func == READ)
		unit |= RCOM;
	else
		unit |= WCOM;

	if ((hmaddr->hmds & MOL) == 0) {
		_prs ("RM05 unit not ready\n");
		while ((hmaddr->hmds & MOL) == 0);
	}

	mbap = (int *) MBA0;
	mbap += MBA_MAP / 4;
	frame = ((int) io->i_ma) >> 9;
	for (i = ((io->i_cc + 511) >> 9) + 1; i > 0; i--)
		*mbap++ = 0x80000000 | frame++;
	*mbap = 0;	/* invalidate mba entry */

	i = 1;

	do {
		if (i) {
			hmaddr->hmdc = _rm05.cn;
			hmaddr->hmda = (_rm05.tn << 8) | _rm05.sn;
			MBA0->mba_var = ((int) io->i_ma) & 0x1ff;
			MBA0->mba_bcr = -io->i_cc;
		}
		hmaddr->hmcs1 = unit;
		while ((MBA0->mba_sr & DTCMP) == 0);
		if ((MBA0->mba_sr & DTABT) == 0
		    || ((hmaddr->hmer1 == DCK) && _hmecc () < 0))
			return (io->i_cc);
		MBA0->mba_cr = INIT;
		hmaddr->hmcs1 = DCLR | GO;
	} while (++errcnt < 10);

	errno = EIO;
	return (-1);
}

static
_hmecc (io, hmaddr)
register struct iob *io;
register struct device *hmaddr; {
	register i, n, b;

	if (hmaddr->hmec2 == 0)
		return (1);
	i = (hmaddr->hmec1 & 0xffff) - 1;
	n = i & 017;
	i = (i & ~017) >> 3;
	b = (((MBA0->mba_bcr & 0xffff) + io->i_cc - 1) >> 9) & 0177;
	i += b << BSHIFT;
	if (i < io->i_cc)
		*((int *) (&io->i_ma[i])) ^= hmaddr->hmec2 << n;
	i += 2;
	if (i < io->i_cc)
		*((int *) (&io->i_ma[i])) ^= hmaddr->hmec2 >> (16-n);
	if (MBA0->mba_bcr == 0)
		return (-1);
	i = _rm05.tn + _rm05.sn + b + 1;
	if (i >= NSECT * NTRAC) {
		i -= NSECT * NTRAC;
		++_rm05.cn;
	}
	_rm05.tn = i / NSECT;
	_rm05.sn = i % NSECT;
	hmaddr->hmdc = _rm05.cn;
	hmaddr->hmda = (_rm05.tn << 8) | _rm05.sn;
	return (0);
}
