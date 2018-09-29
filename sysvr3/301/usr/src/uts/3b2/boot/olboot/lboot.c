/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/lboot.c	10.8"

#include	"sys/types.h"
#include	"sys/psw.h"
#include	"sys/inode.h"
#include	"sys/elog.h"
#include	"sys/iobuf.h"
#include	"sys/boot.h"
#include	"sys/firmware.h"
#include	"sys/id.h"
#include	"a.out.h"
#include 	"sys/ino.h"
#include	"sys/param.h"
#include	"sys/lboot.h"
#include	"sys/sbd.h"
#include 	"sys/csr.h"
#include 	"sys/immu.h"
#include	"sys/nvram.h"
#include	"sys/vtoc.h"

extern lls();
extern ndisk_acs();

int bootstartaddr;

#define LBOOT "/lib/lboot"
#define HMAJOR 1;
#define FMAJOR 2;
#define	restart() { RST=1; while (1) ; }

char	IOBASE[SBUFSIZE];	/* base io buffer */
char	IND3[SBUFSIZE];		/* 3rd level indirect block */
char	DATA[SBUFSIZE];		/* a data block */
char	AHDR[SBUFSIZE];		/* a.out header block for UNIX */
char	IND2[SBUFSIZE];		/* 2nd level indirect block */
char	IND1[SBUFSIZE];		/* 1st level indirect block */
char	INODE[SBUFSIZE];	/* inode block */
char	DIR[SBUFSIZE];		/* directory block */


struct inode	Dinode;		/* Inode of file system root directory */
struct inode	Fndinode;	/* Inode of file found by findfile() */
struct inode	Linode;		/* Inode of last directory found */
char		bfname[BOOTNAME];	/* Boot file name */
int		Fso;		/* File system offset */
int		fstype;		/* File system type flag */
char		MagicMode = 0;	/* see lboot source */
#define MYVTOC ((struct vtoc *) (BOOTADDR + BSIZE))
#define MYPDINFO ((struct pdinfo *)(BOOTADDR + 2 * BSIZE))

main()
{
	register char	*p, *p1;
	struct  inode	binode;		/* inode of boot program */
	struct	aouthdr	*ahp;		/* ptr to UNIX header */
	int	(*uboot)();		/* pointer to boot program entry */
	char	response[2];		/* user response */
	char	major;
	int	fndresp;		/* Return code from findfile() */
	dev_t	dev;
 	int i;
	unsigned long logstrt;		/* logical start # */

		/*
		 * No printf's for anything other than DEMANDBOOT
		 */

	IO = ON;

	/* define file system offset for disk type */

	switch (P_CMDQ->b_dev)
		{
		case FLOPDISK:
			/* assign value for integral floppy disk */
			Fso = FFSO;
			break;

		case HARDDISK0:
		case HARDDISK1:
			/* save logical start # for integral disk */
			logstrt = PHYS_INFO[P_CMDQ->b_dev - HARDDISK0].logicalst;
			break;

		default:
			/* save logical start # for peripheral device */
			logstrt = MYPDINFO->logicalst;
			break;
		}

	if (P_CMDQ->b_dev != FLOPDISK)
		{
		/* Read LBOOT's copy of VTOC for integral disk */
		for (i = 0; i < V_NUMPAR; i++)
			{
			if (MYVTOC->v_part[i].p_tag == V_ROOT)
				{
				/* ROOT partition located */
				Fso = MYVTOC->v_part[i].p_start + logstrt;
				break;
				}
			}

		/* ROOT not found */
		if (i >= V_NUMPAR)
			return(FAIL);
		}
	major = FMAJOR;
	WNVRAM(&major, (char *) &(UNX_NVR->rootdev), 1);


	readsb();	/* read in SUPERBLOCK */

	/* Set up access to root file system. */

	if (!findfs()) {
		PRINTF("Cannot access root file system !\n");
		RUNFLG = FATAL;		/* FATAL error */
		RST = 1;		/* Request firmware reset */
		return;			/* Return to firmware */
	}

	/* Initialize boot file name to that passed by firmware */
	for (p = bfname, p1 = P_CMDQ->b_name; *p++ = *p1++;)
	;
	/* if we're booting from the hard disk, always try lboot first */
	if (P_CMDQ->b_dev != FLOPDISK) {
		/* we will get dgmon and bfname directly, all others through
		   lboot */
		if ((0!=STRCMP(bfname,"dgmon"))&&(0!=STRCMP(bfname,"filledt")))
			for (p = bfname, p1 = LBOOT; *p++ = *p1++;)
			;
	}

	/* Loop until a bootable program is found */
	while (1) {
		if (0 == STRCMP(bfname, "magic mode")) {
			MagicMode = 1;
			PRINTF("POOOF!\n");
		}

		fndresp = findfile(bfname);

		/* if we can't find the requested file */
		if (fndresp == NOTFOUND) {
			/* hard disk */
			if (P_CMDQ->b_dev != FLOPDISK) {
				/* we can't find lboot */
				if (0 == STRCMP(bfname, LBOOT)) {
					/* if this is an autoboot, go for unix */
					/* try to boot the firmware requested file */
					for (p=bfname, p1=P_CMDQ->b_name; *p++ = *p1++;)
					;
					/* if we didn't get a name from firmware */
					if ((bfname[0]!='/')
					&&  ((bfname[0]<'A')
					||  ((bfname[0]>'Z')&&(bfname[0]<'a'))
					||  (bfname[0]>'z'))) {
						for (p=bfname, p1="unix"; *p++ = *p1++;)
						;
						continue;
					}
					/* firmware didn't pass us lboot */
					if (0 != STRCMP(bfname, LBOOT))
						continue;
					/* firmware passed lboot, try something else */
					/* If path is a directory, list the contents */
					if (Linode.i_mode & 0111) {
						PRINTF("%s is not a bootable file !",bfname);
						PRINTF("\nLast valid directory in path contains:\n");
						lls(&Linode);
					}
					if (prompt(bfname)) {
						RUNFLG = REENTRY;
						return;
					}
					continue;
				}
				/* not lboot */
				/* fail on autoboot */
				if (P_CMDQ->b_type == AUTOBOOT) {
					RUNFLG = FATAL;		/* Fatal error */
					RST = 1;		/* Req. FW reset */
					return;
				}
				/* If path is a directory, list the contents */
				if (Linode.i_mode & 0111) {
					PRINTF("%s is not a bootable file !",bfname);
					PRINTF("\nLast valid directory in path contains:\n");
					lls(&Linode);
				}
				if (prompt(bfname)) {
					RUNFLG = REENTRY;
					return;
				}
				continue;
			}
			/* floppy disk */
			switch (P_CMDQ->b_type) {
			case DEMANDBOOT:
				/* If path is a directory, list the contents */
				if (Linode.i_mode & 0111) {
					PRINTF("%s is not a bootable file !",bfname);
					PRINTF("\nLast valid directory in path contains:\n");
					lls(&Linode);
				}
				if (prompt(bfname)) {
					RUNFLG = REENTRY;
					return;
				}
				continue;
			case AUTOBOOT:
				RUNFLG = FATAL;		/* Fatal error */
				RST = 1;		/* Req. FW reset */
				return;
			default:
				RUNFLG = FATAL;		/* Fatal error */
				RST = 1;		/* Req. FW reset */
				return;
			}
		} else if (fndresp == DIRFOUND) {	     /* Request was directory */
			if (P_CMDQ->b_type == AUTOBOOT) {
				RUNFLG = FATAL;		/* Fatal error */
				RST = 1;		/* Req. FW reset */
				return;
			}
			if (bfname[0] == '\0') {
				PRINTF("The first level boot directory contains:\n");
			} else {
				PRINTF("%s is a directory !\nIt contains:\n", bfname);
			}
			lls(&Linode);
			if (prompt(bfname))
				return;
			continue;
		} else {				/* Boot file found */
			binode = Fndinode;
			/*
			 * Boot the file as specified by the inode: binode
			 */
			if (loadprog(binode))
				break;
			PRINTF("Cannot load %s.\n", bfname);
			if (prompt(bfname))
				return;
			continue;
		}
	}

	/* get entry point address */

	ahp = (struct aouthdr *)(AHDR + sizeof(struct filehdr));
	uboot = (int(*)()) ahp->entry;		/* load entry address */

	/* save entry point address in NVRAM */

	WNVRAM((char *) &(ahp->entry), (char *) &(UNX_NVR->spmem), 4);

	if (P_CMDQ->b_dev == FLOPDISK)
		DISK_ACS(Fso+1, IOBASE, DISKRD, LAST);	/* dummy read to unlock floppy */


	/* if (P_CMDQ->b_type == DEMANDBOOT) {
		PRINTF("Do you want %s to be executed (y or n): ", bfname);
		GETS(response);
		if (!STRCMP(response, "n"))
			return;
	} */

	if (MagicMode) {
		PRINTF("go %x to execute %s\n", bootstartaddr, bfname);
		RUNFLG=VECTOR;
		restart();
	}

	/* execute the bootable program */

	uboot = (int (*)())bootstartaddr;
	(*uboot)();		/* call the program */

	IO = ON;			/* Turn printf's back on */
}

/*
 * prompt()
 *
 * Prompt for another boot file. Returns 0 if one was
 * provided, -1 otherwise.
 */
static int
prompt(name)
register char	*name;
{
	do {
		PRINTF("Enter name of boot file or 'q' to quit: ");
		GETS(name);
	} while (name[0] == '\0');
	return (name[0] == 'q' && name[1] == '\0' ? -1 : 0);
}
