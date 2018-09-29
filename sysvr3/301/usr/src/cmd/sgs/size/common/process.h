/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:common/process.h	1.9"
    /*  process.h contains format strings for printing size information
     *
     *  Different format strings are used for hex, octal and decimal
     *  output.  The appropriate string is chosen by the value of numbase:
     *  pr???[0] for hex, pr???[1] for octal and pr???[2] for decimal.
     */


/* FORMAT STRINGS */

static char	*prhead = 
    "\n\tSection         Size      Physical Address    Virtual Address\n\n";
/*	 12345678    123456789012    123456789012       123456789012	*/
/*		 1234            1234            1234567		*/

static char	*prsect[3] = {
		"\t%-8.8s     0x%.8lx      0x%.8lx         0x%.8lx\n",
		"\t%-8.8s    0%.11lo    0%.11lo       0%.11lo\n",
		"\t%-8.8s     %10ld      %10ld         %10ld\n"
		};

static char *prusect[3] = {
	"%lx",
	"%lo",
	"%ld"
	};

static char *prusum[3] = {
	" = 0x%lx\n",
	" = 0%lo\n",
	" = %ld\n"
	};

/*
 */
