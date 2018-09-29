/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:include/switch.h	1.2"
/*
	defines for the SWITCH construct for selecting among character strings
*/

#define	WoRD(x)x


#define	SWITCH(a)	{  char *s_w_i_t_c_h; s_w_i_t_c_h = a;  WoRD(/)WoRD(*)

#define	CASE(a)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/) \
		if( !strcmp(s_w_i_t_c_h, a) ) {

#define	CASE2(a1,a2)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/)\
		if( !strcmp(s_w_i_t_c_h, a1)  ||\
		!strcmp(s_w_i_t_c_h, a2) ) {

#define	CASE3(a1,a2,a3)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/)\
		if( !strcmp(s_w_i_t_c_h, a1)  ||\
		!strcmp(s_w_i_t_c_h, a2)  ||\
		!strcmp(s_w_i_t_c_h, a3) ) {

#define	CASEN(a)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/)\
		if( !strncmp(s_w_i_t_c_h, a, sizeof(a)-1) ) {

#define	CASEN2(a1,a2)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/)\
		if( !strncmp(s_w_i_t_c_h, a1, sizeof(a1)-1)  ||\
		!strncmp(s_w_i_t_c_h, a2, sizeof(a2)-1) ) {

#define	CASEN3(a1,a2,a3)	} else WoRD(/)WoRD(*) WoRD(*)WoRD(/)\
		if( !strncmp(s_w_i_t_c_h, a1, sizeof(a1)-1)  ||\
		!strncmp(s_w_i_t_c_h, a2, sizeof(a2)-1)  ||\
		!strncmp(s_w_i_t_c_h, a3, sizeof(a3)-1) ) {

#define	DEFAULT		} else WoRD(/)WoRD(*) WoRD(*)WoRD(/) {

#define	ENDSW		}  WoRD(/)WoRD(*) WoRD(*)WoRD(/) }
