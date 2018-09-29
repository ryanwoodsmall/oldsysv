/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/parsefcns.c	1.5"
/* UNIX HEADER */
#include	<stdio.h>
#include	<ctype.h>

/* LISTER HEADER */
#include	"defs.h"


    /*	parsefcns.c is a set of functions used to parse the C source file
     *
     *  getnonblank( )
     *
     *	scans the input buffer starting at bufndx, incrementing bufndx
     *	until it indexes the next non-blank character in buffer
     *  a blank is a blank, a tab, a new line, or a comment
     *
     *  calls:
     *	    - getline( ) to get the next line of the source file
     *	      when it finds a new line character
     *	    - endcomment( ) to find the end of a comment string
     *
     *  returns EOF or !EOF
     *
     *  increments the external variable bufndx
     */


int
getnonblank( )

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getline( ),
		endcomment( );

    /* EXTERNAL VARIABLES USED */
    extern char	buffer[ ];
    extern int	bufndx;


    for (	;	;	) {
	switch(buffer[bufndx]) {
	    case '\n':
		if (getline( ) == EOF) {
		    return(EOF);
		}
		break;

	    case '\t':
	    case ' ':
		++bufndx;
		break;

	    case '/':
		if (buffer[bufndx + 1] == '*') {
		    bufndx += 2;
		    if (endcomment( ) == EOF) {
			return(EOF);
		    }
		    break;
		} 
		return(!EOF);
		break;

	    case '\\':
		++bufndx;
		break;

	    default:
		return(!EOF);
		break;
	}
    }
}


    /*	endcomment( )
     *
     *	finds the delimiter that ends a comment
     *
     *  calls:
     *	    - getline( ) to refill buffer when a new line is scanned
     *
     *  returns EOF or !EOF
     *
     *  increments the external variable bufndx
     */


int
endcomment( )

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getline( );

    /* EXTERNAL VARIABLES USED */
    extern char	buffer[ ];
    extern int	bufndx;


    for (	;	;	) {
	switch(buffer[bufndx]) {
	    case '\n':
		if (getline( ) == EOF) {
		    return(EOF);
		}
		break;

	    case '*':
		++bufndx;
		if (buffer[bufndx] == '/') {
		    ++bufndx;
		    return(!EOF);
		}
		break;

	    default:
		++bufndx;
		break;

	}
    }
}


    /*  endquote(qchar)
     *
     *  finds the matching qchar in the input buffer
     *
     +  calls:
     *	    - getline( ) to refill buffer when a new line is read
     *
     *  returns EOF or !EOF
     *
     *  increments external variable bufndx
     */


int
endquote(qchar)

char	qchar;

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getline( );

    /* EXTERNAL VARIABLES USED */
    extern char	buffer[ ];
    extern int	bufndx;


    while (buffer[bufndx] != qchar) {
	switch(buffer[bufndx]) {
	    case '\\':
		++bufndx;
		if (buffer[bufndx] != '\n')
			++bufndx;
		else if (getline( ) == EOF)
			return EOF;
		break;

	    case '\n':
		return( getline( ) );

	    default:
		++bufndx;
		break;

	}
    }

    ++bufndx;
    return(!EOF);
}


    /*  countbrace(lnno, bracecount)
     *
     *	counts the braces (incrementing for {, decrementing for }) in
     *  the input buffer until the number of lines processed reaches 
     *  lnno (the number of the next breakpointable line)
     *
     *	calls:
     *	    - getnonblank( ) to find the next non blank character in buffer
     *	    - endquote(buffer[oldndx]) to find the closing quote character
     *
     *  returns EOF or bracecount
     *        returns bracecount when lnno = linecount
     *	      or when bracecount = 0
     *
     *	increments bufndx
     *
     *
     *  curly braces are counted to check correspondence between source
     *	and object file (last line number should correspond with closing brace)
     *  - but -
     *  ifdefs can throw the bracecount off in a perfectly okay function
     *  hence countbrace( ) should be eliminated (the lister should simply
     *  check to see that the largest line number for a function corresponds
     *  with a source line that contains a closing curly brace)
     */


int
countbrace(lnno, bracecount)

unsigned short	lnno;
int		bracecount;

{
    /* LISTER FUNCTIONS CALLED */
    extern int			getnonblank( ),
				endquote( );

    /* EXTERNAL VARIABLES USED */
    extern char			buffer[ ];
    extern int			bufndx;
    extern unsigned short	linecount;

    /* LOCAL VARIABLES */
    int				oldndx;


    while (linecount < lnno) {
	if (getnonblank( ) == EOF) {
	    return(EOF);
	}

	oldndx = bufndx;
	++bufndx;

	if (isalnum((int) buffer[oldndx])) {
	    for(	; isalnum((int) buffer[bufndx]); ++bufndx);
	} else {
	    switch(buffer[oldndx]) {
		case '\'':
		case '"':
		    if (endquote(buffer[oldndx]) == EOF) {
			return(EOF);
		    }
		    break;

		case '{':
		    ++bracecount;
		    break;

		case '}':
		    --bracecount;
/*begin fix*/
		    if (bracecount < 0)
			bracecount = 0;
/*end fix*/
		    if (bracecount == 0) {
			return(0);
		    }
		    break;

		default:
		    break;
	    }
	}
    }

    return(bracecount);
}

/*
*/
