static char SCCSID[]="@(#)xytek.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"

xytek(scrx,scry,tk)
int scrx,scry,tk[4];
{

	tk[0] = ((scry >> 5) & 037) + 040; /* hy */
	tk[1] =  (scry & 037) + 0140; /* ly */
	tk[2] = ((scrx >> 5) & 037) + 040; /* hx */
	tk[3] =  (scrx & 037) + 0100; /* lx */
	return;
}
