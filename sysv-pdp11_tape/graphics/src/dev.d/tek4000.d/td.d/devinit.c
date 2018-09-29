static char SCCSID[]="@(#)devinit.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
#include "../../include/dev.h"
#define ERASE       printf("\033\014")  /*  ESC NP  */
#define GRAPHMODE   printf("\035")      /*  GS  */
extern struct options {
	int eopt;  /*  erase  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devinit()
{
	extern char *nodename;
/*
 *  devinit initializes the tektronix terminal for displaying
 */
	if(op.eopt) {
		ERASE;
		sleep(1);
	}
	GRAPHMODE;
}
