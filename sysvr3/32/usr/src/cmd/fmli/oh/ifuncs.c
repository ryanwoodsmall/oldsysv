/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/ifuncs.c	1.11"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wish.h"
#include "but.h"
#include "typetab.h"
#include "ifuncdefs.h"
#include "partabdefs.h"
#include "obj.h"
#include "optabdefs.h"
#include "retcds.h"
#include "windefs.h"
#include "terror.h"
#include "token.h"
#include "moremacros.h"

struct ott_entry *name_to_ott(), *dname_to_ott();
char Undel[] = "UNDELDIR";
static char *Arg;
struct ott_entry *Ott;
extern char nil[];


int (*Function[MAX_IFUNCS])();
void docv();
char *getepenv();

IF_badfunc()
{
	mess_temp("That operation is not available in FACE");
	return(FAIL);
}

int
IF_sh()
{ }

int
IF_rn(argv)
char *argv[];
{
	char msg[80];
	char oldname[DNAMESIZ];
	struct ott_entry *entry, *path_to_ott();
	char *filename();

	if (path_isopen(argv[0], "rename", TRUE))
		return(FAIL);
	if (ckperms(parent(argv[0]), 02) == FAIL)
		return(FAIL);
	if ((entry = path_to_ott(argv[0])) == NULL)
		return(FAIL);
	strcpy(oldname, entry->dname);
	if (ott_mv(entry, NULL, argv[1], TRUE) != FAIL) {
		sprintf(msg, "%s renamed as %s", oldname, argv[1]);
		mess_temp(msg);
		return(SUCCESS);
	} else {
		sprintf(msg, "RENAME of %s to %s FAILED, %s already exists", oldname, argv[1],argv[1]);
		mess_temp(msg);
		return(FAIL);
	}
}

int
IF_cp(argv)
char *argv[];
{
	return(mv_or_cp(FALSE, argv));
}

int
IF_mv(argv)
char *argv[];
{
	return(mv_or_cp(TRUE, argv));
}

static int
mv_or_cp(mv, argv)
bool mv;
char *argv[];
{
	char msg[80];
	char oldname[16], newname[16];
	char *display, path[PATHSIZ];
	struct ott_entry *entry, *path_to_ott();
	extern char *Wastebasket;
	char *filename();
	char	*path_to_title();

	working(TRUE);
	if (mv && path_isopen(argv[0], mv ? "move" : "copy", TRUE))
		return(FAIL);
	if (mv && ckperms(parent(argv[0]), 02) == FAIL)
		return(FAIL);
	if (ckperms(argv[1], 02) == FAIL)
		return(FAIL);
	if ((entry = path_to_ott(argv[1])) == NULL)
		return(FAIL);
	if (!(entry->objmask & CL_DIR)) {
		char msg[80];

		sprintf(msg, "%s is not a proper destination for %s", path_to_title(argv[1], NULL), mv ? "move" : "copy");
		mess_temp(msg);
		return(FAIL);
	}
	if ((entry = path_to_ott(argv[0])) == NULL)
		return(FAIL);
	display = strsave(entry->display);
	strcpy(oldname, entry->dname);
	if (argv[2] == NULL)
		strcpy(newname, entry->dname);
	else
		strcpy(newname, filename(argv[2]));
	if (ott_mv(entry, argv[1], newname, mv) != FAIL) {
		sprintf(path, "%s/%s", argv[1], newname);
		if ((entry = path_to_ott(path)) == NULL)
			return(FAIL);
		entry->display = display;
		ott_chg_display(entry);
		if (strncmp(path, Wastebasket, strlen(Wastebasket)) == 0)
			(void) odi_putkey(entry, Undel, parent(argv[0]));
		utime(path, NULL);	/* Touch the file */
		ott_mtime(entry);
		sprintf(msg, "%s %sed to the %s folder ", oldname, mv ? "mov" : "copi", path_to_title(argv[1], NULL));
		if (strcmp(oldname, newname) != 0) {
			strcat(msg, ", ");
			strcat(msg, oldname);
			strcat(msg, " renamed as ");
			strcat(msg, newname);
		}
		mess_temp(msg);
		return(SUCCESS);
	} else {
		sprintf(msg, "%s of %s failed to %s", 
				mv ? "move" : "copy", oldname, path_to_title(argv[1], NULL));
		mess_temp(msg);
		return(FAIL);
	}
}

int
IF_sc(argv)
char *argv[];
{
	if (scram(argv[0]) == FAIL)
		return(FAIL);
	return(SUCCESS);
}

int
IF_unsc(argv)
char *argv[];
{
	if (unscram(argv[0]) == FAIL)
		return(FAIL);
	return(SUCCESS);
}

int
IF_rm(argv)
char *argv[];
{
	token confirm(), t;
	struct ott_entry *ott;
	char path[PATHSIZ], name[ONAMESIZ], buf[BUFSIZ];
	extern char *Filecabinet, *Wastebasket;

	sprintf(buf, "Press return to delete %s:", filename(argv[0]));
	Arg=strsave(argv[0]);

	if ((ott = path_to_ott(argv[0])) == NULL)
		return(FAIL);

	if (strcmp(argv[0], Filecabinet) == 0) {
		mess_temp("You are not allowed to delete your Filecabinet");
		return(FAIL);
	} else if (strcmp(argv[0], Wastebasket) == 0) {
		char command[PATHSIZ + 100];

		if (path_isopen(argv[0], "delete", FALSE))
			return(FAIL);
		sprintf(command,"for i in %s/*; do /bin/rm -rf $i; done 1>/dev/null 2>/dev/null",Wastebasket);
		(void) system(command);
		mess_temp("All objects in WASTEBASKET have been permanently removed");
		return(SUCCESS);
	} else if (path_isopen(argv[0], "delete", TRUE)) {
		return(FAIL);
	} else if (strncmp(argv[0], Wastebasket, strlen(Wastebasket)) == 0) {
		Ott=ott;
		get_string(confirm, buf, "", 0, FALSE, "delete", "delete");
		return(SUCCESS);
	}

	strcpy(name, ott->name);
	get_string(confirm, buf, "", 0, FALSE, "delete", "delete");
	return(SUCCESS);
}

int
blow_away(ott)
struct ott_entry *ott;
{
	char command[10*PATHSIZ + 30];
	struct ott_entry *ott_next_part();
	int len;

	len = sprintf(command, "/bin/rm -rf %s ", ott_to_path(ott));
	while (ott = ott_next_part(ott))
		len += sprintf(command+len, "%s ", ott_to_path(ott));
	(void) system(command);
}

int
IF_unrm(argv)
char *argv[];
{
	struct ott_entry *ott;
	char *path, *odi_getkey();

	if (strncmp(argv[0], Wastebasket, strlen(Wastebasket)) != 0) {
		mess_temp("Undelete can only be used on objects in your WASTEBASKET");
		return(FAIL);
	}
	if ((ott = path_to_ott(argv[0])) == NULL)
		return(FAIL);
	if ((path = odi_getkey(ott, Undel)) == NULL) {
		mess_temp("Unable to find previous folder, use MOVE");
		return(FAIL);
	}
	(void) odi_putkey(ott, Undel, NULL);
	return(objop("move", NULL, argv[0], path, NULL));
}

int
IF_vi(argv)
char *argv[];
{
}

#define MAX_DESCRIP	24

int
redescribe(argv)
char *argv[];
{
	register int i, len;
	struct ott_entry *entry;
	char newdesc[MAX_DESCRIP+1];
	struct ott_entry *path_to_ott();

	char  *filename();

	if ((entry = path_to_ott(argv[0])) == NULL)
		return(FAIL);
	for (i = 1, len = 0; argv[i] && len < MAX_DESCRIP-1; i++)
		len += sprintf(newdesc+len, "%.*s ", MAX_DESCRIP-len-1, argv[i]);
	newdesc[len-1] = '\0';
	if (strchr(newdesc, '|')) {
		mess_temp("The character '|' is not allowed in description, try again");
		return(FAIL);
	}
	if (strcmp(newdesc,"\0") == 0) {
		mess_temp("Null strings are not allowed in description, try again");
		return(FAIL);
	}


	entry->display = strsave(newdesc);
	(void) ott_chg_display(entry);

	mess_temp(nstrcat(filename(argv[0]), " redescribed as ", newdesc, ".", NULL));
}

static char *
permsg(mode)
int mode;
{
	switch (mode) {
	case 01:
		return("search");
	case 02:
		return("modify");
	case 04:
		return("read");
	default:
		return("access");
	}
}

int
ckperms(path, mode)
char *path;
int mode;
{
	char	*path_to_title();

	if (access(path, mode) == FAIL) {
		mess_temp(nstrcat("You do not have permission to ", permsg(mode), " ",
			path_to_title(path), NULL));
		return(FAIL);
	}
	return(SUCCESS);
}

void
fcn_init()
{
	int IF_dvi(), IF_dir_open(), IF_dmv(), IF_dcp(), IF_drn();
	int	IF_sp();
	int IF_aed(), IF_acv(), IF_apr(), IF_aed();
	int IF_omopen();
	int IF_helpopen();
	int IF_ofopen();
	int IF_exec_open();

/* general purpose operations */

	Function[IF_VI] = IF_vi;
	Function[IF_SH] = IF_sh;
	Function[IF_CP] = IF_cp;
	Function[IF_RN] = IF_rn;
	Function[IF_MV] = IF_mv;
	Function[IF_RM] = IF_rm;
	Function[IF_UNRM] = IF_unrm;
	Function[IF_SC] = IF_sc;
	Function[IF_UNSC] = IF_unsc;

/* operations specific to ascii files */

	Function[IF_ACV] = IF_acv;
	Function[IF_AED] = IF_aed;
	Function[IF_APR] = IF_apr;

/* operations specific to menu objects */

	Function[IF_MENOPEN] = IF_omopen;

/* operations specific to help objects */

	Function[IF_HLPOPEN] = IF_helpopen;

/* operations specific to form objects */

	Function[IF_FRMOPEN] = IF_ofopen;

/* operations specific to file folders */

	Function[IF_DED] = IF_dir_open;
	Function[IF_DVI] = IF_dvi;
	Function[IF_DMV] = IF_dmv;
	Function[IF_DCP] = IF_dcp;
	Function[IF_DRN] = IF_drn;

/* operations specific to executables */

	Function[IF_EED] = IF_exec_open;

/* illegal function */

	Function[IF_BADFUNC] = IF_badfunc;

	return;
}

static token
confirm(s, t)
char *s;
token t;
{
	extern char *Wastebasket;
	char buf[BUFSIZ];

	sprintf(buf, "Press return to delete %s:", filename(Arg));

	if (t == TOK_CANCEL)
		return TOK_NOP;

	if (t == TOK_SAVE && *s == NULL) {
		if (strncmp(Arg, Wastebasket, strlen(Wastebasket)) == 0) {
			blow_away(Ott);
			mess_temp(nstrcat("Object ", Ott->dname,
					" permanently removed from WASTEBASKET", NULL));
			ar_checkworld(TRUE);
			return TOK_NOP;
		}

		if (objop("move", NULL, Arg, Wastebasket, NULL) == FAIL) 
			return TOK_NOP;
		else
			ar_checkworld(TRUE);
	}
	else if (*s != NULL) {
		get_string(confirm, buf, "", 0, FALSE, "delete", "delete");
		mess_temp("Please re-enter value");
	}

	return TOK_NOP;
}
