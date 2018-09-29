/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/stubs.c	10.1"
/*
 *   This is a STUB (dummy) mode that is used to define 
 *   variables reference within the kernel
 *   but defined by other modules such as IPC, SEM, MSG ...
 *   The stub defines are found in etc/master.d/stubs
 *   To make this functional, mkboot is run against stubs.o
 *   using the stubs master file.  The output of mkboot (STUBS),
 *   is then used by lboot to define those functions that are
 *   found to be undefined after all "real" drivers and modules
 *   are loaded.
 */
