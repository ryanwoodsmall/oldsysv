/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pwunconv:pwunconv.c	1.5"
/*  pwunconv.c  */
/*  Conversion aid to copy appropriate fields from the	*/
/*  shadow file to the password file.			*/
							
#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <shadow.h>
#include <signal.h>
#include <string.h>

#define PRIVILEGED	0			/* privileged id */	
#define MAX_MN		63	/* max # of weeks allowed for max and min*/
#define MAX_LSTCHG      4095 	/* 64 * 64 -1 (2 base 64 chars)          */
                             	/* max # of weeks allowed for lastchanged*/
#define DAY_WEEK	7			/* days per week */
#define SEC_WEEK	(24 * DAY_WEEK * 60 * 60)	/* seconds per week*/
#define ENTRY_LENGTH	512	/*maximum length of an /etc/passwd entry */

/* exit  code */
#define SUCCESS	0	/* succeeded */
#define NOPERM	1	/* No permission */
#define BADSYN	2	/* Incorrect syntax */
#define FMERR	3	/* File manipulation error */
#define FATAL	4	/* Old file can not be recover */
#define FBUSY	5	/* Lock file busy */
#define NOSHAD	6	/* Shadow file does not exist  */
#define TOOLONG	7	/* Entry exceeds 512-1 limit  */

#define	DELPTMP()	(void) unlink(PASSTEMP)

char *prognamp;

char ctp[128];

main(argc,argv)
int argc;
char **argv;
{
	extern	char	*strcat();
	extern	int	errno;
	extern	struct	passwd *getpwent();
	extern	struct  spwd  *getspnam();
	extern	char	*l64a();
	void	f_err(), f_miss(), no_recover(), no_convert();
	struct  passwd  *pwdp;
	struct	spwd	*sp; 		/* default entry */
	struct	stat buf;
	FILE	*tp_fp;
	ushort  pwd_gid, pwd_uid, i;
  	int 	aging;
	time_t	now, max_wks, min_wks, lst_wks;
 
	/* number of weeks since 1970 */
	now = time ((long * ) 0) / SEC_WEEK;

	prognamp = argv[0];

	/* only PRIVILEGED can execute this command */
	if (getuid() != PRIVILEGED) {
		(void)fprintf(stderr, "%s: Permission denied.\n", prognamp);
		exit(NOPERM);
	}

	/* No argument can be passed to the command*/
	if (argc > 1) {
		(void)fprintf(stderr, "%s: Invalid command syntax.\n", prognamp);
		(void)fprintf(stderr, "Usage: %s\n", prognamp);
		exit(BADSYN);
	}

	
	/* lock file so that only one process can read or write at a time */
	if (lckpwdf() < 0) { 
		(void)fprintf(stderr, "%s: Password file(s) busy.  Try again later.\n", prognamp);
		exit(FBUSY);
	}

	/* If shadow file doesn't exist, no conversion can be done. */
	   if (access (SHADOW, 0) != 0) {
		(void)fprintf(stderr, "%s: Shadow file does not exist\n", prognamp);
		(void)ulckpwdf();
		exit(NOSHAD);
	  } 

	 /* All signals will be ignored during the execution of pwunconv */
	for (i=1; i < NSIG; i++) 
		sigset(i, SIG_IGN);
 
	/* reset errno to avoid side effects of a failed */
	/* sigset (e.g., SIGKILL) */
	errno = 0;

	/* check the file status of the password file */
	/* get the gid of the password file */
	if (stat(PASSWD, &buf) < 0) {
		(void) f_err();
		exit(FMERR);
	} 
	pwd_gid = buf.st_gid;
	pwd_uid = buf.st_uid;
 
	/* mode for the password file should be 444 or less */
	umask(~(buf.st_mode & 0444));

	/* open temporary password file */
	if ((tp_fp = fopen(PASSTEMP,"w")) == NULL) {
		(void) f_err();
		exit(FMERR);
	}

	   if (chown(PASSTEMP, pwd_uid, pwd_gid) < 0) {
		DELPTMP();
		(void) f_err();
		exit(FMERR);
	}



	/* Reads the password file. If a correspondig entry	*/
	/* exists in the shadow file, updates the password	  */
	/* field in the password file from the shadow file.	*/
 
	while ((pwdp = getpwent()) != NULL) {
		if ((sp = getspnam(pwdp->pw_name)) != NULL ) {

			/* Copy the password in the shadow file to */
			/* the password file. */
			pwdp->pw_passwd = sp->sp_pwdp;

			/* If a login has aging info, convert aging from */
			/* days into weeks. Adds 6 to each aging field */
			/* before it is divided by 7 to reduce the impact */
			/* of integer division. */
			/* Due to password (aging) base 64 (0 thru 63) scheme,*/
			/* the maximum week allowed for max and min is 63. */

			/* aging is on if max >= 0 */
		   	if (sp->sp_max >= 0) {
				sp->sp_max += (DAY_WEEK - 1);
				max_wks = sp->sp_max / DAY_WEEK;
				max_wks = (max_wks > MAX_MN ? MAX_MN: max_wks); 

				/* if aging is on, min < 0, make min 0.*/
				if (sp->sp_min < 0) 
					sp->sp_min = 0;
				sp->sp_min += (DAY_WEEK - 1);
				min_wks = sp->sp_min / DAY_WEEK;
				min_wks = (min_wks > MAX_MN ? MAX_MN: min_wks); 

				/* if the lastchanged field exceeds */
				/* the 4095 week limit, make it  equal */
				/* to 4095*/
		   		if (sp->sp_lstchg > 0) {
					sp->sp_lstchg += (DAY_WEEK - 1);
					lst_wks = sp->sp_lstchg / DAY_WEEK;
					lst_wks = (lst_wks > MAX_LSTCHG ? MAX_LSTCHG: lst_wks); 
					/* Concatenate aging fields to an */
					/* integer */
					aging = max_wks + (min_wks <<6) + (lst_wks <<12);
					/* convert aging from long to */
					/* base 64 ascii string for */
					/* use in the password file. */
					pwdp->pw_age = l64a(aging);
				} else {
					aging = max_wks + (min_wks <<6); 
					/* "." is as same as "..." */
					if (aging == 0) 
						pwdp->pw_age = ".";
					else
						pwdp->pw_age = l64a(aging);
				}
			} else {
				/* special case: if aging is off  */
				/* but lstchg field contains a 0, password */
			  	/* field will contain a "." and not null. */
				/* Users will be forced to change password */
			  	/* at the next login. */ 
				if (sp->sp_lstchg == 0) 
					pwdp->pw_age = ".";
				else 
					pwdp->pw_age = "";
			}


			/* Ensure that the combined length of the individual */
			/* fields will fit in a passwd entry. */
			/* The first 1 accounts for the "," */
			/*  if aging is present. */
			/* The second 1 accounts for the */
			/* newline and the 6 accounts for the colons (:'s) */
			if ((strlen  (pwdp->pw_name) + 
				strlen (pwdp->pw_passwd) +
				sprintf (ctp, "%d", pwdp->pw_uid) +
				sprintf (ctp, "%d", pwdp->pw_gid) +
				strlen (pwdp->pw_age) + (*pwdp->pw_age ? 1 : 0) + 1 +
				strlen (pwdp->pw_comment) +
				strlen (pwdp->pw_dir)	+
				strlen (pwdp->pw_shell) + 6) > (ENTRY_LENGTH-1) ) {
				DELPTMP();
				(void)fprintf(stderr,"%s: Entry for %s too long.  Conversion not done.\n",prognamp, pwdp->pw_name);
  				(void) ulckpwdf();
				exit(TOOLONG);
			}
		}
		/* Every entry in the password file,  whether a */
		/* corresponding entry exists in the shadow file or not, */
		/* should be written to the temporary password file. */
		if ((putpwent(pwdp,tp_fp)) != 0 ) {
			(void) no_convert();
			exit(FMERR);
 		}
	}

	(void)fclose(tp_fp);

	/* delete old password file if it exists*/
	if (unlink (OPASSWD) && (access (OPASSWD, 0) == 0)) {
		(void) no_convert();
		exit(FMERR);
	}

	/* link password file to old password file */
	if (link(PASSWD, OPASSWD) < 0) {
		(void) no_convert();
		exit(FMERR);
	}

	/* delete password file */
	if (unlink(PASSWD) < 0) {
		(void) no_convert();
		exit(FMERR);
	}

	/* link temporary password file to password file */
	if (link(PASSTEMP, PASSWD) < 0) {
		/* link old password file to password file */
		if (link(OPASSWD, PASSWD) < 0) {
			(void) no_recover();
			exit(FATAL);
 		}
		(void) no_convert();
		exit(FMERR);
	}
 

	/* delete old shadow password file if it exists */
	if  (access (OSHADOW, 0) == 0) {
		if (unlink (OSHADOW) == 0)
		/* link shadow password file to old shadow password file */
			link(SHADOW, OSHADOW);
	}
	else
		link(SHADOW, OSHADOW);

	/* delete shadow password file */
	if (unlink(SHADOW) != 0) {
		if (unlink(PASSWD) || link (OPASSWD, PASSWD)) {
			(void) no_recover();
			exit(FATAL);
		}
		(void) no_convert();
		exit(FMERR);
	}

 
	DELPTMP();
	(void) ulckpwdf();
	exit(0);
}
 
void
no_recover()
{
	DELPTMP();
	(void) f_miss();
}

void
no_convert()
{
	DELPTMP();
	(void) f_err();
}
 
void
f_err()
{
	(void)fprintf(stderr,"%s: Unexpected failure. Conversion not done.\n",prognamp);
  	(void) ulckpwdf();
}

void
f_miss()
{
	(void)fprintf(stderr,"%s: Unexpected failure. Password file(s) missing.\n",prognamp);
  	(void) ulckpwdf();
}
