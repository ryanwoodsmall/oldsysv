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
/* @(#)xqt.c	1.3 */
#include "uucp.h"
#include <signal.h>

void exit();

/*
 * start up uucico for rmtname
 * return:
 *	none
 */
xuucico(rmtname)
char *rmtname;
{

	/*
	 * start uucico for rmtname system
	 */
	if (fork() == 0) {
		euucico(rmtname);
	}
	return;
}
euucico(rmtname)
char	*rmtname;
{
	char opt[100];

	close(0);
	close(1);
	close(2);
	open("/dev/null", 0);
	open("/dev/null", 1);
	open("/dev/null", 1);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	closelog();
	if (rmtname[0] != '\0')
		sprintf(opt, "-s%.7s", rmtname);
	else
		opt[0] = '\0';
	execle(UUCICO, "UUCICO", "-r1", opt, 0, Env);
	exit(100);
}


/*
 * start up uuxqt
 * return:
 *	none
 */
xuuxqt()
{

	/*
	 * start uuxqt
	 */
	if (fork() == 0) {
		close(0);
		close(1);
		close(2);
		open("/dev/null", 2);
		open("/dev/null", 2);
		open("/dev/null", 2);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		closelog();
		execle(UUXQT, "UUXQT",  0, Env);
		exit(100);
	}
	return;
}
xuucp(str)
char *str;
{
	char text[300];
	unsigned sleep();

	/*
	 * start uucp
	 */
	if (fork() == 0) {
		close(0);
		close(1);
		close(2);
		open("/dev/null", 0);
		open("/dev/null", 1);
		open("/dev/null", 1);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		closelog();
		sprintf(text, "%s -r %s", UUCP, str);
		execle(SHELL, "sh", "-c", text, 0, Env);
		exit(100);
	}
	sleep(15);
	return;
}
