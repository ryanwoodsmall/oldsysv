/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:vsupport.c	1.36"

#include "../forms/muse.h"
#include "vdefs.h"

/*
 * This routine creates a template file, using the %/# convention,
 * from the datastructure.  It is not called when argc==1, i.e.,
 * when no .fs file was read in.
 */

extern char tf_name[];

VOID copy_form_file(tf_n0) /* Assumes 0 or 1 segments per field */
register char *tf_n0;
{
   register i=0, I=0, i0=0, j, col;
   register struct field *f_pt;
   register struct segment *s_pt;
   register FILE *tf_pt;
   int sflag, cflag, cl, sl;
   char first[1024], second[1024];

   if ((tf_pt=fopen(tf_n0,"w")) == NULL) {
      status = 2;
      vdone();
   }
   /*
 * Go thru fields 1 by 1; determine whether caption precedes
 * segment, vice versa, or whether one is missing; store
 * string(s) in first and second, to be printed out in that
 * (horizontal) order.
 * Eliminate certain illegal cases along the way.
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      cflag = (f_pt->caption!=NULL && *f_pt->caption!=null);
      s_pt = f_pt->first_s_pt;
      if (cflag) cl=strlen(f_pt->caption);
      else cl=0;
      sflag = (s_pt!=NULL && s_pt->word!=NULL && *s_pt->word!=null);
      if (sflag) sl=strlen(s_pt->word);
      else sl=0;
      if (cl>1022 || sl>1022) {
         fclose(tf_pt);
         unlink(tf_n0);
         endwin();
         fprintf(stderr," Caption or segment exceeds 1024 characterss \n");
         exit(1);
      }
      for (i=0;i<1023;i++) first[i] = second[i] = null;

      if (cflag) {
         if (sflag) {
            if (f_pt->col<s_pt->col) {
               col = f_pt->col;
               sccat(first,f_pt->caption);
               strcat(first,"%");
               i = s_pt->col - col - strlen(f_pt->caption);
               if (i>1) {
                  strcat(second," ");
                  --i;
                  if (i>1) while (--i>0) strcat(second," ");
                  sccat(second,s_pt->word);
               }
               else
                  sccat(second,s_pt->word);
            }
            else {
               col = s_pt->col;
               sccat(first,s_pt->word);
               strcat(first,"#");
               i = f_pt->col - col - strlen(s_pt->word);
               if (i>1) {
                  strcat(second," ");
                  --i;
                  if (i>1) while (--i>0) strcat(second," ");
                  sccat(second,f_pt->caption);
               }
               else
                  sccat(second,f_pt->caption);
            }
         }
         else {
            col = f_pt->col;
            sccat(first,f_pt->caption);
            if (f_pt->type!=0 && f_pt->type!=9) strcat(first,"%");
         }
      }
      else {
         if (sflag) {
            col = s_pt->col-1;
            strcat(first,"%");
            sccat(first,s_pt->word);
         }
         else {
            status=5;
            vdone();
         }
      }

      for (i0=I+1; i0<f_pt->row; i0++) putc('\n',tf_pt);
      I = f_pt->row;
      for (j=0; j<col; j++) putc(' ',tf_pt);
      esc_amp(first);
      esc_amp(second);
      if (*first) fprintf(tf_pt,"%s",first);
      if (*second) fprintf(tf_pt,"%s",second);
      if (f_pt->type==0) putc('&',tf_pt);
      putc('\n',tf_pt);
   }
   fclose(tf_pt);
}


esc_amp(s)   /* Routine that escapes true ampersands */
char *s;
{  char s0[1024], *c_pt, *c0_pt;
   if (s==NULL || *s==null) return(0);
   strcpy(s0,s);
   c0_pt = s0;
   c_pt = s;
   while (*c0_pt) {
      if (*c0_pt=='&') *c_pt++ = '\\';
      *c_pt++ = *c0_pt++;
   }
   *c_pt = null;
   return(1);
}


/*
 * The next routine reads a template file line for line,
 * and updates the MUSE data structure.
 * It parses any on-blank line using the %/# convention.
 * It stores strings and coordinates in a temporary field, f,
 * and segment, s = *f.first_s_pt.
 * It then tries to match (using the prompt() routine) the
 * field to a field in fields[].
 */

#define BEGIN     00
#define EXPSEGM   01
#define INQ       02
#define INSEGM    03
#define EXPCAP    04
#define INCAP     05

VOID copy_file_form(fp0,flag)
register FILE *fp0;
int flag;   /* =argc: if 1, no interactive prompting */
{  
   register char *c_pt;
   char str[1024], caption[1024], word[1024];
   struct field f0;
   struct segment s;
   int c, state, j=0, i=1, col=0;
   int slflag = 0;

   f0.first_s_pt = NULL;
   f0.first_v_pt = NULL;
   f0.first_i_pt = NULL;
   f0.first_m_pt = NULL;
   f0.first_a_pt = NULL;
   f0.op_name    = NULL;
   f0.def_arg    = NULL;
   f0.loc = f0.bundle = f0.row = f0.col = 0;
   f0.last_row = f0.group = f0.maxfsegms = f0.minfsegms = 0;
   f0.action = &nullchar;
   f0.help = &nullchar;
   f0.first_pr_pt = NULL;
   f0.first_po_pt = NULL;
   f0.first_cpr_pt = NULL;
   f0.first_cpo_pt = NULL;

   *(c_pt = str) = null;
   state = BEGIN;

   *caption = *word = null;
   f0.caption = caption;
   s.word = word;
   s.next = (struct segment*)0;
   f0.first_s_pt = &s;

   while ((c=getc(fp0))!=EOF) {
      if ( c=='\\' && !slflag) slflag = 1; /* set flag; get next char */
      else if (slflag) goto DFLT;       /* every char as default   */
      else                                 /* ordinary case           */
         switch(c) {
         case ' ':
            switch(state) {
            case EXPSEGM:  /* Do not allow blanks after %  */
               status = 6;
               vdone();
               break;
            case BEGIN:    /* Skip initial blanks          */
               break;
            default:       /* INQ or INSEGM                */
               *c_pt++ = c;
               *c_pt = null;
               break;
            }
            j++;
            if (slflag) slflag=0;
            break;
         case '#':
            switch(state) {
            case INQ:
               strcpy(s.word,str);
               s.row = i;
               s.col = col;
               state = EXPCAP;
               col = j+1;
               break;
            default:
               status = 6;
               vdone();
               break;
            }
            *(c_pt=str) = null;
            j++;
            f0.type=1;
            break;
         case '%':
            switch(state) {
            case BEGIN:
               col = j+1;
               break;
            case INQ:
               strcpy(f0.caption,str);
               f0.row = f0.last_row = i;
               f0.col = col;
               col = j+1;
               c_pt = str;
               break;
            default:
               status = 6;
               vdone();
            }
            *(c_pt=str) = null;
            state = EXPSEGM;
            f0.type=1;
            j++;
            break;
         case '\n':
            switch(state) {
            case BEGIN:
               break;
            case INQ:   /* MUST BE A HEADER, CAPTION-ONLY, FIELD */
               if (ampersand(str)) f0.type=0;
               else f0.type=9;
               strcpy(f0.caption,str);
               f0.row = f0.last_row = i;
               f0.col = col;
               *s.word = null;
               if (prompt(&f0,flag)==0) return(0);
               break;
            case INCAP:
               strcpy(f0.caption,str);
               f0.row = f0.last_row = i;
               f0.col = col;
               if (prompt(&f0,flag)==0) return(0);
               break;
            case EXPSEGM:       /* % followed by nothing   */
               *(s.word) = null;
               s.row = i;
               s.col = col;
               if (f0.caption==NULL || *(f0.caption)==null) {
                  f0.row  = f0.last_row = i;
                  f0.col = col;
               }
               if (prompt(&f0,flag)==0) return(0);
               break;
            case EXPCAP:        /* # followed by nothing   */
               *f0.caption = null;
               f0.row  = f0.last_row = i;
               f0.col = col;
               if (prompt(&f0,flag)==0) return(0);
               break;
            case INSEGM:        /* % followed by something */
               strcpy(s.word,str);
               s.row = i;
               s.col = col;
               if (f0.caption==NULL || *(f0.caption)==null) {
                  f0.row  = f0.last_row = i;
                  f0.col = col;
               }
               if (prompt(&f0,flag)==0) return(0);
               break;
            default:
               status = 6;
               vdone();
            }
            /* Reset everythin, for next line */
            *(c_pt=str) = null;
            *f0.caption = *s.word = null;
            f0.type = 0;
            state = BEGIN;
            j = 0;
            i++;
            break;
         default:       /* non-blanks other than \n, #, and %   */
   DFLT:    switch(state) {
            case BEGIN:
               state = INQ;
               col = j;
               *c_pt++ = c; 
               *c_pt = null;
               break;
            case INQ:
            case INSEGM:
            case INCAP:
               *c_pt++ = c; 
               *c_pt = null;
               break;
            case EXPSEGM:
               state = INSEGM;
               *c_pt++ = c; 
               *c_pt = null;
               break;
            case EXPCAP:
               state = INCAP;
               *c_pt++ = c; 
               *c_pt = null;
               break;
            default:
               status = 6;
               vdone();
            }
            j++;
            if (slflag) slflag=0;
            break;
         }
   }
   return(1);
}

/*
 * The prompt() routine attempts to match a temporary field
 * constructed in copy_file_form() with an element of fields[].
 * If there is no EXACT match (of BOTH caption and first_s_pt->word),
 * it asks the user whether the field is entirely new.  If yes,
 * temp field is copied onto last_field_pt.  If no, it cycles
 * through those elements of fields[] that have not been asigned
 * yet during previous prompts.
 */

VOID prompt(f_pt,flag)
register struct field *f_pt;
int flag;   /* = argc; if ==1, no interactive prompting 
               -- all fields new */
{  
   static int taken[MAXFIELDS];
   register struct field *f0_pt;
   int *i_pt;
   char first[1024], second[1024];
   char first0[1024], second0[1024];
   struct first_second *find_f_s();
   static int firstf=0;   /* Special treatment if flag==1 */
   extern char fthelp[128];
   extern int helpnum;
   int test, ci, cj;

   /*
 * Called at beginning of layout()
 */
   if (flag==3) {
      for (i_pt=taken; i_pt<taken+MAXFIELDS; i_pt++) *i_pt=0;
      return(1);
   }

   /*
 * This is a cleanup procedure invoked after all prompting
 * has been finished.  It looks for fields that have been
 * eliminated in vi, and copies the last field into their location.
 * Conceptually, this procedure does not belong in prompt();
 * but it is handy to make use of the static array taken[].
 */
   if (f_pt==(struct field*)0) {

   /*
    * First remove references to non-take fields
    */
      for (f0_pt=fields,i_pt=taken; f0_pt<=last_field_pt; 
          f0_pt++,i_pt++)
         if (*i_pt==0 || f0_pt->type==0 || f0_pt->type==9)
            rm_ref(f0_pt,taken); /* Remove references to this field */

   /*
    * Now close gaps in fields[].
    */
      for (f0_pt=fields,i_pt=taken; f0_pt<=last_field_pt; 
          f0_pt++,i_pt++)
         if (*i_pt==0) {
            if (f0_pt->type==0 || f0_pt->type==9)
               rm_ref(f0_pt,taken); /* Remove references to this field */
            while  (*(taken + (int)(last_field_pt-fields))==0 &&
                last_field_pt > f0_pt) last_field_pt--;
            if (last_field_pt > f0_pt)
               fcopy(f0_pt,last_field_pt--);
            else {
               last_field_pt = (f0_pt>fields) ? f0_pt-1 : fields;
               return(1);
            }
         }
      return(1);
   }

   if (flag==1)   /* No old .fs file  */
      goto new;

   /*
 * Now get on with the real stuff.
 */

REPROMPT:
   strcpy(first,find_f_s(f_pt)->first);
   strcpy(second,find_f_s(f_pt)->second);


   /*
 * First try to find an exact match; if found, return and continue
 * with copy_file_form(). i_pt and taken[] keep track of which
 * elements of fields[] already have been assigned to a screen line.
 */

   for (f0_pt=fields,i_pt=taken; f0_pt<=last_field_pt; f0_pt++,i_pt++) {
      if (*i_pt==0) {                                /* Not taken yet */

         strcpy(first0,find_f_s(f0_pt)->first);
         strcpy(second0,find_f_s(f0_pt)->second);

         if (fequal(f_pt,f0_pt)==1)  {
            part_fcopy(f0_pt,f_pt);
            *i_pt = 1;
            return(1);
         }
      }
   }


   /*
 * If program got here, exact match failed.  FIrst ask whether
 * non-matchable field is entirely new.
 */

   move(0,0); 
   clrtobot();
   show_cmd("",20);
   draw_line(LINES-3);
   move(1,2);
   if (*first) addstr(first);
   if (*second) addstr(second);


XX:
   helpnum = 23;
   switch(ask( "Is the line above a new line or a changed line (n/c):",3,2,"nc",helpnum,fthelp)) {
   case 'n':
      addstr("ew"); refresh();
new:
      if (flag==1 && firstf==0) {
         firstf = 1;
         initfp(last_field_pt);
      }
      else
         initfp(++last_field_pt);
      part_fcopy(last_field_pt,f_pt);
      i_pt = taken + (int)(last_field_pt-fields);
      *i_pt = 1;
      return(1);
      break;
   case 'c':
      test = 0;  /* Check if there are any fields left */
      for (i_pt=taken; i_pt <= taken + (int)(last_field_pt-fields); 
         i_pt++) {
         if (*i_pt == 0) test = 1;
      }
      if (test==0) {
         beep();
         getyx(stdscr,ci,cj);
         mvaddstr(ci+1,2,
"Cannot find old field that you could have changed.  Type any key to continue");
         refresh();
         getch();
         move(ci+1,2);
         clrtoeol();
         goto XX;
      }
      addstr("hanged"); refresh();
      break;
   case CTRL(V):
   case KEY_F(2):
      clear();
      for (i_pt=taken; i_pt<taken+MAXFIELDS; i_pt++) *i_pt=0;
      return(0);
      break;
   case CTRL(T):
   case KEY_F(5):
   case CTRL(R):
   case KEY_F(4):
      beep();
      goto XX;
      break;
   }


/*
 * If program got here, exact match failed BUT field is not new,
 * as least so the user says.
 */

   move(3,2); 
   clrtoeol();

   for (f0_pt=fields,i_pt=taken; f0_pt<=last_field_pt; f0_pt++,i_pt++) {
      if (*i_pt==0) {
         move(5,2);  
         clrtoeol();  
         move(5,2);
         strcpy(first0,find_f_s(f0_pt)->first);
         strcpy(second0,find_f_s(f0_pt)->second);
         if (*first0) addstr(first0);
         if (*second0) addstr(second0);
         refresh();
/*
 * If 'y', copy x/y and caption/word onto old field; keep other
 * info about old field intact -- it may just be a minor
 * rewrite of the caption.
 */

YY:
         helpnum = 24;
         move(3,0); 
         clrtoeol();
         switch(ask(
      "Does the line above replace the line below (y/n):",3,2,"yn",helpnum,fthelp)) {
         case 'y':
            addstr("es"); refresh();
            part_fcopy(f0_pt,f_pt);
            *i_pt = 1;
            return(1);
            break;
         case 'n':
            addstr("o"); refresh();
            break;
         case CTRL(V):
         case KEY_F(2):
            clear();
            return(0);
            break;
         case CTRL(T):
         case KEY_F(5):
         case CTRL(R):
         case KEY_F(4):
            beep();
            goto YY;
            break;
         }



         if (f0_pt==last_field_pt) {
/*
 *  If this is so, the user must have goofed, and has some
 *  explaining to do.
 */
            move(3,2); 
            clrtoeol();
            move(5,2); 
            clrtoeol();
ZZ:
            helpnum = 25;
            switch(ask(
" You responded with \"n\" to all fields. Is the line above a new line (y/n): ",
            3,2,"yn",helpnum,fthelp)) {
            case 'y':
               addstr("es"); refresh();
               initfp(++last_field_pt);
               part_fcopy(last_field_pt,f_pt);
               i_pt = taken + (int)(last_field_pt-fields);
               *i_pt = 1;
               return(1);
               break;
            case 'n':
               addstr("o"); refresh();
               break;
            case CTRL(V):
            case KEY_F(2):
               clear();
               return(0);
               break;
            case CTRL(T):
            case KEY_F(5):
            case CTRL(R):
            case KEY_F(4):
               beep();
               goto ZZ;
               break;
            }

            move(3,2); 
            clrtoeol();
AA:
            helpnum = 26;
            switch(ask(
"Neither new nor old? (f:Reprompt this field, c:Reprompt complete form):",
            3,2,"fc",helpnum,fthelp)) {
            case 'f':
               addstr("ield"); refresh();
               goto REPROMPT;
               break;
            case 'c':
               addstr("omplete"); refresh();
               for (i_pt=taken; i_pt<=taken+(int)(last_field_pt-fields);
                   *i_pt++ = 0);
               return(0);
               break;
            case CTRL(V):
            case KEY_F(2):
               clear();
               return(0);
               break;
            case CTRL(T):
            case KEY_F(5):
            case CTRL(R):
            case KEY_F(4):
               beep();
               goto AA;
               break;
            }
         }
      }
   }
}


initfp(f_pt)
struct field *f_pt;
{  
   f_pt->first_s_pt = NULL;
   f_pt->first_v_pt = NULL;
   f_pt->first_i_pt = NULL;
   f_pt->first_m_pt = NULL;
   f_pt->first_a_pt = NULL;
   f_pt->op_name    = NULL;
   f_pt->def_arg    = NULL;
   f_pt->loc = f_pt->bundle = 0;
   f_pt->caption = &nullchar;
   f_pt->action = &nullchar;
   f_pt->help = &nullchar;
   f_pt->first_pr_pt = NULL;
   f_pt->first_po_pt = NULL;
   dflt(f_pt);
}

dflt(f_pt)
struct field *f_pt;        /* Specify default mapping */
{  struct fix *p_pt;

   if (mode!=MENU) {
      if (f_pt->maxfsegms<1) f_pt->maxfsegms = 30;
      f_pt->type = 7;
      p_pt = f_pt->first_cpr_pt 
           = (struct fix*)malloc(sizeof(struct fix));
      p_pt->name = (char*)calloc(5,sizeof(char));
      strcpy(p_pt->name,"1110");
      p_pt = p_pt->next
           = (struct fix*)malloc(sizeof(struct fix));
      p_pt->name = &nullchar;
      p_pt = p_pt->next
           = (struct fix*)malloc(sizeof(struct fix));
      p_pt->name = &nullchar;
      p_pt->next = NULL;
   
      p_pt = f_pt->first_cpo_pt 
           = (struct fix*)malloc(sizeof(struct fix));
      p_pt->name = &nullchar;
      p_pt = p_pt->next
             = (struct fix*)malloc(sizeof(struct fix));
      p_pt->name = &nullchar;
      p_pt->next = NULL;
   }
   else { /* mode = MENU */
      f_pt->type = 1;
      f_pt->maxfsegms = 1;
      f_pt->action = &nullchar;
   }
}

part_fcopy(of_pt,if_pt) /* Partial field copy: x/y caption/word only */
                        /* Preserves certain types of information in */
                        /* output field -- of_pt.                    */
register struct field *of_pt, *if_pt;
{  
   register struct segment *os_pt, *is_pt=if_pt->first_s_pt;

/* If there is a # or %, but output field is header or obsolete 
   type, set output field to default no-header type */

   if (mode!=MENU && if_pt->type==1 && of_pt->type!=7) dflt(of_pt);
   else if (mode==MENU && if_pt->type==1 
            && of_pt->type!=7 && of_pt->type!=1) 
      dflt(of_pt);

/* There is no # or %, in which case output field must
   be made into (same) header type: */

   if (if_pt->type!=1) of_pt->type = if_pt->type;

/* Caption */


   if (if_pt->caption==NULL || *(if_pt->caption)==null) {
      of_pt->caption = (char*)calloc(2,sizeof(char));
      *(of_pt->caption)=null;
   }
   else {
      of_pt->caption = 
         (char*)calloc((unsigned)(strlen(if_pt->caption)+2),sizeof(char));
      strcpy(of_pt->caption,if_pt->caption);
   }
 
/* x-y coordinates */

   of_pt->row = if_pt->row;
   of_pt->last_row = if_pt->last_row;
   of_pt->col = if_pt->col;


/* Segments */
   if (is_pt!=NULL) {
      os_pt = of_pt->first_s_pt
          = (struct segment*)malloc(sizeof(struct segment));
      os_pt->next = (struct segment*)0;
      if (is_pt->word==NULL || *(is_pt->word)==null) {
         os_pt->word = (char*)calloc(2,sizeof(char));
         *(os_pt->word) = null;
      }
      else {
         os_pt->word 
            = (char*)calloc((unsigned)(strlen(is_pt->word)+2),sizeof(char));
         strcpy(os_pt->word,is_pt->word);
      }
      os_pt->row = is_pt->row;
      os_pt->col = is_pt->col;
   }
   else {
      free(os_pt);
      of_pt->first_s_pt = (struct segment*)0;
   }
}

struct first_second *find_f_s(f_pt)
struct field *f_pt;
{  
   static struct first_second *f_s=NULL;
   register struct segment *s_pt;
   int sflag, cflag;
   static int freeflag = 0;

   if (freeflag) free(f_s);
   freeflag = 1;
   f_s = (struct first_second*)malloc(sizeof(struct first_second));
   *f_s->first = *f_s->second = null;

   if (f_pt==NULL) return(f_s);

   s_pt = f_pt->first_s_pt;
   cflag = (f_pt->caption!=NULL && *f_pt->caption!=null);
   sflag = (s_pt!=NULL && s_pt->word!=NULL && *s_pt->word!=null);

   if (cflag) {
      if (sflag) {
         if (f_pt->col<s_pt->col) {
            strcpy(f_s->first,f_pt->caption);
            strcpy(f_s->second,s_pt->word);
         }
         else {
            strcpy(f_s->first,s_pt->word);
            strcpy(f_s->second,f_pt->caption);
         }
      }
      else strcpy(f_s->first,f_pt->caption);
   }
   else if (sflag) strcpy(f_s->first,s_pt->word);

   return(f_s);
}


fcopy(f1_pt,f0_pt)                /* TO f1_pt, FROM f0_pt        */
register struct field *f0_pt, *f1_pt;
{
   f1_pt->type = f0_pt->type;
   f1_pt->row = f0_pt->row;
   f1_pt->col = f0_pt->col;
   f1_pt->last_row = f0_pt->last_row;
   f1_pt->caption = f0_pt->caption;
   f1_pt->first_s_pt = f0_pt->first_s_pt;
   f1_pt->minfsegms = f0_pt->minfsegms;
   f1_pt->maxfsegms = f0_pt->maxfsegms;
   f1_pt->first_v_pt = f0_pt->first_v_pt;
   f1_pt->first_i_pt = f0_pt->first_i_pt;
   f1_pt->first_m_pt = f0_pt->first_m_pt;
   f1_pt->first_a_pt = f0_pt->first_a_pt;
   f1_pt->help = f0_pt->help;
   f1_pt->action = f0_pt->action;
   f1_pt->first_pr_pt = f0_pt->first_pr_pt;
   f1_pt->first_po_pt = f0_pt->first_po_pt;
   f1_pt->first_cpr_pt = f0_pt->first_cpr_pt;
   f1_pt->first_cpo_pt = f0_pt->first_cpo_pt;
   f1_pt->op_name = f0_pt->op_name;
   f1_pt->def_arg = f0_pt->def_arg;
   f1_pt->loc = f0_pt->loc;
   f1_pt->bundle = f0_pt->bundle;
}

reorder()
{  
   register struct field *f0_pt, *f1_pt;
   struct field f2;
   for (f0_pt=fields; f0_pt<last_field_pt;  f0_pt++)
      for (f1_pt=f0_pt+1; f1_pt<=last_field_pt;  f1_pt++)
         if (f0_pt->row > f1_pt->row) {
            fcopy(&f2,f0_pt);
            fcopy(f0_pt,f1_pt);
            fcopy(f1_pt,&f2);
            relabel(f0_pt,f1_pt);
         }
}


int fequal(fa_pt,fb_pt)
register struct field *fa_pt, *fb_pt;
{  
   register struct segment *sa_pt, *sb_pt;
   register int saflag, sbflag, caflag, cbflag;

   if (fa_pt==NULL || fb_pt==NULL) return(0);

   if ((sa_pt=fa_pt->first_s_pt) == NULL || sa_pt->word==NULL 
       || *(sa_pt->word)==null) saflag=0;
   else saflag=1;

   if ((sb_pt=fb_pt->first_s_pt) == NULL || sb_pt->word==NULL 
       || *(sb_pt->word)==null) sbflag=0;
   else sbflag=1;

   if (fa_pt->caption==NULL || *(fa_pt->caption)==null) caflag=0;
   else caflag=1;

   if (fb_pt->caption==NULL || *(fb_pt->caption)==null) cbflag=0;
   else cbflag=1;

   if (saflag!=sbflag || caflag!=cbflag) return(0);

   if (saflag==0 && caflag==0) return(-1);

   if (saflag==1 && diff(sa_pt->word,sb_pt->word)) return(0);

   if (caflag==1 && diff(fa_pt->caption,fb_pt->caption)) return(0);

   return(1);
}


/*
 * This routine makes sure that when fa_pt and fb_pt are about
 * to be reversed in reorder(), any references to these fields
 * are reversed too.
 */
relabel(fa_pt,fb_pt)
register struct field *fa_pt, *fb_pt;
{  
   register struct field *f0_pt;
   register struct vfunc *v_pt;
   register int a, b;
   for (f0_pt=fields; f0_pt<=last_field_pt; f0_pt++) {
      a = (int)(fa_pt-fields);
      b = (int)(fb_pt-fields);
      sublabel(f0_pt->first_i_pt,a,b);
      sublabel(f0_pt->first_m_pt,a,b);
      sublabel(f0_pt->first_a_pt,a,b);
      for (v_pt=f0_pt->first_v_pt;v_pt!=(struct vfunc *)0;v_pt=v_pt->next)
         if (v_pt->num == 32 && v_pt->pr_pt != (struct fix *)0) 
            shlabel(v_pt->pr_pt->name,a,b);
   }
}

/*
 * This routine does the actual reversing, for any linked list
 * of index pointers.
 */
sublabel(in_pt,a,b)
register int a, b;
register struct index *in_pt;
{
   for (;in_pt!=NULL; in_pt=in_pt->next) {
      if (in_pt->num == a) in_pt->num=b;
      else if (in_pt->num == b) in_pt->num=a;
   }
}

/*
 * changes $Fa into $Fb and vv., in shell validation.
 *
 */
shlabel(s,a,b)
register int a, b;
char *s;
{  char a0, a1, b0, b1;

   a0 = '0' + a/10;
   a1 = '0' + a%10;
   b0 = '0' + b/10;
   b1 = '0' + b%10;

   if (s==(char*)0 || strlen(s)<4) return(0);

   while (*(s+3)) {
      switch(*s) {
      case '$':
         if (*(s+1)=='F') {
            if (*(s+2)==a0 && *(s+3)==a1) {
               *(s+2) = b0;
               *(s+3) = b1;
            }
            else if (*(s+2)==b0 && *(s+3)==b1) {
               *(s+2) = a0;
               *(s+3) = a1;
            }
         }
         break;
      default:
         break;
      }
      s++;
   }
}



#define AGAIN  {fclose(fp1);\
                fprintf(stderr,"\nType e to re-edit or <DEL> to quit:");\
                if ((c = getchar()) == 'e') return(0);\
                while (c!='e') { while (c!='\n' && getchar()!='\n'); fprintf(stderr,"\nType e to re-edit or <DEL> to quit:"); if ((c = getchar()) == 'e') return(0); }\
                }


int toolong(t_n)
char *t_n;
{  
   register int c, dum, poundflag=0, percentflag=0;
   register int tpoundflag=0, tpercentflag=0;
   register int Y_no_pflag = 0;
   register int ampspflag = 0;
   register int cXflag=0;
   register int cZflag=0;
   register int cRflag=0;
   register int pspflag = 0;
   register int tabflag = 0;
   register int pound_n_p = 0;
   register int printfl = 0;
   FILE *fp1;
   char s[1025], *c_pt;
   int Yflag=0, linelength=0, linecount=0, vilc = 0;
   int caplength = 0;

   if ((fp1 = fopen(t_n,"r")) == NULL) {
      fprintf(stderr,"Layout file has no read permission\n",s);
      exit(4);
   }

   c_pt = s;
   while ((c=getc(fp1))!=EOF && linelength<1024)
      switch(c) {
      case '\n':
         vilc++;
         *c_pt=null;
         if (c_pt>s) linecount++;
         while (*--c_pt==SPACE) linelength--;

         if (tabflag) {
            fprintf(stderr,"\n\Warning; possible problem in line %d:\n\n",vilc);
            fprintf(stderr,"%s\n\nProblem: ",s);
            fprintf(stderr,"Tabs may cause alignment errors\n ");
            fprintf(stderr,
            "\nType e followed to re-edit or c to continue:");
            while ((dum=getchar())!='e' && dum!='c'); 
            if (dum=='e') return(0);
         }

         if (linecount>MAXFIELDS-1 || 
             (Yflag!=0 && Yflag!=2) ||
             Y_no_pflag ||
             ampspflag ||
             pound_n_p ||
             cXflag ||
             cRflag ||
             cZflag ||
             pspflag ||
             (linelength>78 && !Yflag) ||
             caplength>75 ||
             (poundflag && mode!=MENU) ||
             poundflag+percentflag>1) {
            fprintf(stderr,"\n\nError in line %d:\n\n",vilc);
            fprintf(stderr,"%s\n\nError: ",s);
         }

         if (linecount>MAXFIELDS-1) {
            fprintf(stderr,"Too many fields (more than 99)");
            AGAIN;
         }


         if (Yflag!=0 && Yflag!=2) {
            fprintf(stderr,"a shell script must be preceded and followed by exactly one ^Y\n");
            AGAIN;
         }
         if (Y_no_pflag) {
            fprintf(stderr,"shell scripts are only allowed in user input areas\n");
            AGAIN;
         }
         if (pound_n_p) {
            fprintf(stderr,"pound sign not preceded by printing character\n");
            AGAIN;
         }
         if (ampspflag) {
            fprintf(stderr,"& must be preceded by non-blank \n");
            AGAIN;
         }
         if (cXflag) {
            fprintf(stderr,"^X may not be used in a layout file \n");
            AGAIN;
         }
         if (cRflag) {
            fprintf(stderr,"^R may not be used in a layout file \n");
            AGAIN;
         }
         if (cZflag) {
            fprintf(stderr,"^Z may not be used in a layout file \n");
            AGAIN;
         }
         if (pspflag) {
            fprintf(stderr,"No blank after %% allowed \n");
            AGAIN;
         }
         if (linelength>78 && !Yflag) {
            fprintf(stderr,"Too long (more than 78 characters)\n");
            AGAIN;
         }
         if (caplength>75) {
            fprintf(stderr,"Caption too long (more than 75 characters)\n");
            AGAIN;
         }
         if (poundflag && mode!=MENU) {
            fprintf(stderr,"Pound sign not allowed in command form\n");
            AGAIN;
         }
         if (poundflag+percentflag>1) {
            if (poundflag>1) {
               fprintf(stderr,"More than one pound sign in one line\n");
               AGAIN;
            }
            else if (percentflag>1) {
               fprintf(stderr,"More than one percent sign in one line\n");
               AGAIN;
            }
            else {
               fprintf(stderr,"Percent sign and poundsign in the same line\n");
               AGAIN;
            }
         }

         tpoundflag += poundflag;
         tpercentflag += percentflag;
         poundflag=percentflag=0;
         c_pt=s;
         Yflag=0;
         linelength=0;
         caplength=0;
         Y_no_pflag = 0,
         ampspflag = 0,
         pound_n_p = 0,
         printfl = 0,
         cXflag=0;
         cZflag=0;
         cRflag=0;
         pspflag = 0;
         tabflag = 0;
         break;
      case '#':
         if (c_pt==s || (*(c_pt-1)!='\\' && !Yflag)) poundflag++;
         if (printfl==0) pound_n_p++;
         *c_pt++ = c;
         linelength++;
         break;
      case '%':
         if (c_pt==s || (*(c_pt-1)!='\\' && !Yflag)) percentflag++;
         *c_pt++ = c;
         linelength++;
         caplength = linelength;
         break;
      case CTRL(Y):
         if (!percentflag)  Y_no_pflag=1;
         Yflag++;
         break;
      case '&':
         if (c_pt==s || *(c_pt-1)==' ') ampspflag = 1;
         linelength++;
         break;
      case CTRL(X):
         cXflag = 1;
         break;
      case CTRL(Z):
         cZflag = 1;
         break;
      case CTRL(R):
         cRflag = 1;
         break;
      default:
         if (c==' ' && *(c_pt-1)=='%') pspflag = 1;
         if (c!=' ' && c!=CTRL(I)) printfl++;
         *c_pt++ = c;
         if (c==CTRL(I)) {
            tabflag = 1;
            linelength = ( (linelength/8) + 1 ) * 8;
         }
         else linelength++;
         break;
      }

   if (tpoundflag + tpercentflag == 0) {
      fprintf(stderr,"\nNo selectables or input areas");
      AGAIN;
   }

   if (linelength>1023) {
      fprintf(stderr,"Error in line:\n%s\n",s);
      fprintf(stderr,"\nshell script + caption exceeds 1023 chars");
      AGAIN;
   }

   return(1);
}



/*
 * editstr(s) copies contents of s, if any, into tempfile;
 * invokes standard editor on temp file; copies contents
 * of editied file into allocated space, and returns
 * pointer to this space.
 */

char *editstr(s)
char *s;
{  
   char *c_pt, *str;
   register int c;
   struct stat buf;
   int ci, cj;
   char tf_n1[128];
   char cmd[256];
   FILE *tf_pt;

   getyx(stdscr,cj,ci);

   if ((c_pt=tmpnam(NULL)) == NULL) return(s);
   strcpy(tf_n1,c_pt);
   sprintf(cmd,"%s %s",editor,tf_n1);

   if ((tf_pt = fopen(tf_n1,"w"))==NULL) {
      status = 7;
      vdone();
   }

   if (s!=NULL && *s!=null) {
      for (c_pt=s; *c_pt; c_pt++) putc(*c_pt,tf_pt);
      if (c_pt==s || *(c_pt-1)!='\n')
         putc('\n',tf_pt); /* To eliminate stupid message from vi */
   }

   fclose(tf_pt);

   store(stdscr,0,LINES,COLS);

   clear();
   move(0,0);
   refresh();
   saveterm(); 
   resetterm();
   updatetty(&termbuf);
   if (system(cmd)==256) {
      fprintf(stderr,"Editor in \"%s\" not found\n",cmd);
      exit(1);
   }
   catchtty(&termbuf);
   fixterm();

   stat(tf_n1,&buf);
   c_pt = str = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));

   if ((tf_pt = fopen(tf_n1,"r"))==NULL)  {
      status = 8;
      vdone();
   }

   while ((c=getc(tf_pt))!=EOF) *c_pt++ = c;
   *c_pt = null;
   while (*c_pt==null || *c_pt=='\012' || *c_pt=='\015') *c_pt-- = null;
   fclose(tf_pt);

   unlink(tf_n1);
   wclear(curscr);

   store(stdscr,1,LINES,COLS);
   move(ci,cj);
   return(str);
}


#define STORE 0
#define RESTORE 1


VOID store(w,flag,l,c) /* Last-in first-out screen memory */
WINDOW *w;
int flag;                             /* 0: store; 1: restore */
int l, c;
{  
   static chtype *cht_pt;
   static chtype **memch_pt=NULL;
   static int *ci_pt=NULL, *cj_pt=NULL;
   register int i, j;

   /* Initialize memch_pt, ci_pt, cj_pt for first-time use  */
   if (memch_pt==NULL) memch_pt = (chtype **)calloc(10,sizeof(chtype));
   if (ci_pt==NULL) ci_pt = (int*)calloc(10,sizeof(int));
   if (cj_pt==NULL) cj_pt = (int*)calloc(10,sizeof(int));

   if (flag==STORE) {         /* Store screen */
      getyx(w,*ci_pt,*cj_pt);      /* Store cursor location */
      cht_pt = *memch_pt                /* Allocate memory; init. cht_pt */
             = (chtype *)calloc((unsigned)((l+1)*(c+1)),sizeof(chtype));
      for (i=0; i<l; i++)
         for (j=0; j<c; j++)
            *cht_pt++ = mvwinch(w,i,j); /* Store screen in allocated area */
      memch_pt++;                       /* Prepare for next store operation */
      ci_pt++;
      cj_pt++;
   }

   else if (flag==RESTORE) {   /* Restore screen.  Presupposes that no restore
                                  ever precedes a store */
      memch_pt--;                       /* Reset to last storage area */
      ci_pt--;
      cj_pt--;
      cht_pt = *memch_pt;               /* Initialize cht_pt */
      for (i=0; i<l; i++) {
         move(i,0);
         for (j=0; j<c; j++)
            waddch(w,*cht_pt++);
      }
      free(*memch_pt);                  
      wmove(w,*ci_pt,*cj_pt);                   /* restore cursor location */
   }
}


termstuff()
{  
   char *c_pt;
   char *getenv();

   term = 0;
   if ((c_pt=(char*)getenv("MSTANDOUT")) != NULL && *c_pt=='1')
      term |= STANDOUT;
   if ((c_pt=(char*)getenv("MFKEYS")) != NULL && *c_pt=='1')
      term |= FKEYS;
   if ((c_pt=(char*)getenv("MALTCHAR")) != NULL && *c_pt=='1')
      term |= ALTCHAR;

   if (term & ALTCHAR)
   {
      HOR = hor = 'q';
      VER = ver = 'x';
      TL  = tl  = 'l';
      TR  = tr  = 'k';
      BL  = bl  = 'm';
      BR  = br  = 'j';
   }
   else
   {
      HOR = hor = '-';
      VER = ver = '|';
      TL  = tl  = SPACE;
      TR  = tr  = SPACE;
      BL  = bl  = SPACE;
      BR  = br  = SPACE;
   }
}

draw_line(row)
int row;
{  
   register int i;
   ALT;
   move(row,0);
   for (i=0;i<COLS-1;i++) addch(HOR);
   NALT;
}


sccat(out,in)
register char *in, *out;
{
   out += strlen(out);
   while (*in)  {
      if (*in == '#' || *in == '%') 
         *out++ = '\\';
      *out++ = *in++;
   }
   *out = null;
}

ampersand(s)  /* Strips & and spaces; tests if string ends on & */
char *s;
{
   register char *c_pt;
   int aflag=0;

   if (*s==null) return(0);

   c_pt = s+strlen(s)-1;
   while (*c_pt=='&' || *c_pt==' ')  {
      if (*c_pt=='&') aflag=1;
      *c_pt-- = null;
   }
   return(aflag);
}


int helpchk(s)
char *s;
{  int lines=1, j=0, maxj=0, Yflag=0, Bflag=0;

   if (s==NULL) return(0);
   while (*s) {
      if (*s!=CTRL(B)) j++;
      if (!isprint(*s) && *s!='\n' && *s!=CTRL(B) && *s!=CTRL(Y))
         return(47);
      if (*s==CTRL(Y)) Yflag++;
      if (*s==CTRL(B)) Bflag++;
      maxj = (j>maxj) ? j : maxj;
      if (*s++ == '\n') {
         if (j>75) return(18);	/*help message has >78 chars/line*/
         if (Yflag%2!=0) return(26);
         if (Bflag%2!=0) return(49);
         Yflag=0;
         Bflag=0;
         j = 0;
         
         lines++;
      }
   }
   if (Yflag%2!=0) return(26);
   if (Bflag%2 != 0) return(49);
   if (j>75) return(18);
   if (lines>17) return(17);	/*help message is ok*/
   if (maxj<6) return(48);  /* MUST BE LAST: WARNING */
   return(0);	/*help message has too many lines*/
}

tdone()
{  int i;
   for (i=1; i<18; i++) signal(i,SIG_IGN);
   if (LINES>1) {
      (void) move(LINES-1,0);		/* to lower left corner */
      clrtoeol();			/* clear bottom line */
      (void) refresh();		/* flush out everything */
   }
   attrset(0);
   endwin();			/* curses cleanup */
   updatetty(&termbuf);
   if (access(tf_name,00)==0) unlink(tf_name);
   exit(0);
}

