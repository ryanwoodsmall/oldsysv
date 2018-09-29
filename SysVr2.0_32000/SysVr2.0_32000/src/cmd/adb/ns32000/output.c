/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)output.c	1.2	*/
#include "defs.h"

SCCSID(@(#)output.c	1.2);

static char printbuf[MAXLIN];
static char *printptr = printbuf;
static char *digitptr;
static void printdate();
static void printoct();
static void printnum();
static void printdbl();

int eqstr(s1, s2)
    STRING s1;
    STRING s2;
{
	return (!strcmp(s1,s2));
}

void printc(c)
    char c;
{
    char   d;
    STRING q;
    short  posn;
    short  tabs;
    short  p;

    if (mkfault)
		return; 
    else if ((*printptr = c) == EOR) {
        tabs = 0;
	posn = 0;
	q = printbuf;
	for (p = 0; p < printptr - printbuf; p++) {
		d = printbuf[p];
		if ((p & 07) == 0 && posn) {
			tabs++;
			posn = 0; 
		}
		if (d == SPACE)
			posn++; 
		else {
			while (tabs > 0 ) {
				*q++ = TB;
				tabs--; 
			}
			while (posn > 0 ) {
				*q++ = SPACE;
				posn--; 
			}
			*q++ = d;
		}
	}
	*q++ = EOR;
	if (-1 == write(outfile, printbuf, (unsigned) (q - printbuf)))
		perror("printc");
	printptr = printbuf;
	}
    else if (c == TB) {
	*printptr++ = SPACE;
        while ((printptr - printbuf) & 07)
		*printptr++ = SPACE; 
    } 
    else if (c)
	printptr++;
}

charpos()
{
	return(printptr - printbuf);
}

void flushbuf()
{
	if (printptr != printbuf) 
		printc(EOR);
}

/*VARARGS1*/
void adbpr(fmat, a1)
    STRING fmat;
    STRING *a1;
{
    STRING  fptr;
    STRING  s;
    short   *vptr;
    int     *dptr;
    double  *r2ptr;
    float   *rptr;
    short   width;
    short   prec;
    char    c;
    char    adj;
    short   x;
    short   n;
    int     lx;
    char    digits[64];

    fptr = fmat;
    vptr = (short *) &a1;
    dptr = (int *) vptr;

    while (c = *fptr++) {
		if (c != '%')
			printc(c);
		else {
			if (*fptr == '-' ) { 
				adj = 'l';
				fptr++;
			}
			else
				adj = 'r';
			width = convert(&fptr);
			if (*fptr == '.') {
				fptr++; 
				prec = convert(&fptr); 
			}
			else 
				prec = -1;
			digitptr = digits;
			rptr = (float *) dptr;
			r2ptr = (double *) dptr;
			x = shorten(lx = *dptr++);
		    	s = 0;
			switch (c = *fptr++) {

			case 'd':
                	case 'u':
                    		printnum(x, c, 10);
		    		break;

                	case 'o':
		    		printoct((unsigned short)x, 0);
		    		break;
					
                	case 'q':
		    		lx = x;
		    		printoct(lx, -1);
		    		break;

                	case 'x':
                    		printdbl(x, 16);
				break;

                	case 'r':
                    		printdbl(lx = x, radix);
				break;

                	case 'R':
                    		printdbl(lx, radix);
		    		vptr++;
		    		break;

                	case 'Y':
                    		printdate((long) lx);
		    		vptr++;
		    		break;

                	case 'D':
                	case 'U':
                    		printdbl(lx, 10);
		    		vptr++;
		    		break;

                	case 'O':
                    		printoct(lx, 0);
		    		vptr++; 
		    		break;

                	case 'Q':
                    		printoct(lx, -1);
		    		vptr++;
		    		break;

                	case 'X':
                    		printdbl(lx, 16);
		    		vptr++;
		    		break;

                	case 'c':
                    		printc(x);
		    		break;

                	case 's':
                    		s = (char *) lx;
                    		break;

                	case 'f':
				(void) sprintf(s = digits, "%-16.9e", *rptr); 
                    		prec = -1; 
                    		break;

                	case 'F':
                    		dptr++;
				(void) sprintf(s = digits, "%-32.18e", *r2ptr); 
                    		prec = -1; 
                    		break;

                	case 'm':
                    		vptr--;
		    		break;

                	case 'M':
                    		width = x;
		    		break;

                	case 'T':
                	case 't':
                    		if (c == 'T')
					width = x;
                    		else
					dptr--;
                    		if (width)
				width -= charpos() % width;
                    		break;

                	default:
                    		printc(c);
		    		dptr--;
		}

            if (s == 0) {
		*digitptr = 0;
		s = digits;
            }
            n = strlen(s);
            n = (prec < n && prec >= 0 ? prec : n);
            width -= n;
            if (adj == 'r') { 
		while (width-- > 0)
			printc(SPACE);
            }
            while (n--)
		printc(*s++);
            while (width-- > 0 )
		printc(SPACE);
            digitptr = digits;
	    }
	}
}

static void printdate(tvec)
 	long tvec;
{
 	int    i;
 	STRING timeptr;

 	timeptr = ctime(&tvec);
 	for (i = 20; i < 24; i++) 
		*digitptr++ = *(timeptr + i);
 	for (i = 3; i < 19; i++) 
		*digitptr++ = *(timeptr + i);
}

void prints(s)
	char *s;
{
	adbpr("%s", s);
}

void newline()
{
    printc(EOR);
}

static int convert(cp)
	STRING *cp;
{
	char c;
    int  n;

	n = 0;
	while (((c = *(*cp)++) >= '0') && (c <= '9')) 
		n = n * 10 + c - '0';
	(*cp)--;
	return(n);
}

static void printnum(n, fmat, base)
    int n;
{
	char k;
	int  *dptr;
	int  digs[15];

	dptr = digs;
	if (n < 0 && fmat == 'd') {
		n = -n;
		*digitptr++ = '-'; 
	}
	while (n) {
		*dptr++ = ((unsigned) n) % base;
		 n = ((unsigned) n) / base;
	}
	if (dptr == digs) 
		*dptr++ = 0;
	while (dptr != digs) {  
		k = *--dptr;
		*digitptr++ = (k + (k <= 9 ? '0' : 'a' - 10));
	}
}

static void printoct(po, s)
    int po;
    int s;
{
    int  i;
    char digs[12];

    if (s) 
		if (po < 0) { 
			po = -po;
			*digitptr++ = '-';
        }
		else
			if (s > 0) 
				*digitptr++ = '+';
    for (i = 0; i <= 11; i++) { 
		digs[i] = po & 07;
		po >>= 3; 
	}
    digs[10] &= 03;
	digs[11] = 0;
    for (i = 11; i >= 0; i--)
		if (digs[i]) 
			break;
    for (i++; i >= 0; i--) 
		*digitptr++ = digs[i] + '0';
}

static void printdbl(lxy, base)
	int lxy;
	int base;
{ 
	char digs[35 * sizeof(int)];
	char *cp1;

    cp1 = digs; 
    if ((lxy & 0xFFFF0000L) == 0xFFFF0000L) {
        *cp1++ = '-'; 
        lxy = -lxy;
	}
	(void) sprintf(cp1, base==16 ? "%lx" : "%ld", lxy);
	cp1 = digs;
	while (*digitptr++ = *cp1++)
		;
	--digitptr;
}

void iclose()
{
    if (infile) {
		if (-1 == close(infile))
            perror("iclose");
	infile = 0;
    }
}

void oclose()
{
    if (outfile != 1) {
		flushbuf();
		if (-1 == close(outfile))
            perror("oclode");
		outfile = 1;
    }
}

void endline()
{
    if (maxpos <= charpos()) 
		adbpr("\n");
}
