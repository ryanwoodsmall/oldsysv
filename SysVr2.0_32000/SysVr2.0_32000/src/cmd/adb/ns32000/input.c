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
/*	@(#)input.c	1.2	*/
/*
 *  Input routines 
 */

#include "defs.h"

SCCSID(@(#)input.c	1.2);

static char line[LINSIZ];

int eocmd(c)            /* TRUE if end of command has been reached */
    char c;
{
    return(c == EOR || c == ';');
}

char rdc()
{
	char c;

    do 
        c = readchar();
    while (c == SPACE || c == TB);

    return(c);
}

char readchar()
{
    if (eof)
        lastc = EOF;
    else {
        if (lp == 0) {
            lp = line;
            do {
                eof = (0 == read(infile, lp, 1));
                if (mkfault)
                    error((STRING) NULL);
            } while (( ! eof) && *lp++ != EOR);
            *lp = 0; 
            lp = line;
        }
        if (lastc = peekc)
            peekc = 0;
        else if (lastc = *lp) 
            lp++;
    }
    return(lastc);
}

int nextchar()
{
    if (eocmd(rdc())) {
        lp--; 
        return(0);
    }
    else 
        return(lastc);
}

char quotchar()
{
    if (readchar() == '\\')
        return(readchar());
    else if (lastc == '\'')
        return('\0');
    else 
        return(lastc);
}

void getformat(deformat)
    STRING deformat;
{
    STRING fptr = deformat; 
    int quote = FALSE;

    while (quote ? readchar() != EOR : ! eocmd(readchar()))
        if ((*fptr++ = lastc) == '"')
            quote = ~quote;
    lp--;
    if (fptr != deformat)
        *fptr++ = '\0';
}
