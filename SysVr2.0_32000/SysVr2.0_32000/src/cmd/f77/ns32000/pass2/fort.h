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
/*	@(#)fort.h	1.2	*/
/*	machine dependent file  */

label( n ){
	printf( ".L%d:\n", n );
	}

tlabel(){
	cerror("code generator asked to make label via tlabel\n");
	}
