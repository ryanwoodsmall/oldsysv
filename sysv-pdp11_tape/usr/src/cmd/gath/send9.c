/*	send9.c		Trap handling code	*/

#include	"send.h"

static char Sccsid[] = "@(#)send9.c	1.1";

struct	trap	*tchain;

rstrp(strp)
struct	trap	*strp;
{
	tchain = strp->str;
}

dotrp()
{
	if(tchain)
		longjmp(tchain->save);
}
