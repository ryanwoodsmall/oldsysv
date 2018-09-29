/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uucp:sysfiles.c	1.4"
#include <stdio.h>
#include "sysfiles.h"


/*
 * manage systems files (Systems, Devices, and Dialcodes families).
 *
 * also manage new file Devconfig, allows per-device setup.
 * present use is to specify what streams modules to push/pop for
 * AT&T TLI/streams network.  see tssetup() in interface.c for details.
 *
 * N.B.: this module does not call ASSERT; it can be used outside of uucp
 *
 * TODO:
 *    call bsfix()?  it would belie the comment above.
 *    combine the 3 versions of everything (sys, dev, and dial) into one.
 *    allow arbitrary classes of service.
 *    need verifysys() for uucheck.
 *    process systems=!nameserv.
 *    pass sysname (or 0) to getsysline().  (might want reg. exp. or NS processing 
 */

/* private variables */
static void tokenize(), serviceparse(), setfile(), setioctl();
static int servicematch(), nextdialers(), nextsystems(), rdsysfile(), scansys();

static char *Systems[64] = {0};	/* should by malloced? */
static char *Devices[64] = {0};	/* should be malloced? */
static char *Dialers[64] = {0};	/* should be malloced? */
static char *Pops[64] = {0};	/* should be malloced? */
static char *Pushes[64] = {0};	/* should be malloced? */

static int nsystems;
static int ndevices;
static int ndialers;
static int npops;
static int npushes;

static FILE *fsystems;
static FILE *fdevices;
static FILE *fdialers;

/* this should be dynamically allocated, but i'm lazy and in a hurry */
#define NTOKENS 16
static char *tokens[NTOKENS], **tokptr;

/* export these */
extern void sysreset(), devreset(), dialreset(), setdevcfg();
extern char *strsave();
extern int setservice();

/* import these */
extern char *strcpy(), *strtok(), *strchr(), *malloc(), *strsave();
extern int errno;

/*
 * read Sysfiles.  allow multiple entries for a given service, allow a service
 * type to describe resources more than once, e.g., systems=foo:baz systems=bar.
 * return 1 if Systems[0] is readable, 0 otherwise.
 */
static int
scansys(service)
	char *service;
{	FILE *f;
	char *tok, buf[BUFSIZ];

	Systems[0] = Devices[0] = Dialers[0] = 0;
	if ((f = fopen(SYSFILES, "r")) != 0) {
		while (rdsysfile(f, buf)) {
			tok = strtok(buf, " \t");
			if (servicematch("service=", tok, service)) {
				tokenize();
				serviceparse();
			}
		}
	}

	if (Systems[0] == 0) {
		Systems[0] = strsave(SYSTEMS);
		Systems[1] = 0;
	}
	if (Devices[0] == 0) {
		Devices[0] = strsave(DEVICES);
		Devices[1] = 0;
	}
	if (Dialers[0] == 0) {
		Dialers[0] = strsave(DIALERS);
		Dialers[1] = 0;
	}
	return(eaccess(Systems[0], 4));

}


/*
 * read Devconfig.  allow multiple entries for a given service, allow a service
 * type to describe resources more than once, e.g., systems=foo:baz systems=bar.
 */
static void
scancfg(service, device)
	char *service, *device;
{	FILE *f;
	char *tok, buf[BUFSIZ];

	npops = npushes = 0;
	Pops[0] = Pushes[0] = 0;
	if ((f = fopen(DEVCONFIG, "r")) != 0) {
		while (rdsysfile(f, buf)) {
			tok = strtok(buf, " \t");
			if (servicematch("service=", tok, service)) {
				tok = strtok((char *)0, " \t");
				if ( servicematch("device=", tok, device)) {
					tokenize();
					serviceparse();
				}
			}
		}
	}

	return;

}

static int
rdsysfile(f, line)
	FILE *f;
	char *line;
{	char *lptr, *lend;

	lptr = line;
	while (fgets(lptr, (line + BUFSIZ) - lptr, f) != NULL) {
		lend = lptr + strlen(lptr);
		if (lend == lptr || lend[-1] != '\n')	/* line too long! */
			break;
		*--lend = 0;
		if (lend[-1] != '\\')
			break;
		/* continuation */
		lend[-1] = ' ';
		lptr = lend;
	}
}

static int
servicematch(label, line, service)
	char *label, *line, *service;
{	char *lend;

	if (strncmp(line, label, sizeof(label)-1)) {
		return(0);	/* probably a comment line */
	}
	line += strlen(label);
	if (*line == 0)
		return(0);
	while ((lend = strchr(line, ':')) != 0) {
		*lend = 0;
		if (strcmp(line, service) == 0)
			return(1);
		line = lend+1;
	}
	return(strcmp(line, service) == 0);
}

static void
tokenize()
{	char *tok;

	tokptr = tokens;
	while ((tok = strtok((char *) 0, " \t")) != 0) {
		*tokptr++ = tok;
		if (tokptr - tokens >= NTOKENS)
			break;
	}
}

static void
serviceparse()
{	char **line, *equals;

	for (line = tokens; line - tokens < NTOKENS && *line; line++) {
		equals = strchr(*line, '=');
		if (equals == 0)
			continue;	/* may be meaningful someday? */
		*equals = 0;
		/* ignore entry with empty rhs */
		if (*++equals == 0)
			continue;
		if (strcmp(*line, "systems") == 0)
			setfile(Systems, equals);
		else if (strcmp(*line, "devices") == 0)
			setfile(Devices, equals);
		else if (strcmp(*line, "dialers") == 0)
			setfile(Dialers, equals);
		else if (strcmp(*line, "pop") == 0)
			setioctl(Pops, equals);
		else if (strcmp(*line, "push") == 0)
			setioctl(Pushes, equals);
		else
			;	/* for future use */
	}
}

static void
setfile(type, line)
	char **type, *line;
{	char **tptr, *tok;
	char expandpath[BUFSIZ];

	if (*line == 0)
		return;
	tptr = type;
	while (*tptr)
		tptr++;		/* concatenate multiple entries */
	for (tok = strtok(line, ":"); tok; tok = strtok((char *) 0, ":")) {
		expandpath[0] = '\0';
		if ( *tok != '/' )
			sprintf(expandpath, "%s/", SYSDIR);
		strcat(expandpath, tok);
		if (eaccess(expandpath, 4) != 0)
			continue;
		*tptr = strsave(expandpath);
		tptr++;
	}
}

static void
setioctl(type, line)
	char **type, *line;
{	char **tptr, *tok;

	if (*line == 0)
		return;
	tptr = type;
	while (*tptr)
		tptr++;		/* concatenate multiple entries */
	for (tok = strtok(line, ":"); tok; tok = strtok((char *) 0, ":")) {
		*tptr = strsave(tok);
		tptr++;
	}
}

setservice(service)
	char *service;
{
	return(scansys(service));
}

void
setdevcfg(service, device)
	char *service, *device;
{
	scancfg(service, device);
	return;
}

void
sysreset()
{
	if (fsystems)
		fclose(fsystems);
	fsystems = 0;
	nsystems = 0;
	devreset();
}

void		
devreset()
{
	if (fdevices)
		fclose(fdevices);
	fdevices = 0;
	ndevices = 0;
	dialreset();
}

void		
dialreset()
{
	if (fdialers)
		fclose(fdialers);
	fdialers = 0;
	ndialers = 0;
}


static int
nextsystems()
{
	devreset();
	dialreset();

	if (fsystems) {
		(void) fclose(fsystems);
		nsystems++;
	} else if (Systems[0] == 0)
		return(0);
	for ( ; Systems[nsystems]; nsystems++)
		if ((fsystems = fopen(Systems[nsystems], "r")) != 0)
			return(1);
	return(0);
}

getsysline(buf, len)
	char *buf;
{
	if (Systems[0] == 0)
		/* uninitialized via setservice(), so default to service=uucico */
		(void) setservice("uucico");

	/* initialize devices and dialers whenever a new line is read from systems */
	devreset();
	if (fsystems == 0)
		if (nextsystems() == 0)
			return(0);
	for(;;) {
		if (fgets(buf, len, fsystems) != NULL)
			return(1);
		if (nextsystems() == 0)
			return(0);
	}
}

static int
nextdevices()
{
	if (fdevices) {
		(void) fclose(fdevices);
		ndevices++;
	} else if (Devices[0] == 0)
		return(0);
	
	for ( ; Devices[ndevices]; ndevices++)
		if ((fdevices = fopen(Devices[ndevices], "r")) != 0)
			return(1);
	return(0);
}
		
getdevline(buf, len)
	char *buf;
{
	if (fdevices == 0)
		if (nextdevices() == 0)
			return(0);
	for(;;) {
		if (fgets(buf, len, fdevices) != NULL)
			return(1);
		if (nextdevices() == 0)
			return(0);
	}
}

static int
nextdialers()
{
	if (fdialers) {
		(void) fclose(fdialers);
		ndialers++;
	} else if (Dialers[0] == 0)
		return(0);
	
	for ( ; Dialers[ndialers]; ndialers++)
		if ((fdialers = fopen(Dialers[ndialers], "r")) != 0)
			return(1);
	return(0);
}
		
getdialline(buf, len)
	char *buf;
{
	if (fdialers == 0)
		if (nextdialers() == 0)
			return(0);
	for(;;) {
		if (fgets(buf, len, fdialers) != NULL)
			return(1);
		if (nextdialers() == 0)
			return(0);
	}
}

getpop(buf, len, optional)
	char *buf;
	int len, *optional;
{
	int slen;

	if ( Pops[0] == 0 || Pops[npops] == 0 )
		return(0);

	/*	if the module name is enclosed in parentheses,	*/
	/*	is optional. set flag & strip parens		*/
	slen = strlen(Pops[npops]) - 1;
	if ( Pops[npops][0] == '('  && Pops[npops][slen] == ')' ) {
		*optional = 1;
		Pops[npops][slen] = '\0';
		strncpy(buf, &(Pops[npops++][1]), len);
	} else {
		*optional = 0;
		strncpy(buf, Pops[npops++], len);
	}
	return(1);
}

getpush(buf, len)
	char *buf;
	int len;
{
	if ( Pushes[0] == 0 || Pushes[npushes] == 0 )
		return(0);
	strncpy(buf, Pushes[npushes++], len);
	return(1);
}

/*	system files access for REAL user id	*/
sysaccess()
{
	return(access(Systems[nsystems], 4));
}

