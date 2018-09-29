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
/*	@(#)iltd.c	1.4	*/
#include <stand.h>
#include <sys/filsys.h>

#define VAX11780	1
#define VAX11750	2
#define NS32000		3

#ifdef ns32000
char	*buf = (char *)(128 * 1024);
#define	BUFSIZE		51200
#else
char	buf[5120];
#define	BUFSIZE		(sizeof buf)
#endif
char	nunix[14] = "unix";
char	ans[256];

char	*boot;
#ifdef ns32000
int	gd_addr, gt_addr;
#else
extern	gdboot(), gd_boot, gd_addr, gt_addr;
#endif

extern	errno;

char	hp[] = "/dev/rp00";
char	hm[] = "/dev/rm0";
char	ts[] = "/dev/ts0";
char	gt[] = "/dev/mt0";
char	mt[] = "/dev/mt/0mn";
char	dsk[] = "/dev/dsk/1s0";

main ()
{
	register iocnt, count, pcount, rdev, wdev;
	register struct filsys *fs;
	register diskunit, tapeunit;
	int cpu;
	int err = 0;
	int bufsize = BUFSIZE;
#ifdef ns32000
	char fsname[6];
	strcpy (fsname, "root");
#endif

	printf ("\nUNIX -- Initial Load: Tape-to-Disk\n");
#ifdef ns32000
	printf ("\nThe drive and slice number of the disk on which the Root file\n");
	printf ("system will reside must be specified below. You will be asked\n");
	printf ("if you want to load the Usr file system when the Root file\n");
	printf ("system copy is complete.  If you do you must specify the\n");
	printf ("drive and slice number of the disk where the Usr file system\n");
	printf ("will reside.\n");
	printf ("\nAnswer the questions with a 'y', 'n' or a number followed by\n");
#else
	printf ("\nThe type of disk drive on which the Root file system will reside,\n");
	printf ("as well as the type of tape drive that will be used for Tape 1\n");
	printf ("must be specified below.\n");
	printf ("\nAnswer the questions with a 'y' or 'n' followed by\n");
#endif
	printf ("a carriage return or line feed.\n");
	printf ("There is no type-ahead -- wait for the question to complete.\n");
	printf ("The character '@' will kill the entire line\n");
	printf ("while the character '#' will erase the last character typed.\n\n");

#ifdef ns32000
	cpu = NS32000;
#else
	cpu = (mfpr(0x3e) >> 24) & 0xff; /* get cpu type from internal reg 3E */
#endif

	switch (cpu) {
	case VAX11780:
		if (yes ("RP06, RM05, RM80, RP07 on MBA 8"))
			gd_addr = 0x20000000 + 8 * 0x2000;
		else err++;
		break;
	case VAX11750:
		if (yes ("RP06, RM05, RM80 on MBA 0"))
			gd_addr = 0xf28000;
		else err++;
		break;
	case NS32000:
		break;
	default:
		printf ("Nebulous processor not supported\n");
		exit (1);
	}
	if (err) {
		printf ("This release only supports the above disk drives.\n");
		printf ("Restart and answer correctly.\n\n");
		exit (1);
	}
#ifdef ns32000
getdisk:
	printf ("Enter disk drive number for %s (0-7): ", fsname);
	for (;;) {
		getline (ans);
		if (ans[0] >= '0' && ans[0] <= '7')
			break;
		printf ("Drive number out of range\nEnter correct drive number : ");
	}
	dsk[9] = ans[0];
	printf ("Enter disk slice number for %s (0-7): ", fsname);
	for (;;) {
		getline (ans);
		if (ans[0] >= '0' && ans[0] <= '7')
			break;
		printf ("Slice number out of range\nEnter correct slice number :");
	}
	dsk[11] = ans[0];
	diskunit = (atoi (&dsk[9])) * 8 + (atoi (&dsk[11]));
	if ((wdev = open (dsk, 1)) < 0) {
		perror (dsk);
		exit (1);
	}
	printf ("Disk selected: %s\n", dsk);
	if (strcmp(fsname, "usr") == 0)
		goto copyfs;
#else
	if (gd_boot == 0)
		hp[8] = '\0';
	else
		hp[7] = gd_boot + '0';
	if ((wdev = open (hp, 1)) < 0) {
		perror (hp);
		exit (1);
	}
	boot = (char *) gdboot;
	strcat (nunix, "gd");
	diskunit = number ("Drive number (0-7)");
	if (diskunit > 7) {
		printf ("Out of range; 0 assumed.\n");
		diskunit = 0;
	}
	_iobuf[wdev-3].i_dp->dt_unit = diskunit;
	printf ("Disk drive %d selected.\n", diskunit);
	printf ("\nMount a formatted pack on drive %d.\n", diskunit);
#endif
	while (!yes ("Ready"))
		printf ("So what's the matter?\n");
	putchar ('\n');

	switch (cpu) {
	case VAX11780:
		if (yes ("TU16/77, TU78 at NEXUS 9")) {
			gt_addr = 0x20012000;
			if ((rdev = open (gt, 0)) < 0) {
				perror (gt);
				exit (1);
			}
			strcat (nunix, "gt");
		}
		else
			err++;
		break;
	case VAX11750:
		if (yes ("TS11 at address 172520")) {
			if ((rdev = open (ts, 0)) < 0) {
				perror (ts);
				exit (1);
			}
			strcat (nunix, "ts");
			break;
		}
		if (yes ("TU16/77 on MBA 1")) {
			gt_addr = 0xf2a000;
			if ((rdev = open (gt, 0)) < 0) {
				perror (gt);
				exit (1);
			}
			strcat (nunix, "gt");
			break;
		}
		err++;
		break;
	case NS32000:
		if ((rdev = open (mt, 0)) < 0) {
			perror (mt);
			exit (1);
		}
		strcat (nunix, "sys32");
		break;
	default:
		printf ("Nebulous processor not supported\n");
		exit (1);
	}
	if (err) {
		printf ("This release only supports the above tape drives.\n");
		printf ("Restart and answer correctly.\n\n");
		exit (1);
	}
#ifdef ns32000
	printf ("Tape = %s\n", mt);
	for (tapeunit=4; tapeunit>1; --tapeunit){
		close (rdev);
		rdev = open(mt, 0);
	}
copyfs:
	printf ("\nThe tape will be read from the current position\n");
	printf ("and written onto %s starting at block 1.\n\n", dsk);
#else
	tapeunit = number ("Drive number (0-7)");
	if (tapeunit > 7) {
		printf ("Out of range; 0 assumed.\n");
		tapeunit = 0;
	}
	_iobuf[rdev-3].i_dp->dt_unit = tapeunit;
	printf ("Tape drive %d selected.\n", tapeunit);
	printf ("\nThe tape on drive %d will be read from the current position\n", tapeunit);
	printf ("at 1600bpi, 5120 characters (10 blocks) per record,\n");
	printf ("and written onto the pack on drive %d starting at block 0.\n\n", diskunit);
#endif
	while (!yes ("Ready"))
		printf ("So what's the matter?\n");
	fs = (struct filsys *) &buf[512];
	fs->s_fsize = 0;
	if ((iocnt = read (rdev, buf, BUFSIZE)) != BUFSIZE) {
		printf ("Hard tape error -- abort\n", iocnt);
		printf ("read = %d in block %d\n", iocnt, (int) (tell (rdev) >> 9));
		exit (1);
	}
#ifdef ns32000
	if (!(strncmp (buf, "Volcopy", 7))) {
		bcopy (&buf[1024], &buf[0], BUFSIZE-1024);
		if ((iocnt = read (rdev, &buf[BUFSIZE-1024], 1024)) != 1024) {
			printf ("Hard tape error -- abort\n", iocnt);
			printf ("read = %d in block %d\n", iocnt, (int) (tell (rdev) >> 9));
			exit (1);
		}
	} 
#endif
	if (fs->s_fsize) {
		if (fs->s_magic != FsMAGIC) /* test for 512b or 1k fs */
			fs->s_type = 1;
		switch (fs->s_type) {
		case Fs1b:
			count = fs->s_fsize;
			break;
		case Fs2b:
			count = fs->s_fsize * 2;
			break;
		default:
			printf ("File System type unknown--get help\n");
			exit (1);
		}
		printf ("Filesystem to be copied is '%s'\n", fs->s_fname);
		printf ("Size of filesystem is %d blocks.\n",count);
		strcpy (fsname, fs->s_fname);
	} else {
		printf ("Bad format on tape -- abort\n");
		exit (1);
	}
	printf ("What is the pack volume label? (e.g. p0001): ");
	getline (ans);
	if (ans[0] == '\0')
		strcpy (ans, "p0001");
#ifndef ns32000
	ans[5] = '\0';
#endif
	printf ("The pack will be labeled %s.\n", ans);
	strcpy (fs->s_fpack, ans);
	pcount = (count % (BUFSIZE/512)) * 512;
	count = (count + (BUFSIZE/512) - 1) / (BUFSIZE/512);
	lseek (wdev, (off_t)512, 0);
	if ((iocnt = write (wdev, &buf[512], bufsize-512)) != bufsize-512) {
		printf ("Hard disk error -- abort\n", iocnt);
		printf ("write = %d in block %d\n", iocnt, (int) (tell (wdev) >> 9));
		exit (1);
	}
	for (;;) {
		if (--count <= 0)
			break;
		if (count == 1) { 
			if (pcount)
				bufsize = pcount;
		}
		if ((iocnt = read (rdev, buf, bufsize)) != bufsize) {
			printf ("Hard tape error -- abort\n", iocnt);
			printf ("read = %d in block %d\n", iocnt, (int) (tell (rdev) >> 9));
			exit (1);
		}
		if ((iocnt = write (wdev, buf, bufsize)) != bufsize) {
			printf ("Hard disk error -- abort\n", iocnt);
			printf ("write = %d in block %d\n", iocnt, (int) (tell (wdev) >> 9));
			printf ("bufsize = %d, count = %d\n", bufsize, count);
			exit (1);
		}
	} 
#ifndef ns32000
	printf ("The boot block for your type of disk drive will now be installed.\n");
	lseek (wdev, (off_t) 0, 0);
	if ((iocnt = write (wdev, boot, 512)) != 512)
		printf ("Warning: cannot write boot block (write %d).\n", iocnt);
#endif
	printf ("\nThe file system copy is now complete.\n");
#ifdef ns32000
	if (strcmp(fsname, "root") == 0) {
		if (yes ("Do you want to load the usr file system now")) {
			strcpy (fsname, "usr");
			bufsize = BUFSIZE;
			goto getdisk;
		}
	}
	printf ("You are now in the SYS32 Firmware Monitor.  From here there are several ways\n");
	printf ("you can boot up UNIX.  One way is to enter 'k' followed by a carriage return\n");
	printf ("or line feed.  This will boot up the stand-alone shell.  When you get the\n");
	printf ("$$ prompt enter '%s' followed by a carriage return or line feed.\n", nunix);
	printf ("	Example:  $$ %s\n", nunix);
	printf ("Another way is to enter 'b' followed by a carriage return or line feed.  This\n");
	printf ("will run the monitor's boot program.  When you get the Boot: prompt enter \n");
	printf ("'dc(%c,%c)vmunix' and proceed as above when the $$ prompt appears.\n", '1', '0');
	printf ("	Example:  Boot: dc(%c,%c)vmunix\n", '1', '0');
	printf ("You can also reset the machine and after it runs its diagnostics it will\n");
	printf ("automatically boot the stand-alone shell.  From the stand-alone shell boot\n");
	printf ("'%s' as in the first example.\n", nunix);
	printf ("The system will initially come up single-user; see init(1M).\n");
	printf ("If you have an upper case only console terminal,\n");
	printf ("you must execute: stty lcase; see stty(1).\n");
	printf ("After UNIX is up, link the file %s to unix using ln(1).\n", nunix);
	printf ("        # ln /%s /unix\n", nunix);
	printf ("Set the date(1).\n");
	printf ("Good Luck!\n");
	close (rdev);
	mt[10] = '\0';
	if ((rdev = open (mt, 0)) < 0) {
		perror (mt);
		exit (1);
	}
	close (rdev);
	exit (0);
#else
	printf ("\nTo boot the basic UNIX for your disk and tape drives\n");
	printf ("as indicated above, mount this pack on drive 0\n");
	printf ("and read in the boot block (block 0) using whatever\n");
	if (cpu == VAX11780)
		printf ("means you have available; see 11/780 ops(8).\n");
	else
		printf ("means you have available; see 11/750 ops(8).\n");
	printf ("\nThen boot the program %s using the stand-alone shell.\n", nunix);
	printf ("Normally:  $$ %s\n\n", nunix);
	printf ("The system will initially come up single-user; see init(1M).\n");
	printf ("If you have an upper case only console terminal,\n");
	printf ("you must execute: stty lcase; see stty(1).\n");
	printf ("\nAfter UNIX is up, link the file %s to unix using ln(1).\n", nunix);
	printf ("        # ln /%s /unix\n", nunix);
	printf ("\nSet the date(1).\n");
	printf ("\nGood Luck!\n\n");
	printf ("The tape will now be rewound.\n\n");
	close (rdev);
	exit (0);
#endif
}

mfpr (regno)
{
#ifndef ns32000
	asm (" mfpr 4(ap),r0");
#endif
}

yes (qs)
char *qs; {

	printf (qs);
	printf ("?: ");
	getline (ans);
	if (ans[0] == 'y')
		return (1);
	if (ans[0] == 'n' || ans[0] == '\0')
		return (0);
	printf ("assumed 'n'\n");
	return (0);
}

number (qs)
char *qs; {
	register c, num;

	num = 0;
	printf (qs);
	printf ("?: ");
	getline (ans);
	c = ans[0];
	if ('0' <= c && c <= '9')
		num = num*10 + c - '0';
	return (num);
}

getline (s)
register char *s; {
	register char c;

	while ((c = getchar ()) != '\n')
		*s++ = c;
	*s = '\0';
}

#ifndef ns32000
exit (n) {

	if (n)
		_exit ();
	else
		asm ("	halt");
}
#endif
#ifdef ns32000
/* block copy from from to to, count bytes */
bcopy (from, to, count)
	register char *from, *to;
	register int count;
{
	while ((count--) > 0)
		*to++ = *from++;
}
#endif
