/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/ptog.d/linemod.c	1.1"
/* <: t-5 d :> */
linemod(str,ls)
	char *str;
	int *ls; {
/*
 *  linemod sets the following line style values:
 *  solid		ls = 0
 *  dotted		ls = 1
 *  dotdashed		ls = 2
 *  shortdashed		ls = 3
 *  longdashed		ls = 4
 */
	switch(str[0]){
	case 'l' :
		*ls = 4;
		break;
	case 'd' :
		if(str[3] != 'd') *ls =1;
		else *ls =2;
		break;
	case 's' :
		if(str[5] != '\0') *ls = 3;
		else *ls = 0;
	}
	return;
}
