/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlsadmin:util.c	1.6"

#include "fcntl.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "nlsadmin.h"

 /*
  * utility routines for nlsadmin.  adapted from dmk's 
  * listener code by sar.
  */

extern char	*arg0;
extern int	Errno;
extern char	serv_str[];
extern char	flag_str[];
extern char	mod_str[];
extern char	path_str[];
extern char	cmnt_str[];
extern char	*errmalloc;

/*
 * read_dbf:    Parses the next non-comment line into the
 *	      globals serv_str, flag_str, name_str, mod_str, and path_str.
 *	      Skips 'ignore' lines.
 *
 *	      Returns:	line number read or -1 on EOF or -2 on error.
 */

int
read_dbf(fp)
FILE *fp;
{
	int linenum = 0;
	char *bp, *start;
	char *clean_line();

	if ((start = (char *)malloc(DBFLINESZ)) == NULL)
		error(errmalloc, NLSMEM);
	for(;;) {
		bp = start;
		if (!fgets(bp, DBFLINESZ, fp))  {
			if (!feof(fp)) {
				fprintf(stderr,"%s: error in reading data base file\n",arg0);
				Errno = NLSRDFIL;
				return(-2);
			}
			free(start);
			return(-1);
		}
		if (*(bp+strlen(bp)-1) != '\n')  {  /* terminating newline? */
			fprintf(stderr,"%s: data base file: error on line %d\n",arg0,linenum);
			free(start);
			Errno = NLSDBF;
			return(-2);
		}
		cmnt_str[0] = (char)0;
		bp = clean_line(bp);
		if (strlen(bp))	 /* anything left?       */
			if (parse_line(bp))
				break;
		linenum++;
	}
	free(start);
	return(linenum);
}

/*
 * parse a non-white space line
 *	      0 = error;
 *	      1 = success;
 *
 *      non-null lines have the following format:
 *
 *      service_code: flags: serv_name: modules_to_push: cmd_line # comments
 */

int
parse_line(bp)
register char *bp;
{
	char *servp, *flagp, *modp, *pathp;
	int length;
	char *from, *to;

	length = strlen(bp);
	if (!(servp = strtok(bp, DBFTOKENS))) {
		return(0);
	}
	if (!(flagp = strtok((char *)0, DBFTOKENS)))  {
		return(0);
	}
	if (!(modp = strtok((char *)0, DBFTOKENS))) {
		return(0);
	}
	pathp = modp + strlen(modp) + 1;      /* go past separator */
	if ((pathp - bp) >= length)
		return(0);
	strcpy(serv_str, servp);
	strcpy(flag_str, flagp);
	mod_str[0] = (char)0;
	for (from = modp, to = mod_str; *from; ) {
		if (strncmp(from, DBFNULL, strlen(DBFNULL)) == 0) {
			if ((from == modp) || (*(from - 1) == ','))
				from += strlen(DBFNULL);
			else
				*to++ = *from++;
		} else {
			*to++ = *from++;
		}
	}
	*to = (char)0;
	if (mod_str[strlen(mod_str) - 1] == ',')
		mod_str[strlen(mod_str) - 1] = (char)0;
	strcpy(path_str, pathp);
	return(1);
}

char *
clean_line(s)
char *s;
{
	char *ns;
	char *cs;

	*(s + strlen(s) -1) = (char)0; /* delete newline	*/
	if (strlen(s) && (ns = strchr(s, DBFCOMMENT))) {
		*ns++ = (char)0;	/* delete comments      */
		if (strlen(ns)) {
			cs = ns + strlen(ns) -1;
			while ((cs != ns) && (isspace(*cs))) {
				*cs = (char)0;
				--cs;
			}
			while (*ns)
				if (isspace(*ns))
					++ns;
				else
					break;
		}
		if(strlen(ns))
			strcpy(cmnt_str, ns);
	}
	if (!strlen(s))
		return(s);
	ns = s + strlen(s) - 1; /* s->start; ns->end    */
	while ((ns != s) && (isspace(*ns)))  {
		*ns = (char)0;	  /* delete terminating spaces */
		--ns;
	}
	while (*s)		      /* del beginning white space*/
		if (isspace(*s))
			++s;
		else
			break;
	return(s);
}

