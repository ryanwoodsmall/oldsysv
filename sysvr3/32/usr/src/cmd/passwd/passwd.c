/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)passwd:passwd.c	1.4.1.10"
/*
 * passwd is a program whose sole purpose is to manage 
 * the password file. It allows system administrator
 * to add, change and display password attributes.
 * Non privileged user can change password or display 
 * password attributes which corresponds to their login name.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>         /* isalpha(c), isdigit(c), islower(c), toupper(c) */
/* flags  indicate password attributes to be modified */

#define PFLAG 0x080		/* change user's password */  
#define LFLAG 0x001		/* lock user's password  */
#define DFLAG 0x002		/* delete user's  password */
#define MFLAG 0x004		/* set max field -- # of days passwd is valid */		 
#define NFLAG 0x008		/* set min field -- # of days between password changes */
#define SFLAG 0x010		/* display password attributes */
#define FFLAG 0x020		/* expire  user's password */
#define AFLAG 0x040		/* display password attributes for all users*/
#define SAFLAG 0x050		/* display password attributes for all users */

/* exit  code */

#define SUCCESS	0	/* succeeded */
#define NOPERM	1	/* No permission */
#define BADSYN	2	/* Incorrect syntax */
#define FMERR	3	/* File manipulation error */
#define FATAL	4	/* Old file can not be recover */
#define FBUSY	5	/* Lock file busy */
#define BADOPT	6	/* Invalid argument to option */
 
/* define error messages */
#define MSG_NP	"Permission denied"
#define MSG_NV  "Invalid argument to option"
#define MSG_BS	"Invalid combination of options"
#define MSG_FE	"Unexpected failure. Password file unchanged."
#define MSG_FF	"Unexpected failure. Password file missing."
#define MSG_FB	"Password file(s) busy. Try again later."

/* return code from ckarg() routine */
#define FAIL 		-1

#define NUMCP	13	/* number of characters for valid password */
#define MINLENGTH 6  	/* for passwords */

#define DAY_WEEK 7	/* days per weeks */
#define DIG_CH	63	/* A character represents upto 64 
			   digits in a radix-64 notation */
#define SEC_WEEK (24 * DAY_WEEK * 60 * 60 )    /* seconds per week */

/*	 Convert base-64 ASCII string to long integer.  *
 * 	 Convert long integer to maxweek, minweeks.     *
 * 	 Convert current time from seconds to weeks.	*
 */ 	
#define CNV_AGE()	{\
	when = (long) a64l(pwd->pw_age);\
	maxweeks = when & 077;\
	minweeks = (when >> 6) & 077;\
	now = time ((long *) 0) / SEC_WEEK;\
}

/* print password status */

#define PRT_PWD(pwdp)	{\
	if (*pwdp == NULL) \
		fprintf(stdout, "NP  ");\
	else if (strlen(pwdp) < NUMCP) \
		(void) fprintf(stdout, "LK  ");\
	else\
		(void) fprintf(stdout, "PS  ");\
}
#define PRT_AGE()	{\
	if (sp->sp_max != -1) { \
		if (sp->sp_lstchg) {\
			lstchg = sp->sp_lstchg * DAY;\
			tmp = gmtime(&lstchg);\
			(void) fprintf(stdout,"%.2d/%.2d/%.2d  ",(tmp->tm_mon + 1),tmp->tm_mday,tmp->tm_year);\
		} else\
			(void) fprintf(stdout,"00/00/00  ");\
		(void) fprintf(stdout, "%d  %d ", sp->sp_min, sp->sp_max);\
	}\
}

extern int optind;
extern struct	passwd *getpwent();
extern  struct passwd *getpwnam();
struct passwd *pwd;
struct spwd *sp;
char 	lkstring[] = "*LK*";		/*lock string  to lock user's password*/
char	nullstr[] = "";
/* usage message of non privileged user */
char 	usage[]  = "	Usage:\n\tpasswd [-s] [name]\n";
/* usage message of privileged user */
char 	sausage[]  = "	Usage:\n\tpasswd [name]\n\tpasswd  [-l|-d]  [-n min] [-f] [-x max] name\n\tpasswd -s [-a]\n\tpasswd -s [name]\n";
char	opwbuf[10];
char	*pw, *uname, *prognamep;
int 	uid, retval,opwlen;
int 	 mindays, maxdays;		/* password aging information */
int 	when;
long	a64l();
char 	*l64a();
time_t 	now, maxweeks, minweeks;
ushort shadow_exist;


main (argc, argv)
int argc;
char *argv[];
{
	extern char *crypt();
	extern char *getpass();
	extern char  *getlogin();

	/* passwd calls getpass() to get the password. The getpass() routine
	   returns at most 8 charaters. Therefore, array of 10 chars is
	   sufficient.
	*/
	char	pwbuf[10];			
	char	buf[10];
	char 	*p, *o;
	char 	saltc[2];	 /* crypt() takes 2 char string as a salt */
	 int	count; 		 /* count verifications */
	long 	salt;
	int 	insist;
	int 	flags;
	int	c;
	int 	i, j, k, flag;	/* for triviality checks */
	int 	pwlen;  /* length of old passwords */

	flag = 0;
	insist = 0;		/* # of times the program  prompts 
				 * for valid password */
	count = 0;
	prognamep = argv[0];	
	uid = getuid();		/* get the user id */
	
	/* check for shadow existence */
	if ( access(SHADOW,0) == 0)
		shadow_exist = 1; 
	if (argc > 1) 
		/* 
	    	 * ckarg() parses the arguments. In case of an error, 
		 * it sets the retval and returns FAIL (-1). It return
		 * the value which indicate which password attributes
		 * to modified 
		*/

		switch (flag = ckarg( argc, argv)) { 
		case FAIL:		/* failed */
			exit(retval); 
		case SAFLAG:		/* display password attributes */
			if (shadow_exist)
				exit(sp_display((char *) NULL));
			else
				exit(pwd_display((char *) NULL));
		default:	
			break;
		}

	argc -= optind;

	if (argc < 1 ) {
		if ((uname = getlogin()) == NULL) { 
			(void) fprintf(stderr, "%s", uid > 0 ? usage :sausage);
			exit(NOPERM);
		} else if (!flag)	/* if flag set, must be displaying or */
					/* modifying password aging attributes */
			(void) fprintf(stderr,"%s:  Changing password for %s\n",
				       prognamep,  uname);
	} else
		uname = argv[optind];

	if ((pwd = getpwnam(uname)) == NULL || 
            (shadow_exist &&  (sp = getspnam(uname)) == NULL)) { 
		(void) fprintf(stderr, "%s:  %s does not exist\n", 
   			       prognamep,  uname);
		exit(NOPERM);
	}

	if (uid != 0 && uid != pwd->pw_uid) { 
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_NP);
		exit(NOPERM);
	}
	/* If the flag is set display/update password attributes.*/

	switch (flag) {
	case SFLAG:		/* display password attributes */
		if (shadow_exist)
			exit(sp_display(uname));
		else
			exit(pwd_display(uname));
	case 0:			/* changing user password */
		break;
	default:		/* changing user password attributes */
		exit(update(flag));
		break;
	}

	if (shadow_exist)
		retval = ck_sppasswd();
	else
		retval = ck_pwdpasswd();
	if (retval)
		exit(retval);
tryagn:
	if( insist >= 3) {       /* three chances to meet triviality standard */
		(void) fprintf(stderr, "Too many failures - try later\n", prognamep);
		exit(NOPERM);
	}
	(void) strcpy (pwbuf, getpass ("New password:"));
	pwlen = strlen (pwbuf);

	/* Make sure new password is long enough */

	if (uid != 0 && (pwlen < MINLENGTH ) ) { 
		(void) fprintf(stderr, "Password is too short - must be at least 6 digits\n");
		insist++;
		goto tryagn;
	}

	/* Check the circular shift of the logonid */
	 
	if( uid != 0 && circ(uname, pwbuf) ) {
		(void) fprintf(stderr, "Password cannot be circular shift of logonid\n");
		insist++;
		goto tryagn;
	}

	/* Insure passwords contain at least two alpha characters */
	/* and one numeric or special character */               

	flags = 0;
	flag = 0;
	p = pwbuf;
	if (uid != 0) {
		while (c = *p++) {
			if (isalpha(c) && flag )
				 flags |= 1;
			else if (isalpha(c) && !flag ) {
				flags |= 2;
				flag = 1;
			} else if (isdigit(c) ) 
				flags |= 4;
			else 
				flags |= 8;
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
			(void) fprintf(stderr,"Password must contain at least two alphabetic characters and\n");
			(void) fprintf(stderr,"at least one numeric or special character.\n");
			insist++;
			goto tryagn;
		}
	}
	if ( uid != 0 ) {
		p = pwbuf;
		o = opwbuf;
		if ( pwlen >= opwlen) {
			i = pwlen;
			k = pwlen - opwlen;
		} else {
			i = opwlen;
			k = opwlen - pwlen;
		}
		for ( j = 1; j  <= i; j++ ) 
			if ( *p++ != *o++ ) 
				k++;
		if ( k  <  3 ) {
			(void) fprintf(stderr, "Passwords must differ by at least 3 positions\n");
			insist++;
			goto tryagn;
		}
	}

	/* Ensure password was typed correctly, user gets three chances */

	(void) strcpy (buf, getpass ("Re-enter new password:"));
	if (strcmp (buf, pwbuf)) {
		if (++count > 2) { 
			(void) fprintf(stderr, "%s: Too many tries; try again later\n", prognamep);
			exit(NOPERM);
		} else
			(void) fprintf(stderr, "They don't match; try again.\n");
		goto tryagn;
	}

	/* Construct salt, then encrypt the new password */

	(void) time(&salt);
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
	exit(update(PFLAG));
}

/* ck_pwdpasswd():  Verify user old password. It also check 
 * password aging information to varify that user is authorized
 * to change password.
 */ 
  
int
ck_pwdpasswd()
{
	if (pwd->pw_passwd[0] && uid != 0) {
		(void) strcpy (opwbuf, getpass ("Old password:"));
		opwlen = strlen(opwbuf);       /* get length of old password */
		pw = crypt (opwbuf, pwd->pw_passwd);
		if (strcmp (pw, pwd->pw_passwd) != 0) {
			(void) fprintf(stderr, "Sorry.\n");
			return (NOPERM);
		}
	} else
		opwbuf[0] = '\0';
	/* password age checking applies */
	if (*pwd->pw_age != NULL) {
		CNV_AGE();
		when >>= 12 ;
		if (when <= now) {
			if (uid != 0 && ( now < when + minweeks)) { 
				(void) fprintf(stderr, "%s:  Sorry: < %ld weeks since the last change\n", prognamep, minweeks);
				return (NOPERM);
			}
			if ( minweeks > maxweeks && uid != 0) { 
				(void) fprintf(stderr, "%s: You may not change this password\n", prognamep);
				return (NOPERM);
			}
		}
	}
	return (SUCCESS);
}

/* ck_sppasswd():  Verify user old password. It also check 
 * password aging information to varify that user is authorized
 * to change password.
 */ 

int
ck_sppasswd()
{
	if (sp->sp_pwdp[0] && uid != 0) {
		(void) strcpy (opwbuf, getpass ("Old password:"));
		opwlen = strlen(opwbuf);       /* get length of old password */
		pw = crypt (opwbuf, sp->sp_pwdp);
		if (strcmp (pw, sp->sp_pwdp) != 0) {
			(void) fprintf(stderr, "Sorry.\n");
			return (NOPERM);
		}
	} else
		opwbuf[0] = '\0';
	/* password age checking applies */
	if (sp->sp_max != -1 && sp->sp_lstchg != 0) {
		now  =  DAY_NOW;
		if (sp->sp_lstchg <= now) {
			if (uid != 0 && ( now < sp->sp_lstchg  + sp->sp_min)) { 
				(void) fprintf(stderr, "%s:  Sorry: < %ld days since the last change\n", prognamep, sp->sp_min);
				return (NOPERM);
			}
			if (sp->sp_min > sp->sp_max && uid != 0) { 
				(void) fprintf(stderr, "%s: You may not change this password\n", prognamep);
				return (NOPERM);
			}
		}
	}
	return (SUCCESS);
}

int 
update(flag)
int flag;
{
	register int ret;

	/* lock the password file */
	if (lckpwdf() != 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FB);
		return (FBUSY);
	}
	if ( access(SHADOW,0) == 0) {
		if (!shadow_exist) {
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
			(void) ulckpwdf();
			return(FMERR);
		} else
			ret = sp_update(flag);

	} else  {
		if (shadow_exist) {
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
			(void) ulckpwdf();
			return(FMERR);
		} else
			ret = pwd_update(flag);
	}
	(void) ulckpwdf();
	return (ret);
} 

/************************************************************
 *							    *
 * pwd_update(): updates the password file.	    *
 * It takes "flag" as an argument to determine which        *
 * password attributes to modify. It returns 0 for success  *
 * and  > 0 for failure.			            *
 *							    *
 ***********************************************************/

int
pwd_update(flag)
int flag;
{
	register int i;
	extern int errno;
	struct stat buf;
	FILE *tpfp;

	

	/* ignore all the signals */

	for (i=1; i < NSIG; i++)
		(void) sigset(i, SIG_IGN);

 	/* Clear the errno. It is set because SIGKILL can not be ignored. */

	errno = 0;

	/* Mode  of the passwd file should be 444 or less */

	if (stat(PASSWD, &buf) < 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}
	(void) umask(~(buf.st_mode & 0444));

	if ((tpfp = fopen(PASSTEMP, "w")) == NULL) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/*
	 *	copy passwd  files to temps, replacing matching lines
	 *	with new password attributes.
	 */

	/* entry should be there -- we just checked it */

	while ((pwd = getpwent()) != NULL) {
	   if (strcmp(pwd->pw_name, uname) == 0) { 
		/* LFLAG and DFLAG should be checked before FFLAG.
		   FFLAG clears the lastchg field. We do not
		   want lastchg field to be set if one execute
		   passwd -d -f name or passwd -l -f name.
		*/

		if (flag & LFLAG) {	 /* lock password */
			pwd->pw_passwd = pw;
			if (*pwd->pw_age != NULL) {
				CNV_AGE();
				when = maxweeks + (minweeks << 6) + (now << 12);
				pwd->pw_age = l64a(when);
			}
		} 
		if (flag & DFLAG) {	 /* delete password */
			pwd->pw_passwd = nullstr;
			if (*pwd->pw_age != NULL) {
				CNV_AGE();
				when = maxweeks + (minweeks << 6) + (now << 12);
				pwd->pw_age = l64a(when);
			}
		} 
		if (flag & FFLAG) {	 /* expire password */
			if (*pwd->pw_age != NULL)  {
				CNV_AGE();
				when = maxweeks + (minweeks << 6);
				if ( when == 0)
					pwd->pw_age = ".";
				else
					pwd->pw_age = l64a(when);
			} else
				pwd->pw_age = ".";
		}
		/* MFLAG should be checked before NFLAG.  One 
		 * can not set min field without setting max field.
		 * If aging is off, force to expire a user password --
		 * set lastchg field to 0.
		*/
		if (flag & MFLAG)  { 	/* set max field */
			if ( maxdays == -1) {
				pwd->pw_age = nullstr; 
			} else  {
				if (*pwd->pw_age == NULL) {  
					minweeks  = 0;
					when = 0;
				} else
					CNV_AGE();
				maxweeks = (maxdays + (DAY_WEEK -1)) / DAY_WEEK;
				maxweeks = maxweeks > DIG_CH ? DIG_CH : maxweeks;
				when = maxweeks + (minweeks << 6) + (when & ~0xfff);
				if (when == 0)
					pwd->pw_age = ".";
				else
					pwd->pw_age = l64a(when);
			}
		}
		if (flag & NFLAG) {   /* set min field */
			if (*pwd->pw_age == NULL)  {
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage); 
				(void) unlink(PASSTEMP);
				return (BADSYN);
			}
			CNV_AGE();
			minweeks = (mindays + (DAY_WEEK  - 1)) / DAY_WEEK;
			minweeks = minweeks > DIG_CH ? DIG_CH : minweeks;
			when = maxweeks + (minweeks << 6) + (when & ~0xfff);
			if (when == 0)
				pwd->pw_age = ".";
			else
				pwd->pw_age = l64a(when);
		}
		if (flag & PFLAG)  {	/* change password */
			pwd->pw_passwd = pw;
			if (*pwd->pw_age != NULL) {
				CNV_AGE();
				if (maxweeks  == 0) {   /* turn off aging */
					pwd->pw_age = nullstr;
				} else  {
					when = maxweeks + (minweeks << 6) + (now << 12);
					pwd->pw_age = l64a(when);
				}
			}

		}
	   }
		if (putpwent (pwd, tpfp) != 0) { 
			(void) unlink(PASSTEMP);
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
			return (FMERR);
		}

	}
	(void) fclose (tpfp);
	return(update_pfile(PASSWD,OPASSWD,PASSTEMP));
}

int 
update_pfile(pfilep, opfilep, tpfilep)
char *pfilep;		/* password file */
char *opfilep;		/* old passwd file */
char *tpfilep;		/* temparory password file */
{
	/*
	 *	Rename temp file back to  appropriate passwd file.
	 */

	/* remove old passwd file */
	if (unlink(opfilep) && access(opfilep, 0) == 0) {
		(void) unlink(tpfilep);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		(void) ulckpwdf();
		return (FMERR);
	}

	/* line password file to old passwd file */
	if (link(pfilep, opfilep)) {
		(void) unlink(tpfilep);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/* unlink passwd file */
	if (unlink(pfilep)) {
		(void) unlink(tpfilep);
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/* link temparory password file to password file */
	if (link(tpfilep, pfilep)) {
		(void) unlink(pfilep);
		if (link (opfilep, pfilep)) { 
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FF);
			return (FATAL);
		}
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/* unlink temparory password file. If fails, don't care because 
	  password file is already updated */

	(void) unlink(tpfilep);
	return(SUCCESS);
}

/************************************************************
 *							    *
 * sp_update(): updates the shadow password file.	    *
 * It takes "flag" as an argument to determine which        *
 * password attributes to modify. It returns 0 for success  *
 * and  > 0 for failure.			            *
 *							    *
 ***********************************************************/

int
sp_update(flag)
int flag;
{
	register int i;
	extern int errno;
	struct spwd *sp;	/* pointer to a shadow password entry */
	struct stat buf;
	FILE *tsfp;

	

	/* ignore all the signals */

	for (i=1; i < NSIG; i++)
		(void) sigset(i, SIG_IGN);

 	/* Clear the errno. It is set because SIGKILL can not be ignored. */

	errno = 0;

	/* Mode  of the shadow file should be 400 or 000 */

	if (stat(SHADOW, &buf) < 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	(void) umask(~(buf.st_mode & 0400));
	if ((tsfp = fopen(SHADTEMP, "w")) == NULL) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
		return (FMERR);
	}

	/*
	 *	copy passwd  files to temps, replacing matching lines
	 *	with new password attributes.
	 */

	/* entry should be there -- we just checked it */

	while ((sp = getspent()) != NULL) {
		if (strcmp(sp->sp_namp, uname) == 0) { 
			/* LFLAG and DFLAG should be checked before FFLAG.
			   FFLAG clears the sp_lstchg field. We do not
			   want sp_lstchg field to be set if one execute
			   passwd -d -f name or passwd -l -f name.
			*/

			if (flag & LFLAG) {	 /* lock password */
				sp->sp_pwdp = pw;
				sp->sp_lstchg = DAY_NOW;
			} 
			if (flag & DFLAG) {	 /* delete password */
				sp->sp_pwdp = nullstr;
				sp->sp_lstchg = DAY_NOW;
			} 
			if (flag & FFLAG)	 /* expire password */
				sp->sp_lstchg = (long) 0;
			if (flag & MFLAG)  { 	/* set max field */
				if (!(flag & NFLAG) && sp->sp_min == -1)
					sp->sp_min = 0;
				if (maxdays == -1) 	/* trun off aging */
					sp->sp_min = -1;
				else if (sp->sp_max == -1)
					sp->sp_lstchg = 0;
				sp->sp_max = maxdays;
			}
			if (flag & NFLAG) {   /* set min field */
				if (sp->sp_max == -1) {
					(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage); 
					(void) unlink(SHADTEMP);
					return (BADSYN);
				}
				sp->sp_min = mindays;
			}
			if (flag & PFLAG)  {	/* change password */
				sp->sp_pwdp = pw;
				/* update the last change field */
				sp->sp_lstchg = DAY_NOW;
				if (sp->sp_max == 0) {   /* turn off aging */
					sp->sp_max = -1;
					sp->sp_min = -1;
				}
			}
		}
		if (putspent (sp, tsfp) != 0) { 
			(void) unlink(SHADTEMP);
			(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FE);
			return (FMERR);
		}

	}
	(void) fclose (tsfp);
	return(update_pfile(SHADOW,OSHADOW,SHADTEMP));
}

circ( s, t )
char *s, *t;
{
	char c, *p, *o, *r, buff[25], ubuff[25], pubuff[25];
	int i, j, k, l, m;
	 
	m = 2;
	i = strlen(s);
	o = &ubuff[0];
	for ( p = s; c = *p++; *o++ = c ) 
		if ( islower(c) )
			 c = toupper(c);
	*o = '\0';
	o = &pubuff[0];
	for ( p = t; c = *p++; *o++ = c ) 
		if ( islower(c) ) 
			c = toupper(c);

	*o = '\0';
	 
	p = &ubuff[0];
	while ( m-- ) {
		for ( k = 0; k  <=  i; k++) {
			c = *p++;
			o = p;
			l = i;
			r = &buff[0];
			while (--l) 
				*r++ = *o++;
			*r++ = c;
			*r = '\0';
			p = &buff[0];
			if ( strcmp( p, pubuff ) == 0 ) 
				return (1);
		}
		p = p + i;
		r = &ubuff[0];;
		j = i;
		while ( j-- ) 
			*--p = *r++;
	}
	return (SUCCESS);
}

/********************************************************
 *							*
 * ckarg(): This function parses and verifies the 	*
 * arguments.  It takes two parameters:			*
 * argc => # of arguments				*
 * argv => pointer to an argument			*
 * In case of an error it prints the appropriate error 	*
 * message, sets the retval and returns FAIL(-1).	* 		
 *							*
 ********************************************************/

int
ckarg(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	char *char_p;
	register int c, flag = 0;

	while ((c = getopt(argc, argv, "aldfsx:n:")) != EOF) {
		switch (c) {
		case 'd':		/* delet the password */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (LFLAG|SAFLAG)) {
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage); 
				retval = BADSYN;
				return (FAIL);
			}
			flag | = DFLAG;
			pw = nullstr;
			break;
		case 'l':		/* lock the password */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & (DFLAG|SAFLAG))  {
				(void) fprintf(stderr,"%s\n %s", MSG_BS,sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag | = LFLAG;
			pw = &lkstring[0];
			break;
		case 'x':		/* set the max date */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & SAFLAG) {
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag | = MFLAG;
			if ((maxdays = (int) strtol(optarg, &char_p, 10)) < -1
			    || *char_p != '\0' || strlen(optarg)  <= 0) {
				(void) fprintf(stderr, "%s: %s -x\n", prognamep, MSG_NV);
				retval = BADOPT;
				return (FAIL);
			}
			break;
		case 'n':		/* set the min date */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & SAFLAG) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			}
			flag |= NFLAG;
			if (((mindays = (int) strtol(optarg, &char_p,10)) < 0 
			    || *char_p != '\0') || strlen(optarg)  <= 0) {
				(void) fprintf(stderr, "%s: %s -n\n", prognamep, MSG_NV);
				retval = BADOPT;
				return (FAIL);
			} 
			break;
		case 's':		/* display password attributes */
			if (flag && (flag != AFLAG)) { 
				(void) fprintf(stderr,"%s %\n", MSG_BS, sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag |= SFLAG;
			break;
		case 'a':		/* display password attributes */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag && (flag != SFLAG)) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS,sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag |= AFLAG;
			break;
		case 'f':		/* expire password attributes */
			/* Only privileged process can execute this */
			if (ckuid() != 0)
				return (FAIL);
			if (flag & SAFLAG) { 
				(void) fprintf(stderr,"%s\n %s", MSG_BS, uid > 0 ? usage : sausage);
				retval = BADSYN;
				return (FAIL);
			} 
			flag | = FFLAG;
			break;
		case '?':
			(void) fprintf(stderr,"%s", uid > 0 ? usage : sausage);
			retval = BADSYN;
			return (FAIL);
		}
	}

	argc -=  optind;
	if (argc > 1) {
		fprintf(stderr,"%s", uid > 0 ? usage : sausage);
		retval = BADSYN;
		return (FAIL);
	}

	/* If no options are specified or only the show option */
	/* is specified, return because no option error checking */
	/* is needed */
	if (!flag || (flag == SFLAG)) 
		return (flag);

	if (flag == AFLAG) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if (flag != SAFLAG && argc < 1) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if (flag == SAFLAG && argc >= 1) {
		(void) fprintf(stderr,"%s", sausage);
		retval = BADSYN;
		return (FAIL);
	}
	if ((maxdays == -1) &&  (flag & NFLAG)) {
		(void) fprintf(stderr, "%s: %s -x\n", prognamep, MSG_NV);
		retval = BADOPT;
		return (FAIL);
	}
	return (flag);
}

/******************************************************
 *						      *
 *  pwd_display():  displays password attributes.     * 
 *  It takes user name as a parameter. If the user    *
 *  name is NULL then it displays password attributes *
 *  for all entries on the file. It returns 0 for     *
 * success and positive  number for failure.	      *
 *						      *
 *****************************************************/

int
pwd_display(unamep)
char *unamep;
{
	extern struct tm *gmtime();
	struct tm *tmp;
	int when, maxweeks, minweeks;
	register int found = 0;

	if (unamep != NULL) {
		(void) fprintf(stdout,"%s  ",unamep);
		PRT_PWD(pwd->pw_passwd);
		if (*pwd->pw_age != NULL) {
			CNV_AGE();
			when >>= 12;
			if (when) {
				when = when * SEC_WEEK;
				tmp = gmtime (&when);
				(void) fprintf(stdout,"%.2d/%.2d/%.2d  ",(tmp->tm_mon + 1),tmp->tm_mday,tmp->tm_year);
			} else
				(void) fprintf(stdout,"00/00/00  ");
			(void) fprintf(stdout, "%d  %d ", minweeks * DAY_WEEK, maxweeks * DAY_WEEK);
		}
		(void) fprintf(stdout,"\n");
		return (SUCCESS);
	}
	while ((pwd = getpwent()) != NULL) {
		found++;
		(void) fprintf(stdout,"%s  ",pwd->pw_name);
		PRT_PWD(pwd->pw_passwd);
		if (*pwd->pw_age != NULL) {
			CNV_AGE();
			when >>= 12;
			if (when) {
				when = when * SEC_WEEK;
				tmp = gmtime (&when);
				(void) fprintf(stdout,"%.2d/%.2d/%.2d  ",(tmp->tm_mon + 1),tmp->tm_mday,tmp->tm_year);
			} else
				(void) fprintf(stdout,"00/00/00  ");
			(void) fprintf(stdout, "%d  %d ", minweeks * DAY_WEEK, maxweeks * DAY_WEEK);
		}
		(void) fprintf(stdout,"\n");
	}
	/* If password files do not have any entries or files are missing, 
	   return fatal error.
	*/

	if (found == 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FF);
		return (FATAL);
	}
	return (SUCCESS);
}

/******************************************************
 *						      *
 *  sp_display():  displays password attributes.      * 
 *  It takes user name as a parameter. If the user    *
 *  name is NULL then it displays password attributes *
 *  for all entries on the file. It returns 0 for     *
 * success and positive  number for failure.	      *
 *						      *
 *****************************************************/

int
sp_display(unamep)
char *unamep;
{
	extern struct tm *gmtime();
	struct tm *tmp;
	struct spwd *sp;
	struct passwd  *pwdp;
	register int found = 0;
	long lstchg;

	if (unamep != NULL) {
		if ((getpwnam(unamep) != NULL) && ((sp = getspnam(unamep)) != NULL)) {
			(void) fprintf(stdout,"%s  ",unamep);
			PRT_PWD(sp->sp_pwdp);
			PRT_AGE();
			(void) fprintf(stdout,"\n");
			return (SUCCESS);
		} 
		(void) fprintf(stderr, " %s: uname %s does not exist\n", prognamep, unamep);
		return (NOPERM);
	} 
	while ((pwdp = getpwent()) != NULL)
		if ((sp = getspnam(pwdp->pw_name)) != NULL) {
			found++;
			(void) fprintf(stdout,"%s  ",pwdp->pw_name);
			PRT_PWD(sp->sp_pwdp);
			PRT_AGE();
			(void) fprintf(stdout,"\n");
		}
	/* If password files do not have any entries or files are missing, 
	   return fatal error.
	*/

	if (found == 0) {
		(void) fprintf(stderr, "%s: %s\n", prognamep, MSG_FF);
		return (FATAL);
	}
	return (SUCCESS);
}
int
ckuid()
{
	if(uid != 0) {
		(void) fprintf(stderr,"%s: %s\n", prognamep, MSG_NP);
		return (retval = NOPERM); 
	} 
	return (SUCCESS);
}

