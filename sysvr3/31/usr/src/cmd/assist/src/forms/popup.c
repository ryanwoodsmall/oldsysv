/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:popup.c	1.8"
#include "muse.h"
#include "mmuse.h"

/* popup.c contains three functions:
 *  bottom_write(), cf(), refer(), and shell().
 */


/*
 * Displays prompt on bottom line.  Allows user to enter and edit 
 * string.  Returns 1 if user enters string, 2 if user
 * enters only a RETURN, and 0 o.w.  
 */
int bottom_write(prompt,flag,rmbl)   
char *prompt;  /* Prompt string */
int flag, rmbl;  /* flag=1: highlight current item (on menu) */
                 /* rmbl=1: remove superfluous blanks from user string */
{  register int c;
   register int i, j, i0;
   char *c_pt, *d_pt, *b_pt, helpmess[80];
   int help();
   int x, y;

/* Display prompt */
   move(LINES-1,0); clrtoeol();
   if (mode==MENU && flag) highlight(Segm_pt,ON,(struct field *)0);
   REV;
   mvaddstr(LINES-1,0,prompt);
   NREV;
   addstr(": ");
   j = strlen(prompt)+2;
   refresh();

/* Prepare bottom_str -- a global char* to store user input */
   i=0;
   i0 = LINES-1;
   c_pt = bottom_str + strlen(bottom_str);
   while (c_pt>bottom_str) *--c_pt = null;
   b_pt = c_pt = bottom_str;
   c = getch();

/* Main loop */
   while (c!=KEY_F(2) && c!= CTRL(V) && c!=CTRL(G) &&
          c!=KEY_F(1) && c!=CTRL(D) && c!='\015')  /* Exit chars */
   {
      if (c == CTRL(A) || c == KEY_F(8)) {  /* Show command help */
         getyx(stdscr,y,x);
         if (c==CTRL(A))
            sprintf(helpmess,
"^V  CANCEL PROMPT       ^G  EXECUTE PROMPT         ^D  EXIT ASSIST%c",'\n');
         else
            sprintf(helpmess,
"f2  CANCEL PROMPT       f1  EXECUTE PROMPT         ^D  EXIT ASSIST%c",'\n');
         help(helpmess,LINES-1-i);
         move(y,x);
         refresh();
         c = getch();
         if (c==KEY_F(2) || c== CTRL(V) || c==CTRL(G) ||
          c==KEY_F(1) || c==CTRL(D) || c=='\015') goto SWITCH;
         i0 = LINES - 1 - i;
         move(i0-1,0); clrtobot();
         copy_form_std(2);
         if (mode==MENU && flag)
            highlight(Segm_pt,ON,(struct field *)0);
         else if (mode!=MENU) show_cmd(command);
         REV;
         mvaddstr(i0,0,prompt);
         NREV;
         addstr(": ");
         j = strlen(prompt)+2;
         move(i0,j);
         b_pt = bottom_str;
         for (d_pt=bottom_str; *d_pt; d_pt++)
         {
            addch(*d_pt);
            if (++j>COLS-2)
            {
               if (*d_pt!=SPACE)
               {
                  move(i0,j-(d_pt-b_pt+1)); clrtoeol();
                  d_pt = b_pt-1;
               }
               j = 0;
               move(++i0,0); clrtoeol();
            }
            else if (*d_pt == SPACE) b_pt=d_pt+1;
            move(i0,j);
         }
      }  /* End of show command help */

      switch(c)
      {
         case '\010':   /* Backspace.  Non-trivial, because
                           function allows for dynamic
                           multi-line input. */
            if (c_pt>bottom_str)
            {
               *(c_pt--) = null;  *c_pt = null;  
               if (j==0)
               {
                  i0 = LINES - 1 - --i;
                  move(i0-1,0); clrtobot();
                  copy_form_std(2);
                  if (mode==MENU && flag)
                     highlight(Segm_pt,ON,(struct field *)0);
                  else if (mode!=MENU) show_cmd(command);
                  REV;
                  mvaddstr(i0,0,prompt);
                  NREV;
                  addstr(": ");
                  j = strlen(prompt)+2;
                  move(i0,j);
                  b_pt = bottom_str;
                  for (d_pt=bottom_str; *d_pt; d_pt++)
                  {
                     addch(*d_pt);
                     if (++j>COLS-2)
                     {
                        if (*d_pt!=SPACE)
                        {
                           move(i0,j-(d_pt-b_pt+1)); clrtoeol();
                           d_pt = b_pt-1;
                        }
                        j = 0;
                        move(--i0,0); clrtoeol();
                     }
                     else if (*d_pt == SPACE) b_pt=d_pt+1;
                     move(i0,j);
                  }
               }
               else
               {
                  mvaddch(LINES-1,--j,SPACE); move(LINES-1,j);
               }
            }
            else
               goto OUT;
            break;
         default:   /* Character insertion. Non-trivial for
                       same reason as backspace */
            if (!isprint(c)) beep();
            else
            {
               *(c_pt++) = c;  *c_pt = null; 
               if (j > COLS-3)
               {
                  if (i>LINES-4) goto OUT;
                  i0 = LINES - 1 - ++i;
                  move(i0,0); clrtobot();
                  REV;
                  mvaddstr(i0,0,prompt);
                  NREV;
                  addstr(": ");
                  j = strlen(prompt)+2;
                  move(i0,j);
                  b_pt = bottom_str;
                  for (d_pt=bottom_str; d_pt<c_pt; d_pt++)
                  {
                     addch(*d_pt);
                     if (++j>COLS-2)
                     {
                        if (*d_pt != SPACE)
                        {
                           move(i0,j-(d_pt-b_pt+1)); clrtoeol();
                           d_pt = b_pt-1;
                        }
                        j = 0;
                        move(++i0,0); clrtoeol();
                     }
                     else if (*d_pt == SPACE) b_pt=d_pt+1;
                     move(i0,j);
                  }
               }
               else
               {
                  mvaddch(LINES-1,j++,c); move(LINES-1,j);
               }
            }
            break;
      }

      refresh();
      c = getch();
   }

SWITCH:  /* Switch on exit character */
   switch(c)
   {
   case KEY_F(1):  /* "go" */
   case CTRL(G):
   case '\015':
      if (rmbl) rmblnks(bottom_str);
      move(LINES-1,strlen(prompt)+2);
      refresh();
      if (strlen(bottom_str)) return(1);
      else 
      {
         c_pt = bottom_str + strlen(bottom_str);
         while (c_pt>bottom_str) *--c_pt = null;
         return(2);
      }
      break;
   case KEY_F(2):  /* Abort bottom line prompting */
   case CTRL(V):
OUT:  move (2,0);
      clrtobot();
      copy_form_std(2);
      if (mode==MENU && flag)
         highlight(Segm_pt,ON,(struct field *)0);
      else if (mode != MENU) show_cmd(command);
      return(0);
      break;
   case CTRL(D):  /* Exit ASSIST */
      status=0;
      done();
      break;
   }
   return(0);
}




int cf()   /* Used by popup menu [COMMAND FORMS]. */
{  
   int code;
   if (code=bottom_write(
    "TYPE UNIX SYSTEM COMMAND [LEAVE BLANK TO GET MENU] ",1,1)) 
      return(code);
   move(LINES-1,0); clrtoeol();
   return(0);
}



int refer(flag)   /* Called from popup menu.  Collects keyword(s)
                     and passes "msearch .. " command line back
                     (stored in bottom_str)  */
int flag;   /* Obsolete; always 1 */
{  char s[500];
   int code;
   if (flag)
   code=bottom_write(
    "TYPE SEARCH WORD ",1,1);
   else
   code=bottom_write(
    "TYPE REFERENCE NAME OR STRIKE RETURN FOR MENU ",1,1);
   if (code)
   {
      if (flag && code==1) 
      {
         sprintf(s,"%smsearch -p -s %s -n%d unix",
                 musebin,bottom_str,LINES);
         strcpy(bottom_str,s);
         return(code);
      }
      else if (!flag)   /* Obsolete: always 1 */
      {
         sprintf(s,"%smsearch -m ",musebin);
         strcat(s,bottom_str);
         strcpy(bottom_str,s);
         return(code);
      }
   }
   move(LINES-1,0); clrtoeol();
   return(0);
}



#define  CMDSIZE   80

/* Shell escape prompter */
int shell()
{  register int c1 = CTRL(E);
   register int flag = 1;

   while (c1==CTRL(E))
   {
      if (bottom_write(" TYPE UNIX SYSTEM COMMAND",flag,0))
      {
         if ((c1=exec_cmd(0,bottom_str,2)) != CTRL(E)) return(c1);
         flag = 0;
      }
      else
      {
         move(LINES-1,0); clrtoeol();
         return(0);
      }
   }
   return(1);
}



