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
/*	@(#)defs.h	1.4	*/
/*
 *  Common definitions
 */

#include	<sys/param.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	"a.out.h"
#include	<setjmp.h>
#include	<sys/text.h>
#include	<fcntl.h>
#include	<sys/dir.h>
#include	<sys/file.h>
#include	<sys/proc.h>
#include	<sys/tty.h>
#include	<sys/termio.h>
#include	<sys/inode.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/reg.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<ctype.h>
#include	"mode.h"
#include	"functs.h"
#include	"extern.h"
#include	"sys.h"
#include	"lib.h"
#include	"storclass.h"

#define VARNO 36
#define VARB  11
#define VARD  13
#define VARE  14
#define VARF 15
#define VARM  22
#define VARS  28
#define VART  29

#define RD 0
#define WT 1

#define NSP      0
#define ISP      1
#define DSP      2
#define NSYM     0
#define ISYM     4
#define DSYM     8
#define IDSYM    12
#define ENDSYMS  (-1)
#define EXTRASYM (-2)

#define BPT      0xf2
#define BKPTSET  1
#define BKPTEXEC 2

/*
 *  Ptrace mode defines
 */
#define SETTRC 0
#define RIUSER 1
#define RDUSER 2
#define RUREGS 3
#define WIUSER 4
#define WDUSER 5
#define WUREGS 6
#define CONTIN 7
#define EXIT   8
#define SINGLE 9

#define MAXREGS   12
#define MAXFREGS   9
#define ADBREG(A) (u.u_ar0[A])
#define SYSREG(A) ((int) (((char *) (&ADBREG(A))) - ((char *) &u)))

#define EOR    '\n'
#define SPACE  ' '
#define TB     '\t'

#define DBNAME  "adb\n"

#define LPRMODE "%R"
#define OFFMODE "+%R"

/* Symbol table in a.out file */
#define SYMBOLSIZE  (20)

#define DEFRADIX  16
#define MAXARG	  32
#define LINSIZ	1024
#define STAR       4
#define MAXOFF   255
#define MAXPOS    80
#define MAXLIN   128
#define QUOTE   0200
#define STRIP   0177
#define LOBYTE  0377
#define EVEN      -2

#define TRUE   (-1)
#define FALSE  0

#define shorten(a) ((short)(a))

#define SCCSID(A) static char Sccsid[] = "A"

#define TRPRINT(S) if (trace) (void) fprintf(stderr, S)
#define TR1PRINT(S, V) if (trace) (void) fprintf(stderr, S, V)

#define SECT1 1
#define SECT2 2
#define SECT3 3
