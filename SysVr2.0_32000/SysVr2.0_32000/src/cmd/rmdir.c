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
/*	@(#)rmdir.c	1.2	*/
/*
 * Remove directory
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

int	Errors = 0;
char	*strrchr();
char	*strcat();
char	*strcpy();

main(argc,argv)
int argc;
char **argv;
{

	if(argc < 2) {
		fprintf(stderr, "rmdir: usage: rmdir dirname ...\n");
		exit(2);
	}
	while(--argc)
		rmdir(*++argv);
	exit(Errors?2:0);
}

rmdir(d)
char *d;
{
	int	fd;
	char	*np, name[500];
	struct	stat	st, cst;
	struct	direct	dir;

	strcpy(name, d);
	if((np = strrchr(name, '/')) == NULL)
		np = name;
	if(stat(name,&st) < 0) {
		fprintf(stderr, "rmdir: %s non-existent\n", name);
		++Errors;
		return;
	}
	if (stat(".", &cst) < 0) {
		fprintf(stderr, "rmdir: cannot stat \".\"\n");
		++Errors;
		return;
	}
	if((st.st_mode & S_IFMT) != S_IFDIR) {
		fprintf(stderr, "rmdir: %s not a directory\n", name);
		++Errors;
		return;
	}
	if(st.st_ino==cst.st_ino &&st.st_dev==cst.st_dev) {
		fprintf(stderr, "rmdir: cannot remove current directory\n");
		++Errors;
		return;
	}
	if((fd = open(name,0)) < 0) {
		fprintf(stderr, "rmdir: %s unreadable\n", name);
		++Errors;
		return;
	}
	while(read(fd, (char *)&dir, sizeof dir) == sizeof dir) {
		if(dir.d_ino == 0) continue;
		if(!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
			continue;
		fprintf(stderr, "rmdir: %s not empty\n", name);
		++Errors;
		close(fd);
		return;
	}
	close(fd);
	if(!strcmp(np, ".") || !strcmp(np, "..")) {
		fprintf(stderr, "rmdir: cannot remove . or ..\n");
		++Errors;
		return;
	}
	strcat(name, "/.");
	if((access(name, 0)) < 0) {		/* name/. non-existent */
		strcat(name, ".");
		goto unl;
	}
	strcat(name, ".");
	
	if (stat(name, &cst) < 0)		/* name/.. non-existent */
		goto unl2;
	if(access(name, 02)) {
		name[strlen(name)-3] = '\0';
		fprintf(stderr, "rmdir: %s: no permission\n", name);
		++Errors;
		return;
	}
	if(st.st_dev != cst.st_dev ) {
		fprintf(stderr, "rmdir: cannot remove mountable directory\n");
		++Errors;
		return;
	}
unl:
	unlink(name);	/* unlink name/.. */
unl2:
	name[strlen(name)-1] = '\0';
	unlink(name);	/* unlink name/.  */
	name[strlen(name)-2] = '\0';
	if (unlink(name) < 0) {
		fprintf(stderr, "rmdir: %s not removed\n", name);
		++Errors;
	}
}
