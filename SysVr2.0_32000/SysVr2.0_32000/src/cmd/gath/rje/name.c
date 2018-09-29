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
/*
*	name puts the simple prefix to "suf", from "str"
*	into "buf".
*/

static char Sccsid[] = "@(#)name.c	1.1";

name(str,suf,buf)
register char *str, *buf;
char *suf;
{
	char *strrchr(), *index(), *strcpy();
	register char *ptr;

	ptr = strrchr(str,'/');
	if(ptr)
		strcpy(buf,++ptr);
	else
		strcpy(buf,str);
	ptr = index(buf,suf);
	if(*ptr) {
		*ptr = '\0';
		return(1);
	}
	return(0);
}

/*
*	index returns the address of the second arg within the first.
*/

char *
index(as1,as2)
char *as1,*as2;
{
	register char *s1,*s2,c;
	char *loc;

	s1 = as1;
	s2 = as2;
	c = *s2;

	while (*s1)
		if (*s1++ == c) {
			loc = s1 - 1;
			s2++;
			while ((c = *s2++) == *s1++ && c) ;
			if (c == 0)
				return(loc);
			s1 = loc + 1;
			s2 = as2;
			c = *s2;
		}
	return((char *)0);
}
