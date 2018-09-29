/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:include/setop.h	1.2"
#include "ctype.h"

#define SETINT(x,p)	x = atoi(p);\
			for(; isdigit(*p); p++)
#define SETSINT(x,p)	x = atoi(p);\
			for(; isdigit(*p)||*p=='-'; p++)
#define SETDOUB(x,p)	x = atof(p);\
			for(; ISDOUB(*p); p++)
#define SETSIGN(x,p)	x=(*p=='-'?(p++,-1):1);\
			if(*p=='+')p++
#define SETSTR(s,p,e)	{int j=0;\
			for(;*p!='\0' && *p!=',';p++)\
			 s[j<e-1?j++:j] = *p!='\\'?*p:*++p;\
			s[j]='\0';}

#define SETFILE(fd,p)	{char *q=p;\
 if(*p=='-')fd=stdin;else{\
  for(;*q!=' '&&*q!=','&&*q!='\0';q++);\
  if(*q=='\0')--q; else *q='\0';\
  if(!(fd=fopen(p,"r")))\
   fprintf(stderr,"!!  %s: cannot open %s\n",nodename,p);\
  }p=q+1;}
double atof();
