/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/mneeds.c	1.2"

/**
 ** mneeds() -  RETURN NUMBER OF FILE DESCRIPTORS NEEDED BY mopen()
 **/

int			mneeds ()
{
	/*
	 * This is the expected number of file descriptors
	 * needed when FIFOs will be used instead of IPC.
	 * Even while we still deliver the IPC mechanism,
	 * leave this value here, so people can test the
	 * dependency. ``Later'' we can make this real.
	 */
	return (4);
}
