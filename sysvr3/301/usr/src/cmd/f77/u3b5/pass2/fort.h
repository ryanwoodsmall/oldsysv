/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)f77-3b20-p2:fort.h	1.1.2.1"
/*	@(#)f77-3b20-p2:fort.h	1.1.2.1	*/
/*	machine dependent file  */

label( n ){
	printf( ".L%d:\n", n );
	}

tlabel(){
	cerror("code generator asked to make label via tlabel\n");
	}
