/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:show_cap.c	1.5"
#include "../forms/muse.h"

#define   dotdot() {if (*c_pt!=null) mvaddstr(ci,75," ...");}

show_caption(f_pt,caption)
struct field *f_pt;   /*current field pointer*/
char *caption;      /*string to be placed on row 0*/
{
   char *c_pt;
   int ci, cj;

   if (caption != NULL && *caption != null)
      mvaddstr(0,40-(strlen(caption) / 2),caption);
   clrtoeol();
   mvaddstr(1,0,"Current Field: ");
   getyx(stdscr,ci,cj);

   if (f_pt != NULL) {
      if (f_pt<last_field_pt && (f_pt+1)->row - f_pt->row == 1 
                             && f_pt->type == 0) {
         c_pt = f_pt->caption;
         for (;*c_pt && cj<79;c_pt++,cj++) addch(*c_pt);
         dotdot();
         move(ci=2,cj=17);
         c_pt = (f_pt+1)->caption;
         for (;*c_pt && cj<79;c_pt++,cj++) addch(*c_pt);
         dotdot();
      }
      else if (f_pt>fields && f_pt->row-(f_pt-1)->row==1 
                           && (f_pt-1)->type==0) {
         c_pt = (f_pt-1)->caption;
         for (;*c_pt && cj<79;c_pt++,cj++) addch(*c_pt);
         dotdot();
         move(ci=2,cj=17);
         c_pt = f_pt->caption;
         for (;*c_pt && cj<79;c_pt++,cj++) addch(*c_pt);
         dotdot();
      }
      else {     
         c_pt = f_pt->caption;
         for (;*c_pt && cj<79;c_pt++,cj++) addch(*c_pt);
         dotdot();
      }
   }

   else addstr("[none selected]");   /*null f_pt*/
   draw_line(3);
   draw_line(LINES-3);
}
