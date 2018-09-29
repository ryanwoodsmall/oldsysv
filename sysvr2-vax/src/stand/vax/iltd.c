/*	@(#)iltd.c	1.8	*/
#include <stand.h>
#include <sys/filsys.h>

#define VAX11780	1
#define VAX11750	2

char	buf[5120];
char	nunix[14] = "unix";
char	ans[256];

char	*boot;
extern	gdboot(), gd_boot, gd_addr, gt_addr;

char	hp[] = "/dev/rp00";
char	hm[] = "/dev/rm0";
char	ts[] = "/dev/ts0";
char	gt[] = "/dev/mt0";

main ()
{
	register iocnt, count, rdev, wdev;
	register struct filsys *fs;
	register diskunit, tapeunit;
	int cpu;
	int err = 0;

	printf ("\nUNIX -- Initial Load: Tape-to-Disk\n");
	printf ("\nThe type of disk drive on which the Root file system will reside,\n");
	printf ("as well as the type of tape drive that will be used for Tape 1\n");
	printf ("must be specified below.\n");
	printf ("\nAnswer the questions with a 'y' or 'n' followed by\n");
	printf ("a carriage return or line feed.\n");
	printf ("There is no type-ahead -- wait for the question to complete.\n");
	printf ("The character '@' will kill the entire line\n");
	printf ("while the character '#' will erase the last character typed.\n\n");

	cpu = (mfpr(0x3e) >> 24) & 0xff; /* get cpu type from internal reg 3E */

	switch (cpu) {
	case VAX11780:
		if (yes ("RP06, RM05, RM80, RP07 on NEXUS 8"))
			gd_addr = 0x20000000 + 8 * 0x2000;
		else err++;
		break;
	case VAX11750:
		if (yes ("RP06, RM05, RM80 on MBA 0"))
			gd_addr = 0xf28000;
		else err++;
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
	default:
		printf ("Nebulous processor not supported\n");
		exit (1);
	}
	if (err) {
		printf ("This release only supports the above tape drives.\n");
		printf ("Restart and answer correctly.\n\n");
		exit (1);
	}
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
	while (!yes ("Ready"))
		printf ("So what's the matter?\n");
	fs = (struct filsys *) &buf[512];
	fs->s_fsize = 0;
	if ((iocnt = read (rdev, buf, sizeof buf)) != sizeof buf) {
		printf ("Hard tape error -- abort\n", iocnt);
		printf ("read = %d in block %d\n", iocnt, (int) (tell (rdev) >> 9));
		exit (1);
	}
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
		printf ("Size of filesystem to be copied is %d blocks.\n",count);
	} else {
		printf ("Bad format on tape -- abort\n");
		exit (1);
	}
	printf ("What is the pack volume label? (e.g. p0001): ");
	getline (ans);
	if (ans[0] == '\0')
		strcpy (ans, "p0001");
	ans[5] = '\0';
	printf ("The pack will be labeled %s.\n", ans);
	strcpy (fs->s_fpack, ans);
	count = (count + 9) / 10;
	for (;;) {
		if ((iocnt = write (wdev, buf, sizeof buf)) != sizeof buf) {
			printf ("Hard disk error -- abort\n", iocnt);
			printf ("write = %d in block %d\n", iocnt, (int) (tell (wdev) >> 9));
			exit (1);
		}
		if (--count <= 0)
			break;
		if ((iocnt = read (rdev, buf, sizeof buf)) != sizeof buf) {
			printf ("Hard tape error -- abort\n", iocnt);
			printf ("read = %d in block %d\n", iocnt, (int) (tell (rdev) >> 9));
			exit (1);
		}
	}
	printf ("The boot block for your type of disk drive will now be installed.\n");
	lseek (wdev, (off_t) 0, 0);
	if ((iocnt = write (wdev, boot, 512)) != 512)
		printf ("Warning: cannot write boot block (write %d).\n", iocnt);
	printf ("\nThe file system copy is now complete.\n");
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
}

mfpr (regno)
{
	asm (" mfpr 4(ap),r0");
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

exit (n) {

	if (n)
		_exit ();
	else
		asm ("	halt");
}
