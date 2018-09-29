/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)getprm.c	1.4 */
#include <stdio.h>

#define LQUOTE	'('
#define RQUOTE ')'
#define NOSYSPART	0
#define HASSYSPART	1

/*
 * get next parameter from s
 *	s	-> string to scan
 *	prm	-> pointer to use to return token
 * return:
 *	 s	-> pointer to next character 
 */
char *
getprm(s, prm)
register char *s; 
char *prm;
{
	register char *c;
	char *strchr();

	/*
 	 * skip white space
 	 */
	while (*s == ' ' || *s == '\t' || *s == '\n')
		s++;

	*prm = '\0';
	if (*s == '\0')
		return(NULL);

	if (*s == '>' || *s == '<' || *s == '|'
	  || *s == ';' || *s == '&') {
		*prm++ = *s++;
		*prm = '\0';
		return(s);
	}

	/*
	 * look for quoted argument
	 */
	if (*s == LQUOTE) {
		if ((c = strchr(s + 1, RQUOTE)) != NULL) {
			c++;
			while (c != s)
				*prm++ = *s++;
			*prm = '\0';
			return(s);
		}
	}

	/*
	 * look for `  ` string
	 */
	if (*s == '`') {
		if ((c = strchr(s + 1, '`')) != NULL) {
			c++;
			while (c != s)
				*prm++ = *s++;
			*prm = '\0';
			return(s);
		}
	}

	while (*s != ' ' && *s != '\t' && *s != '<'
	&& *s != '>' && *s != '|' && *s != '\0'
	&& *s != '&' && *s != ';' && *s != '\n')
		*prm++ = *s++;
	*prm = '\0';

	return(s);
}

/*
 * split name into system and file part
 *	name	-> string to scan
 *	sys	-> return area for system name
 *	rest	-> return area for remainder
 * return:
 *	NOSYSPART	-> no system prefix
 *	HASSYSPART	-> system prefix return in sys
 */
split(name, sys, rest)
register char *name;
char *sys, *rest;
{
	register char *c;
	char *strchr(), *strcpy();

	if (*name == LQUOTE) {
		if ((c = strchr(name + 1, RQUOTE)) != NULL) {

			/*
		 	* strip off quotes
	 	 	*/
			name++;
			while (c != name)
				*rest++ = *name++;
			*rest = '\0';
			*sys = '\0';
			return(NOSYSPART);
		}
	}

	if ((c = strchr(name, '!')) == NULL) {
		strcpy(rest, name);
		*sys = '\0';
		return(NOSYSPART);
	}

	/*
	 *  ignore escaped '!'
	 */
	if ((c != name) && (*(c-1) == '\\')) {
		*(c-1) = '\0';
		strcpy(rest, name);
		strcat(rest, c);
		*sys = '\0';
		return(NOSYSPART);
	}

	*c = '\0';
	{
		register  i;
		for (i = 0; i < 7; i++)
		if ((*sys++ = *name++) == '\0')
			break;

	}
	strcpy(rest, ++c);
	return(HASSYSPART);
}
