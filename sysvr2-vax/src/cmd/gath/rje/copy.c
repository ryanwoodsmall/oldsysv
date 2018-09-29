/*
 * Copy N bytes of data from SRC to DST
 */

static char Sccsid[] = "@(#)copy.c	1.1";

copy(src,dst,n)
register char *src,*dst;
register int n;
{
	do {
		*dst++ = *src++;
	} while(--n);
}
