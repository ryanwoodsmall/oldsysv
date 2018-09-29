static char SCCSID[]="@(#)unixesc.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"

unixesc(restrict)
int restrict;
{
	int pid,rpid,retcode;

	if((pid = fork()) == 0) {
		if(!restrict) execl("/bin/sh","sh", "-t", 0);
		else execl("/bin/rsh","rsh", "-t", 0);
		exit(0);
	}
	while((rpid = wait(&retcode)) != pid && rpid != -1);
	return;
}
