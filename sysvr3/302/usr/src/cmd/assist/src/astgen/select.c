/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:select.c	1.24"

/*
 * select() draws a screen, and allows the user to move from field to field.
 * Two uses: 
 * 1: current field selector (no fl argument; retruns one fl,
 *    that points to the current field just selected (flag=0).
 * 2: selecting fields that are incompatible (etc.) with the 
 *    already selected current field (flag=1)
 * Major portions of this routine are copied from fill_out().
 */

char nums[] = {'0','1','2','3','4','5','6','7','8','9' };
#include "../forms/muse.h"
#include "vdefs.h"

VOID copy_form_std();

struct fieldlink *select(fl,s,flag)
struct fieldlink *fl;
int flag;
char *s;
{  static struct field *mem_f_pt=fields;
   struct field *f_pt, *f0_pt, *cap_f_pt;
   struct fieldlink *fl0;
   struct first_second *find_f_s();
   char title[80];
   int i;
   register int c, c0;
   char n = '0';
   int ask();
   extern int helpnum;
   extern char fthelp[];

   if (flag==0 || flag==2 || flag==3) cap_f_pt = (struct field *)0;
   else if (fl!=(struct fieldlink *)0) cap_f_pt = fl->f_pt;

   if (strlen(s)<80) strcpy(title,s);
   else {
      strncpy(title,s,75);
      *(title+75) = null;
      strcat(title," ...");
   }

   clear();

   SCRLINES = LINES-5;

   switch(flag) {
   case 0:
      Field_pt = mem_f_pt;
      break;
   case 1:
   case 2:
      Field_pt = fields;
      break;
   case 3: /* Field_pt set extenally, in cf_global.c */
      if (Field_pt < fields || Field_pt > last_field_pt)
         Field_pt = fields;
      break;
   }


   while (Field_pt->type==0 || Field_pt->type==9)   {
      if (Field_pt<last_field_pt) Field_pt++;
      else Field_pt = fields;
   }

   Row = Field_pt->row;
   Col = Field_pt->col+3;
   
   maxpage = (last_field_pt->row - 1) / (SCRLINES-4) + 1;

   page = ((Row==0)?1:(Row-1))/(SCRLINES-4) + 1;
   Stdscr_loc = (page-1) * (SCRLINES-4) - 4;

AA:
   colmove(1);
   copy_form_std(4);
   colmove(0);

   draw_bottom(flag,title,cap_f_pt);

   draw_line(LINES-3);
   page_num();

   switch(flag) {
   case 0:
   case 3:
      indicate(1,Field_pt);
      break;
   case 1:
      if (fl!=(struct fieldlink*)0) {
         for (fl0=fl->next; fl0!=(struct fieldlink*)0; fl0=fl0->next)
            indicate(1,fl0->f_pt);
         if (fl->f_pt!=(struct field*)0 && 
             fl->f_pt->row-Stdscr_loc > 4 &&
             fl->f_pt->row-Stdscr_loc <= SCRLINES)
            mvaddstr(fl->f_pt->row - Stdscr_loc,0,">>>");
      }
      move(Std_row,Std_col);
      break;
   case 2:
      loccopy();
      move(Std_row,1);
      break;
   }
   refresh();


   while ((c=getch())!=CTRL(T) && c!=KEY_F(5))
   {
      switch(flag) {
         case 0:
         case 3:
            indicate(0,Field_pt);
            break;
         case 1:
            move(Std_row,Std_col);
            break;
         case 2:
            move(Std_row,1);
            break;
      }
      draw_line(LINES-3);
      page_num();

      if (!isdigit(c)) n='0';

      switch(c)
      {
      case KEY_F(3):
      case CTRL(L):
         clearok(curscr,TRUE);
         wrefresh(curscr);
         break;
      case CTRL(F):  
         if (maxpage>1) {
            if (page == maxpage)
               page = 1;
            else page++;
            Stdscr_loc = (page-1)*(SCRLINES-4) -4;
            for (Field_pt=fields;Field_pt->row <= (page-1)*(SCRLINES-4);
                 Field_pt++);
            while (Field_pt->type==0 || Field_pt->type==9)   {
               if (Field_pt<last_field_pt) Field_pt++;
               else Field_pt = fields;
            }
            colmove(1);
            copy_form_std(4);
            colmove(0);
            if (flag==2) loccopy();
            else if (flag==1 && fl!=NULL) 
                  for (fl0=fl->next; fl0!=NULL; fl0=fl0->next)
                     indicate(1,fl0->f_pt);
            draw_bottom(flag,title,cap_f_pt);
         }
         else
            err_rpt(6,1);
         break;
      case CTRL(B):
         if (maxpage>1) {
            if (page == 1)
               page = maxpage;
            else page--;
            Stdscr_loc = (page-1)*(SCRLINES-4) - 4;
            for (Field_pt=fields;Field_pt->row <= (page-1)*(SCRLINES-4);
                 Field_pt++);
            while (Field_pt->type==0 || Field_pt->type==9)   {
               if (Field_pt<last_field_pt) Field_pt++;
               else Field_pt = fields;
            }
            colmove(1);
            copy_form_std(4);
            colmove(0);
            if (flag==2) loccopy();
            else if (flag==1 && fl!=NULL) 
                  for (fl0=fl->next; fl0!=NULL; fl0=fl0->next)
                     indicate(1,fl0->f_pt);
            draw_bottom(flag,title,cap_f_pt);
         }
         else
            err_rpt(6,1);
         break;
      case CTRL(P):  
      case KEY_UP:
         do {
            if (Field_pt>fields) Field_pt--;
            else Field_pt=last_field_pt;
         } while (Field_pt->type==0 || Field_pt->type==9);
         break;
      case CTRL(Y):
      case KEY_F(6):
         helpnum = 27+flag;
         store(stdscr,0,LINES,COLS);
         fhelp(fthelp,helpnum,4,3);
         store(stdscr,1,LINES,COLS);
         refresh();
         break;
      case CTRL(N): 
      case '\015':
      case KEY_DOWN:
         do {
            if (Field_pt<last_field_pt) Field_pt++;
            else Field_pt=fields;
         } while (Field_pt->type==0 || Field_pt->type==9);
         break;
      case CTRL(G):
      case KEY_F(1):
         switch(flag) {
         case 0:
         case 3:
            clear();
            if (flag==0) mem_f_pt=Field_pt;
            fl0 = (struct fieldlink *)malloc(sizeof(struct fieldlink));
            fl0->next = (struct fieldlink *)0;
            fl0->f_pt = Field_pt;
            return(fl0);
            break;
         case 1:
            if (fl!= (struct fieldlink *)0 && Field_pt==fl->f_pt)
               err_rpt(46,1);
            else {
               if (!is_in_fl(fl,Field_pt)) {
                  add_f_pt(fl,Field_pt);
                  indicate(1,Field_pt);
               }
               else {
                  rm_f_pt(fl,Field_pt);
                  indicate(0,Field_pt);
               }
            }
            break;
         case 2:
            err_rpt(6,1);
            break;
         }
         break;
      case CTRL(R):
      case KEY_F(4):
         switch(flag) {
         case 0:
            clear();
            return((struct fieldlink *)0);
            break;
         case 1:
            clear();
            return(fl);
            break;
         case 2:
            clear();
            return((struct fieldlink*)1);
            break;
         case 3:
            err_rpt(6,1);
            break;
         }
         break;
      default:
         if (flag==2 && isdigit(c)) {
            if (Field_pt->bundle!=1) {
               mvaddch(Field_pt->row-Stdscr_loc,0,nums[n-'0']);
               addch(nums[(char)c-'0']);
               Field_pt->loc = 10*(n-'0') + (char)c-'0';
               n = (char)c;
            }
            else err_rpt(45,1);
         }
         else if (isalpha(c)) {
            f0_pt=NULL;
            f_pt = (Field_pt==last_field_pt) ? fields : Field_pt+1;
            while (f_pt!=Field_pt && f0_pt==NULL)  {
               c0 = *(find_f_s(f_pt)->first);
               if (tolower(c)==tolower(c0) 
                   && f_pt->type!=0 && f_pt->type!=9) 
                  f0_pt=f_pt;
               f_pt = (f_pt==last_field_pt) ? fields : f_pt+1;
            }
            if (f0_pt!=NULL)
               Field_pt = f0_pt;
            else if (tolower(c) != tolower(*(find_f_s(Field_pt)->first)))
               err_rpt(3,1);
         }
         else err_rpt(6,1);
         break;
      }

      Row = Field_pt->row;
      Col = Field_pt->col+3;

/*
 * Page, if necessary.
 */
      page = (Row-1)/(SCRLINES-4) + 1;
      Stdscr_loc  =  (page-1)*(SCRLINES-4) - 4;
      colmove(1);
      copy_form_std(4);
      colmove(0);
      if (flag==2) loccopy();
      else if (flag==1 && fl!=NULL) 
         for (fl0=fl->next; fl0!=NULL; fl0=fl0->next)
            indicate(1,fl0->f_pt);
      draw_bottom(flag,title,cap_f_pt);

      switch(flag) {
         case 0:
         case 3:
            indicate(1,Field_pt);
            break;
         case 1:
            if (fl!=(struct fieldlink*)0 && fl->f_pt!=(struct field*)0) {
               i = fl->f_pt->row - Stdscr_loc;
               if (i>4 && i<=SCRLINES)
                  mvaddstr(i,0,">>>");
            }
            move(Std_row,Std_col);
            break;
         case 2:
            move(Std_row,1);
            break;
      }

      refresh();


   }

/*
 * When break out of loop (^T/f5), return NULL pointer
 */
   if (flag==1)  {
      move(LINES-4,0); clrtoeol(); beep();
      switch(ask(
"This would undo any changes you made on this screen.  Confirm (y/n):",
      LINES-4,0,"yn",32,fthelp)) {
      case 'y':
         return((struct fieldlink *)0);
         break;
      default:
         move(LINES-4,0); clrtoeol();
         goto AA;
         break;
      }
   }
   else if (flag==3)  {
      move(LINES-4,0); clrtoeol(); beep();
      switch(ask(
"Selected exit field will not be saved.  Confirm (y/n):",
      LINES-4,0,"yn",helpnum,fthelp)) {
      case 'y':
         return((struct fieldlink *)0);
         break;
      default:
         move(LINES-4,0); clrtoeol();
         goto AA;
         break;
      }
   }
   else return((struct fieldlink *)0);
}

VOID indicate(on_off,f_pt)
struct field *f_pt;
int on_off;
{  register int j, col;
   struct segment *s_pt;
   int row;

   if (f_pt == NULL) return(0);

   row = f_pt->row - Stdscr_loc;
   s_pt = f_pt->first_s_pt;

   if (row<=4|| row>SCRLINES) return(0);

   if (s_pt!=NULL && 
       (s_pt->col < f_pt->col || f_pt->caption==NULL ||
        *f_pt->caption==null))
      col = s_pt->col;
   else col = f_pt->col;

   col+=3;

   if (on_off) {
      ALT;
      for (j=1;j<col-1; j++) mvaddch(row,j,HOR);
      NALT;
      if (col>1) {
         mvaddch(row,col-1,'>');
         move(row,col-1);
      }
      else move(row,0);
   }

   else {
      for (j=1;j<col-1; j++) mvaddch(row,j,' ');
      if (col>1) mvaddch(row,col-1,' ');
   }
   return(1);
}

int rm_f_pt(fl,f_pt)
struct fieldlink *fl;
struct field *f_pt;
{  
   if (fl== (struct fieldlink*)0) return(0);
   for (;fl->next!=NULL && fl->next->f_pt!=f_pt; fl=fl->next);
   if (fl->next==NULL) return(0);     /* f_pt not found. */
   fl->next = fl->next->next;         /* drop f_pt.      */
   return(1);
}


int add_f_pt(fl,f_pt)
struct fieldlink *fl;
struct field *f_pt;
{  
   if (fl== (struct fieldlink*)0) return(0);
   for (; fl->next!=NULL; fl=fl->next);
   fl->next = (struct fieldlink *)malloc(sizeof(struct fieldlink));
   fl = fl->next;
   fl->f_pt = f_pt;
   fl->next = NULL;
   return(1);
}

int is_in_fl(fl,f_pt)
struct fieldlink *fl;
struct field *f_pt;
{  
   if (fl== (struct fieldlink*)0) return(0);
   for (; fl!=NULL; fl=fl->next)
      if (fl->f_pt==f_pt) return(1);
   return(0);
}


VOID loccopy()
{  struct field *f_pt;
   int row;
   char s[3];

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++) {
      row = f_pt->row-Stdscr_loc;
      if (row>4 && row<=SCRLINES && f_pt->type!=0 && f_pt->type!=9) {
         if (f_pt->bundle==1) 
            mvaddstr(row,0,"BU");
         else {
            sprintf(s,"%c%c",(f_pt->loc)/10 + '0',(f_pt->loc)%10 + '0');
            mvaddstr(row,0,s);
         }
      }
   }
}

draw_bottom(flag,title,f_pt)
int flag;
char *title;
struct field *f_pt;
{

   if (f_pt != (struct field*)0) show_caption(f_pt,title);
   else {
      mvaddstr(1,(COLS-strlen(title))/2,title);
      draw_line(3);
   }

   switch(flag) {
   case 0:  /* SELECT WORK FIELD */
   case 3:  /* SELECT EXIT FIELD */
      if (maxpage==1) show_cmd("",11);
      else show_cmd("",1);
      break;
   case 1:  /* MV valfuncs */
      if (maxpage==1) show_cmd("",12);
      else show_cmd("",3);
      break;
   case 2:  /* Order */
      if (maxpage==1) show_cmd("",10);
      else show_cmd("",9);
      break;
   }


   page_num();

}

page_num()
{
   page = (Row-1)/(SCRLINES-4) + 1;
   draw_line(LINES-3);
   if (maxpage>1)
      mvprintw(LINES-3,60,"[page %d of %d]",page, maxpage);
}


colmove(m_nm)
int m_nm;
{  struct field *f0_pt;
   if (m_nm)
   for (f0_pt=fields; f0_pt<=last_field_pt; f0_pt++) {
      f0_pt->col+=3;
      if (f0_pt->first_s_pt!=NULL) f0_pt->first_s_pt->col+=3;
   }
   else
   for (f0_pt=fields; f0_pt<=last_field_pt; f0_pt++) {
      f0_pt->col-=3;
      if (f0_pt->first_s_pt!=NULL) f0_pt->first_s_pt->col-=3;
   }
}
