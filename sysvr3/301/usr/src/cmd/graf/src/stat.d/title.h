/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/title.h	1.2"
#define TITOUT0(fd,lab,max)	if(getlabel(fd,lab,max))\
				 printf("\1%s\n",lab); else
#define TITOUT1(fd,lab,max,node)	if(getlabel(fd,lab,max))\
				 printf("\1%s %s\n","node",lab); else
#define TITOUT2(fd,lab,max,node,par)	if(getlabel(fd,lab,max))\
					 printf("\1%s%g %s\n","node",par,lab); else
