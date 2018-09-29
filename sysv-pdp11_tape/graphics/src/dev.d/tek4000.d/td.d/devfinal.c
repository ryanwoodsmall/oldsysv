static char SCCSID[]="@(#)devfinal.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
#include "../../include/dev.h"
#define SMALLFONT	printf("\033;")
devfinal()
{
/*
 *  devfinal resets the tek terminal after a display
 */
	sleep(1); /* wait for output to complete */
	devcursor(0,763);
	SMALLFONT;
	return;
}
