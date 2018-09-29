/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/setwtst.c	1.1"
#include <stdio.h>
#include "gsl.h"
#include "vtoc.h"
extern char *nodename;

char *
setwtst(buf,weight,style)
char *buf,*weight,*style;
{	int wt=NARROW,st=SOLID;
	char *ptr=buf;
/*
 *  setwtst checks for valid weight-style TTOC specification
 *  and sets values into weitht and style
 */
	*style=st; *weight=wt;
	while(*ptr != EOS){
		switch(*ptr++){
		case 'n':  wt=NARROW; break;
		case 'm':  wt=MEDIUM; break;
		case 'b':  wt=BOLD; break;
		case 'd':
				if(*ptr=='d') {ptr++; st=DOTDASH; break;}
				if(*ptr=='a') {ptr++; st=DASHED; break;}
				if(*ptr=='o') {ptr++; st=DOTTED; break;}
				return(buf);
		case 'l': if(*ptr=='d') {ptr++; st=LONGDASH; break;}
				else return(buf);
		case 's': if(*ptr=='o') {ptr++; st=SOLID; break;}
				else return(buf);
		default:  return(buf);
		}
		if(*ptr==',') ptr++;
	}
	*style=st;  *weight=wt;
	return(++ptr);
}
