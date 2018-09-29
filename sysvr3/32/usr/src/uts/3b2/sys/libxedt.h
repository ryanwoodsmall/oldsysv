/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/libxedt.h	1.2"
/*==============================================================*/
/*								*/
/*	libxedt.h - header file for the library used		*/
/*		    to read the EDT and the extended EDT's	*/
/*==============================================================*/

struct partedt {
	char dev_name[10];
	unsigned short code; /* Null if no board code  ====> edt.h*/
	unsigned long major;
	unsigned long parentmaj;	/* Null if no parent */
};

struct libxedt {
	int edtentries;
	struct partedt edt[1];
};

struct edtsze {
	short esize;
	short ssize;
};

struct mainedt {
	struct edtsze edtsze;
	struct edt mainedt[1];
};
