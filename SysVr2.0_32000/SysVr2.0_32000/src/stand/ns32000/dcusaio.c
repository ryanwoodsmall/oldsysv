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
/*
 *      dcusaio.c:  standalone disk routines for dcutest
 */

#include <sys/types.h>
#include "dcudisk.h"
#include "dcusaio.h"

#define	PAGESIZE	512		/* page and sector size */
#define	PAGEOFFSET	511		/* maximum offset into a page */
#define	MAXSECTORS	128		/* maximum sectors drive can transfer */
#define	BURST		7		/* burst (must match KERNEL/HARDWARE) */
#define	INTERLEAVE	0		/* default interleave value */
#define	RETRIES		10		/* number of retries on errors */
#define	GETB(d,i) (((unsigned char *)(&d))[i])	/* to reference data as chars */
#define BADR(n) ((unsigned char *)(n))	/* byte address recast */
#define DCUA_STATUS	BADR(0xd00008)  /* immediate status (read only) */
#define	DCUA_CHAN0	BADR(0xd00000)  /* chan 0 controller bytes */
#define DCUA_ACK0	BADR(0xd00008)  /* chan 0 interrupt acknowledge */
#define	DCUA_SETDAFF	BADR(0xd00002)	/* enable the DCU to write to drives */
#define DCUS_READY	0x80		/* command port is ready */
#define	DCUS_SELFTEST	0x20		/* port executing selftest */
#define DCUSTS_BUSY	0		/* channel is executing a command */
#define DCUSTS_DONE	2		/* command completed successfully */
#define	DCUSTS_IDLE	3		/* channel is idle */
#define	STATUS0(data)	(((data)>>2)&3)	/* macro to get DCUS_CHAN0 */
#define	SELFTESTCODE	((char) 0xbf)	/* status code when selftest is done */
#define	DCUC_SELFTEST	0x16		/* self test */
#define	DCUC_START	0x18		/* start channel operation code */
#define DCUC_SETBURST	0x74		/* set burst/interleave */
#define	DCUC_ASSDRIVE	0x20		/* assign drive */
#define DCUC_ASSCYLINDER 0x2e		/* assign cylinder */
#define DCUC_ASSBUFFER	0x42		/* assign buffer address */
#define DCUC_READ	0xde		/* disk read code */
#define	DCUC_WRITE	0xe6		/* disk write code */
#define	DCUC_FORMAT	0xbc		/* format write */
#define	DCUC_SPECIFIC	0xe8		/* perform drive specific action */
#define	DCUC_ENABLE	0x41		/* DCUC_SPECIFIC code to write enable */
#define	DCUC_SEEK	0x03		/* DCUC_SPECIFIC code to seek */

#define	DCUD_DHS(drive,head,sector) (((drive)<<13)|((head)<<8)|(sector))
					/* data for DCU_READ and DCU_WRITE */
#define	DCUD_DRIVE(drive)	((drive)<<13)
					/* data for DCU_ASSDRIVE */
#define	DCUD_ENABLE(bit)	((DCUC_ENABLE<<8)|((bit)<<7))
					/* data for DCUC_ENABLE */
#define	DCUD_SEEK	((DCUC_SEEK)<<8)
					/* data for DCUC_SEEK */
#define	DCUD_SETBURST(burst,interleave) (((interleave)<<8)|(burst))
					/* data for DCUC_SETBURST */
#define	DCUD_FORMAT(drive,head,uploz,uplonz) \
	(((uploz)<<16)|((head)<<8)|((drive)<<13)|(uplonz))
					/* data for DCUC_FORMAT */


int	interleave = INTERLEAVE;		/* interleave value */
struct	iocb	iocb;				/* iocb data */
long	ptdata[MAXSECTORS];			/* storage for data pointers */
struct disktab disktab[MAXUNITS];		/* disk database */
extern struct unitdata unitsizetable[];
long	dcerror = NOERROR;			/* the disk error */


/* Subroutine to read/write the hard disk.
 * Parameters:
 *	writeflag/	Zero to read, nonzero to write
 *	unit/		Unit number for transfer
 *	sector/		beginning disk sector for transfer
 *	count/		number of sectors to transfer
 *	address/	memory address for transfer (must be on page blondary)
 * Returns:
 *	The number of sectors successfully transferred, and
 *	    ...sets dcerror to one of the following:
 *	NOERROR/	IO was successful
 *	ARGUMENTERROR/	Bad calling arguments
 *	DCUERROR/	DCU failed to accept commands
 *	SENSEERROR/	IO failed, but sense bytes were zero
 *	others/		IO failed, returned sense bytes describe why
 */
dodiskio(writeflag, unit, sector, count, address)
{
	register long	remaining, curcount, retries, nsecpercyl;
	long	opcode;

	dcerror = dc_initunit(unit);
	if (dcerror) goto done;
	if (((address & PAGEOFFSET) != 0)
		|| (sector < 0) || (count <= 0)
		|| (sector + count > unittable[unit]->dt_secperunit)) {
			dcerror = ARGUMENTERROR;
			goto done;
	}
	nsecpercyl = unittable[unit]->dt_secpercyl;
	opcode = writeflag ? DCUC_WRITE : DCUC_READ;
	remaining = count;
	while (remaining > 0) {
		curcount = nsecpercyl - (sector % nsecpercyl);
		if (curcount > count) curcount = remaining;
		if (curcount > MAXSECTORS) curcount = MAXSECTORS;
		retries = 0;
		do {
			if (retries) curcount = (curcount / 2) | 1;
			dcerror = realdiskio(opcode,unit,sector,
					curcount,address);
		} while (dcerror && (noretries == 0) && (retries++ <= RETRIES));
		remaining -= curcount;
		if (dcerror) {
			remaining += iocb.dcuio_resid;
			goto done;
		}
		sector += curcount;
		address += curcount * PAGESIZE;
	}
done:	
	return (count-remaining);
}


/* Routine to do a channel operation for the disk, and wait for it
 * to complete.  It is assumed that all operands are legal.  That is,
 * cylinder boundaries are not crossed, legal memory addresses are passed,
 * a legal opcode is given, the unit number is legal, and so on.
 */
realdiskio(opcode, unit, disksector, count, addr)
	register long	addr;
{
	register struct	iocb	*io;
	register long	i;
	long	nsecpertrk, nsecpercyl, cylinder, head, sector;

	nsecpertrk = unittable[unit]->dt_nsectors;
	nsecpercyl = unittable[unit]->dt_secpercyl;
	cylinder = disksector / nsecpercyl;
	head = (disksector % nsecpercyl) / nsecpertrk;
	sector = disksector % nsecpertrk;
	for (i=0; i<count; i++, addr+=PAGESIZE) ptdata[i] = addr;
	io = &iocb;
	io->dcuio_opcode = opcode;
	io->dcuio_count = count;
	io->dcuio_cylinder = cylinder;
	io->dcuio_drhdsec = DCUD_DHS(unit, head, sector);
	io->dcuio_buffer = (int) ptdata;
	io->dcuio_chain = 0;
	io->dcuio_sense = 0;
	io->dcuio_ecc = 0;
	io->dcuio_status = 0;
	if (dcucommand(DCUC_START, ((int)io)+2) == 0) return (NOERROR);
	erroriocb = iocb;		/* save error status */
	if (io->dcuio_sense) return (io->dcuio_sense);
	return (SENSEERROR);
}


/* Do a seek operation for a unit.  This routine does not check the unit nor
 * the cylinder number for legality.  This fact is relied upon by the
 * dc_initunit routine in order to determine the type of a disk.
 */
dcudoseek(unit, cylinder)
{
	register long	error;

	error = dcucommand(DCUC_ASSDRIVE, DCUD_DRIVE(unit));
	if (error) goto done;
	error = dcucommand(DCUC_ASSCYLINDER, cylinder);
	if (error) goto done;
	error = dcucommand(DCUC_SPECIFIC, DCUD_SEEK);

done:	return(error);
}



/* Do a formatting operation for a track of the disk.
 */
doformat(unit, cylinder, track)
{
	register long	error;
	long	ploz, plonz;

	error = dc_initunit(unit);	/* check unit and write enable it */
	if (error) goto done;
	ploz = unittable[unit]->dt_ploz;
	plonz = unittable[unit]->dt_plonz;
	error = dcudoseek(unit, cylinder);
	if (error) goto done;
	error = dcucommand(DCUC_FORMAT,DCUD_FORMAT(unit, track, ploz, plonz));

done:	return(error);
}


/* Set the interleave factor for formatting */
dointerleave(value)
{
	register int	error;

	error = dcucommand(DCUC_SETBURST, DCUD_SETBURST(BURST, interleave));
	if (error == NOERROR) interleave = value;
	return(error);
}


/* Do the selftest operation for the port.  This clears the unit table
 * so that it will be rebuilt as necessary.  The selftest operation can
 * not return a failure (if it fails, it never returns).
 */
doselftest()
{
	register unsigned int	unit;

	for (unit = 0; unit < MAXUNITS; unit++) {
		unittable[unit] = 0;
	}
	dcucommand(DCUC_SELFTEST, 0);
	return(NOERROR);
}


/* Initialize a unit of the DCU.  This sets the burst value, allows writes,
 * figures out the type of disk based on the number of cylinders the disk has,
 * and fills in unit table with a pointer to the disk information.  The type
 * of a unit is only determined on the first call.
 */
dc_initunit(unit)
	register unsigned int	unit;
{
	register long	error;
	static	int	dummy;		/* to make optimizer do references */

	if (unit >= MAXUNITS) return(ARGUMENTERROR);
	if (unittable[unit]) return(NOERROR);
	dummy += *DCUA_SETDAFF;		/* allow DCU to write */
	error = dcucommand(DCUC_SETBURST, DCUD_SETBURST(BURST, interleave));
	if (error) goto done;
	error = dcucommand(DCUC_ASSDRIVE, DCUD_DRIVE(unit));
	if (error) goto done;
	error = dcucommand(DCUC_SPECIFIC, DCUD_ENABLE(1)); /* write-enable */
	if (error) goto done;
	error = configure(unit);
	if (error != NOERROR) {
		/* as a last resort search table of known drives */
		error = dcuautoconfig(unit);
		printf("\7\nWARNING: Unit %d needs a header\n",unit);
	}

done:	return(error);
}

dcuautoconfig(unit)
register int unit;
{
	register struct disktab *dt;
	register struct	unitdata *ud;
	register int error;

	printf("\nUnit %d auto configure ",unit);
	dt = &disktab[unit];
	for (ud=unitsizetable; ud->d_nsectors; ud++) {	/* search type table */
		error = dcudoseek(unit, ud->d_ncylinders-1);/* seek to last cyl */
		if (error) continue;			/* failed, wrong type */
		error = dcudoseek(unit, ud->d_ncylinders);	/* seek beyond end */
		if (error == NOERROR) continue;		/* worked, wrong type */
		/* initialize the disk configuration */
		dt->dt_dc = ud->dc;
		/* initialize the partition table */
		*(partitions *)dt->dt_partition = *(partitions*)ud->ud_pt;
		/* initialize the name of the unit */
		strcpy(dt->dt_name,ud->ud_name);
		/* initialize formatting info */
		dt->dt_ploz = ud->ud_ploz;
		dt->dt_plonz = ud->ud_plonz;
		unittable[unit] = dt;			/* store pointer */
		printf("succeeds\n");
		return(NOERROR);			/* successful!! */
	}
	unittable[unit] = (struct disktab *)0;
	printf("failed\n");
	return(TYPEERROR);		/* don't know this type of disk */
}

/*
 * Attempt to read block 0 from the unit. This block will contain the unit
 * geometry. If the read is successful, the unit has been formatted before.
 * Verify (by the magic number) that the unit configuration has been written
 * before filling in the unit table.
 */

configure(unit)
{
	register struct disktab *dp,*dt;
	register struct iocb *io;
	long ptdata[2];
	char buffer[BSIZE+PAGESIZE];

	dp = (struct disktab*)((((int)buffer)+PAGESIZE) & ~PAGEOFFSET);

	ptdata[0] = (long)dp;
	ptdata[1] = (long)dp + PAGESIZE;
	io = &iocb;
	io->dcuio_opcode = DCUC_READ;
	io->dcuio_count =  2;
	io->dcuio_cylinder = 0;
	io->dcuio_drhdsec = DCUD_DHS(unit, 0, 0);
	io->dcuio_buffer = (int) ptdata;
	io->dcuio_chain = 0;
	io->dcuio_sense = 0;
	io->dcuio_ecc = 0;
	io->dcuio_status = 0;
	if (dcucommand(DCUC_START, ((int)io)+2) == 0) {
		if (dp->dt_magic == DMAGIC) {
			dt = &disktab[unit];
			*dt = *dp;
			unittable[unit] = dt;
			return(NOERROR);
		}
		return(TYPEERROR);
	}
	erroriocb = iocb;		/* save error status */
	if (io->dcuio_sense) return (io->dcuio_sense);
	return (SENSEERROR);
}



/* Execute a DCU command.  This routine does not return until the command
 * is complete, with or without an error.
 */
dcucommand(opcode, data)
{
	register unsigned char *cp = DCUA_CHAN0;	/* command bytes addr */
	register char	stat;		/* port status */
	register char	code;		/* port code */

	while ((*DCUA_STATUS & DCUS_READY) == 0) optdummy();
	cp[0] = GETB(data,0);		/* write data byte 0 to port */
	cp[2] = GETB(data,1);		/* write data byte 1 */
	cp[4] = GETB(data,2);		/* write data byte 2 */
	cp[6] = opcode;			/* write command byte */
	while (1) {
		optdummy();
		stat = *DCUA_STATUS;
		if (stat == SELFTESTCODE) return(NOERROR);
		if (stat & DCUS_SELFTEST) continue;
		if ((stat & DCUS_READY) == 0) continue;
		code = STATUS0(stat);
		if (code != DCUSTS_BUSY) break;
	}
	if (code != DCUSTS_IDLE) *DCUA_ACK0 = '\0';	/* ack the operation */
	if (code == DCUSTS_DONE) return (NOERROR);	/* successful */
	return (DCUERROR);				/* unsuccessful */
}


/* Subroutine called in order to make optimizer do device register references */
optdummy() {}
