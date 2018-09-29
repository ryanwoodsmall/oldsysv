/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include <stand.h>
#include <sys/dcu.h>
#include <sys/disk.h>


/*	Disk configuration information
 *
 *	NOTE: <sys/disk.h> must be included before these definitions
 */


/* Parameters of the known disk types */

/* 3M 20 MB */
#define	S_T_3M20 30		/* sectors per track */
#define	T_C_3M20 4		/* tracks per cylinder */
#define	C_U_3M20 280		/* cylinders per unit */
#define	S_C_3M20 (30*4)		/* sectors per cylinder */
#define	S_U_3M20 (30*4*280)	/* sectors per unit */
#define	PZ_3M20 29		/* formatting data for sector zero */
#define	PNZ_3M20 57		/* formatting data for nonzero sectors */

/* 3M 60 MB */
#define	S_T_3M60 32		/* sectors per track */
#define	T_C_3M60 4		/* tracks per cylinder */
#define	C_U_3M60 838		/* cylinders per unit */
#define	S_C_3M60 (32*4)		/* sectors per cylinder */
#define	S_U_3M60 (32*4*838)	/* sectors per unit */
#define	PZ_3M60	9		/* formatting data for sector zero */
#define	PNZ_3M60 21		/* formatting data for nonzero sectors */

/* IMI 20 MB */
#define	S_T_IMI20 18		/* sectors per track */
#define	T_C_IMI20 5		/* tracks per cylinder */
#define	C_U_IMI20 388		/* cylinders per unit */
#define	S_C_IMI20 (18*5)	/* sectors per cylinder */
#define	S_U_IMI20 (18*5*388)	/* sectors per unit */
#define	PZ_IMI20 29		/* formatting data for sector zero */
#define	PNZ_IMI20 61		/* formatting data for nonzero sectors */

/* IMI 60 MB */
#define	S_T_IMI60 18		/* sectors per track */
#define	T_C_IMI60 7		/* tracks per cylinder */
#define	C_U_IMI60 776		/* cylinders per unit */
#define	S_C_IMI60 (18*7)	/* sectors per cylinder */
#define	S_U_IMI60 (18*7*776)	/* sectors per unit */
#define	PZ_IMI60 29		/* formatting data for sector zero */
#define	PNZ_IMI60 61		/* formatting data for nonzero sectors */

/* PRIAM 70 MB */
#define	S_T_PRI70 22		/* sectors per track */
#define	T_C_PRI70 5		/* tracks per cylinder */
#define	C_U_PRI70 1049		/* cylinders per unit */
#define	S_C_PRI70 (22*5)	/* sectors per cylinder */
#define	S_U_PRI70 (22*5*1049)	/* sectors per unit */
#define	PZ_PRI70 23		/* formatting data for sector zero */
#define	PNZ_PRI70 73		/* formatting data for nonzero sectors */

/* PRIAM 85 MB */
#define	S_T_PRI85 34		/* sectors per track */
#define	T_C_PRI85 5		/* tracks per cylinder */
#define	C_U_PRI85 850		/* cylinders per unit */
#define	S_C_PRI85 (34*5)	/* sectors per cylinder */
#define	S_U_PRI85 (34*5*850)	/* sectors per unit */
#define	PZ_PRI85 31		/* formatting data for sector zero */
#define	PNZ_PRI85 53		/* formatting data for nonzero sectors */

/* HITACHI 48 MB */
#define	S_T_HIT48 25		/* sectors per track */
#define	T_C_HIT48 6		/* tracks per cylinder */
#define	C_U_HIT48 522		/* cylinders per unit */
#define	S_C_HIT48 (25*6)	/* sectors per cylinder */
#define	S_U_HIT48 (25*6*522)	/* sectors per unit */
#define	PZ_HIT48 17		/* formatting data for sector zero */
#define	PNZ_HIT48 61		/* formatting data for nonzero sectors */

struct	partition pt_3m20[8] = { /* partitions for 3M20 drives */
	238*S_C_3M20,	0,	/* A = cylinders 0 through 237 */
	42*S_C_3M20,	238,	/* B = cylinders 238 through 279 */
	S_U_3M20,	0,	/* C = cylinders 0 through 279 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_3m60[8] = {/* partitions for 3M60 drives */
	796*S_C_3M60,	0,	/* A = cylinders 0 through 795 */
	42*S_C_3M60,	796,	/* B = cylinders 796 through 837 */
	S_U_3M60,	0,	/* C = cylinders 0 through 837 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_imi20[8] = { /* partitions for IMI20 drives */
	332*S_C_IMI20,	0,	/* A = cylinders 0 through 331 */
	56*S_C_IMI20,	332,	/* B = cylinders 332 through 387 */
	S_U_IMI20,	0,	/* C = cylinders 0 through 387 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_imi60[8] = { /* partitions for IMI 60Mb drives */
	720*S_C_IMI60,	0,	/* A = cylinders 0 through 719 */
	56*S_C_IMI60,	720,	/* B = cylinders 720 through 775 */
	S_U_IMI60,	0,	/* C = cylinders 0 through 775 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_pri70[8] = { /* partitions for PRIAM 70Mb drives */
	1000*S_C_PRI70,	0,	/* A = cylinders 0 through 999 */
	49*S_C_PRI70,	1000,	/* B = cylinders 1000 through 1048 */
	S_U_PRI70,	0,	/* C = cylinders 0 through 1048 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_pri85[8] = { /* partitions for PRIAM 85Mb drives */
	800*S_C_PRI85,	0,	/* A = cylinders 0 through 799 */
	49*S_C_PRI85,	1000,	/* B = cylinders 800 through 850 */
	S_U_PRI85,	0,	/* C = cylinders 0 through 850 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct	partition pt_hit48[8] = { /* partitions for HITACHI 48Mb drives */
	488*S_C_HIT48,	0,	/* A = cylinders 0 through 487 */
	34*S_C_HIT48,	488,	/* B = cylinders 488 through 521 */
	S_U_HIT48,	0,	/* C = cylinders 0 through 521 */
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

struct unitdata {
	short	ud_ploz;	/* formatting data for sector zero */
	short	ud_plonz;	/* formatting data for non-zero sectors */
	short	ud_nsec;	/* # of sectors per track */
	short	ud_ntr;		/* # of tracks per cylinder */
	short	ud_ncyl;	/* # of cylinders per unit */
	short	ud_spcyl;	/* # of sectors per cylinder */
	long	ud_spunit;	/* # of sectors per unit */
	struct partition *ud_pt; /* pointer to disk partition table */
	char	*ud_name;	/* name of disk type */
};

typedef struct {		/* Convenient for assigning partition tables */
	struct partition pt[8];
} partitions;


/* The table of all known disk types */

#define	DD(name, s_t, t_c, c_u, ploz, plonz, pt) \
	 ploz, plonz, s_t, t_c, c_u, (s_t)*(t_c), (s_t)*(t_c)*(c_u), pt, name

struct	unitdata unitsizetable[] = {
    DD("3M20", S_T_3M20, T_C_3M20, C_U_3M20, PZ_3M20, PNZ_3M20, pt_3m20),
    DD("3M60", S_T_3M60, T_C_3M60, C_U_3M60, PZ_3M60, PNZ_3M60, pt_3m60),
    DD("IMI20",S_T_IMI20,T_C_IMI20,C_U_IMI20,PZ_IMI20,PNZ_IMI20,pt_imi20),
    DD("IMI60",S_T_IMI60,T_C_IMI60,C_U_IMI60,PZ_IMI60,PNZ_IMI60,pt_imi60),
    DD("PRI70",S_T_PRI70,T_C_PRI70,C_U_PRI70,PZ_PRI70,PNZ_PRI70,pt_pri70),
    DD("PRI85",S_T_PRI85,T_C_PRI85,C_U_PRI85,PZ_PRI85,PNZ_PRI85,pt_pri85),
    DD("HIT48",S_T_HIT48,T_C_HIT48,C_U_HIT48,PZ_HIT48,PNZ_HIT48,pt_hit48),
    0	/* end of table */
};

#define DCUERROR	-2
#define NOHDR		-3
#define	TYPEERROR	-4

#define	NRETRIES	8
int _nleft;

struct device {
	unsigned char chan00;
	char fill0;
	unsigned char chan01;
	char fill1;
	unsigned char chan02;
	char fill2;
	unsigned char chan03;
	char fill3;
	unsigned char status;
	char fill4[7];
	unsigned char errack;
};

#define DCUADDR	((struct device *)0xd00000)

int gd_boot = 1*8+0;	/* drive 1, slice 0 */

#define MAXUNITS	8

struct disktab *unittbl[MAXUNITS], holdtbl[MAXUNITS];

struct dcu_iocb iocb, erriocb;

#define MAXNBL	2
char dskbuf[(MAXNBL+1)*512];


/* ********************************************************************** */

_dcopen(io)
struct iob *io;
{
	register int rc;		/* return code from initunit */

	_nleft = NRETRIES;	/* number of retries */
#ifdef	DEBUG
	printf("Entering dcopen...\n");
#endif
	do {
		rc = initunit(io->i_dp->dt_unit >> 3);
		if (rc == 0)
			return (0);	/* Initunit was OK */
		else
			_nleft--;
#ifdef	DEBUG
		printf("initunit failed (rc = %d), %d tries left.\n",
			rc, _nleft);
#endif
	} while (_nleft > 0);
	return (rc);			/* Unsuccessful */
}

_dcstrategy(io, func)
struct iob *io;
{
#ifdef	DEBUG
	printf("Dcstrategy: cmd = %d, unit = %d, blk # = %d\n",
		func, io->i_dp->dt_unit, io->i_bn);
#endif
	if (dc_many(func, io->i_dp->dt_unit, io->i_bn,
	    io->i_cc/512, io->i_ma) == 0) {
		return(io->i_cc);
	} else {
		errno = EIO;
		return(-1);
	}
}

dc_cmd(cmd, args)
unsigned char cmd;
{
	register struct device *dp;
	register unsigned char status;
	register char *p;
	int i;

	dp = DCUADDR;
	while ((dp->status & DS_READY) == 0)
		;
	for (i = 1; i < 1000; i++)	/* Short delay for hardware */
		;
	p = (char *)&args;
	dp->chan00 = *p++;
	dp->chan01 = *p++;
	dp->chan02 = *p;
	dp->chan03 = cmd;
wait:
	status = dp->status;
	if ((status & DS_READY) == 0)
		goto wait;
	if (status & DS_SELFTEST)
		goto wait;
	if (status & DS_ERINT)
		dp->errack = 0;
	status = STATUS0(status);
	if (status == DS_BUSY)
		goto wait;
	if (status != DS_IDLE)
		dp->status = 0;
	if (status == DS_DONE) {
		return (0);
	} else {
#ifdef	DEBUG
		printf("dc_cmd: BAD\n");
		printf("   Status = %x\n", status);
#endif
		return (DCUERROR);
	}
}

initunit(drive)
register int drive;	/* disk drive # */
{
	register int rc;

	if (drive < 0 || drive >= MAXUNITS)
		return (-1);
	if (unittbl[drive] != NULL) {
#ifdef	DEBUG
		printf("Unit %d table pointer = 0x%x, assuming drive is OK.\n",
			drive, unittbl[drive]);
#endif
		return (0);
	}
	rc = DCUADDR->chan01; /* enable writes */
	if ((rc=dc_cmd(DC_SETBURST, 7)) != 0)
		return(rc);
	if ((rc=dc_cmd(DC_ASSDRIVE, DD_DRIVE(drive))) != 0)
		return(rc);
	if ((rc=dc_cmd(DC_SPECIFIC, DD_ENABLE(1))) != 0)
		return(rc);
	if ((rc=get_dtab(drive)) != 0) {
		rc = autoconfig(drive);
		printf("\007\nWARNING: Unit %d needs a header\n", drive);
		return(rc);
	}
	return (0);
}

get_dtab(drive)
register int drive;
{
	register struct dcu_iocb *ip;
	char *bp;

	bp = (char *)((int)&dskbuf[512] & ~0x1ff);
	ip = &iocb;
	ip->dc_opcode = DC_READ;
	ip->dc_count = 1;
	ip->dc_cylinder = 0;
	ip->dc_drhdsec = DD_DHS(drive,0,0);
	ip->dc_buffer = (int)&bp;
	ip->dc_chain = 0;
	ip->dc_sense = 0;
	ip->dc_ecc = 0;
	ip->dc_status = 0;
	if (dc_cmd(DC_START, &ip->dc_opcode) != 0) {
#ifdef	DEBUG
		printf("dc_cmd error in trying to read header on drive %d\n",
			drive);
#endif
		erriocb = iocb;
		return (DCUERROR);
	}
#ifdef DEBUG
	{ struct disktab *dt;
	dt = (struct disktab *)bp;
	printf("The drive header was read.  Here are the parameters found:\n");
	printf("nsec %d, ntr %d, ncyl %d, spcyl %d, spunit %d, ploz %d, plonz %d\n",
	   dt->dt_nsec, dt->dt_ntr, dt->dt_ncyl, dt->dt_spcyl, dt->dt_spunit,
	   dt->dt_ploz, dt->dt_plonz);
	printf("0n %d, 0c %d, 1n %d, 1c %d, 2n %d, 2c %d, 3n %d, 3c %d\n",
	   dt->dt_part[0].nbl, dt->dt_part[0].cyloff,
	   dt->dt_part[1].nbl, dt->dt_part[1].cyloff,
	   dt->dt_part[2].nbl, dt->dt_part[2].cyloff,
	   dt->dt_part[3].nbl, dt->dt_part[3].cyloff);
	}
#endif
	if (*(int *)bp != DMAGIC) {
		printf("Bad block 0 magic\n");
		return (NOHDR);
	}
	holdtbl[drive] = *(struct disktab *)bp;
	unittbl[drive] = &holdtbl[drive];
#ifdef	DEBUG
	printf("Initunit successful: address of drive %d table = 0x%x\n",
		drive, unittbl[drive]);
	printf("Magic number of drive %d = 0x%x\n",drive,(int *)holdtbl[drive]);
#endif
	return (0);
}

autoconfig(unit)
register int unit;
{
	register struct disktab *dt;
	register struct	unitdata *ud;
	register int error;

	printf("\nUnit %d auto configure ", unit);
	dt = &holdtbl[unit];
	for (ud=unitsizetable; ud->ud_nsec; ud++) {	/* search type table */
		error = doseek(unit, ud->ud_ncyl-1);	/* seek to last cyl */
		if (error) continue;			/* failed, wrong type */
		error = doseek(unit, ud->ud_ncyl);	/* seek beyond end */
		if (error == 0) continue;		/* worked, wrong type */
	/* initialize the disk configuration */
		dt->dt_magic = DMAGIC;
		dt->dt_nsec = ud->ud_nsec;
		dt->dt_ntr = ud->ud_ntr;
		dt->dt_ncyl = ud->ud_ncyl;
		dt->dt_spcyl = ud->ud_spcyl;
		dt->dt_spunit = ud->ud_spunit;
	/* initialize the partition table */
		*(partitions *)dt->dt_part = *(partitions *)ud->ud_pt;
	/* initialize the name of the unit */
		strcpy(dt->dt_name,ud->ud_name);
	/* initialize formatting info */
		dt->dt_ploz = ud->ud_ploz;
		dt->dt_plonz = ud->ud_plonz;
		unittbl[unit] = dt;			/* store pointer */
		printf("succeeds\n");
#ifdef DEBUG
	printf("The autoconfigure parameters for the drive follow:\n");
	printf("nsec %d, ntr %d, ncyl %d, spcyl %d, spunit %d, ploz %d, plonz %d\n",
	   dt->dt_nsec, dt->dt_ntr, dt->dt_ncyl, dt->dt_spcyl, dt->dt_spunit,
	   dt->dt_ploz, dt->dt_plonz);
	printf("0n %d, 0c %d, 1n %d, 1c %d, 2n %d, 2c %d, 3n %d, 3c %d\n",
	   dt->dt_part[0].nbl, dt->dt_part[0].cyloff,
	   dt->dt_part[1].nbl, dt->dt_part[1].cyloff,
	   dt->dt_part[2].nbl, dt->dt_part[2].cyloff,
	   dt->dt_part[3].nbl, dt->dt_part[3].cyloff);
#endif
		return(0);			/* successful!! */
	}
	unittbl[unit] = (struct disktab *)0;
	printf("failed\n");
	return(TYPEERROR);		/* don't know this type of disk */
}


/* Do a seek operation for a unit.  This routine does not check the unit nor
 * the cylinder number for legality.  This fact is relied upon by the
 * initunit routine in order to determine the type of a disk.
 */
doseek(unit, cylinder)
{
	register long	error;

	error = dc_cmd(DC_ASSDRIVE, DD_DRIVE(unit));
	if (error) goto done;
	error = dc_cmd(DC_ASSCYLINDER, cylinder);
	if (error) goto done;
	error = dc_cmd(DC_SPECIFIC, DD_SEEK);
#ifdef	DEBUG
done:	printf("Seeking to cylinder %d of unit %d, status = %d\n",
		cylinder, unit, error);
	return(error);
#else
done:	return(error);
#endif
}


/*
 * This routine does the actual disk read/write.
 * addr must be 512 aligned.
 * Unit must have been initialized.
 * nbl must be <= MAXNBL.
 * All blocks must be on same cylinder.
 * unit # = 8*(drive #) + (partition #)
 */

dc_io(cmd, unit, bn, nbl, addr)
char *addr;
{
	register struct dcu_iocb *ip;
	register struct disktab *tp;
	int track, sector;
	char *bp[MAXNBL];
	int i, cylofs;	    /* cylofs = cyl. offset for the partition */

#ifdef	DEBUG
	printf("Dc_io1: cmd = %d, unit = %d, bn = %d, nbl = %d, addr = 0x%x\n",
		cmd, unit, bn, nbl, addr);
#endif
	for (i=0; i<nbl; i++)
		bp[i] = addr + (i*512);
	ip = &iocb;
	tp = unittbl[unit >> 3];
	if (bn < 0 || bn >= tp->dt_part[unit&07].nbl) {
	   _prs("dc_io: block # not in partition\n");
#ifdef	DEBUG
	   printf("Ptr tp = 0x%x, # of blks in pt %d of drive %d = %d, cylofs = %d\n",
		tp, unit&07, unit>>3, tp->dt_part[unit&07].nbl, tp->dt_part[unit&07].cyloff);
	   printf("Command = %d, unit = %d, attempted blk # = %d, nblks = %d\n",
		cmd, unit, bn, nbl);
#endif
	   erriocb = iocb;
	   return (ENXIO);
	}
	ip->dc_opcode = cmd;
	ip->dc_count = nbl;
	cylofs = tp->dt_part[unit&07].cyloff;
	ip->dc_cylinder = bn / tp->dt_spcyl + cylofs;
	track = (bn % tp->dt_spcyl) / tp->dt_nsec;
	sector = bn % tp->dt_nsec;
#ifdef	DEBUG
	printf("Dc_io2: drive = %d, abscyl = %d (pt = %d, cylofs = %d, blk in pt = %d)\n",
		unit>>3, bn/tp->dt_spcyl+cylofs, unit&07, cylofs, bn);
	printf("        track (head) = %d, sector = %d\n", track, sector);
#endif
	ip->dc_drhdsec = DD_DHS(unit >> 3, track, sector);
	ip->dc_buffer = (int)bp;
	ip->dc_chain = 0;
	ip->dc_sense = 0;
	ip->dc_ecc = 0;
	ip->dc_status = 0;
	if (dc_cmd(DC_START, &ip->dc_opcode) == 0)
		return (0);
	erriocb = iocb;
	return (DCUERROR);
}

/*
 * dc_many(R/W, unit, bn, nbl, addr)
 * splits requests into MAXNBL at a time
 * makes sure request is all on one cylinder
 * changes R/W to DCU command
 * makes sure addr is on 512 boundary (do copy if not)
 */

dc_many(func, unit, bn, nbl, addr)
char *addr;
{
	register struct disktab *tp;
	register char *myaddr;
	char *mybuf;
	int cmd, rc, count;

	tp = unittbl[unit >> 3];
	if (tp == NULL)
		return (-1);
	cmd = func == READ ? DC_READ : DC_WRITE;
	mybuf = (char *)((int)&dskbuf[512] & ~0x1ff);

	while (nbl > 0) {
		count = nbl > MAXNBL ? MAXNBL : nbl;
		/* make sure all within cylinder */
		rc = tp->dt_spcyl - (bn % tp->dt_spcyl);
		if (count > rc)
			count = rc;
		myaddr = ((int)addr & 0x1ff) ? mybuf : addr;

		if (func == WRITE && addr != myaddr)
			wcopy(addr, myaddr, count*128);

		_nleft = NRETRIES;
		do {
			rc = dc_io(cmd, unit, bn, count, myaddr);
			if (rc != 0)
				_nleft--;
		} while (rc != 0 && _nleft > 0);
		if (rc != 0)
			return (rc);

		if (func == READ && addr != myaddr)
			wcopy(myaddr, addr, count*128);

		bn += count;
		addr += count*512;
		nbl -= count;
	}
	return (0);
}

wcopy(from, to, n)
int *from, *to;
{
	while (n--)
		*to++ = *from++;
}
