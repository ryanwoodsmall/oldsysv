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
static char sccsid[] = "@(#)passwd.c	1.3";
/*
 * Enter a password in the password file.
 * This program should be suid with the owner
 * having write permission on /etc
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>         /* isalpha(c), isdigit(c), islower(c), toupper(c) */

char	passwd[] = "/etc/passwd";
char	opasswd[] = "/etc/opasswd";
char	temp[]	 = "/etc/ptmp";
struct	passwd *pwd, *getpwent();
int	endpwent();
char	*crypt();
char	*getpass();
char	*pw;
char	pwbuf[10];
char	opwbuf[10];
char	buf[10];

time_t	when;
time_t	now;
time_t	maxweeks;
time_t	minweeks;
long	a64l();
char	*l64a();
long	time();
int	count; /* count verifications */

#define WEEK (24L * 7 * 60 * 60) /* seconds per week */
#define MINLENGTH 6  /* for passwords */

main (argc, argv)
	int argc;
	char *argv[];
{
	char *p;
	int i;
	char saltc[2];
	long salt;
	int u;
	int insist;
	int ok, flags;
	int c;
	int j;                     /* for triviality checks */
	int k;                     /* for triviality checks */
	int flag;                  /* for triviality checks */
	int opwlen;                /* for triviality checks */
	int pwlen;
	FILE *tf;
	char *o;                   /* for triviality checks */
	char *uname, *getlogin();

	insist = 0;
	count = 0;
	if (argc < 2) {
		if ((uname = getlogin()) == NULL) {
			fprintf (stderr, "Usage: passwd user\n");
			goto bex;
		} else
			fprintf (stderr, "Changing password for %s\n", uname);
	} else
		uname = argv[1];

	while ((pwd = getpwent()) != NULL && strncmp (pwd->pw_name, uname, L_cuserid-1) != 0)
		;
	u = getuid();
	if (pwd == NULL || (u != 0 && u != pwd->pw_uid)) {
		fprintf (stderr, "Permission denied.\n");
		goto bex;
	}
	endpwent();
	if (pwd->pw_passwd[0] && u != 0) {
		strcpy (opwbuf, getpass ("Old password:"));
		opwlen = strlen(opwbuf);       /* get length of old password */
		pw = crypt (opwbuf, pwd->pw_passwd);
		if (strcmp (pw, pwd->pw_passwd) != 0) {
			fprintf (stderr, "Sorry.\n");
			goto bex;
		}
	} else
		opwbuf[0] = '\0';
	if (*pwd->pw_age != NULL) {
		/* password age checking applies */
		when = (long) a64l (pwd->pw_age);
		/* max, min and week of last change are encoded in radix 64 */
		maxweeks = when & 077;
		minweeks = (when >> 6) & 077;
		when >>= 12;
		now  = time ((long *) 0)/WEEK;
		if (when <= now) {
			if (u != 0 && (now < when + minweeks)) {
				fprintf (stderr, "Sorry: < %ld weeks since the last change\n", minweeks);
				goto bex;
			}
			if (minweeks > maxweeks && u != 0) {
				fprintf (stderr, "You may not change this password.\n");
				goto bex;
			}
		}
	}
tryagn:
	if( insist >= 3) {      /* three chances to meet triviality standard */
		fprintf(stderr, "Too many failures - try later.\n");
		goto bex;
		}
	strcpy (pwbuf, getpass ("New password:"));
	pwlen = strlen (pwbuf);

	/* Make sure new password is long enough */

	if (u != 0 && (pwlen < MINLENGTH ) ) { 
		fprintf(stderr, "Password is too short - must be at least 6 digits\n");
		insist++;
		goto tryagn;
		}

	/* Check the circular shift of the logonid */
	 
	if( u != 0 && circ(uname, pwbuf) ) {
		fprintf(stderr, "Password cannot be circular shift of logonid\n");
		insist++;
		goto tryagn;
		}

	/* Insure passwords contain at least two alpha characters */
	/* and one numeric or special character */               

	flags = 0;
	flag = 0;
	p = pwbuf;
	if (u != 0) {
		while (c = *p++){
			if( isalpha(c) && flag ) flags |= 1;
				else if( isalpha(c) && !flag ){
					flags |= 2;
					flag = 1;
					}
				else if( isdigit(c) ) flags |= 4;
				else flags |= 8;
				}
		 
		/*		7 = lca,lca,num
		 *		7 = lca,uca,num
		 *		7 = uca,uca,num
		 *		11 = lca,lca,spec
		 *		11 = lca,uca,spec
		 *		11 = uca,uca,spec
		 *		15 = spec,num,alpha,alpha
		 */
		 
			if ( flags != 7 && flags != 11 && flags != 15  ) {
				fprintf(stderr,"Password must contain at least two alphabetic characters and\n");
				fprintf(stderr,"at least one numeric or special character.\n");
				insist++;
				goto tryagn;
				}
		}
if ( u != 0 ) {
		p = pwbuf;
		o = opwbuf;
		if( pwlen >= opwlen) {
			i = pwlen;
			k = pwlen - opwlen;
			}
			else {
				i = opwlen;
				k = opwlen - pwlen;
				}
		for( j = 1; j  <= i; j++ ) if( *p++ != *o++ ) k++;
		if( k  <  3 ) {
			fprintf(stderr, "Passwords must differ by at least 3 positions\n");
			insist++;
			goto tryagn;
			}
	}

	/* Insure password was typed correctly, user gets three chances */

	strcpy (buf, getpass ("Re-enter new password:"));
	if (strcmp (buf, pwbuf)) {
		if (++count > 2) {
			fprintf (stderr, "Too many tries; try again later.\n");
			goto bex;
		} else
			fprintf (stderr, "They don't match; try again.\n");
		goto tryagn;
	}

	/* Construct salt, then encrypt the new password */

	time (&salt);
	salt += getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i=0; i<2; i++) {
		c = saltc[i] + '.';
		if (c>'9') c += 7;
		if (c>'Z') c += 6;
		saltc[i] = c;
	}
	pw = crypt (pwbuf, saltc);
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);

	umask (0333);

	if (access (temp, 0) >= 0) {
		fprintf (stderr, "Temporary file busy; try again later.\n");
		goto bex;
	}

/*
 *	Between the time that the previous access completes
 *	and the following fopen completes, it is possible for
 *	some other user to sneak in and foul things up.
 *	It is not possible to solve this by using creat to
 *	create the file with mode 0444, because the creat will
 *	always succeed if our effective uid is 0.
 */

	if ((tf = fopen (temp, "w")) == NULL) {
		fprintf (stderr, "Cannot create temporary file\n");
		goto bex;
	}

/*
 *	copy passwd to temp, replacing matching lines
 *	with new password.
 */

	while ((pwd = getpwent()) != NULL) {
		if (strcmp (pwd->pw_name, uname, L_cuserid-1) == 0) {
			u = getuid();
			if (u != 0 && u != pwd->pw_uid) {
				fprintf (stderr, "Permission denied.\n");
				goto out;
			}
			pwd->pw_passwd = pw;
			if (*pwd->pw_age != NULL) {
				if (maxweeks == 0) 
					*pwd->pw_age = '\0';
				else {
					when = maxweeks + (minweeks << 6) + (now << 12);
					pwd->pw_age = l64a (when);
				}
			}
		}
		putpwent (pwd, tf);
	}
	endpwent ();
	fclose (tf);

/*
 *	Rename temp file back to passwd file.
 */

	if (unlink (opasswd) && access (opasswd, 0) == 0) {
		fprintf (stderr, "cannot unlink %s\n", opasswd);
		goto out;
	}

	if (link (passwd, opasswd)) {
		fprintf (stderr, "cannot link %s to %s\n", passwd, opasswd);
		goto out;
	}

	if (unlink (passwd)) {
		fprintf (stderr, "cannot unlink %s\n", passwd);
		goto out;
	}

	if (link (temp, passwd)) {
		fprintf (stderr, "cannot link %s to %s\n", temp, passwd);
		if (link (opasswd, passwd)) {
			fprintf (stderr, "cannot recover %s\n", passwd);
			goto bex;
		}
		goto out;
	}

	if (unlink (temp)) {
		fprintf (stderr, "cannot unlink %s\n", temp);
		goto out;
	}

	exit (0);

out:
	unlink (temp);

bex:
	exit (1);
}
circ( s, t )
char *s, *t;
{
	char c, *p, *o, *r, buff[25], ubuff[25], pubuff[25];
	int i, j, k, l, m;
	 
	m = 2;
	i = strlen(s);
	o = &ubuff[0];
	for( p = s; c = *p++; *o++ = c ) if( islower(c) ) c = toupper(c);
	*o = '\0';
	o = &pubuff[0];
	for( p = t; c = *p++; *o++ = c ) if( islower(c) ) c = toupper(c);
	*o = '\0';
	 
	p = &ubuff[0];
	while( m-- ) {
		for( k = 0; k  <=  i; k++) {
			c = *p++;
			o = p;
			l = i;
			r = &buff[0];
			while(--l) *r++ = *o++;
			*r++ = c;
			*r = '\0';
			p = &buff[0];
			if( strcmp( p, pubuff ) == 0 ) return(1);
			}
		p = p + i;
		r = &ubuff[0];;
		j = i;
		while( j-- ) *--p = *r++;
	}
	return(0);
}
