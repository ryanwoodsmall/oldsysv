static char SCCSID[]="@(#)erase.c	1.1";
#include	"stdio.h"

/*
	This program will erase the screen of a tektronix 4010-1.
*/

main()
{
	printf("\033\014");
	exit(0);
}
