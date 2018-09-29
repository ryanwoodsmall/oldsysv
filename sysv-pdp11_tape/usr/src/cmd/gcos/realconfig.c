/*	@(#)realconfig.c	1.3	*/
/*
 *  config -- update station-id's and device names
 *	from file gcosconfig.
 *	gcosconfig file format:
 *		one line per variable to be changed
 *		tokens are separated by space, tab, comma, quote, etc.
 *		first token must match name of variable (vartab[] below)
 *		subsequent tokens are string values of variable
 *		"\" means NULL string or end of array.
 */

#include <stdio.h>
#include <pwd.h>

#include "gcos.h"

/* The following parameters are now defined in "gcos.h".
 *	but may have values changed by config. */
char	ph_id[3] = PHONE_ID;	/*PHONE_ID must be defined in header file gcos.h*/
char	*phone_id = ph_id;
char	fg_id[3] = FGET_ID;	/*FGET_ID must be defined in header file gcos.h*/
char	*fget_id = fg_id;
char	*du = DEVDU;		/*dataphone or VPM device*/
char	*dn = DEVDN;		/*ACU device*/
char	*numbers[6] = NUMBERS;	/*phone numbers to dial GCOS*/
int	NNUMB = sizeof(numbers)/sizeof(char *);
char	*console = CONSOLE;	/*device for manual dial message*/

extern	char	dname[];

#define	WRMODE	2
#define	DELIM	" \t:;,=.\"'\n{}"
char	PARAMS[] = "/usr/lib/gcosconfig";
char	logconf[] = "/usr/spool/dpd/logconfig";

struct	vart{
	char	*name;
	char	**loc;
	int	size;
};
struct vart vartab[] = {
	{ "CONS",	&console,	1 },
	{ "DEVDN",	&dn,		1 },
	{ "DEVDU",	&du,		1 },
	{ "DEVVPM",	&du,		1 },
	{ "DN",		&dn,		1 },
	{ "DU",		&du,		1 },
	{ "FGET",	&fget_id,	1 },
	{ "NUMB",	numbers,	sizeof(numbers)/sizeof(char *) },
	{ "PHONE",	&phone_id,	1 },
	{ "VPM",	&du,		1 },
};
struct vart *lookup();
struct passwd *getpwuid();
char	*strtok(), *strchr(), *malloc();
char	*strcpy();

config()
{
#define	LLINE	80
	char line[LLINE];
	FILE *fp;
	int i;
	char *p, *val, *name;
	struct vart *vp;

	if((fp = fopen(PARAMS, "r")) == NULL)
		return(0);
	while(fgets(line, LLINE, fp) != NULL){
#ifdef DEBUG
		fprintf(stderr, "parameter line: %s", line);
#endif
		if(strchr(line, '\n') == NULL){
			logpar("line too long: %s", line);
			return(1);
		}
		if((name = strtok(line, DELIM)) == NULL)
			continue;
#ifdef DEBUG
		fprintf(stderr, "name = '%s'\n", name);
#endif
		if((vp = lookup(name)) == (struct vart *)NULL){
			logpar("Unknown name: '%s'.", name);
			continue;
		}
		for (i = 0; i <= vp->size; i++){
			if ((val = strtok(0, DELIM)) == NULL)
				break;
#ifdef DEBUG
			fprintf(stderr, "value = '%s'\n", val);
#endif
			if(i >= vp->size){
				logpar("More than %d values", i);
				break;
			}
			if(*val == '\\')
				(vp->loc)[i] = 0;
			else{
				p = malloc(strlen(val) + 1);
				strcpy(p, val);
				(vp->loc)[i] = p;
			}
			logpar("%s[%d] = '%s'", name, i, (vp->loc)[i]);
		}
	}
	fclose(fp);
	return(0);
}


struct vart
*lookup(name)
char	*name;
{
	int i, j;
	char *p;
	struct vart *vp;

	for (i = 0; name[i]; i++)
		if(name[i] >= 'a' && name[i] <= 'z')
			name[i] -= 'a' - 'A';
	for(i = 0; i < sizeof(vartab)/sizeof(vartab[1]); i++){
		vp = &vartab[i];
		p = vartab[i].name;
		j = 0;
		while(p[j]){
			if(p[j] > name[j])
				return(NULL);
			if(p[j] < name[j])
				break;
			j++;
		}
		if(p[j] == 0 && j != 0)
			return(vp);
	}
	return(NULL);
}


/*VARARGS*/
logpar(s, a1, a2, a3, a4)
char	*s;
int	a1, a2, a3, a4;
{
	long static tb;
	register i;
	FILE *f;
	struct passwd *pwp;

#ifdef	DEBUG
	fprintf(stderr, s, a1, a2, a3, a4);
	fprintf(stderr, "(logpar)\n"); /* PRINTF */
#endif
	if(access(logconf, WRMODE) != 0)
		return;
	if((f = fopen(logconf, "a")) == NULL)
		return;
	time(&tb);
	fprintf(f, "%.19s:%s:", ctime(&tb), dname);
	fprintf(f, s, a1, a2, a3, a4);
	if((pwp = getpwuid(getuid())) != NULL){
		putc(' ', f);  putc(' ', f);
		i = 0;
		while(pwp->pw_name[i])
			putc(pwp->pw_name[i++], f);
	}
	putc('\n', f);
	fclose(f);
}
