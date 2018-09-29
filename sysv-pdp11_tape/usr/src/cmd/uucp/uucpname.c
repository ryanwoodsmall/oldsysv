/* @(#)uucpname.c	1.5 */
#include "uucp.h"

#ifdef UNAME
#include <sys/utsname.h>
#endif

/*
 * get the uucp name
 * return:
 *	none
 */
uucpname(name)
register char *name;
{
	register char *s, *d;
/*  the following 3 variables are for debugging */
	int ret;
	char msg[BUFSIZ];
	char loginusr[NAMESIZE];

#ifdef UNAME
	struct utsname utsn;

	uname(&utsn);
	s = utsn.nodename;
#endif

#ifndef UNAME
	s = MYNAME;
#endif

	d = name;
	while ((*d = *s++) && d < name + SYSNSIZE)
		d++;
	*(name + SYSNSIZE) = '\0';
/* the following statements are for debugging  */
#ifdef TESTB
	ret = guinfo(getuid(), loginusr, msg);
	switch (*loginusr){
		case 'd':
			name[4] = 'D';
			break;
		case 'e':
			name[4] = 'E';
			break;
		case 'f':
			name[4] = 'F';
			break;
		}
#endif
	return;
}
