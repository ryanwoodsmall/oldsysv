/*	@(#)prf.c	1.1	*/
/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	} else {
		putchar('%');
		putchar(c);
		goto loop;
	}
	adx++;
	goto loop;
}

printn(n, b)
long n;
{
	register i;
	char d[12];

	for (i=0;i<12;i++) {
		d[i] = (n%b);
		n = n/b;
		if (n==0)
			break;
	}
	for (;i>=0;i--) {
		putchar("0123456789ABCDEF"[d[i]]);
	}
}
