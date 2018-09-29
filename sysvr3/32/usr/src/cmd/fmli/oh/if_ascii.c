/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_ascii.c	1.6"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wish.h"
#include "but.h"
#include "typetab.h"
#include "obj.h"
#include "retcds.h"
#include "procdefs.h"

char *ott_to_path(), *strcpy();


int
IF_acv(argv)
char *argv[];
{
	return(0);
}

/*ARGSUSED*/
int
IF_aed(argv)
char *argv[];
{
	char title[PATHSIZ];
	struct ott_entry *ott, *path_to_ott();
	char	*path_to_title();

	strcpy(title, "Suspended ");
	strcat(title, path_to_title(argv[0], NULL));
	proc_open(PR_ERRPROMPT, title, NULL, "$EDITOR", argv[0], NULL);
	if ((ott = path_to_ott(argv[0])) != NULL)
		ott_mtime(ott);
	return(SUCCESS);
}

int
IF_apr(argv)
char *argv[];
{
	struct ott_entry *entry, *path_to_ott();
	struct stat buf;
	int ret;

	if ((entry = path_to_ott(argv[0])) == NULL)
		return(FAIL);

	if ((ret=stat(argv[0],&buf))== 0)
		if (buf.st_size == 0) {
			mess_temp("Cannot print zero length files");
			return(FAIL);
		}

	return(obj_print(entry, NULL, NULL));
}

int
obj_print(entry, draftstyle, prclass)
struct ott_entry *entry;
char *draftstyle;
char *prclass;
{
	FILE *pinfo;
	char prname[PATHSIZ];
	int i, retval;
	char buf[PATHSIZ];
	char *command[10], objtypebuf[20], titlebuf[40], jobclassbuf[40],
			lponbuf[80], lpoffbuf[80], draftbuf[20], pdefbuf[80];
	char jobclass[4];		/* might as well make it 4 since it gets aligned*/
	char *pdefs;

	struct ott_entry *name_to_ott();
	char *odi_getkey();
	static char Pdefaults[] = "PRINTOPTS";
	static char Printopt[] = "printopt";

	if (((pdefs = odi_getkey(entry, Pdefaults))) != NULL && *pdefs)
		sprintf(jobclass, "%c", *pdefs);
	else if (prclass != NULL)
		strcpy(jobclass, prclass);
	else if (entry->objmask & CL_DOC) {
		strcpy(jobclass, "d");
	} else if (entry->objmask & CL_MAIL)
		strcpy(jobclass, "m");
	else
		strcpy(jobclass, "d");

#ifdef _DEBUG
	_debug(stderr, "PDEFAULTS=%s jobclass=%s\n", pdefs, jobclass);
#endif

	i = 0;
	command[i++] = "$VMSYS/OBJECTS/Menu.print";
	if (pdefs && *pdefs) {
		sprintf(pdefbuf, "-u%s", pdefs);
		command[i++] = pdefbuf;
	}
/***********
	sprintf(jobclassbuf, "-j%s", jobclass);
	command[i++] = jobclassbuf;
***********/
	if (draftstyle) {
		sprintf(draftbuf, "-F%s", draftstyle);
		command[i++] = draftbuf;
	}
	sprintf(titlebuf, "-t%s", entry->dname);
	command[i++] = titlebuf;
	sprintf(objtypebuf, "-f%s", entry->objtype);
	command[i++] = objtypebuf;
	command[i++] = ott_to_path(entry);
	command[i++] = NULL;

	objopv("OPEN", "MENU", command);
	
	strcpy(prname, entry->dirpath);
	strcat(prname, "/.P");
	strcat(prname, entry->name);

	if ((pinfo = fopen(prname, "r")) != NULL) {
		if (fgets(buf, BUFSIZ, pinfo) != NULL) {
			buf[strlen(buf)-1] = '\0';
			ott_lock_dsk(entry->dirpath);
			if (entry = name_to_ott(entry->name)) {
#ifdef _DEBUG
				_debug(stderr, "putting new printer info:%s\n",buf);
#endif
				odi_putkey(entry, Pdefaults, buf);
				ott_dirty();
				ott_synch(FALSE);
			}
#ifdef _DEBUG
			_debug(stderr, "PRINTDEFS: %s\n", buf);
#endif
		}
		(void) fclose(pinfo);
		(void) unlink(prname);
	}
#ifdef _DEBUG
	 else
		_debug(stderr, "PRINT SAVE FAIL: jobclass=%s prname=%s\n", jobclass, prname);
#endif

	return(SUCCESS);
}
