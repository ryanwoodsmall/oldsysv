/*	@(#)strout.c	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

void
_strout(string, count, adjust, file)
register char *string;
register int adjust, count;
register FILE *file;
{
	while(adjust > 0) {
		(void) putc(' ', file);
		--adjust;
	}
	while(--count >= 0)
		(void) putc(*string++, file);
	while(adjust < 0) {
		(void) putc(' ', file);
		++adjust;
	}
}
