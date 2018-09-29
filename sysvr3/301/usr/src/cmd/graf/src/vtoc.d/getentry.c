/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/getentry.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "vtoc.h"
#define  ID 0
#define  LS 1
#define  TEXT 2
#define  MARK 3

getentry(fpi,buf,tokens)
FILE *fpi;
char *buf,*tokens[];
{	char *ptr=buf;
	int tok, c;

	for (tok=ID;tok<= MARK; tok++){
		tokens[tok]=ptr;
		if(tok==LS){
			while(isspace(c=getc(fpi)));
			ungetc(c,fpi);
			if(c=='\"'){tokens[LS]=NULL; continue;}
		}
		if(tok==MARK){
			while(isspace(c=getc(fpi))) if(c=='\n')break;
			ungetc(c,fpi);
			if(c=='\n'){tokens[MARK]=NULL;break;}
		}
		if((gettokf(fpi,ptr,MAXLENGTH-(ptr-buf)))==EOF)return(EOF);
		else ptr += strlen(ptr)+1;
	}
	return(SUCCESS);
}

#define COND(c,len)	(c != EOS && c != EOF && len>0)
gettokf(fpi,str,len)
FILE *fpi;
char *str; 
int len;
{
#if u370 | u3b | u3b5 | u3b2
	int c;
	char *p=str;
#else
	char c,*p=str;
#endif
/*
 *  gettokf gets next token from input file and places
 *  into string pointed to by str
 *  if non-escaped quotes are found, end of token is
 *  looked for after next non escaped quote.  len is max
 *  number of characters placed.  if EOF is encountered
 *  or len is exceeded, FAIl is returned, else 1 is returned
 */
	*str = EOS;
	while((isspace(c=getc(fpi)) || c=='\\')&& COND(c,len));
	if(c == EOF) return(EOF);
#if u370
	ungetc((char) c,fpi);
#else
	ungetc(c,fpi);
#endif
	for(;(!(isspace(*str=getc(fpi)))) && COND(*str,len)
	;str++,len--) {
		if(*str == '\"'){
			for(;(*str = getc(fpi)) != '\"'&& COND(*str,len)
			;str++,len--){
				if(*str == '\\')
					if((*str=getc(fpi))=='\n') *str=' ';
			}
			if(*str=='\"') { str--; len++;}
		}
		else if(*str == '\\') 
			if((*str=getc(fpi))=='\n') {*str++=' '; *str=EOS; break;}
	}
	if(isspace(*str)) ungetc(*str,fpi);
	*str = EOS;
	if(*str == EOF ||  len<=0) return(FAIL);
	else return(SUCCESS);
}
