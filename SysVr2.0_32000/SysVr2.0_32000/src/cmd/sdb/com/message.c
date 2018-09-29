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
	/*	@(#) message.c: 1.3 10/4/82	*/

/*
 *	UNIX debugger
 *		Can put this file in text space !
 */



#include	"mode.h"
#include	"machdep.h"	/* for NSIG in signal.h */


#ifdef ns32000
MSG		ADWRAP	=  "Address wrap around";
#endif
MSG		BADMOD	=  "Bad modifier";
MSG		NOBKPT	=  "No breakpoint set";
MSG		NOPCS	=  "No process";
MSG		BADTXT	=  "Text address not found";
MSG		BADDAT	=  "Data address not found";
MSG		EXBKPT	=  "Too many breakpoints";
MSG		ENDPCS	=  "Process terminated";
MSG		SZBKPT	=  "Bkpt: Command too long";
MSG		BADWAIT	=  "Wait error: Process disappeared!";
MSG		NOFORK	=  "Try again";
MSG		BADMAGN =  "Bad core magic number";

STRING		signals[NSIG] = {
		"",
		"Hangup (1)",
		"Interrupt (2)",
		"Quit (3)",
		"Illegal Instruction (4)",
		"Trace/BPT (5)",
		"IOT (6)",
		"EMT (7)",
		"Floating Exception (8)",
		"Killed (9)",
		"Bus Error (10)",
		"Memory Fault (11)",
		"Bad System Call (12)",
		"Broken Pipe (13)",
		"Alarm Call (14)",
		"Terminated (15)",
		"User Signal 1 (16)",
		"User Signal 2 (17)",
		"Death of a Child (18)",
		"Power-Fail Restart (19)",
};
