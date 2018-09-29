/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setup:mset.h	1.8"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#define MAXCHAR 128
#define NCHAR 20
#define TRUE 1
#define FALSE 0
#define SUCCESS 3
#define FAILURE 4
#define EXIT 0
#define CONTINUE 1
#define WRONG_ANS '~' /* ~ is always wrong answer in the setup part of assist*/
#define NO_MSCRIPT 3 /* mscript returns 3 if error */

/*  characters used in intro to assist teaching of control characters */
#define CONTROL_A  1   /*  ^A */
#define CONTROL_D  4   /*  ^D */
#define CONTROL_J 10   /*  ^J - newline */
#define CONTROL_M 13   /*  ^M - carriage return */
#define DELETE    127    /* delete key */


/*  Use these definitions with access(2)  */
#define EXIST 00
#define EXECUTE 01
#define WRITE 02
#define READ 04

#define BREAK 23    /*  break exit code from mscript program */

#define MASK 0177  /* Set to zero all but low-order seven bits */

void exit(), perror();
