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
/*	@(#)open.c	1.3	*/

/*
	   Open was modified to allocate a device table entry if none
	existed previously for a device associated with an open request.
	This saves having to preallocate every possible drive (and slice)
	and tape designation in the Ainit file (which uses most of the
	available device table entries).  This way, only the devices which
	will actually be accessed are put into the table, avoiding the
	likelihood of running out of device table entry slots.
	   A routine "chkentry" was added to see if the name argument of
	the open request corresponds to a valid device name.  If so, the
	unit numbers are checked for validity according to the device
	type, and a MKNOD is done if the name is a valid device.  If the
	names or unit numbers for the devices change, it may be necessary
	to edit the device information table below and the unit checking
	code in the "chkentry" routine.
*/

# include <stand.h>

ino_t	_find();
char	*strchr(), *strrchr();
int	strlen(), strncmp();

/*	Definitions for the device information table	*/

#define DSKTYPE	1
#define	RPTYPE	2
#define MTTYPE	3
#define VALDEV	8

static struct	{
	char	*devname;	/* name of device (excluding unit #'s */
	int	nlen;		/* length of matching part of name */
	int	devtype;	/* rp disk, dsk disk, or mt tape */
	int	majnum;
	int	unitnum;
} devinfo[] =

/* For the string comparisons to work correctly, the "0mn" entries must come
   before the "0m" entries for each tape device */

  {	/*  name		len	type		maj#	unit#  */

	{ "/dev/dsk/",		9,	DSKTYPE,	0,	0 },
	{ "/dev/rp",		7,	RPTYPE,		0,	0 },
	{ "/dev/mt/0mn",	11,	MTTYPE,		1,	4 },
	{ "/dev/mt/0m",		10,	MTTYPE,		1,	0 },
	{ "/dev/rmt/0mn",	12,	MTTYPE,		1,	4 },
	{ "/dev/rmt/0m",	11,	MTTYPE,		1,	0 },
	{ "/dev/mt0",		8,	MTTYPE,		1,	0 },
	{ "/dev/mt4",		8,	MTTYPE,		1,	4 }
  };

static char str[NAMSIZ];	/* These were made external so open and */
static struct dtab *dp;		/*  chkentry can access them directly */

open(strx, how)
char *strx; {
	register char *cp1, *cp2;
	register struct iob *io;
	struct mtab *mp;
	int fdesc;
	char c;
	ino_t n;

	if (strx == NULL || how < 0 || how > 2) {
		errno = EINVAL;
		return (-1);
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (_iobuf[fdesc].i_flgs == 0)
			goto gotfile;
	errno = EMFILE;
	return (-1);
gotfile:
	(io = &_iobuf[fdesc])->i_flgs |= F_ALLOC;

	_cond(strx, str);

	for (dp = &_dtab[0]; dp < &_dtab[NDEV]; dp++) {
#ifdef DEBUG
		printf("Open1: dp = %x, dt_name = '%s', str = '%s'\n", (int)dp,
			dp->dt_name, str);
#endif
		if (dp->dt_name == 0 || strcmp(str, dp->dt_name) == 0)
			break;
	}


/*	This is the addition to the previous version of "open" --
   If name wasn't in the device table, is it a valid device to be entered? */

	if ((dp->dt_name == 0) || dp == &_dtab[NDEV]) {
#ifdef DEBUG
	printf("Open2: name not in the device table, checking if it's a device\n");
#endif
		if (chkentry() < 0) return (-1); /* quit (invalid dev., etc.) */
	}


#ifdef DEBUG
	printf("Open3: after chkentry: dp = %x, dp->dt_name = '%s'\n", (int) dp,
		dp->dt_name);
#endif
	if (dp->dt_name && dp < &_dtab[NDEV]) {
#ifdef DEBUG
	printf("Open4: an entry was located, so now doing the devopen stuff\n");
#endif
		io->i_ino.i_dev = dp->dt_devp - &_devsw[0];
		io->i_dp = dp;
		_devopen(io);
		io->i_flgs |= how+1;
		io->i_offset = 0;
#ifdef DEBUG
	printf("\tdt_name = '%s', dt_devp = %x, dt_unit = %d, dt_boff = %d\n",
	  io->i_dp->dt_name, io->i_dp->dt_devp, io->i_dp->dt_unit, io->i_dp->dt_boff);
#endif
		return (fdesc+3);
	}

	c = '\0';
	cp2 = strchr(str, '\0');

#ifdef DEBUG
	printf("Open5: no entry was located, so checking the mount table\n");
#endif
	for (;;) {
		for (mp = &_mtab[0]; mp < &_mtab[NMOUNT]; mp++) {
			if (mp->mt_name == 0)
				continue;
			if (strcmp(str, mp->mt_name) == 0)
				break;
		}
		if (mp != &_mtab[NMOUNT]) {
			dp = mp->mt_dp;
			goto gotname;
		}
		cp1 = strrchr(str, '/');
		*cp2 = c;
		if (cp1 == NULL)
			break;
		c = '/';
		*(cp2 = cp1) = '\0';
	}
	io->i_flgs = 0;
	errno = ENOENT;
	return (-1);
gotname:
#ifdef DEBUG
	printf("Open6: found something in the mount table\n");
#endif
	io->i_ino.i_dev = dp->dt_devp - &_devsw[0];
	io->i_dp = dp;
	_devopen(io);

	if ((n = _find(++cp2, io)) == 0) {
		io->i_flgs = 0;
		return (-1);
	}

	if (how != 0) {
		io->i_flgs = 0;
		errno = EACCES;
		return (-1);
	}

	_openi(n, io);
	io->i_offset = 0;
	io->i_cc = 0;
	io->i_flgs |= F_FILE | (how+1);
	return (fdesc+3);
}


chkentry()	/* Routine checks dev. table entries, allocating as needed */
{
	register int i;
	int cpos, slice, unit;

	for (i = 0; i < VALDEV; i++) {	/* does the name match a device? */

#ifdef DEBUG
	printf("Chkentry1: str = '%s', devinfo[%d].devname = '%s'\n", str, i,
		devinfo[i].devname);
#endif
	   if (strncmp(str, devinfo[i].devname, devinfo[i].nlen) == 0) {
		cpos = devinfo[i].nlen;
#ifdef DEBUG
	printf("Chkentry2: cpos = %d\n", cpos);
#endif
		switch (devinfo[i].devtype) {

		case DSKTYPE:
#ifdef DEBUG
	printf("Chkentry3: device is DSKTYPE\n");
#endif
		   if (strlen(str) == cpos + 3) {
		       unit = str[cpos] - '0';
		       slice = str[cpos+2] - '0';
		   } else
		       unit = slice = -1;
#ifdef DEBUG
	printf("Chkentry4: unit = %d, slice = %d, s = '%c'\n", unit, slice,
		str[cpos+1]);
#endif
		   if ((unit >= 0 && unit <= 7) &&
		       (str[cpos+1] == 's') &&
		       (slice >= 0 && slice <= 7)) {
			 if (MKNOD(str, devinfo[i].majnum,
			      unit*8+slice, (daddr_t) 0) < 0) {
				_prs("open:\t"); perror(str);
				return(-1);
			 }
#ifdef DEBUG
	printf("Chkentry5: open of DSKTYPE device succeeded\n");
#endif
		   } else {
			_prs("open:\t"); _prs(str); _prs(":\tinvalid device\n");
			return(-1);
		   }
		   break;

		case RPTYPE:
#ifdef DEBUG
	printf("Chkentry3: device is RPTYPE\n");
#endif
		   unit = str[cpos];
		   if ((strlen(str) == cpos + 1) &&
		       (unit >= '0') && (unit <= '7')) {
			if (MKNOD(str, devinfo[i].majnum,
			    unit - '0', (daddr_t) 0) < 0) {
				_prs("open:\t"); perror(str);
				return(-1);
			}
#ifdef DEBUG
	printf("Chkentry4: open of RPTYPE device succeeded\n");
#endif
		   } else {
			_prs("open:\t"); _prs(str); _prs(":\tinvalid device\n");
			return(-1);
		   }
		   break;

		case MTTYPE:
#ifdef DEBUG
	printf("Chkentry3: device is MTTYPE\n");
#endif
		   if (strlen(str) == cpos) {
		      if (MKNOD(str, devinfo[i].majnum,
			   devinfo[i].unitnum, (daddr_t) 0) < 0) {
			     _prs("open:\t"); perror(str);
			     return(-1);
		      }
#ifdef DEBUG
	printf("Chkentry4: open of MTTYPE device succeeded\n");
#endif
		   } else {
			_prs("open:\t"); _prs(str); _prs(":\tinvalid device\n");
			return(-1);
		   }
		   break;	/* break out of switch */

		default:
#ifdef DEBUG
	printf("Chkentry3: illegal switch value %d\n", devinfo[i].devtype);
#endif
		   break;
		}
		break;		/* break out of for loop */
	   }		/* end of "if(strncmp(str ...)" */

	}		/* end of for loop */

/* if device entry was created, reset dp to point to it */

	if (i < VALDEV) {
#ifdef DEBUG
	printf("Chkentry6: device entry was created, finding the pointer to it\n");
#endif
	    for (dp = &_dtab[0]; dp < &_dtab[NDEV]; dp++)
		if (dp->dt_name == 0 || strcmp(str, dp->dt_name) == 0)
		    break;
	}
#ifdef DEBUG
	else printf("Chkentry6: no match of str with device table\n");
	printf("Chkentry7: final dp = %x, dp->dt_name = '%s'\n", (int)dp, dp->dt_name);
#endif
	return(0);	/* status OK if entry made or if no name match */

}
