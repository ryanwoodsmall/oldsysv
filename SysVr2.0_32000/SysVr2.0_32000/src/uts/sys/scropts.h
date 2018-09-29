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
/*	scropts.h 6.1 of 8/22/83
	@(#)scropts.h	6.1		*/

/* 
This file contains the bisync script compile time options
*/

#define		DEBUG	0	/*
				   if the script is to generate execution
				   trace messages, then this value must be
				   non zero.
				   if execution trace messages are to be
				   suppressed, then DEBUG shoud be 0.
				*/

#define	FAVOR_OUTPUT	0	/*
				   if the script is to favor all output over
				   input from any terminal, then this value
				   should be non zero.
				   if the script is to alternate between
				   output and input, then this value should
				   be 0.
				*/

#define BOTH		0	/*
				   If the script is to dynamically determine
				   whether a controller is ascii or ebcdic,
				   this value must be non-zero. (the defines 
				   for ASCII and EBCDIC
				   below must also be non-zero)  Otherwise this
				   value must be zero.
                                */

#define ASCII		1	/*
				   If the script is to be used to
				   dynamically determine the type of
				   controller, or is to be used with
				   ascii controllers, then this value
				   must be non-zero.  Otherwise it
				   must be zero.
                                */

#define EBCDIC		0	/*
				   If the script is to be used to dynamically
				   determine the type of controller, or is
				   to be used with ebcdic controllers, then
				   this value must be non-zero.  Otherwise
				   it must be zero.
                                */




/*

		Retry Values

*/

#define	PTRY	3		/* number of times to try to poll */
#define	STRY	3		/* number of times to try to select */
#define	XTRY	3		/* number of times to try to transmit */
#define	WTRY	5		/* number of times to try after WAK */
#define	RTRY	1		/* number of times to try after RVI */
