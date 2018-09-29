/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:file.c	1.3"
/* crash/file.c */
/*
 *		Copyright 1984 AT&T
 */
#include "sys/param.h"
#include "a.out.h"
#include "sys/signal.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/var.h"

#include <crash.h>
#include <sys/file.h>
#include <sys/inode.h>

extern struct syment *File;
extern struct syment *Inode;
extern long lseek();

/* 
 * prfile() is the function crash uses to format the file table. The file
 * table is a collection of entries, one entry for each open/creat/pipe call
 * The main use is to hold the read/write pointer associated with each open
 * file.
 * prfile() expects to receive one argument, that is the individual entry to
 * to be printed. If this argument is null, the default, all file entries in 
 * the table will be printed.
 * prfile() will find the file table, read the entry and print the entry #, the
 * reference count, the inode address and the file flags which indicate the
 * state of the file.
 */

prfile(c, all)
{
	struct file fbuf;
	long 	file_off;
	long	prdes();

	if(c == -1)
		return;				/* processing complete */
	if(c >= v.v_file) {
		printf("%4d out of range\n", c);	/* requested file # too large */
		return;
	}
	file_off = ((long)(File->n_value + c * sizeof fbuf));
	lseek(mem, prdes(file_off,-1), 0);		/* locate & read the file entry */
	if(read(mem, &fbuf, sizeof fbuf) != sizeof fbuf) {
		printf("%4d read error on file table\n", c);
		return;
	}

	if(fbuf.f_count == 0)
		if(all)
			return;			/* entry processing complete */
		else
		{
			printf("%4d not in use\n",c) ;
			return ;
		}
		
	printf(fbuf.f_inode ? "%4d  %3d   %3d  %8x" : "%4d  %3d   -      -     ",
		 c, fbuf.f_count, ((unsigned)fbuf.f_inode - Inode->n_value)
		/ sizeof (struct inode)),fbuf.f_rcvd;
	printf("%s%s%s%s%s%s%s%s%s\n", fbuf.f_flag & FREAD ? " read" : "",
		fbuf.f_flag & FWRITE ? " write" : "",  /* print the file flag */
		fbuf.f_flag & FAPPEND ? " append" : "",
		fbuf.f_flag & FSYNC ? " sync" : "",
		fbuf.f_flag & FNET ? " net" : "",
		fbuf.f_flag & FCREAT ? " creat" : "",
		fbuf.f_flag & FTRUNC ? " trunc" : "",
		fbuf.f_flag & FEXCL ? " excl" : "",
		fbuf.f_flag & FNDELAY ? " ndelay" : "");
}
