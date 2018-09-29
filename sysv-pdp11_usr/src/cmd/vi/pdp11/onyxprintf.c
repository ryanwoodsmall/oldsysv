/* Copyright (c) 1979 Regents of the University of California */

/*
 * This version of printf calls doprnt, and as such is not portable,
 * since doprnt is written in pdp-11 assembly language.  (There is a
 * vax doprnt which has the first 2 arguments reversed.  We don't use it.)
 * This version is used because it is about 900 bytes smaller than the
 * portable version, which is also included in case it is needed.
 */
#ifdef TRACE
#include	<stdio.h>
#undef putchar
#endif

printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, 0);
}

#define CBS	100	/* byte ct for conversion buffer */


_doprnt(fp,argp,iop) register char *fp; register int *argp; {
    long lwork;		/* long dividend for integer conversion */
    long _udvd();	/* unsigned long-divide routine */
    int  base;		/* base for integer conversion */
    int  sflg;		/* prepend '-' to converted integer if set */
    int  width;		/* width of cur fld */
    int  lj;		/* nonzero if curfld to be left just */
    int  ndig;		/* maxno digits/chars to be put out */
    int  fillch;	/* fill char, blank or ascii zero */
    register fch;	/* cur char from format string */
    int  t0,t1;		/* work temps */
    char conbuf[CBS];	/* conversion buffer */
    char *fba,*lba1;	/* first/last+1 byte ptrs to converted fld */

    for (;;) {
	while ((fch = *fp++) && fch != '%') {
#ifdef TRACE
	    if (iop)
		putc(fch,iop);
	    else
#endif
		putchar(fch);
	}
	if (fch == 0) return;
	if ((fch = *fp++) == '-') {
	    lj = 1;
	    fch = *fp++;
	} else {
	    lj = 0;
	}
	fillch = fch == '0' ? '0' : ' ';
	width = 0;
	while (fch >= '0' && fch <= '9'/* || fch == '*' */) {
	    width = width * 10 + (/* fch == '*' ? *argp++ : */ fch - '0');
	    fch = *fp++;
	}
	ndig = 0;
	if (fch == '.') {
	    while ((fch = *fp++) >= '0' && fch <= '9'/* || fch == '*' */) {
		ndig = ndig * 10 + (/* fch == '*' ? *argp++ : */ fch - '0');
	    }
	}
	fba = lba1 = &conbuf[CBS];
	switch (fch) {

	case 0:			/* unexp end of format string */
	    return;

	case 'r':		/* remote format & args */
	    argp = (int *)*argp;
	    fp = (char *)*argp++;
	    continue;

	case 'c':		/* single char in lo-order part of int */
	    fch = *argp++;
	    /* flow into default */

	default:		/* unknown keyletter - emit asis */
	    fillch = ' ';
	    *--fba = fch;
	    break;

	case 's':		/* byte pointer to string */
	    fillch = ' ';
	    fba = (char *)*argp++;
	    if (fba == 0) fba = "(null)";
	    lba1 = fba;
	    while (*lba1) {
		lba1++;
		if (--ndig == 0) break;
	    }
	    break;

	case 'U':		/* long -- unsgd dcml */
	    goto Ucase;

	case 'O':		/* long -- unsgd oct */
	    goto Ocase;

	case 'X':		/* long -- unsgd hex */
	    goto Xcase;

	case 'D':		/* long -- sgd dcml */
	    goto Dcase;

	case 'L':		/* L[uoxd] => long, L => unsgd int (?) */
	case 'l':		/* l[uoxd] => long, l => unsgd int */
	    switch (*fp++) {

	    case 'u':			/* long -- unsgd dcml */
	    Ucase:
		base = 10;
		goto UOX;

	    case 'o':			/* long -- unsgd oct */
	    Ocase:
		base = 8;
		goto UOX;

	    case 'x':			/* long -- unsgd hex */
	    Xcase:
		base = 16;
	    UOX:
		lwork = *(long *)argp;
		argp += sizeof(long) / sizeof(int);
		goto usgcon;

	    case 'd':			/* long -- sgd dcml */
	    Dcase:
		lwork = *(long *)argp;
		argp += sizeof(long) / sizeof(int);
		base = 10;
		goto sgcon;

	    default:			/* int -- unsgd dcml */
		fp--;
	    }
	    /* flow into case 'u' */

	case 'u':		/* int -- unsgd dcml */
	    base = 10;
	    goto uox;

	case 'o':		/* int -- unsgd oct */
	    base = 8;
	    goto uox;

	case 'x':		/* int -- unsgd hex */
	    base = 16;
	uox:
	    lwork = (unsigned)*argp++;
	    goto usgcon;

	case 'd':		/* int -- sgd dcml */
	    lwork = *argp++;
	    base = 10;
	    /* flow into sgcon */

	sgcon:
	    if (lwork < 0L) {
		sflg = 1;
		if ((lwork = -lwork) == 0L) lwork = 0x80000000;
	    } else {
	usgcon:
		sflg = 0;
	    }
	    do {
		lwork = _udvd(lwork, base, &t0);
		*--fba = "0123456789ABCDEF"[t0];
	    } while (lwork);
	    if (sflg) *--fba = '-';
	}
	t0 = lba1 - fba;
	width -= t0;
	if (width < 0) width = 0;
	_strout(fba, t0, lj ? width : -width, iop, fillch);
    }
}

_strout(string, count, adjust, file, fillch)
register char *string;
register count;
int adjust;
register int file;
{
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putchar(*string++);
			count--;
		}
		putchar(fillch);
		adjust++;
	}
	while (--count>=0)
		putchar(*string++);
	while (adjust) {
		putchar(fillch);
		adjust--;
	}
}
