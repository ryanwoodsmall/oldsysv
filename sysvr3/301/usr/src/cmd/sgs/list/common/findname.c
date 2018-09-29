/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/findname.c	1.5"
/* UNIX HEADER */
#include	<stdio.h>
#include	<ctype.h>

/* COMMON SGS HEADER */
#include	"syms.h"

/* LISTER HEADERS */
#include	"findname.h"
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
char	fcname[2][BUFSIZ];

/* STATIC VARIABLES */
static int	tokendx,
		charcount;


    /*	findname.c is a collection of functions used to find a function name
     *	in a C source file
     *
     *  the static variables tokendx and charcount are used to capture a
     *  function name:
     *      - tokendx indexes an identifier in buffer (a possible function name)
     *      - charcount is the number of characters in the identifier
     *
     *	findname(filename)
     *
     *	finds a function name in a C source file using the state table 
     *	    defined in findname.h (statable)
     *	certain states have actions associated with them
     *	the variable toggle is used to determine which identifier (LISTID)
     *	    in a potential function definition string is the function name
     *
     *  calls:
     *	    - getoken( ) to get the next token from the input buffer
     *	    - findsemi(filename) to find the end of a parameter declaration
     *	      in the function definition
     *
     *  returns a pointer to the function name (fcnname[toggle])
     *  returns NULL on ERROR (unexpected EOF)
     */


char *
findname(filename)

char	*filename;

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getoken( ),
		findsemi( );

    /* EXTERNAL VARIABLES USED */
    extern char	buffer[ ];

    /* LOCAL VARIABLES */
    int		state,
		toggle,
		i;


    state = START;
    toggle = 0;

    while ((state = statable[state][getoken( )]) != ACCEPT) {

	switch(state) {
	    case START:
		/*  when a potential function name is found put it in fcname[0]
		 *  if there is any question about where to put it
		 */
		toggle = 0;
		break;

	    case LPIDLP:
		/*  the name in fcname[1] might be a function name 
		 *  the name in fcname[0] certainly is not
		 */
		toggle = 1;
		break;

	    case BEGID:
		/*  the name at buffer[tokendx] may be a function
		 *  put the name in fcname[0]
		 */
#if FLEXNAMES
		for (i = 0; charcount > 0; --charcount, ++i, ++tokendx) {
#else
		for (i = 0; (i < SYMNMLEN) && (charcount > 0); --charcount, ++i,
		     ++tokendx) {
#endif
		    fcname[0][i] = buffer[tokendx];
		}
		fcname[0][i] = '\0';
		break;

	    case LPID:
	    case BEGPRM:
		/*  the LISTID at buffer[tokendx] might be the first parameter
		 *  in the parameter list (BEGPRM)
		 *  -- or --
		 *  the LISTID at buffer[tokendx] might be a function name 
		 *  preceded by a LPAR
		 *  -- in any case, the name goes in fcname[1]
		 */
#if FLEXNAMES
		for (i = 0; charcount > 0; -- charcount, ++i, ++tokendx) {
#else
		for (i = 0; (i < SYMNMLEN) && (charcount > 0); --charcount, ++i,
		     ++tokendx) {
#endif
		    fcname[1][i] = buffer[tokendx];
		}
		fcname[1][i] = '\0';
		break;

	    case DECL:
		/*  findsemi(filename) has the logic to recognize declarations
		 */
		if ((state = findsemi(filename)) == ERROR) {
		    return(NULL);
		}
		break;

	    case ERROR:
		return(NULL);
		break;

	    default:
		break;

	}
    }

    return(fcname[toggle]);
}


    /*	getoken( )
     *
     *  scans the input buffer (buffer) and returns the next token found
     *
     *  calls:
     *	    - getnonblank( ) to get the next non-blank character from the 
     *	      input buffer
     *	      a blank is a blank, tab, or new-line character or a comment
     *	    - endquote(buffer[tokendx]) to find the quote delimiter balancing
     *	      the character at buffer[tokendx]
     *
     *  returns a small integer that specifies which token was found
     *
     +  meaningful tokens are EOF ',' '(' ')' '{' '*' and an alphanumeric
     *	string beginning with an alphabetic character.
     *  all other characters or strings are DONTCARE
     *
     *  getoken increments the external variable bufndx
     *  getoken sets the static variables tokendx and charcount
     */


int
getoken( )

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getnonblank( ),
		endquote( );

    /* EXTERNAL VARIABLES */
    extern char	buffer[ ];
    extern int	bufndx;


    if (getnonblank( ) == EOF) {
	return(LISTEOF);
    }

    tokendx = bufndx;
    ++bufndx;

    switch(buffer[tokendx]) {
	case ',':
	    return(COMMA);
	    break;

	case '(':
	    return(LPAR);
	    break;

	case ')':
	    return(RPAR);
	    break;

	case '{':
	    return(LBRACE);
	    break;

	case '*':
	    return(STAR);
	    break;

	case '"':
	case '\'':
	    if (endquote(buffer[tokendx]) == EOF) {
		return(LISTEOF);
	    }
	    return(DONTCARE);
	    break;

	default:
	    break;
    }

    if ((isalpha((int) buffer[tokendx])) || (buffer[tokendx] == '_')) {
	for (charcount = 1; (isalnum((int) buffer[bufndx])) ||
	     (buffer[bufndx] == '_'); ++bufndx, ++charcount);

	return(LISTID);
    }

    if (isdigit(buffer[tokendx])) {
	for (	; isdigit((int) buffer[bufndx]); ++bufndx);
    }

    return(DONTCARE);
}


    /*	findsemi(filename)
     *
     *	finds the semicolon that ends a parameter declaration in a function def
     *
     *  calls:
     *	    - getnonblank( ) to find the first non-blank character in the buffer
     *	    - endquote(buffer[oldndx] to find the closing char of a quote string
     *	    - error(filename, string, SOFT) if there are too many '}'
     *
     *  returns the appropriate state of the finite state machine
     *  ERROR on unexpected EOF
     *  DECL when an appropriate semicolon is found
     *  START if a character or string inappropriate to a parameter declaration
     *	      is found
     */


int
findsemi(filename)

char	*filename;

{
    /* LISTER FUNCTIONS CALLED */
    extern int	getnonblank( ),
		endquote( ),
		error( );

    /* EXTERNAL VARIABLES USED */
    extern char	buffer[ ];
    extern int	bufndx;

    /* LOCAL VARIABLES */
    int		oldndx,
		bracecount;


    /*  a parameter declaration may contain a structure definition complete with
     *  curly braces.
     *  hence count braces (+ for { and - for }) and don't let the number become
     *  negative
     *  the curly braces in a declaration must balance.
     */
    bracecount = 0;

    for (	;	;	) {

	if (getnonblank( ) == EOF) {
	    return(ERROR);
	}

	for (	; isalnum((int)buffer[bufndx]) || buffer[bufndx] == '_' ; ++bufndx);
	oldndx = bufndx;
	++bufndx;

	switch (buffer[oldndx]) {
	    /*  note that no initialization may appear in a paramtetr def'n */
	    case '\'':
	    case '"':
		if (endquote(buffer[oldndx]) == EOF) {
		    return(ERROR);
		}
		/*	NO BREAK	*/

	    case '=':
		return(START);
		break;

	    case '{':
		++bracecount;
		break;

	    case '}':
		--bracecount;
		if (bracecount < 0) {
		    error(filename, "out of sync:  too many \"}\"", SOFT);
		    return(ERROR);
		}
		break;

	    case ';':
		if (bracecount == 0) {
		    return(DECL);
		}
		break;

	    default:
		break;
	}
    }
}

/*
*/
