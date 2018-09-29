static char SCCSID[]="@(#)devbuff.c	1.1";
#include <stdio.h>
#include "dev.h"
#include "../../include/dev.h"
extern struct device dv;
extern char *nodename;

devbuff()
{
	static FILE *devname;
	FILE *fopen();
	static int openflag = 1;
	char *ptr, *ttyname();

	if (openflag) {
		openflag = 0;
		if ((ptr = ttyname(fileno(stdout))) == NULL) {
			dv.hbuff = FALSE;
			return;
		}
		else if ((devname = fopen(ptr, "r")) == NULL) {
			devfinal();
			ERRPR0(can not open tty for reading);
			exit(1);
		}
		else {
			SETOMODE;
			SETHSHK1;
		}
	}
	putc(SHKENAB,stdout);
	getc(devname);
}
