/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)script:util2.c	1.6"
#include "script.h"
extern int termalt, termso, termfkey;
int rarrow, larrow, uarrow, darrow; /* arrows right, left, etc */
int callstate, alternate;
int call_row, call_col;
int cortl_c, cortr_c, corbl_c, corbr_c, row_c, col_c;

addcoch(alt_s)
int alt_s;
{
     if( (termalt==TRUE) && (alt_s== TRUE) ) {
	alternate = TRUE;
	cortl_c = TOPLEFT;
	cortr_c = TOPRIGHT;
	corbl_c = BOTLEFT;
	corbr_c = BOTRIGHT;
	row_c   = HOR;
	col_c   = VERT;
     }
     else {         /* either terminal doesnot have alt char set or alt not on */
	alternate = FALSE;
	cortl_c = cortr_c = corbl_c = corbr_c = ' ';
	row_c = '-';
	col_c = '|';
     }
     rarrow = RIGHTARROW;
     larrow = LEFTARROW;
     uarrow = UPARROW;
     darrow = DOWNARROW;
}

clrcoch()
{
        alternate = FALSE;
	cortl_c = cortr_c = corbl_c = corbr_c = ' ';
	row_c = col_c = ' ';
        larrow =rarrow = uarrow = darrow = ' ';
}


callout(ln_buf)
char *ln_buf;
{
     int kount;
     char b, *p;
     getyx(stdscr, call_row, call_col);
     callstate = BEGIN;
     p = ln_buf + 4 ;
     while ( (*p != '\n') && (*p != '\0') ) {
         while (*p == ' ') {
             p++;
         }
         if( (*p == '\n') || (*p == '\0')) {
                return(0);
         }
         b = *p++;
         kount = 0;
         while( (p[0]  >= '0' ) && (p[0]  <= '9') ) {
            kount = p[0] + 10*kount -'0';
            p++;
         }
         switch(b) {
             case 'a':
		if (alternate == TRUE) {
		   attroff(A_ALTCHARSET);
		   addarrow(kount);
		   attron(A_ALTCHARSET);
		}
		else addarrow(kount);
                break;

             case 'u':
                switch(callstate) {
                  case DOWN:
                  case UP:
                  case BEGIN:
                       addch(col_c);
                       upline(kount-1);
                       break;
		  case RIGHT:
		       addch(corbr_c);
		       upline(kount-1);
		       break;
		  case LEFT:
		       addch(corbl_c);
		       upline(kount-1);
		       break;
                  default:
                       fprintf(stderr,"Nonsensical case\n");
                       break;
                }
		callstate = UP;
		break;

	     case 'd':
	 	 switch(callstate) {
                   case UP:
		   case DOWN:
		   case BEGIN:
			addch(col_c);
			downline(kount-1);
			break;
		   case RIGHT:
			addch(cortr_c);
			downline(kount-1);
			break;
		   case LEFT:
			addch(cortl_c);
			downline(kount-1);
			break;
		   default:
			fprintf(stderr,"Nonsensical case\n");
			break;
		 }
		 callstate = DOWN;
		 break;

	      case 'r':
		  switch(callstate) {
		    case BEGIN:
		    case RIGHT:
                    case LEFT:
			 addch(row_c);
			 rightline(kount-1);
			 break;
		    case UP:
			 addch(cortl_c);
			 rightline(kount-1);
			 break;
		    case DOWN:
			 addch(corbl_c);
			 rightline(kount-1);
			 break;
		    default:
			 fprintf(stderr,"Nonsensical case\n");
			 break;
		  }
                  callstate = RIGHT;
		  break;

	      case 'l':
		  switch(callstate) {
		    case BEGIN:
		    case LEFT:
                    case RIGHT:
			 addch(row_c);
			 leftline(kount-1);
			 break;
		    case UP:
			 addch(cortr_c);
			 leftline(kount-1);
			 break;
		    case DOWN:
			 addch(corbr_c);
			 leftline(kount-1);
			 break;
		    default:
			 fprintf(stderr, "Nonsensical case\n");
			 break;
		   }
		   callstate = LEFT;
		   break;

	      default:
		 fprintf(stderr,"Non-acceptable direction-only adul & r\n");
		 break;
	  }
     }
}
                         
downline(n)
int n;
{
    move(++call_row,call_col);
    while (n-- > 0) {
	addch(col_c);
	move(++call_row, call_col);
    }
}

upline(n)
int n;
{
    move(--call_row, call_col);
    while (n-- > 0) {
	addch(col_c);
	move(--call_row, call_col);
    }
}

rightline(n)
int n;
{
    move(call_row, ++call_col);
    while (n-- > 0) {
	addch(row_c);
	move(call_row, ++call_col);
    }
}

leftline(n)
int n;
{
    move(call_row, --call_col);
    while (n-- > 0) {
	addch(row_c);
	move(call_row, --call_col);
    }
}

addarrow(n)
int n;
{
    switch(callstate) {
	case LEFT:
	   while (n-- > 0) {
	      addch(larrow);
	      move(call_row,--call_col);
	   }
	   break;
	case RIGHT:
	   while (n-- > 0) {
	      addch(rarrow);
	      move(call_row,++call_col);
	   }
	   break;
	case UP:
	   while (n-- > 0) {
	      addch(uarrow);
	      move(--call_row, call_col);
	   }
	   break;
	case DOWN:
	   while (n-- > 0) {
	      addch(darrow);
	      move(++call_row, call_col);
	   }
	   break;
	default:
	   fprintf(stderr, "Must have direction before arrow\n");
	   break;
    }
}

/*
 *  parse the ifterm lines to get terminal requirements
 */
getttest(p)
char *p;
{
     int t_test = TRUE;
     char *q;
     q = p;
     while ( (*p != '\0') && (t_test == TRUE) ) {
          switch(*p++) {
              case 'a' :
                   if (termalt==TRUE) t_test = FALSE;
                   break;
              case 'A':
                   if (termalt==FALSE) t_test = FALSE;
                   break;
              case 'f':
                   if (termfkey==TRUE) t_test = FALSE;
                   break;
              case 'F':
                   if (termfkey==FALSE) t_test = FALSE;
                   break;
              case  's':
                   if (termso==TRUE) t_test = FALSE;
                   break;
              case 'S':
                   if (termso==FALSE) t_test = FALSE;
                   break;
              default:
                   done2();
                   fprintf(stderr,"Only acceptable terminal ");
                   fprintf(stderr, "conditionals are a A f F s S\n");
                   fprintf(stderr, "Specified terminal string is: ");
                   fprintf(stderr, "%s\n", q);
                   exit(BADCMD);
                   break;
            }
       return(t_test);
      }
}
