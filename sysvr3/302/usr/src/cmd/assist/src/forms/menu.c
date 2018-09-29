/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:menu.c	1.13"
#include  "muse.h"
#include "mmuse.h"
#define CLBOT        {move(LINES-1,0); clrtoeol(); move(LINES-1,0);}
int implement();

int menu()
{  
   register int c;
   int post_pop=0;
   register int i, j, row0, row1, col0, col1;
   register char *c_pt;
   register struct field *f_pt;
   register struct segment *s_pt;
   static struct field *mem_f_pt=menus;
   int help(), cf(), refer(), gotofield(), implement();
   char s[512], *getcwd();
   int help_flag;


   row0 = 2;
   row1 = row0 + m_lines + 3;
   col0 = 2;
   col1 = col0 + m_length + 3;

   draw_box(row0,row1,col0,col1);

   Field_pt = mem_f_pt;
   for (f_pt=menus, i=0;f_pt<=last_menu; f_pt++, i++)
   {
      (s_pt=f_pt->first_s_pt)->row = row0-1+i + Stdscr_loc + 2;
      s_pt->col = col0+2;
   }
   move(LINES-1,0); clrtoeol();
   redraw();
   highlight(Segm_pt=Field_pt->first_s_pt,1,(struct field *)0);
   move(i=row0+1+(int)(Field_pt-menus),j=col0+1);
   refresh();

   for (;;) {
      c = getch();
      highlight(Segm_pt=Field_pt->first_s_pt,0,(struct field *)0);
      move(LINES-1,0); clrtoeol();
      while (c==KEY_F(6) || c==KEY_F(7) || c==KEY_F(8) || c==CTRL(A) ||
             c==CTRL(O)  || c==CTRL(Y))
      {
         post_pop = 1;
         switch(c)
         {
         case KEY_F(8):    /* SHOW KEY HELP                           */
            help_flag = help(pu_help,-1);
            break;
         case CTRL(A):     /* SHOW KEY HELP                           */
            help_flag = help(pu_help0,-1);
            break;
         case CTRL(Y):     /* SHOW ITEM HELP                           */
         case KEY_F(6):
            help_flag = help(Field_pt->help,i);
            break;
         case CTRL(O):     /* CURRENT FORM HELP                        */
         case KEY_F(7):
            help_flag = help(pu_menu_help,-1);
            break;
         }
         CLBOT;
         strcpy(error_mess,
            " STRIKE ^V OR f2 TO CLEAR MESSAGE, OR TYPE ASSIST COMMAND ");
         REV; mvaddstr(LINES-1,0,error_mess); NREV;
         if (help_flag!=2) {
            highlight(Segm_pt=Field_pt->first_s_pt,1,(struct field *)0);
            i = row0+1+(int)(Field_pt-menus);
            j = col0+1;
            move(i,j);
         }
         else
            move(LINES-1,strlen(error_mess));
         refresh();
         c=getch();
         *error_mess = null;
         copy_form_std(2);
         show_header();
         if (mode!=MENU) show_cmd(command);
         if (maxpage>1)
            mvprintw(SCRLINES+1,COLS-13," PAGE %1d of %1d ",page,maxpage);
         draw_box(row0,row1,col0,col1);
         redraw();
      }
      move(LINES-1,0); clrtoeol();
      switch(c)
      {
      case CTRL(D):
         status=0; done();
         break;
      case KEY_F(4):
      case CTRL(R):
         return(1);
         break;
      case KEY_F(2):
      case CTRL(V):
         if (post_pop==OFF)
            sprintf(error_mess,
          " USE %s TO CLEAR HELP OR TO KILL PROMPT ",f[2]);
         break;
      case KEY_F(3):
      case CTRL(L):
         if (post_pop==ON)
         {
            copy_form_std(2);
            show_header();
            draw_box(row0,row1,col0,col1);
            redraw();
            post_pop = OFF;
         }
         clearok(curscr,TRUE);
         wrefresh(curscr);
         break;
      case CTRL(N):
      case '\015':
      case KEY_DOWN:
         if (Field_pt<last_menu) Field_pt++;
         else Field_pt = menus;
         while (Field_pt->type==0) 
         {
            if (Field_pt<last_menu) Field_pt++;
            else Field_pt = menus;
         }
         break;
      case CTRL(P):
      case KEY_UP:
         if (Field_pt>menus) Field_pt--;
         else Field_pt = last_menu;
         while (Field_pt->type==0) 
         {
            if (Field_pt>menus) Field_pt--;
            else Field_pt = last_menu;
         }
         break;
      case CTRL(G):
      case KEY_F(1):
         switch(Field_pt->loc)
         {
         case 0:
IN0:        switch(cf())
            {
            case 0:
               post_pop=ON;
               break;
            case 1:
               for (c_pt=bottom_str;*c_pt!=SPACE && *c_pt; c_pt++);
               *c_pt = null;
               if (implement(bottom_str,1))
               {
                  next_screen(bottom_str,NEW);
                  return(0);
               }
               else if (*bottom_str)
               {
                  CLBOT;
                  REV;
                  mvaddstr(LINES-1,0,error_mess);
                  addstr("[TYPE RETURN]");
                  NREV;
                  addstr(": ");
                  *error_mess = null;
                  refresh();
                  while ((c=getch())!=CTRL(N) && c!=CTRL(J) &&
                          c!=CTRL(M)) beep();
                  goto IN0;
               }
               break;
            case 2:
               if (implement("Cflist",0))
               {
                  next_screen("Cflist",NEW);
                  return(0);
               }
               break;
            default:
               post_pop = ON;
               break;
            }
            break;
         case 3:
            if (implement("assistwalk",0))
            {
               next_screen("assistwalk",NEW);
               return(0);
            }
            break;
         case 5:
            status = 0; done();
            break;
         case 8:
            next_screen("",TOP);
            return(0);
            break;
         case 7:
            if (implement("unixwalk",0))
            {
               next_screen("unixwalk",NEW);
               return(0);
            }
            break;
         case 6:
IN2:        sprintf(s," CURRENT DIRECTORY: %s  NEW DIRECTORY",
               getcwd((char *)NULL,256));
            if (strlen(s)>60)
               sprintf(s," NEW DIRECTORY");
            switch(bottom_write(s,1,1))
            {
            case 0:
            case 2:
               post_pop = ON;
               break;
            case 1:
               if (*bottom_str)
               {
                  shexp(bottom_str);
                  if (mchdir(bottom_str) == -1)
                  {
                     CLBOT;
                     REV;
                     mvaddstr(LINES-1,0,
       " DIRECTORY DOES NOT EXIST OR IS NOT ACCESSIBLE [TYPE RETURN]");
                     NREV;
                     addstr(": ");
                     refresh();
                     while ((c=getch())!=CTRL(N) && c!=CTRL(J) &&
                             c!=CTRL(M))  beep();
                     CLBOT;
                     goto IN2;
                  }
                  else
                  {
                     CLBOT;
                     sprintf(s,
                         " MOVED TO: %s [TYPE RETURN]", 
                          getcwd((char *)NULL,256));
                     if (strlen(s)>75)
                       sprintf(s," MOVED TO NEW DIRECTORY [TYPE RETURN]");
                     REV;
                     mvaddstr(LINES-1,0,s);
                     NREV;
                     addstr(": ");
                     refresh();
                     while ((c=getch())!=CTRL(J) && c!=CTRL(N) &&
                             c!=CTRL(M))  beep();
                     CLBOT;
                     return(1);
                   }
                }
                else
                   post_pop = ON;
               break;
            default:
               break;
            }
            break;
         case 1:
            if (refer(1)==1)
            {
               clear(); move(0,0); refresh();
               saveterm(); resetterm();
               updatetty(&termbuf);
               system(bottom_str);
               putchar('\n');
               catchtty(&termbuf);
               fixterm();
               strcpy(s, " STRIKE RETURN TO CONTINUE WITH");
               if (mode==MENU) strcat(s," MENU");
               else strcat(s," FORM");
               strcat(s, ", OR TYPE COMMAND NAME");
IN1:           bottom_write(s,0,1);
               for (c_pt=bottom_str;*c_pt!=SPACE && *c_pt; c_pt++);
               *c_pt = null;
               if (*bottom_str != null && implement(bottom_str,1)) 
               {
                  clear();
                  next_screen(bottom_str,NEW);
                  return(0);
               }
               else if (*bottom_str)
               {
                  REV;
                  mvaddstr(LINES-1,0,error_mess);
                  addstr("[TYPE RETURN]");
                  NREV;
                  addstr(": ");
                  *error_mess = null;
                  refresh();
                  while ((c=getch())!=CTRL(N) && c!=CTRL(J) &&
                          c!=CTRL(M)) beep();
                  goto IN1;
               }
               else if (*bottom_str==null)
               {
                  clear();
                  return(1);
               }
            }
            post_pop = ON;
            break;
         case 2:
            return(4);
            break;
         case 4:
            clear();
            move(0,0);
            refresh();
            if (c==KEY_F(1)) {
               sprintf(s,"cat %sg_help_t",forms);
            }
            else
               sprintf(s,"cat %sd_help_t",forms);
            saveterm(); resetterm();
            updatetty(&termbuf);
            system(s);
            while ((c=getchar()) != CTRL(N) && c!= CTRL(J));
            catchtty(&termbuf);
            fixterm();
            clear();
            return(1);
            break;
            }
         break;
      default:
         if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
         {
            if (gotofield(c,menus,last_menu) == 0)
            {
               sprintf(error_mess,
                      " NO ITEM ON THIS MENU STARTS WITH \'%c\' ",c);
               beep();
            }
         }
         else strcpy(error_mess,
                 " NOT A VALID COMMAND ON THE POPUP MENU ");
         break;
      }
      if (post_pop) 
      {
         post_pop = 0;
         show_header();
         copy_form_std(2);
         draw_box(row0,row1,col0,col1);
         redraw();
      }
      if (*error_mess) 
      {
         CLBOT;
         REV;
         mvaddstr(LINES-1,0,error_mess);
         NREV;
         *error_mess = null;
      }
      mem_f_pt = Field_pt;
      highlight(Segm_pt=Field_pt->first_s_pt,1,(struct field *)0);
      i = row0+1+(int)(Field_pt-menus);
      j = col0+1;
      move(i,j);
      refresh();
   }
}

redraw()
{  register struct field *f_pt;
   for (f_pt=menus; f_pt<=last_menu; f_pt++)
      show(f_pt->first_s_pt);
}

draw_box(r0,r1,c0,c1)
register int r0, r1, c0, c1;
{  register int i, j;

   for (i=r0 ;i<=r1;i++)
   {
      move(i,j=c0-1);
      for (; j<=c1+1; j++) addch(SPACE);
   }
   ALT;
   mvaddch(r0,c0,TL);
   mvaddch(r0,c1,TR);
   mvaddch(r1,c0,BL);
   mvaddch(r1,c1,BR);
   for (i=r0+1;i<=r1-1;i++)
   {
      mvaddch(i,c0,VER);
      mvaddch(i,c1,VER);
   }
   move(r0,j=c0+1);
   for (;j<=c1-1;j++) addch(HOR);
   move(r1,j=c0+1);
   for (;j<=c1-1;j++) addch(HOR);
   NALT;
}
