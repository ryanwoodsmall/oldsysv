/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/regerror.c	1.3"
/*
	return the a message explaining a regexp(5) error code
*/


char *
regerror(code)
int	code;
{
	switch( code ) {
	case 11:
		return "Range endpoint too large";
	case 16:
		return "Bad number";
	case 25:
		return "'\\digit' out of range";
	case 36:
		return "Illegal or missing delimiter";
	case 41:
		return "No remembered search string";
	case 42:
		return "'\\( \\)' imbalance";
	case 43:
		return "Too many '\\('";
	case 44:
		return "More than 2 numbers given in \\{ \\}";
	case 45:
		return "} expected after \\";
	case 46:
		return "First number exceeds second in \\{ \\}";
	case 49:
		return "[] imbalance";
	case 50:
		return "regular expression too long or too complex";
	}
	return "unknown code!";
}
