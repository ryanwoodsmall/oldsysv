/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/doscan.c	2.17"
/*LINTLIBRARY*/
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <values.h>
#include <errno.h>

#define NCHARS	(1 << BITSPERBYTE)
#define locgetc()	(chcount+=1,getc(iop))
#define locungetc(x)	(chcount-=1,ungetc(x,iop))

extern double atof();
extern char *memset();
static int chcount,flag_eof;

int
_doscan(iop, fmt, va_alist)
register FILE *iop;
register unsigned char *fmt;
va_list va_alist;
{
	extern unsigned char *setup();
	char tab[NCHARS];
	register int ch;
	int nmatch = 0, len, inchar, stow, size;
	chcount=0; flag_eof=0;

	/*******************************************************
	 * Main loop: reads format to determine a pattern,
	 *		and then goes to read input stream
	 *		in attempt to match the pattern.
	 *******************************************************/
	for ( ; ; ) 
	{
		if ( (ch = *fmt++) == '\0')
			return(nmatch); /* end of format */
		if (isspace(ch)) 
		{
		  	if (!flag_eof) 
			{
			   while (isspace(inchar = locgetc()))
	 		        ;
			   if (locungetc(inchar) == EOF)
				flag_eof = 1;
		        }
		  continue;
                }
		if (ch != '%' || (ch = *fmt++) == '%') 
                {
			if ( (inchar = locgetc()) == ch )
				continue;
			if (locungetc(inchar) != EOF)
				return(nmatch); /* failed to match input */
			break;
		}
		if (ch == '*') 
		{
			stow = 0;
			ch = *fmt++;
		}
		else
			stow = 1;

		for (len = 0; isdigit(ch); ch = *fmt++)
			len = len * 10 + ch - '0';
		if (len == 0)
			len = MAXINT;
		if ( (size = ch) == 'l' || (size == 'h') )
			ch = *fmt++;
		if (ch == '\0' ||
		    ch == '[' && (fmt = setup(fmt, tab)) == NULL)
			return(EOF); /* unexpected end of format */
		if (isupper(ch))  /* no longer documented */
		{
			size = 'l';
			ch = _tolower(ch);
		}
		if (ch!= 'n' && !flag_eof)
		  if (ch != 'c' && ch != '[') 
		  {
			while (isspace(inchar = locgetc()))
				;
			if(locungetc(inchar) == EOF )
				break;
		  }
		switch(ch)
		{
		 case 'c':
		 case 's':
		 case '[':
			  size = string(stow,ch,len,tab,iop,&va_alist);
			  break;
                 case 'n':
			  if (size == 'h')
				*va_arg(va_alist, short *) = (short) chcount;
		          else if (size == 'l')
				*va_arg(va_alist, long *) = (long) chcount;
			  else
			  	*va_arg(va_alist, int *) = (int) chcount;
			  continue;
		 case 'i':
                 default:
			 size = number(stow, ch, len, size, iop, &va_alist);
			 break;
                 }
		   if (size) 
		          nmatch += stow;   
                   else return(nmatch);
                continue;
	}
	return (nmatch != 0 ? nmatch : EOF); /* end of input */
}

/***************************************************************
 * Functions to read the input stream in an attempt to match incoming
 * data to the current pattern from the main loop of _doscan().
 ***************************************************************/
static int
number(stow, type, len, size, iop, listp)
int stow, type, len, size;
register FILE *iop;
va_list *listp;
{
	char numbuf[64], inchar, lookahead;
	register char *np = numbuf;
	register int c, base;
	int digitseen = 0, dotseen = 0, expseen = 0, floater = 0, negflg = 0;
	long lcval = 0;
	switch(type) 
	{
	case 'e':
	case 'f':
	case 'g':
		floater++;
	case 'd':
	case 'u':
        case 'i':
		base = 10;
		break;
	case 'o':
		base = 8;
		break;
	case 'x':
		base = 16;
		break;
	default:
		return(0); /* unrecognized conversion character */
	}
	switch(c = locgetc()) 
	{
	case '-':
		negflg++;
	case '+': /* fall-through */
		if (--len <= 0)
		   break;
		if ( (c = locgetc()) != '0')
		   break;
        case '0':
                if ( (type != 'i') || (len <= 1) )  
		   break;
	        if ( ((inchar = locgetc()) == 'x') || (inchar == 'X') ) 
	        {
		      /* If not using sscanf and *
		       * at the buffer's end     *
		       * then LOOK ahead         */

                   if ( (iop->_file == _NFILE) || (iop->_cnt != 0) )
		      lookahead = locgetc();
		   else
		   {
		      if ( read(fileno(iop),np,1) == 1)
		         lookahead = *np;
                      else
		         lookahead = EOF;
                      chcount += 1;
                   }    
		   if ( isxdigit(lookahead) )
		   {
		       base =16;

		       if ( len <= 2)
		       {
			  locungetc(lookahead);
			  len -= 1;            /* Take into account the 'x'*/
                       }
		       else 
		       {
		          c = lookahead;
			  len -= 2;           /* Take into account '0x'*/
		       }
                   }
	           else
	           {
	               locungetc(lookahead);
	               locungetc(inchar);
                   }
		}
	        else
	        {
		    locungetc(inchar);
	            base = 8;
                }
	}
	for (; --len  >= 0 ; *np++ = c, c = locgetc()) 
	{
		if (np > numbuf + 62)           
		{
		    errno = ERANGE;
		    return(0);
                }
		if (isdigit(c) || base == 16 && isxdigit(c)) 
		{
			int digit = c - (isdigit(c) ? '0' :  
			    isupper(c) ? 'A' - 10 : 'a' - 10);
			if (digit >= base)
				break;
			if (stow && !floater)
				lcval = base * lcval + digit;
			digitseen++;


			continue;
		}
		if (!floater)
			break;
		if (c == '.' && !dotseen++)
			continue;
		if ( (c == 'e' || c == 'E') && digitseen && !expseen++) 
                {
			*np++ = c;
			c = locgetc();
			if (isdigit(c) || c == '+' || c == '-' || isspace(c))
				continue;
		}
		break;
	}


	if (stow && digitseen)
		if(floater) 
		{
			register double dval;
	
			*np = '\0';
			dval = atof(numbuf);
			if (negflg)
				dval = -dval;
			if (size == 'l')
				*va_arg(*listp, double *) = dval;
			else
				*va_arg(*listp, float *) = (float)dval;
		}
		else
		{
			/* suppress possible overflow on 2's-comp negation */
			if (negflg && lcval != HIBITL)
				lcval = -lcval;
			if (size == 'l')
				*va_arg(*listp, long *) = lcval;
			else if (size == 'h')
				*va_arg(*listp, short *) = (short)lcval;
			else
				*va_arg(*listp, int *) = (int)lcval;
		}
	if (locungetc(c) == EOF)
	    flag_eof=1;
	return (digitseen); /* successful match if non-zero */
}

static int
string(stow, type, len, tab, iop, listp)
register int stow, type, len;
register char *tab;
register FILE *iop;
va_list *listp;
{
	register int ch;
	register char *ptr;
	char *start;

	start = ptr = stow ? va_arg(*listp, char *) : NULL;
	if (type == 'c' && len == MAXINT)
		len = 1;
	while ( (ch = locgetc()) != EOF &&
	    !(type == 's' && isspace(ch) || type == '[' && tab[ch])) 
        {
		if (stow) 
			*ptr = ch;
		ptr++;
		if (--len <= 0)
			break;
	}
	if (ch == EOF ) 
	{
	       flag_eof = 1;
	       chcount-=1;
        }
        else if (len > 0 && locungetc(ch) == EOF)
		flag_eof = 1;
	if (ptr == start)
		return(0); /* no match */
	if (stow && type != 'c')
		*ptr = '\0';
	return (1); /* successful match */
}

static unsigned char *
setup(fmt, tab)
register unsigned char *fmt;
register char *tab;
{
	register int b, c, d, t = 0;

	if (*fmt == '^') 
	{
		t++;
		fmt++;
	}
	(void) memset(tab, !t, NCHARS);
	if ( (c = *fmt) == ']' || c == '-')  /* first char is special */
	{
		tab[c] = t;
		fmt++;
	}
	while ( (c = *fmt++) != ']') 
	{
		if (c == '\0')
			return(NULL); /* unexpected end of format */
		if (c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) 
		{
			(void) memset(&tab[b], t, d - b + 1);
			fmt++;
		} 
		else
			tab[c] = t;
	}
	return (fmt);
}
