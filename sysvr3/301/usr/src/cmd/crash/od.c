/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:od.c	1.2"
/* crash/od.c */
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

#include <stdio.h>
#include <crash.h>

extern	struct	prmode prm[];
extern long lseek();
extern long prdes();

/*
 * prod() dumps the data values beginning with a symbol or address. The expected
 * arguments are: the address or symbol at which to begin the dump, the number
 * of data values to dump and the format in which to print the data values. 
 * The  fourth argument indicates whether or not the address is virtual or 
 * physical. (Phys = 1 if the address is physical )
 * There is no default value for the address/symbol argument, however the number
 * of data values to dump will default to one and the format for printing will
 * default to hexadecimal. Upon entering prod(), the type format is checked.
 * An unrecognizable option results in an error message and return to main().
 * If the type is octal, decimal, or hex, word alignment is performed 
 * immediately. After alignment the address/symbol is found and a loop is
 * established to read and print the requested number of data values in the
 * format requested. If the print format requested was character, byte, or
 * string, a loop is setup to process the number of data values requested.
 * Differentiation is then made between print format and character or byte
 * format. Depending on the format, the data values are then read and printed.
 * The functions and system calls made from within prod() are: strcmp(), 
 * lseek(), printf(), read(), putch(), and putchar().
 */

prod(slot,addr, units, style, phys)
int slot;
long addr;
int units;
char *style;
int phys;
{
	register int i;
	register struct prmode *pp;
	int word;
	long 	pa_addr;
	char ch;

	if(units == -1)		/* processing completed */
		return;
	for(pp = prm; pp->pr_sw != 0; pp++)
		if(strcmp(pp->pr_name, style) == 0)	/* determine the print format */
			break;
	if (phys == 1)
		pa_addr = addr & ~MAINSTORE;
	else if((pa_addr = prdes(addr, slot)) == -1) {
		printf("invalid segment descriptor OR out of bias bounds\n");
		return;
	}
	if(lseek(mem, pa_addr, 0) == -1)
	{	/* locate the requested address */
		char	err_buff[80] ;
		sprintf(err_buff,"offset %x out of range\n",pa_addr) ;
		error(err_buff);
		return;
	}

	switch(pp->pr_sw) {		/* determine treatment of data by print format type */
	default:
	case NULL:
		printf("invalid print format\n");
		break;

	case OCTAL:
	case DECIMAL:
	case HEX:
		if(addr & 0x3) {	/* perform word alignment */
			printf("warning: word alignment performed\n");
			addr &= ~0x3;
			if((pa_addr = prdes(addr, slot)) == -1) {
				printf("invalid segment desriptor OR out of bias bounds\n");
				return;
			}
			lseek(mem, pa_addr, 0);	/* locate the data values to be printed */
		}
		for(i = 0; i < units; i++) {	/* until all requested data values are printed */
			if(i % 4 == 0) {	/* print in hex format */
				if(i != 0)
					printf("\n");
				printf("%8.8x:", addr + i * sizeof(int));
			}
			/* read the data value */
			if(read(mem, (char *)&word, sizeof(int)) != sizeof(int)) {
				printf("  read error");
				break;
			}
			printf(pp->pr_sw == OCTAL? " %11.11o" :
			       pp->pr_sw == DECIMAL? " %10u" : " %8.8x", word);
		}
		break;

	case CHAR:
	case BYTE:
	case STRING:
		for(i = 0; i < units; i++) {
			if(pp->pr_sw != STRING &&
			  i % (pp->pr_sw == CHAR ? 16 : 8) == 0) {
				if(i != 0)
					printf("\n");
				printf("%8.8x: ", addr + i * sizeof (char));
			}
			if(read(mem, &ch, sizeof ch) != sizeof ch) {
				printf("  read error");
				break;
			}
			if(pp->pr_sw == CHAR)
				putchar(ch);      /* putch()? */
			else if(pp->pr_sw == BYTE)
				printf(" %02x", ch & 0377);
			else
				putchar(ch);
		}
		break;
	}
	printf("\n");
}
