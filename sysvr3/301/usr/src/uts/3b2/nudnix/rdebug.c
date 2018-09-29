/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rdebug.c	10.16"

#include "sys/inline.h"
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/comm.h"
#include "sys/rdebug.h"

/*
 *	System call to turn on/off RFS debug printfs.  (See rdebug.h.)
 */

long	dudebug = 0;

rdebug ()
{
	register struct a {
		int	level;
	}  *uap = (struct a *) u.u_ap;

	if(!suser())
		return;

	
	if(uap->level !=  -2)
		if(uap->level == 0)
			dudebug = 0;
		else 
			dudebug |= uap->level;

	u.u_rval1 = dudebug;
}

struct sysnm {
	char	s_name[16];
	int	s_sysno;
};

struct sysnm sysnm[] =
{
"ACCESS",	33,
"SYSACCT",	51,
"CHDIR",	12,
"CHMOD",	15,
"CHOWN",	16,
"CHROOT",	61,
"CLOSE",	6,
"CREAT",	8,
"EXEC",		11,
"EXECE",	59,
"FCNTL",	62,
"FSTAT",	28,
"FSTATFS",	38,
"IOCTL",	54,
"LINK",		9,
"MKNOD",	14,
"MOUNT",	21,
"OPEN",		5,
"READ",		3,
"SEEK",		19,
"STAT",		18,
"STATFS",	35,
"UMOUNT",	22,
"UNLINK",	10,
"UTIME",	30,
"UTSSYS",	57,
"WRITE",	4,
"GETDENTS",	81,
"MKDIR",	80,
"RMDIR",	79,
"ADVERTISE",	70,
"UNADVERTISE",	71,
"RMOUNT",	72,	/* nami half of rmount */
"RUMOUNT",	73,
"SRMOUNT",	97,	/* msg from ns to server */
"SRUMOUNT",	98,
"COPYIN",	106,
"COPYOUT",	107,
"COREDUMP",	111,
"WRITEI",	112,
"READI",	113,
"LBMOUNT",	115,	/*  lbin mount (second namei in smount)	*/
"LBUMOUNT",	116,	/*  lbin unmount			*/
"RSIGNAL",	119,	/* remote signal */
"GDPACK",	120,
"GDPNACK",	121,
"SYNCTIME",	122,
"DOTDOT",	124,
"LBIN",		125,
"FUMOUNT",	126,
"IPUT",		131,
"IUPDAT",	132,
"UPDATE",	133,
"unknown",	0,
};

char *
sysname (number)
 int number;
{
	static	char buffer[16];
	int i;

	for (i=0; sysnm[i].s_sysno != 0 && sysnm[i].s_sysno != number; i++)
			;

	strcpy (buffer, sysnm[i].s_name);
	return (buffer);
}
