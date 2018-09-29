/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:include/debug.h	1.2"
#define DB(f,x)	fprintf(stderr,"x=%f ",(x))
#define DB1(f,x1)		DB(f,x1),putc('\n',stderr)
#define DB2(f,x1,x2)		DB(f,x1),DB1(f,x2)
#define DB3(f,x1,x2,x3)		DB(f,x1),DB2(f,x2,x3)
#define DB4(f,x1,x2,x3,x4)	DB(f,x1),DB3(f,x2,x3,x4)
#define DB5(f,x1,x2,x3,x4,x5)	DB(f,x1),DB4(f,x2,x3,x4,x5)
