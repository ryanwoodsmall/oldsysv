static char	sccsid[] = "@(#)PDPgd.c	1.2";

/*
 * RP04/RP06 and RM05 disk driver
 */

#include "stand.h"

int gd_addr, gd_boot;

struct	device {
	int	cs1;		/* Control and Status register 1 */
	int	wc;		/* Word count register */
	caddr_t	ba;		/* UNIBUS address register */
	int	da;		/* Desired address register */
	int	cs2;		/* Control and Status register 2*/
	int	ds;		/* Drive Status */
	int	err1;		/* Error register 1 */
	int	as;		/* Attention Summary */
	int	la;		/* Look ahead */
	int	db;		/* Data buffer */
	int	mr;		/* Maintenance register */
	int	dt;		/* Drive type */
	int	sn;		/* Serial number */
	int	of;		/* Offset register */
	int	dc;		/* Desired Cylinder address register */
	int	cc;		/* Current Cylinder */
	int	er2;		/* Error register 2 */
	int	er3;		/* Error register 3 */
	int	ecc1;		/* Burst error bit position */
	int	ecc2;		/* Burst error bit pattern */
	int	bae;		/* 11/70 bus extension */
	int	cs3;
};

struct dlimits {
	int dtype, nsect, ntrak;
	char *dname;
} d_limit[] = {
	{ 0x2010, 22, 19, "RP04"},
	{ 0x2011, 22, 19, "RP05"},
	{ 0x2012, 22, 19, "RP06"},
	{ 0x2017, 32, 19, "RM05"},
	{ 0x2022, 50, 32, "RP07"},
	{ 0, 0, 0, "???"}	/* leave last in case of funny type */
};
int dr_typ;

#define NTYPES ((sizeof d_limit) / (sizeof(struct dlimits)))

#define HPADDR	((struct device *) 0176700)

#define NSECT d_limit[dr_typ].nsect
#define NTRAC d_limit[dr_typ].ntrak
#define DNAME d_limit[dr_typ].dname


#define GO	01
#define DCLR	010
#define PRESET	020
#define WCOM	060
#define RCOM	070
#define RDY	0200

#define VV	0100
#define MOL	010000
#define TRE	040000

#define FMT22	010000

#define DCK	0100000

static struct {
	int	cn;
	int	tn;
	int	sn;
} _drive;

_gdstrategy (io, func)
register struct iob *io; {
	register i, unit;
	int errcnt = 0;
	daddr_t bn;

	bn = io->i_bn;

	HPADDR->cs2 = unit;

	if ((HPADDR->ds & VV) == 0) {
		HPADDR->cs1 = PRESET | GO;
		HPADDR->of = FMT22;
	}

	for(dr_typ = 0; dr_typ < NTYPES; dr_typ++)
		if((HPADDR->dt&070777) == d_limit[dr_typ].dtype) break;
	if(d_limit[dr_typ].nsect == 0){
		_prs("NO DEVICE\n");
		errno = ENXIO;
		return(-1);
	}

	_drive.cn = bn / (NSECT * NTRAC);
	_drive.sn = bn % (NSECT * NTRAC);
	_drive.tn = _drive.sn / NSECT;
	_drive.sn %= NSECT;

	unit = (segflag << 8) | GO;
	if (func == READ)
		unit |= RCOM;
	else
		unit |= WCOM;

	if ((HPADDR->ds & MOL) == 0) {
		_prs ("\n");
		_prs (DNAME);
		_prs (" : unit not ready\n");
		while ((HPADDR->ds & MOL) == 0);
	}

	i = 1;

	do {
		if (i) {
			HPADDR->dc = _drive.cn;
			HPADDR->da = (_drive.tn << 8) | _drive.sn;
			HPADDR->ba = io->i_ma;
			HPADDR->wc = -(io->i_cc >> 1);
		}
		HPADDR->cs1 = unit;
		while ((HPADDR->cs1 & RDY) == 0);
		if ((HPADDR->cs1 & TRE) == 0
		    || ((HPADDR->err1 == DCK) && (i = _eccc ()) < 0)){
			return (io->i_cc);
		}
_prs ("\n");
_prs (DNAME);
_prs(" : Data Check");
		HPADDR->cs1 = TRE | DCLR | GO;
	} while (++errcnt < 10);

_prs ("\n");
_prs (DNAME);
_prs(" : Hard Error");
	errno = EIO;
	return (-1);
}

static
_eccc (io)
register struct iob *io; {
	register i, n;
	int b;

	if (HPADDR->ecc2 == 0){
		return (1);
	}
	i = HPADDR->ecc1 - 1;
	n = i & 017;
	i = (i & ~017) >> 3;
	b = ((HPADDR->wc + (io->i_cc >> 1) - 1) >> 8) & 0377;
	i += b << BSHIFT;
	if (i < io->i_cc)
		*((int *) (&io->i_ma[i])) ^= HPADDR->ecc2 << n;
	i += 2;
	if (i < io->i_cc)
		*((int *) (&io->i_ma[i])) ^= HPADDR->ecc2 >> (16-n);
	if (HPADDR->wc == 0){
		return (-1);
	}
	i = _drive.tn + _drive.sn + b + 1;
	if (i >= NSECT * NTRAC) {
		i -= NSECT * NTRAC;
		++_drive.cn;
	}
	_drive.tn = i / NSECT;
	_drive.sn = i % NSECT;
	HPADDR->dc = _drive.cn;
	HPADDR->da = (_drive.tn << 8) | _drive.sn;
	return (0);
}
