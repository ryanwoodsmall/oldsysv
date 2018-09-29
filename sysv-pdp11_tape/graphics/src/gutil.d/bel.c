static char SCCSID[]="@(#)bel.c	1.2";
#include	"stdio.h"

/*
	Program to issue bel character.
*/

main()
{
	putchar('\07');
	exit(0);
}
