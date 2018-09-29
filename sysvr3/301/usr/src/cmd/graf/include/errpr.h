/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:include/errpr.h	1.2"
#define ERRPR0(str)	fprintf(stderr,"!!  %s: str\n",nodename)
#define ERRPR1(str,par)	fprintf(stderr,"!!  %s: str\n",nodename,par);
#define ERRPR2(str,par1,par2)	fprintf(stderr,"!!  %s: str\n",nodename,par1,par2);
