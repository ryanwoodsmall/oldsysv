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
/*
*	reboot creates the dead file with msg,
*	trys to reboot the proper rje from "who",
*	otherwise it creates the dead file containing
*	the message "Can't reboot"
*/

static char Sccsid[] = "@(#)reboot.c	1.1";

static char dead[] = "dead";

reboot(msg,who,home)
char *msg;
register char *who, *home;
{
	char init[24];
	int i;

	close(0);
	close(1);
	for(i=3; i<20; i++)
		close(i);
	chdir(home);
	unlink(dead);
	i = creat(dead,0644);
	write(i,msg,strlen(msg));
	close(i);
	strcpy(init,who);
	strcat(init,"init");
	execl(init,init,"+",0);
	rjedead("Can't reboot\n",".");
}
