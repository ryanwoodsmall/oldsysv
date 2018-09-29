/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:fill_out.c	1.28"
#include "muse.h"
#include "mmuse.h"

#define  MOVE {move(Std_row,Std_col-1);}

#define   INVALID      strcpy(error_mess," NOT A VALID ASSIST COMMAND ")
#define   TOPHELP     1000
#define   TOPHELP0    1001
#define   BLANK_OK    (dqflag || sqflag || slflag)

int find_field(), val_all(), shell(), expand(), cf(), implement();
int help(), menu(), pop_exit(), gotofield(), validate();

extern char *visible[];

/* FUNCTION THAT HANDLES ALL INTERACTIONS                                         */

VOID fill_out()
{
   register int c, ret;
   register int j, post_pop=0, literal=0;
   register int sqflag=0, dqflag=0, slflag=0, ll;
   char *action_pt;
   register struct segment *new_s_pt, *s_Segm_pt;
   struct field *s_Field_pt;
   int s_Stdscr_loc;
   int help_flag, s_mode;
   char *s = NULL;
   int badinsert = 0;

   arrow.row = arrow_buff.row = 0;
   arrow.col = arrow_buff.col = 0;
   strcpy(arrow.word,">");
   *arrow_buff.word = null;

   Stdscr_loc = -2;

   move(0,0); clrtobot();

   error_mess[0] = null;
   show_header();


   if (mode==MENU)
   {
      Field_pt = fields + lab_pt->memory;
      if (find_field(0,fields,last_field_pt)==0) 
         find_field(1,fields,last_field_pt);
      page = (Field_pt->row-1)/(SCRLINES-2) + 1;
      Stdscr_loc = (page-1)*(SCRLINES-2) - 2;
      copy_form_std(2);
      if (maxpage>1)
            mvprintw(SCRLINES+1,COLS-13," PAGE %1d OF %1d ", page,maxpage);
      refresh();
      highlight(Segm_pt,ON,(struct field *)0);
      MOVE;
   }
   else
   {
      command_line();
      show_cmd(command);
      Field_pt = fields;
      page = 1;
      if (find_field(0,fields,last_field_pt)==0) 
         find_field(1,fields,last_field_pt);
      page = (Field_pt->row-1)/(SCRLINES-2) + 1;
      Stdscr_loc = (page-1)*(SCRLINES-2) - 2;
      copy_form_std(2);
      if (maxpage>1)
            mvprintw(SCRLINES+1,COLS-13," PAGE %1d OF %1d ", page,maxpage);
      move(Std_row,Std_col);
   }

   refresh();
   if (*error_mess) beep();

   error_mess[0] = null;
   warn_mess[0] = null;

   for (;;)
   {
      c = getch();
      if (mode==MENU) 
         highlight(Segm_pt,OFF,(struct field *)0);
      if (mode!=INSERT) slflag=dqflag=sqflag=0;
      if (literal)
      {
         if (c==CTRL(J) || c==CTRL(M))
         {
            badinsert = 1;
            literal = 0;
            goto next;
         }
         else goto insert;
      }
HELP: while (c==KEY_F(6) || c==KEY_F(7) || c==KEY_F(8) || c==CTRL(A) ||
             c==CTRL(Y)  || c==CTRL(O)  || c==TOPHELP || c==TOPHELP0)
      {
         post_pop=ON;
         switch(c)
         {
         case KEY_F(8):
            if (mode==MENU) 
               help_flag=help(menu_help,-1);
            else
               help_flag=help(cf_help,-1);
            break;
         case CTRL(A):     /* SHOW KEY HELP                                     */
            if (mode==MENU) 
               help_flag=help(menu_help0,-1);
            else
               help_flag=help(cf_help0,-1);
            break;
         case TOPHELP:
            help_flag=help(top_help,-1);
            break;
         case TOPHELP0:
            help_flag=help(top_help0,-1);
            break;
         case CTRL(Y):     /* SHOW ITEM HELP                                    */
         case KEY_F(6):
            help_flag=help(Field_pt->help,Std_row);
            break;
         case CTRL(O):     /* CURRENT FORM HELP                                 */
         case KEY_F(7):
            help_flag=help(form_help,-1);
            break;
         }
         if (help_flag==0 || help_flag==3) 
            strcat(error_mess,
               " STRIKE ^V OR f2 TO CLEAR MESSAGE, OR TYPE ASSIST COMMAND ");
         else
            strcpy(error_mess,
               " STRIKE ^V OR f2 TO CLEAR MESSAGE, OR TYPE ASSIST COMMAND ");
         move(LINES-1,0); clrtoeol();
         REV;
         mvaddstr(LINES-1,0,error_mess);
         NREV;

         if (help_flag==1)
         {
            if (mode==MENU)
            {
               highlight(Segm_pt,ON,(struct field *)0);
               MOVE;
            }
            else 
               move(Std_row,Std_col);
         }
         else
            move(LINES-1,strlen(error_mess));
         refresh();
         c = getch();
         if (mode==MENU) 
            highlight(Segm_pt,OFF,(struct field *)0);
         copy_form_std(2);
         show_header();
         if (mode!=MENU) show_cmd(command);
         if (maxpage>1)
            mvprintw(SCRLINES+1,COLS-13," PAGE %1d of %1d ",page,maxpage);
         *error_mess = null;
      }


      move(LINES-1,0); clrtoeol();

      switch(c)
      {
      case KEY_F(3):    /* CLEAR SCREEN                                      */
      case CTRL(L):
         if (post_pop==ON)
         {
            copy_form_std(2);
            post_pop=OFF;
         }
         clearok(curscr,TRUE);
         wrefresh(curscr);
         break;
      case CTRL(V):
         if (post_pop==OFF)
            sprintf(error_mess,
          " USE ^V TO CLEAR HELP OR TO KILL PROMPT ");
         break;
      case KEY_F(2):
         if (post_pop==OFF)
            sprintf(error_mess,
          " USE f2 TO CLEAR HELP OR TO KILL PROMPT ");
         break;
      case CTRL(D):
         status=0;
         done();
         break;
      case CTRL(E):
         if (shell()==CTRL(R)) return;
         post_pop=ON;
         break;
      case KEY_LEFT:
      case CTRL(B):      /* MOVE CURSOR TO FIRST CHARACTER OF SEGMENT LEFT OF    */
                         /* CURRENT SEGMENT.  IF CURRENT IS FIRST, CREATE NEW    */
                         /* SEGMENT                                              */
         switch(mode)
         {
         case INSERT:
            sprintf(error_mess, 
               " FIRST HAVE WORD VALIDATED BY STRIKING RETURN ");
            break;
         case MENU: 
            strcpy(error_mess," NOT USED IN MENUS ");
            break;
         case NEWLSEGMENT:
            strcpy(error_mess," CANNOT MOVE FARTHER TO THE LEFT ");
            break;
         case NEWRSEGMENT:
            Segm_pt->word = NULL;  /* Abort newly started segment   */
            mode = NEWFIELD;
            update(Field_pt->last_row);
            break;
         default:
            if (Segm_pt==Field_pt->first_s_pt &&
                (Segm_pt->word==NULL || *(Segm_pt->word)==null))
               strcpy(error_mess," CANNOT MOVE FARTHER TO THE LEFT ");
            else 
               if ( mode==GRERROR || mode==WARN || expand(UPDATE)==1)
               {
                  if (mode==GRERROR) copy_form_std(2);
                  mode = NEWFIELD;
                  next_segment(-1);
               }
            break;
         }
         break;
      case CTRL(W):      /* ANALOGOUS TO CTRL(B)                                 */
      case KEY_RIGHT:
         switch(mode)
         {
         case MENU:
            strcpy(error_mess," NOT USED IN MENUS ");
            break;
         case INSERT:
            sprintf(error_mess,
               " FIRST HAVE WORD VALIDATED BY STRIKING RETURN ");
            break;
         case NEWRSEGMENT:
            strcpy(error_mess," CANNOT MOVE FARTHER TO THE RIGHT ");
            break;
         case NEWLSEGMENT:
            Segm_pt->word = NULL;  /* Abort newly started segment   */
            mode = NEWFIELD;
            update(Field_pt->last_row);
            break;
         default:
            if (Segm_pt==Field_pt->first_s_pt &&
                (Segm_pt->word==NULL || *(Segm_pt->word)==null))
               strcpy(error_mess," CANNOT MOVE FARTHER TO THE RIGHT ");
            else 
               if ( mode==GRERROR || mode==WARN || expand(UPDATE)==1)
               {
                     if (mode==GRERROR) copy_form_std(2);
                     mode = NEWFIELD;
                     next_segment(1);
               }
            break;
         }
         break;
      case CTRL(U):      /* PAGE DOWN                                            */
         switch(mode)
         {
         case INSERT:
            strcpy(error_mess,
               " FIRST HAVE WORD VALIDATED BY STRIKING RETURN ");
            break;
         case INCORRECT:
            validate(NO_UPDATE,1);  /* To recover error message   */
            break;
         default:
            if (maxpage>1)
            {
               if (page == maxpage)
                  page = 1;
               else page++;
               Stdscr_loc = (page-1)*(SCRLINES-2) - 2;
               for (Field_pt=fields;
                    Field_pt->row <= (page-1)*(SCRLINES-2);
                    Field_pt++);
               if (find_field(0,fields,last_field_pt)==0) 
                  find_field(1,fields,last_field_pt);
               copy_form_std(2);
            }
            else
               strcpy(error_mess," THERE IS ONLY ONE PAGE ");
            break;
         }
         break;
      case CTRL(T): 
         if (implement("TOP",1))
         {
            next_screen("",TOP);
            return;
         }
         break;
      case CTRL(P):      /* VALIDATE AND GO TO PRECEDING NON-HEADER FIELD        */
      case KEY_UP:
         switch (mode)
         {
         case MENU:
            find_field(-1,fields,last_field_pt);
            break;
         case NEWLSEGMENT:
         case NEWRSEGMENT:
            Segm_pt->word = NULL;  /* Abort newly started segment   */
            update(Field_pt->last_row);
            mode=NEWFIELD;
            find_field(-1,fields,last_field_pt);
            break;
         default:
            if ( mode==GRERROR || mode==WARN || expand(UPDATE)==1)
               {
                  if (mode==GRERROR) copy_form_std(2);
                  mode = NEWFIELD;
                  find_field(-1,fields,last_field_pt);
               }
         }
         break;
      case CTRL(M):      /* VALIDATE AND GO TO NEXT NON-HEADER FIELD        */
      case CTRL(N):
      case CTRL(J):
      case KEY_DOWN:
         switch (mode)
         {
         case MENU:
            find_field(1,fields,last_field_pt);
            break;
         case NEWLSEGMENT:
         case NEWRSEGMENT:
            Segm_pt->word = NULL;  /* Abort newly started segment   */
            update(Field_pt->last_row);
            mode=NEWFIELD;
            find_field(1,fields,last_field_pt);
            break;
         default:
            if ( mode==GRERROR || mode==WARN || expand(UPDATE)==1)
               {
                  if (mode==GRERROR) copy_form_std(2);
                  mode = NEWFIELD;
                  find_field(1,fields,last_field_pt);
               }
         }
         break;
      case CTRL(R):
      case KEY_F(4):
         if (equaln((lab_pt-1)->files_name,"UNIX",4) &&
                 equaln(lab_pt->files_name,"TOP",3))
         {
            REV;
            mvaddstr(LINES-1,0,
             " YOU ARE ABOUT TO EXIT ASSIST; STRIKE ");
            if (c==CTRL(R)) addstr("^R");
            else addstr("f4");
            addstr(" TO CONFIRM, ANY OTHER KEY TO CONTINUE ");
            NREV; 
            highlight(Segm_pt,ON,(struct field *)0);
            MOVE;
            refresh();
            c=getch();
            highlight(Segm_pt,OFF,(struct field *)0);
            move(LINES-1,0); clrtoeol();
         }
         if (c==CTRL(R) || c==KEY_F(4))
         {
            next_screen("",BACK);
            return;
         }
         break;
      case KEY_F(5):
      case CTRL(F):
         s_Field_pt = Field_pt;
         s_Segm_pt = Segm_pt;
         s_Stdscr_loc = Stdscr_loc;
         s_mode = mode;
         if (mode!=MENU) {
            show_cmd(command);
            mode = POPUP;
         }
         copy_form_std(2);
         show_header();
         ret=menu();
         switch(ret)
         {
         case 0:
            return;
            break;
         case 1:
            post_pop = ON;
            break;
         case 2:
            if (c==KEY_F(5)) c = TOPHELP;
            else c = TOPHELP0;
            Field_pt = s_Field_pt;
            Segm_pt = s_Segm_pt;
            Stdscr_loc = s_Stdscr_loc;
            mode = s_mode;
            goto HELP;
            break;
         case 3:     /* Reference */
            fixterm(); copy_form_std(2); show_header(); 
            clearok(curscr,TRUE); wrefresh(curscr);
            break;
         case 4:
            clear(); move(0,0); refresh();
            saveterm(); resetterm();
            updatetty(&termbuf);
            system("sh");
            catchtty(&termbuf);
            fixterm(); copy_form_std(2); show_header(); 
            clearok(curscr,TRUE); wrefresh(curscr);
            break;
         default:
            post_pop = ON;
            break;
         }
         Field_pt = s_Field_pt;
         Segm_pt = s_Segm_pt;
         Stdscr_loc = s_Stdscr_loc;
         mode = s_mode;
         break;
      case CTRL(G):
      case KEY_F(1):
         if (c==CTRL(G)) j=1; else j=2;
         switch(mode)
         {
         case MENU:
            if (*(action_pt = Field_pt->action) == null)
               strcpy(error_mess," NOT IMPLEMENTED ");
            else
            {
               if (*action_pt!=CTRL(X))
                           /* UNIX Command in executable menu */
               {
                  clear(); move(0,0); refresh();  saveterm(); resetterm();
                  updatetty(&termbuf);
                  ret = system(action_pt);
                  catchtty(&termbuf);
                  fixterm();
                  clearok(curscr,TRUE);
                  switch(ret)
                  {
                  case 21:          /* ^T  */
                  case 5376:
                     next_screen("",TOP);
                     return;
                     break;
                  case 22:          /* ^D, <DEL>  */
                  case 5632:
                  case 23: 
                  case 5888:
                     clear();
                     curses(OFF);
                     status=0;
                     done();
                     break;
                  default:         /* Continue with present screen */
                     copy_form_std(2);
                     show_header();
                     clearok(curscr,TRUE);
                     wrefresh(curscr);
                     break;
                  }
               }
               else
               {
                  action_pt++;              /* Skip ^X                       */
                  if (equal(action_pt,"UNIX"))
                  {
                     status=0; done();
                  }
                  else if (implement(action_pt,1))   /* Muse screen; skip ^X */
                  {
                     next_screen(action_pt,NEW);
                     return;
                  }
               }
            }
            break;
         default:
            if (mode==INSERT) 
            {
               switch(expand(UPDATE))
               {
               case 0:
                  break;
               case 1:
                  mode = NEWFIELD;
                  break;
               case 2:
                  break;
               }
            }
            j=1;
            if (val_all(NO_UPDATE) && (j=pop_exit()))
            {
               if (s!=NULL) {
                  free(s);
                  s = NULL;
               }
               s = (char*)calloc((unsigned)(
   strlen(command)+strlen(dum_cmd_name)+10),sizeof(char));
               sprintf(s,"%s%s",dum_cmd_name,command);
               post_pop=ON;
               if ((c=exec_cmd(j,s,ON))==CTRL(R) || c==KEY_F(4)) return;
               else if (c==CTRL(E)) if ((c=shell())==CTRL(R)) return;
            }
            else  
            {
               mode=NEWFIELD;
               if (j==0) post_pop=ON;
            }
            break;
         }
         break;
      case CTRL(K):
         switch(mode)
         {
         case MENU:
            strcpy(error_mess,
" ^K IS USED ONLY IN COMMAND FORMS (TO STORE COMMAND LINE IN A FILE) ");
            break;
         default:
            if (mode==INSERT) 
            {
               switch(expand(UPDATE))
               {
               case 0:
                  break;
               case 1:
                  mode = NEWFIELD;
                  break;
               case 2:
                  break;
               }
            }

            if (val_all(NO_UPDATE))
               write_file();
            else mode=NEWFIELD;

            break;
         }
         break;
      case SPACE:          /* TERMINATE CURRENTLY INSERTED  SEGMENT AND SET UP FOR */
                         /* NEW SEGMENT, OR WIPE OUT A PREVIOUSLY ENTERED SEGMENT*/
                         /* AT WHOSE FIRST CHARACTER THE CURSOR IS LOCATED       */
/*  TEMP DISABLED   ref: r.292
         if (Field_pt->bundle==2) goto insert;
*/
         switch(mode)
         {
         case MENU:
            strcpy(error_mess,
 " <SPACE> IS USED ONLY IN COMMAND FORMS ");
            break;
         case INSERT:
            if (BLANK_OK) goto insert;
            if (*(Segm_pt->word))  /* TERMINATE STRING TYPED IN      */
            {
               if (Field_pt->maxfsegms==1)
               {
                  strcpy(error_mess," ONE STRING ONLY ");
               }
               else
               {
                  if ( mode==GRERROR || mode==WARN || expand(UPDATE)==1)
                  {
                     if (mode==GRERROR) copy_form_std(2);
                     mode = NEWRSEGMENT;
                     Col = Segm_pt->col + length(Segm_pt->word) + 2;
                     Row = Segm_pt->row;
                     new_s_pt = last_segm_pt++;
                     new_s_pt->previous = Segm_pt;
                     if (Segm_pt->next != NULL)
                        (Segm_pt->next)->previous = new_s_pt;
                     new_s_pt->next = Segm_pt->next;
                     Segm_pt->next = new_s_pt;
                     Segm_pt = new_s_pt;
                     Segm_pt->word = buf_pt;  /* Set equal to \0, not NULL! */
                     TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
                     *buf_pt++ = null;
                     Field_pt->last_row = Segm_pt->row = Row;
                     update(Field_pt->last_row);
                     command_line();
                  }
               }
            }
            break;
         default:
            if (Segm_pt->word && *(Segm_pt->word))
                                            /* WIPING OUT NON_NULL STRINGS          */
            {
               j = Field_pt->last_row;
               wipe_out();
               update(j);
               command_line();
               post_pop = ON;
            }
            else 
               strcpy(error_mess," NOTHING TO ERASE ");
            break;
         }
         break;
      case '\010':                     /* BACKSPACE DURING INSERTION             */
         switch(mode)
         {
         case INSERT:
            if (*(Segm_pt->word) && buf_pt>Segm_pt->word)
            {
               j = length(Segm_pt->word);
               mvaddch(Segm_pt->row-Stdscr_loc,Segm_pt->col,SPACE);
               while (--j>0)  addch(SPACE);
               buf_pt--;
               if (buf_pt==Segm_pt->word || *(buf_pt-1) != '\\')
               switch(*buf_pt) {
                  case '"':
                     dqflag = 1-dqflag;
                     break;
                  case '\'':
                     sqflag = 1-sqflag;
                     break;
                  case '\\':
                     slflag = 1-slflag;
                     break;
                  default:
                     break;
               }
               *buf_pt = null;
               show(Segm_pt);
               update(Field_pt->last_row);
            }
            else
            {
               strcpy(error_mess," CANNOT BACKSPACE FURTHER ");
               mode = NEWFIELD;
            }
            break;
         case MENU:
            strcpy(error_mess,
" BACKSPACE IS USED ONLY IN COMMAND FORMS (WHILE ENTERING AN INPUT STRING) ");
            break;
         default:
            strcpy(error_mess,
" BACKSPACE IS USED ONLY WHILE ENTERING AN INPUT STRING ");
            break;
         }
         break;
      case CTRL(C):
         if (mode==MENU) 
            strcpy(error_mess,
"^C IS USED ONLY IN COMMAND FORMS (TO QUOTE CHARACTERS THAT ARE ASSIST COMMANDS");
         else
         {
            if (literal==OFF) literal=ON;
            else
            {
               literal=OFF;
               goto insert;
            }
         }
         break;
      default:                         /* CHARACTER INSERTION; FIRST CHARACTER   */
                                       /* WIPES OUT PREVIOUSLY ENTERED SEGMENT AT*/
                                       /* WHOSE FIRST CHARACTER THE CURSOR IS    */
                                       /* LOCATED                                */
/*
         isgraph(c)
*/
         {
            if (Field_pt->maxfsegms==0)
               INVALID;
            else if (mode==MENU)
            {
               if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
               {
                  if (gotofield(c,fields,last_field_pt) == 0)
                     sprintf(error_mess,
        " NO ITEM ON THIS MENU STARTS WITH \'%c\' OR \'%c\' ",
                     tolower(c),toupper(c));
               }
               else 
               {
                  if (c<=27) 
                     sprintf(error_mess,
                      " %s IS NOT A VALID ASSIST COMMAND ",visible[c]);
                  else 
                     sprintf(error_mess,
                      " %c IS NOT A VALID ASSIST COMMAND ",(char)c);
               }
            }
            else
            {
insert:        if (literal) literal=OFF;
               switch(c) {
                  case '"':
                     if (!slflag) dqflag = 1-dqflag;
                     break;
                  case '\'':
                     if (!slflag) sqflag = 1-sqflag;
                     break;
                  case '\\':
                     if (!slflag) slflag = 1-slflag;
                     break;
                  default:
                     slflag = 0;
                     break;
               }
               if (mode==NEWFIELD || mode==NEWRSEGMENT || 
                   mode==GRERROR || mode==INCORRECT || 
                   mode==NEWLSEGMENT || mode==WARN)
               {
                  if (mode==GRERROR) copy_form_std(2);
                  wipe_out();
                  mode = INSERT;
                  Segm_pt->word = buf_pt;
                  *buf_pt = null;
                  post_pop = ON;
               }
               if (mode==INSERT)
               {
                  if (Segm_pt->col + (ll=length(Segm_pt->word)) < COLS-2
                      && ll < COLS - 6 - cmd_col)
                  {
                     TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
                     *buf_pt++ = c;
                     *buf_pt = null;
                     if (Segm_pt->next==NULL)
                     {
                        show(Segm_pt);
                     }
                     else
                        update(Field_pt->last_row);
                  }
                  else
                  {  
                     if (ll < COLS - 6 - cmd_col)
                     {
                        TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
                        *buf_pt++ = c;
                        *buf_pt = null;
                        update(Field_pt->last_row);
                     }
                     else
                     {
                        strcpy(error_mess,
" INPUT STRING MUCH TOO LONG; TYPE ANY KEY TO CONTINUE ");
                        REV;
                        mvaddstr(LINES-1,0,error_mess);
                        NREV;
                        refresh();
                        getch();
                        *error_mess = null;
                        j = Field_pt->last_row;
                        wipe_out();
                        update(j);
                        command_line();
                        mode = INCORRECT;
                        Col = Segm_pt->col;
                        post_pop = ON;
                     }
                  }
               }
            }
         }
         break;
      }




      Row = Segm_pt->row;
      Col = (mode==INSERT) ? Segm_pt->col + length(Segm_pt->word) : Segm_pt->col;
next:
      if (badinsert)
            strcpy(error_mess,
                     " RETURN OR NEWLINE NOT ALLOWED IN INPUT STRING ");
      badinsert = 0;
      move(LINES-1,0);
      clrtoeol();
      REV;
      if (*error_mess) mvaddstr(LINES-1,0,error_mess);
      else if (*warn_mess) mvaddstr(LINES-1,0,warn_mess);
      NREV;


      if (mode!=INSERT || post_pop==ON)   /* Skip lots of stuff if mode=INSERT */
      {
         if (mode!=MENU) show_cmd(command); /* Calculate new SCRLINES */

         if (mode==NEWFIELD || mode==MENU) /* IF WE TABBED TO A FIELD     */
         {                                 /* NOT ON THE CURRENT SCREEN   */
            if (Std_row>SCRLINES || Std_row<3)
            {
               page = (Row-1)/(SCRLINES-2) + 1;
               Stdscr_loc  = (page-1)*(SCRLINES-2) - 2;
               copy_form_std(2);
               post_pop = OFF;
            }
         }

         if (post_pop==ON)
         {
            copy_form_std(2);
            post_pop=OFF;
         }

         show_header();
         if (mode==MENU)
         {
            move (Std_row,Std_col);   /* These two superfluous lines are  */
            refresh();                  /* needed for proper highlighting   */
            highlight(Segm_pt,ON,(struct field *)0);
            if (maxpage>1)
               mvprintw(SCRLINES+1,COLS-13," PAGE %1d OF %1d ", page,maxpage);
            move(Std_row,Std_col-1);
            refresh();
            if (*error_mess) beep();
         }
         else
         {
            show_cmd(command);
            refresh();
            if (*error_mess) beep();
         }
      }
      else                         /* Refresh if mode=INSERT                   */
      {
         move (Std_row,Std_col);
         refresh();
         if (*error_mess) beep();
      }

      *error_mess = null;
      *warn_mess = null;
   }
}
