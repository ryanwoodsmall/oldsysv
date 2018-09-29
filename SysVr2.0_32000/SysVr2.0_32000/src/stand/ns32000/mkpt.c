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
 * mkpt.c
 */

#include <stand.h>
#include <sys/ino.h>
#include <sys/disk.h>

#define NULL	0
#define HEADERBLOCK 0
#define PGSIZE	512
#define MAXUNITS 8

extern struct disktab holdtbl[];	/* holds disk unit entries (dc.c) */
extern int strncmp();			/* string comparison for getpt */

static union diskun {
	struct disktab tab;
	char pad[PGSIZE];
}dun;

#define dtsize sizeof(dun)
#define disktab dun.tab

extern struct	iob _iobuf[NFILES];	/* defined in Aconf.c */

char *baddev = "Bad device name\n";
char *openfail = "Open failure: %s\n";
char *iofail = "Header %s failed\n";

char devname[20];			/* device name buffer */
char drivenm[13] = "/dev/dsk/0s0";	/* name of drive with partition = 0 */
static int curunit = -1;		/* the current unit in core */
static int maxcyl,maxblocks;		/* should be obvious */
static int partition;
static fn;


/*
 *		Main program
 */

main()
{
	struct partition *getpt();
	register struct partition *pp;
	int offset,nblocks;

	printf("\nmkpt -\tDisk names are of the form /dev/dsk/1s0.\n");
	printf("\tUse '#' for character erase, '@' for line kill.\n");

	for(;;) {
		pp = getpt(devname);
		printf("Current offset %d, size %d\n",pp->cyloff,pp->nbl);
		printf("New cylinder offset (0-%d), or <RET> to cancel: ",maxcyl);
		offset = check(0,maxcyl);
		if (offset < 0)
			continue;
		maxblocks -= offset * disktab.dt_spcyl;
		printf("New size (0-%d), or <RET> to cancel: ",maxblocks);
		nblocks = check(0,maxblocks);
		if (nblocks < 0)
			continue;
		pp->cyloff = offset;
		pp->nbl = nblocks;
#ifdef	DEBUG
		printf("New offset = %d, size = %d\n", offset, nblocks);
#endif
		update(devname);
	}
}

struct partition*
getpt(str)
char *str;
{
	register char *cp;
	register struct iob *file;
	register int pt;
	char reply[5];
	int drv, i, rc, savenbl, saveoffs, tstunit;

restart:
	printf("\nDisk (or ?n for drive 'n' partitions): ");
	gets(str);
	if (*str == 0) _stop("Done\n");
	if (*str == '?') {	/* Print out the info on the indicated drive */
		cp = str;
		cp++;
		drv = *cp++;
		if (drv < '0' || drv > '7' || *cp != 0) {
			printf("Bad drive specification\n");
			goto restart;
		}
		drivenm[9] = drv;
		if((fn = open(drivenm,0)) < 0) {
			printf(openfail,drivenm);
			goto restart;
		}
		file = &_iobuf[fn-3];
		tstunit = file->i_dp->dt_unit >> 3;   /* unit = drive<<3+pt */
		rc = gethdr(tstunit,file);
		close(fn);
		if (rc < 0) {
			printf(iofail,"read");
			goto restart;
		}
		printf("\nDrive %c:    Partition    Cyl. offset    Blocks\n",
			drv);
		for (i=0; i<MAXUNITS; i++) {
			printf("                %d          %-10d    %-10d\n",
				i, disktab.dt_part[i].cyloff,
				disktab.dt_part[i].nbl);
		}
		goto restart;
	}

	if (strncmp(str, "/dev/dsk/", 9) != 0) {
		printf(baddev);
		goto restart;
	}
	for (cp = (char *)((int)str + 9); *cp && *cp != 's'; cp++)
		;		/* look for slice */
	if (*cp++ != 's') {
		printf(baddev);
		goto restart;
	}
	pt = *cp++ - '0';
	if (*cp >= '0' && *cp <= '9')
		pt = pt * 10 + *cp++ - '0';

	if (pt < 0 || pt > 7) {
		printf("Bad offset specification\n");
		goto restart;
	}
	if (*cp != '\0') {
		printf(baddev);
		goto restart;
	}
	if((fn = open(str,0)) < 0) {
		printf(openfail,str);
		goto restart;
	}
	file = &_iobuf[fn-3];
	tstunit = file->i_dp->dt_unit >> 3;   /* unit = drive<<3 + partition */
	if (tstunit != curunit) {
		rc = gethdr(tstunit,file);
		close(fn);
		if (rc < 0) {
			printf(iofail,"read");
			goto restart;
		}
			
		/* Verify the header.  If it has bad magic, let the user
 		   decide whether to write the header with
		   default data or rewrite the current (if an intermittent
		   disk problem). */

		if (disktab.dt_magic != DMAGIC) {
			printf("\nCAUTION:  Drive %d has a bad disk magic number.  You can fix the disk header\n",
				tstunit);
			printf("            with the autoconfigure parameters (recommended), rewrite the\n");
			printf("            current header to the disk, or do nothing at all to the disk.\n");
			showdisk(tstunit);
			printf("\nDo you want to rewrite the header with the Autoconfigure defaults (y or n)? ");
			gets(reply);
			if (reply[0] == 'y' || reply[0] == 'Y') {
				disktab = holdtbl[tstunit]; /* defaults */
				printf("\nRewriting the autoconfigure header...  You may change the partitions if needed.\n");
				update(str);
				goto restart;
			}
			printf("\Do you want to rewrite the header with its current data (y or n)? ");
			gets(reply);
			if (reply[0] == 'y' || reply[0] == 'Y') {
				printf("\nRewriting the current header...  You may change the partitions if needed.\n");
				update(str);
				goto restart;
			}
			printf("\nNo update was made to the header.\n");
			goto restart;
		}
		curunit = tstunit;
	}
	maxcyl = disktab.dt_ncyl-1;
	maxblocks = disktab.dt_spunit;
	partition = pt;
	close(fn);
	return(&disktab.dt_part[pt]);
}

showdisk(unit)
int unit;
{
	int i;

	printf("\nCurrent header data & Autoconfigure data for '%s' disk %d:\n\n",
		holdtbl[unit].dt_name, unit);
	printf("    Parameter             Current Header Data     Autoconfigure Data\n");
	printf("Magic number                 %10x            %10x\n",
		disktab.dt_magic, holdtbl[unit].dt_magic);
	printf("Sector 0 format code         %10d            %10d\n",
		disktab.dt_ploz, holdtbl[unit].dt_ploz);
	printf("Non-zero format code         %10d            %10d\n",
		disktab.dt_plonz, holdtbl[unit].dt_plonz);
	printf("Sectors per track            %10d            %10d\n",
		disktab.dt_nsec, holdtbl[unit].dt_nsec);
	printf("Tracks per cylinder          %10d            %10d\n",
		disktab.dt_ntr, holdtbl[unit].dt_ntr);
	printf("Cylinders per unit           %10d            %10d\n",
		disktab.dt_ncyl, holdtbl[unit].dt_ncyl);
	printf("Sectors per unit             %10d            %10d\n",
		disktab.dt_spunit, holdtbl[unit].dt_spunit);
	for (i=0; i<MAXUNITS; i++) {
		printf("Pt. %1d cyloff, # sec       %10d,%-10d  %10d,%-10d\n",
			i, disktab.dt_part[i].cyloff, disktab.dt_part[i].nbl,
			holdtbl[unit].dt_part[i].cyloff, holdtbl[unit].dt_part[i].nbl);
	}
}

update(name)
char *name;
{
	register struct iob *file;
	int drv, fd, savenbl, saveoffs;

	fd = open(name,1);
	if (fd < 0) {
		printf(openfail,name);
		return;
	}
	file = &_iobuf[fd-3];
	drv = file->i_dp->dt_unit >> 3;
	file->i_dp->dt_unit &= ~07;	/* Force it to partition 0 */
	file->i_dp->dt_boff = HEADERBLOCK; /* Force to log. block 0 */
	savenbl = holdtbl[drv].dt_part[0].nbl;
	saveoffs = holdtbl[drv].dt_part[0].cyloff;
	holdtbl[drv].dt_part[0].nbl = 1; /* Force to read 1 block */
	holdtbl[drv].dt_part[0].cyloff = 0; /* Force to ph. bk 0 */
	if (write(fd,&disktab,dtsize) != dtsize) {
		printf(iofail,"write");
	}
	holdtbl[drv].dt_part[0].nbl = savenbl; /* Restore */
	holdtbl[drv].dt_part[0].cyloff = saveoffs;
	close(fd);
}

getnum()
{
	int c;
	char buffer[30];		/* a string buffer */
	register int n,base;
	register char *cp = buffer;

	gets(cp);
	if (*cp == '\0')	/* If they hit <RETURN>, then just exit */
		return(-1);
	n = 0;
	base = 10;
	while(c = *cp++) {
		if(c<'0' || c>'9') {
			printf("%s: bad number\n", buffer);
			return(-1);
		}
		n = n*base + (c-'0');
	}
	return(n);
}

check(lo,hi)
register int lo,hi;
{
	register int n;

	n = getnum();
	if (n < 0) {
		printf("\tCancelled\n");
		return(-1);
	}
	if (n < lo || n > hi) {
		printf("number out of range\n");
		return(-1);
	}
	return(n);
}

_stop(s)
char	*s;
{
	int i;

	for (i=0; i < NFILES; i++)
		if(_iobuf[i].i_flgs != 0)
			close(i);
	printf("%s\n", s);
	asm ("bpt");
}

int gethdr(unit,file)
int unit;
struct iob *file;
{
	int rc, savenbl, saveoffs;

	/*
 	 * We have to be able to get at block 0 (header data) of the
	 * drive, so we fix the iob by telling it we want partition 0.
 	 */
	file->i_dp->dt_unit &= ~07; /* Force to partition 0 */
	file->i_dp->dt_boff = HEADERBLOCK; /* Force to log. block 0 */
	savenbl = holdtbl[unit].dt_part[0].nbl;
	saveoffs = holdtbl[unit].dt_part[0].cyloff;
	holdtbl[unit].dt_part[0].nbl = 1; /* Force to read 1 block */
	holdtbl[unit].dt_part[0].cyloff = 0; /* Force to ph. bk 0 */
	/* read the header */
#ifdef	DEBUG
	printf("Reading the header from unit %d, block 0\n",
		file->i_dp->dt_unit);
#endif
	rc = read(fn,&disktab,dtsize);
	holdtbl[unit].dt_part[0].nbl = savenbl; /* Restore */
	holdtbl[unit].dt_part[0].cyloff = saveoffs;
	if (rc != dtsize) {
		return(-1);
	}
	return(0);
}
