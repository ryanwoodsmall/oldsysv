/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:bottom.c	1.14"

#include <curses.h>

show_cmd(msg,num)
char *msg;	/*string to be inserted*/
int num;	/*which command prompt template is desired 0-9*/
{
	int i;

	switch(num)	{
	case 0:	/*TOP Screen*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move   f1/^G Select");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw                      f6/^Y Help");
		clrtoeol();
		break;
	case 1:	/*Select (multi-page)*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move   f1/^G Select   ^F Next page   ^B Last page");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw                      f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 2:	/*ATTRIBUTES*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move         f1/^G ");
		addstr(msg);	/*context sensitive action*/
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R Select new field   f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 3:	/*INCOMP, REQUIRES, AT LEAST (multi-page)*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move     f1/^G (Un)pick   ^F Next page   ^B Last page");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'Attributes'   f6/^Y Help       f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 4:	/*VALIDATIONS: SELECTED WITH ARGS*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move          f8/^O Delete   f1/^G Edit arguments");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'Attributes'        f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 5:	/*FEDIT: NON-INSERT MODE*/
		mvaddstr(LINES-2,0, "Arrows,RETURN Move                     f8/^O Delete   f1/^G Change  f7/^E Editor");
		clrtoeol();
		mvaddstr(LINES-1,0,"f3/^L Redraw   f4/^R ");
		addstr(msg);	/*screen name to return to*/
		for (i=strlen(msg);i<18;i++)  /*pad with spaces*/
			addch(32);
		addstr("f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 6:	/*FEDIT: INSERT MODE*/
		mvaddstr(LINES-2,0, "[Enter text]   Arrows,RETURN End input      BACKSPACE Erase");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f6/^Y Help                   ^C Quote");
		clrtoeol();
		break;
	case 7:	/*FIELDTYPE: SINGLE LETTER INPUT (ask)*/
		mvaddstr(LINES-2,0, 
"[Enter character and RETURN]            f2/^V Restart mapping module");
		clrtoeol();
		mvaddstr(LINES-1,0, 
"f3/^L Redraw   f3/^R Field Attributes   f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 8:	/*FIELDTYPE: INSERT MODE (askstring)*/
		mvaddstr(LINES-2,0, "[Enter text]   Arrows,RETURN End input      BACKSPACE Erase");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f6/^Y Help                   ^C Quote               f7/^E Editor");
		clrtoeol();
		break;
	case 9:	/*ORDER (multi-page)*/
		mvaddstr(LINES-2,0, "[Enter digits]   Arrows,RETURN,first letter Move   ^F Next page   ^B Last page");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw     f4/^R 'Global'     f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 10:	/*ORDER (single page)*/
		mvaddstr(LINES-2,0, "[Enter digits]   Arrows,RETURN,first letter Move");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw     f4/^R 'Global'     f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 11:	/*Select (single-page)*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move   f1/^G Select");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw                      f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 12:	/*INCOMP, REQUIRES, AT LEAST (single-page)*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move        f1/^G (Un)pick");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'Attributes'      f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 13:	/*VALIDATIONS: SELECTED WITH NO ARGS*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move          f8/^O Delete");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'Attributes'        f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 14:	/*VALIDATIONS: NOT SELECTED*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move          f1/^G Select");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'Attributes'        f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 15:	/*ATTRIBUTES: when shell expansion not allowed*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R Select new field   f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 16:	/*ERROR PROMPT*/
		mvaddstr(LINES-2,0,"[Hit any key to return to editor]");
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
		break;
	case 17:	/*ATTRIBUTES*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move         f1/^G ");
		addstr(msg);	/*context sensitive action*/
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R 'validations'      f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 18:	/*WARNING/CONFIRMATION PROMPT (cf_global*/
		mvaddstr(LINES-2,0,"^R to return to Top astgen Menu");
		clrtoeol();
		mvaddstr(LINES-1,0,"Any other key to remain on this screen");
		clrtoeol();
		break;
	case 19:	/*prompt(): SINGLE LETTER INPUT (ask)*/
		mvaddstr(LINES-2,0, 
"[Enter character and RETURN]            f2/^V Restart mapping module");
		clrtoeol();
		mvaddstr(LINES-1,0, 
"f3/^L Redraw                            f6/^Y Help");
		clrtoeol();
		break;
	case 20:	/*prompt(): SINGLE LETTER INPUT (ask)*/
		mvaddstr(LINES-2,0, 
"[Enter character and RETURN]            f2/^V Restart prompt module");
		clrtoeol();
		mvaddstr(LINES-1,0, 
"f3/^L Redraw                            f6/^Y Help");
		clrtoeol();
		break;
	case 21:	/*LINES ALLOWING ASKSTRING()*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move         f1/^G change   f7/^E call editor");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R TOP astgen Menu    f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 22:	/*GLOBAL ATTRIBUTES (no select new field)*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move         f1/^G ");
		addstr(msg);	/*context sensitive action*/
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R TOP astgen Menu    f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 23:	/*ATTRIBUTES allowing ^E*/
		mvaddstr(LINES-2,0, "Arrows,RETURN,first letter Move         f1/^G change   f7/^E call editor");
		clrtoeol();
		mvaddstr(LINES-1,0, "f3/^L Redraw   f4/^R Select new field   f6/^Y Help     f5/^T TOP astgen Menu");
		clrtoeol();
		break;
	case 24:	/*ERROR ON VAL*/
		mvaddstr(LINES-2,0, "[Enter any character to try again]");
		clrtoeol();
		mvaddstr(LINES-1,0, "f8/^O Delete validation");
		clrtoeol();
		break;
	case 25:	/*WARNING PROMPT*/
		mvaddstr(LINES-2,0,"[Hit any key to continue]");
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
		break;
	case 26:	
		mvaddstr(LINES-2,0,"[Hit RETURN to re-type, f2/^V to restart mapping]");
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
		break;
	case 27:	/*ERROR PROMPT*/
		mvaddstr(LINES-2,0,
                   "[You are about to enter your editor.  Hit RETURN]");
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
		break;
	case 28:	/*ERROR PROMPT*/
		mvaddstr(LINES-2,0,
                   "[Use BACKSPACE to correct input.]");
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
		break;
	default:
		move(LINES-2,0);
		clrtoeol();
		move(LINES-1,0);
		clrtoeol();
	}; /*switch*/
}
