/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:field_type.c	1.30"

/*
 */

#include "../forms/muse.h"
#include "vdefs.h"

#define BLANKS {for (j=cj;j<COLS-1;j++) addch(' ');}

int helpnum;

int field_type(f_pt)   /* Returns fieldtype label */
struct field *f_pt;
{  char *c_pt, *c0_pt;
   char typelabel[128];
   extern char fthelp[128];
   int ask(), maxfsegms;
   int frow;
   struct field f0;
   struct fix *cprA_pt, *cprB_pt, *getlist();
   struct fix *cpox0_pt, *cpoy0_pt;
   struct fix *cpox1_pt, *cpoy1_pt;
   struct fix *cpox2_pt, *cpoy2_pt;
   struct fix *cpox3_pt, *cpoy3_pt;
   struct fix *first_cpr_pt, *first_cpo_pt;
   struct charstr *askstring(), *chstr;
   int ci, cj, test, c;
   int bundle;
   int yesnoflag;

   strcpy(fthelp,"ftype.help");

RESTART:
   bundle = -1;
   show_caption(f_pt,"Command Line Mapping");

   show_cmd("",7);
   helpnum = 0;
   switch(show_map(f_pt,"Do you want to change this mapping (y/n):")) {
      case 0:
        return(0);
        break;
      case 1:
      case 4:
        return(1);
        break;
      case 2:
        break;
      case 3:
        clrlns(4,LINES-1);
        goto RESTART;
        break;
   }

   frow = 5;
   maxfsegms = 1;
   helpnum = 1;
   yesnoflag=0;
   first_cpr_pt = (struct fix *)malloc(sizeof(struct fix));
   first_cpr_pt->name = (char*)calloc(6,sizeof(char));
   cprA_pt = first_cpr_pt->next 
           = (struct fix *)malloc(sizeof(struct fix));
   cprB_pt = cprA_pt->next = (struct fix *)malloc(sizeof(struct fix));
   cprB_pt->next = NULL;

   cpox0_pt = first_cpo_pt = (struct fix *)malloc(sizeof(struct fix));

   cpoy0_pt = cpox0_pt->next = (struct fix *)malloc(sizeof(struct fix));
   cpoy0_pt->next = NULL;

   cprA_pt->name = cprB_pt->name = cpox0_pt->name 
                 = cpoy0_pt->name =  &nullchar;
   
/* Initialize f0 */
   f0.first_cpr_pt = f0.first_cpo_pt = (struct fix *)0;
   f0.caption = f_pt->caption;
   f0.first_s_pt = f_pt->first_s_pt;

   clrlns(4,LINES-4);
   show_caption(f_pt,"Command Line Mapping");
   show_cmd("",7);


   mvaddstr(5,1,
"Identical -- Command line strings ALWAYS same as input strings.");
   mvaddstr(6,1,
"             Rule: A mapped onto A.");
   mvaddstr(7,1,
"             Example: \"file\" onto \"file\" [in \"cat\"].");
   mvaddstr(8,1,
"Yes/No    -- Option without argument, invoked by a yes/no question]");
   mvaddstr(9,1,
"             Rule: \"y\" mapped onto string0, \"n\" onto string1.");
   mvaddstr(10,1,
"             Example: \"y\" onto \"-print\", \"n\" onto <empty> [in \"find\"].");
   mvaddstr(11,1,
"Other     -- Does not fall into either of these two categories.");
   mvaddstr(12,1,
"             Rule: typically, \"A\" is mapped onto \"<string>A<string>\".");
   mvaddstr(13,1,
"             Example: \"user\" onto \"-u user\" [in \"ps\"].");
ZZZ:
   switch(ask(
"Type i, y, or o:" ,15,1,"iyo",helpnum,fthelp)) {
   case 'i':
      addstr("dentical"); refresh();
      if (f_pt->bundle==1) bundle = 0;  /* Old y/n item */
      else bundle= -1;  /* Do not touch if 0 or 2 */
      strcpy(first_cpr_pt->name,"1110");
      clrlns(3,15);
      frow = 7;
      goto SPACES;
      break;
   case CTRL(V):
   case KEY_F(2):
      clear();
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto ZZZ;
      break;
   case CTRL(R):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto ZZZ;
      break;
   case 'y':
      addstr("es/no"); refresh();
      clrlns(5,15); clrtoeol();
      yesnoflag=1;
      break;
   case 'o':
      addstr("ther"); refresh();
      clrlns(3,15); refresh();
      if (f_pt->bundle==1) bundle = 0;  /* Old y/n item */
      else bundle= -1;  /* Do not touch if 0 or 2 */
      mvaddstr(3,0,
"Notation. A = User input string in current field (there may be more than one)");
      mvaddstr(4,0,
"          B = Corresponding command line string(s)");
      draw_line(5);
      break;
   }


   if (yesnoflag) {
      c_pt = &nullchar;
      helpnum = 2;
      show_cmd("",8);
      frow = 5;
KKK:
      c_pt = (chstr=askstring(
         "Type delimiter and option name:",5,1,COLS,helpnum,fthelp,1))->str;
      switch(chstr->ch) {
      case CTRL(V):
      case KEY_F(2):
         clear();
         goto RESTART;
         break;
      case CTRL(T):
      case KEY_F(5):
         if (restart()==0) return(0);
         clrlns(LINES-4,LINES-4);
         goto KKK;
         break;
      case CTRL(R):
      case KEY_F(4):
         if (restart()==0) return(1);
         clrlns(LINES-4,LINES-4);
         goto KKK;
         break;
      default:
         break;
      }
      clrlns(LINES-1,LINES-1);
      show_cmd("",7);
      cpox0_pt->name = (char*)calloc(2,sizeof(char));
      cpoy0_pt->name = (char*)calloc((unsigned)(strlen(c_pt)+2),sizeof(char));
 
      cpox1_pt = cpoy0_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpoy1_pt = cpox1_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpox1_pt->name = (char*)calloc(2,sizeof(char));
      cpoy1_pt->name = (char*)calloc((unsigned)(strlen(c_pt)+2),sizeof(char));

      cpox2_pt = cpoy1_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpoy2_pt = cpox2_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpox2_pt->name = (char*)calloc(2,sizeof(char));
      cpoy2_pt->name = &nullchar;

      cpox3_pt = cpoy2_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpoy3_pt = cpox3_pt->next = (struct fix*)malloc(sizeof(struct fix));
      cpox3_pt->name = (char*)calloc(2,sizeof(char));
      cpoy3_pt->name = &nullchar;

      cpoy3_pt->next = NULL;

      strcpy(cpoy0_pt->name,c_pt);
      strcpy(cpoy1_pt->name,c_pt);


      helpnum = 3;
LLL:
      switch(ask(
"Should option be set when user types y or when user types n (y/n):",
      7,1,"yn",helpnum,fthelp)) {
      case 'y':
         strcpy(cpox0_pt->name,"y");
         strcpy(cpox1_pt->name,"Y");
         strcpy(cpox2_pt->name,"n");
         strcpy(cpox3_pt->name,"N");
         break;
      case 'n':
         strcpy(cpox0_pt->name,"n");
         strcpy(cpox1_pt->name,"N");
         strcpy(cpox2_pt->name,"y");
         strcpy(cpox3_pt->name,"Y");
         break;
      case CTRL(V):
      case KEY_F(2):
         clear();
         goto RESTART;
         break;
      case CTRL(T):
      case KEY_F(5):
         if (restart()==0) return(0);
         clrlns(LINES-4,LINES-4);
         goto LLL;
         break;
      case CTRL(R):
      case KEY_F(4):
         if (restart()==0) return(1);
         clrlns(LINES-4,LINES-4);
         goto LLL;
         break;
      }

/* Check if 1-letter option; if not, finalize */
      if (strlen(cpoy0_pt->name)!=2) {
         strcpy(first_cpr_pt->name,"1011");
         bundle = 0;
         helpnum = 5;
         finalize(1,&f0,bundle,first_cpr_pt,first_cpo_pt);
zzz:
         switch(show_map(&f0,"Is this how you want it (y/n):")) {
            case 0:
              if (restart()==0) return(0);
              clrlns(LINES-4,LINES-4);
              goto zzz;
              break;
            case 1:
            case 3:
              goto RESTART;
              break;
            case 2:
              finalize(1,f_pt,bundle,first_cpr_pt,first_cpo_pt);
              set_valstrs(f_pt);
              return(1);
              break;
            case 4:
              if (restart()==0) return(1);
              clrlns(LINES-4,LINES-4);
              goto zzz;
              break;
         }
      }

      helpnum = 4;
MMM:
      switch(ask(
"Is option bundled with other 1-letter options (y/n):",
      9,1,"yn",helpnum,fthelp)) {
      case 'y':
         addstr("es"); refresh();
         strcpy(first_cpr_pt->name,"0001");
         bundle = 1;
         helpnum = 5;
         finalize(1,&f0,bundle,first_cpr_pt,first_cpo_pt);
aaa:
         switch(show_map(&f0,"Is this how you want it (y/n):")) {
            case 0:
              if (restart()==0) return(0);
              clrlns(LINES-4,LINES-4);
              goto aaa;
              break;
            case 1:
            case 3:
              goto RESTART;
              break;
            case 2:
              finalize(1,f_pt,bundle,first_cpr_pt,first_cpo_pt);
              set_valstrs(f_pt);
              return(1);
              break;
            case 4:
              if (restart()==0) return(1);
              clrlns(LINES-4,LINES-4);
              goto aaa;
              break;
         }
         break;
      case 'n':
         addstr("o"); refresh();
         strcpy(first_cpr_pt->name,"1011");
         bundle = 0;
         helpnum = 5;
         finalize(1,&f0,bundle,first_cpr_pt,first_cpo_pt);
bbb:
         switch(show_map(&f0,"Is this how you want it (y/n):")) {
            case 0:
              if (restart()==0) return(0);
              clrlns(LINES-4,LINES-4);
              goto bbb;
              break;
            case 1:
            case 3:
              goto RESTART;
              break;
            case 2:
              finalize(1,f_pt,bundle,first_cpr_pt,first_cpo_pt);
              set_valstrs(f_pt);
              return(1);
              break;
            case 4:
              if (restart()==0) return(1);
              clrlns(LINES-4,LINES-4);
              goto bbb;
              break;
         }
         break;
      case CTRL(V):
      case KEY_F(2):
         clear();
         goto RESTART;
         break;
      case CTRL(T):
      case KEY_F(5):
         if (restart()==0) return(0);
         clrlns(LINES-4,LINES-4);
         goto MMM;
         break;
      case CTRL(R):
      case KEY_F(4):
         if (restart()==0) return(1);
         clrlns(LINES-4,LINES-4);
         goto MMM;
         break;
      }
      helpnum = 5;
      finalize(1,&f0,bundle,first_cpr_pt,first_cpo_pt);
ccc:
      switch(show_map(&f0,"Is this how you want it (y/n):")) {
         case 0:
           if (restart()==0) return(0);
           clrlns(LINES-4,LINES-4);
           goto ccc;
           break;
         case 1:
         case 3:
           goto RESTART;
           break;
         case 2:
           finalize(1,f_pt,bundle,first_cpr_pt,first_cpo_pt);
           set_valstrs(f_pt);
           return(1);
           break;
         case 4:
           if (restart()==0) return(1);
           clrlns(LINES-4,LINES-4);
           goto ccc;
           break;
      }
   }

   show_cmd("",7);
   helpnum = 8;
NNN:
   switch(ask( 
"Does A itself EVER appear on the command line (y/n):", 7,1,"yn",helpnum,fthelp)) {
   case 'n':
      *(first_cpr_pt->name+3) = '1';
      addstr("o"); refresh();
      if ((first_cpo_pt = getlist(f_pt,fthelp,6))==NULL)
         return(0);
      frow = 9;
      goto SPACES;
      break;
   case CTRL(V):
   case KEY_F(2):
      clear();
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto NNN;
      break;
   case CTRL(r):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto NNN;
      break;
   case 'y':
      *(first_cpr_pt->name+3) = '0';
      addstr("es"); refresh();
      show_cmd("",8);
      test=0;
      while (test==0) {   /* Stay in loop until string contains A */
         move(9,1); clrtoeol();
         helpnum = 9;
OOO:
         c_pt = (chstr=askstring(
"Type what A should be mapped onto [e.g., -o A]: ",
      9,1,COLS,helpnum,fthelp,1))->str;
         ci = 9;
         cj = 48;
         switch(chstr->ch) {
         case CTRL(V):
         case KEY_F(2):
            clear();
            goto RESTART;
            break;
         case CTRL(T):
         case KEY_F(5):
            if (restart()==0) return(0);
            clrlns(LINES-4,LINES-4);
            goto OOO;
            break;
         case CTRL(R):
         case KEY_F(4):
            if (restart()==0) return(1);
            clrlns(LINES-4,LINES-4);
            goto OOO;
            break;
         default:
            break;
         }
         for (c0_pt=c_pt; *c0_pt; c0_pt++)
            if (*c0_pt=='A' && (c0_pt==c_pt || *(c0_pt-1)!='\\')) 
               test++;
         if (test==0 || test>1) {
            if (test==0)
               err_rpt(34,TRUE);
            else
               err_rpt(35,TRUE);
            show_cmd("",26);
            refresh();
            while ((c=getch()) != CTRL(V) && c!=KEY_F(2) &&
                   c!=CTRL(M) && c!=CTRL(J)) beep();
            if (c == CTRL(V) || c==KEY_F(2)) {
               clear();
               goto RESTART;
            }
            show_cmd("",8);
            err_rpt(0,0);
            move(ci,cj);
            clrtoeol();
            test=0;  /* Stay in loop; reinitialize */
         }
      }

      clrlns(LINES-1,LINES-1);
      show_cmd("",7);

      /* Find first non-escaped occurrence of A */
      for (c0_pt=c_pt; *c0_pt!='A' || (c0_pt>c_pt && *(c0_pt-1)=='\\'); 
           c0_pt++);
      *c0_pt = null;   /* Terminate c_pt at location of A           */
      cprB_pt->name = c0_pt+1;  /* Start B_pt at location after the A */
      rmslash(cprB_pt->name);
      if (*--c0_pt=='\\') *c0_pt = null; /* strip ^C if needed */
      cprA_pt->name = c_pt;     /* Start B at c_pt */
      rmslash(cprA_pt->name);

      helpnum = 10;
PPP:
      switch(ask(
"Are there exceptions to the mapping rule you just specified (y/n):",11,1,"yn",helpnum,fthelp)) {
      case 'y':
         addstr("es"); refresh();
         if ((first_cpo_pt = getlist(f_pt,fthelp,6))==NULL)
           return(0);
         *(first_cpr_pt->name+3) = '0';
         frow = 13;
         goto SPACES;
         break;
      case 'n':
         addstr("o"); refresh();
         *(first_cpr_pt->name+3) = '0';
         frow = 13;
         goto SPACES;
         break;
      case CTRL(V):
      case KEY_F(2):
         clear();
         goto RESTART;
         break;
      case CTRL(T):
      case KEY_F(5):
         if (restart()==0) return(0);
         clrlns(LINES-4,LINES-4);
         goto PPP;
         break;
      case CTRL(R):
      case KEY_F(4):
         if (restart()==0) return(1);
         clrlns(LINES-4,LINES-4);
         goto PPP;
         break;
      }
      break;
   }


SPACES:

      clrlns(3,3);
      mvaddstr(3,0,
"Notation. A = User input string in current field (there may be more than one)");
      mvaddstr(4,0,
"          B = Corresponding command line string(s)");
      draw_line(5);

QQQ:
   strcpy(typelabel,
"Put space to the left of B on the command line (y/n):");
   helpnum = 11;
   switch(ask(typelabel,frow,1,"yn",helpnum,fthelp)) {
   case 'y':
      addstr("es"); refresh();
      *first_cpr_pt->name='1';
      break;
   case 'n':
      addstr("o"); refresh();
      *first_cpr_pt->name='0';
      break;
   case CTRL(V):
   case KEY_F(2):
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto QQQ;
      break;
   case CTRL(R):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto QQQ;
      break;
   }

   helpnum = 12;
RRR:
   switch(ask( 
"Put space between B's from current field (enables multiple strings) (y/n):",frow+1,1,"yn",helpnum,fthelp)) {
   case 'y':
      maxfsegms=30;
      addstr("es"); refresh();
      *(first_cpr_pt->name+1)='1';
      break;
   case 'n':
      addstr("o"); refresh();
      *(first_cpr_pt->name+1)='0';
      break;
   case CTRL(V):
   case KEY_F(2):
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto RRR;
      break;
   case CTRL(R):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto RRR;
      break;
   }

   helpnum = 13;
   strcpy(typelabel,
"Put space after B (y/n):");
SSS:
   switch(ask(typelabel,frow+2,1,"yn",helpnum,fthelp)) {
   case 'y':
      addstr("es"); refresh();
      *(first_cpr_pt->name+2)='1';
      break;
   case 'n':
      addstr("o"); refresh();
      *(first_cpr_pt->name+2)='0';
      break;
   case CTRL(V):
   case KEY_F(2):
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto SSS;
      break;
   case CTRL(R):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto SSS;
      break;
   }

   helpnum = 14;
   move(frow+6,1);
   addstr("<strings>");
   if (*first_cpr_pt->name == '1') addstr("<SPACE>");
   if (maxfsegms>1) {
      if (*(first_cpr_pt->name+1) == '1') 
         addstr("B<SPACE>B<SPACE> ... <SPACE>B");
      else addstr("BB...B");
   }
   else addch('B');
   if (*(first_cpr_pt->name+2) == '1') addstr("<SPACE>");
   addstr("<strings>");
TTT:
   switch(ask("Is the following spacing pattern the way you want it (y/n):",frow+4,1,"yn",helpnum,fthelp)) {
   case 'y':
      addstr("es"); refresh();
      break;
   case 'n':
      addstr("o"); refresh();
      clrlns(frow,frow+6);
      maxfsegms = 1;
      goto SPACES;
      break;
   case CTRL(V):
   case KEY_F(2):
      goto RESTART;
      break;
   case CTRL(T):
   case KEY_F(5):
      if (restart()==0) return(0);
      clrlns(LINES-4,LINES-4);
      goto TTT;
      break;
   case CTRL(R):
   case KEY_F(4):
      if (restart()==0) return(1);
      clrlns(LINES-4,LINES-4);
      goto TTT;
      break;
   }
   finalize(maxfsegms,&f0,bundle,first_cpr_pt,first_cpo_pt);
   helpnum = 5;
ddd:
   switch(show_map(&f0,"Is this how you want it (y/n):")) {
      case 0:
        if (restart()==0) return(0);
        clrlns(LINES-4,LINES-4);
        goto ddd;
        break;
      case 1:
      case 3:
        goto RESTART;
        break;
      case 2:
        finalize(maxfsegms,f_pt,bundle,first_cpr_pt,first_cpo_pt);
        set_valstrs(f_pt);
        return(1);
        break;
      case 4:
        if (restart()==0) return(1);
        clrlns(LINES-4,LINES-4);
        goto ddd;
        break;
   }
}

struct fix *getlist(f_pt,hfile,hnum)
struct field *f_pt;
char *hfile;
int hnum;
{  struct fix *fi0_pt, *fi1_pt, *first_cpo_pt;
   static struct segment *ss0=(struct segment *)0, *ss1=(struct segment *)0;
   struct segment *s0_pt, *s1_pt;
   char *prompt0, *prompt1;
   int ask();
   int i, flag;

   if (ss0!=(struct segment*)0) {
      free(ss0);
      ss0 = (struct segment*)0;
   }

   if (ss1!=(struct segment*)0) {
      free(ss1);
      ss1 = (struct segment*)0;
   }
   ss0 = (struct segment *)calloc(LINES,sizeof(struct segment));
   ss1 = (struct segment *)calloc(LINES,sizeof(struct segment));

   for (s0_pt = ss0+LINES-1; s0_pt >= ss0; s0_pt--) s0_pt->word = NULL;
   for (s1_pt = ss1+LINES-1; s1_pt >= ss1; s1_pt--) s1_pt->word = NULL;

   store(stdscr,0,LINES,COLS);
   clear();

   prompt0 = "A:";
   prompt1 = "B:";

   if (f_pt->first_cpo_pt==NULL) {
      f_pt->first_cpo_pt = (struct fix*)malloc(sizeof(struct fix));
      f_pt->first_cpo_pt->name = NULL;
      f_pt->first_cpo_pt->next = NULL;
   }

   s0_pt = ss0;
   s1_pt = ss1;

   for (fi0_pt=f_pt->first_cpo_pt, i=5; i<LINES-3; i++) {
      if (fi0_pt!=NULL && (fi1_pt=fi0_pt->next)!=NULL &&
          fi0_pt->name!=NULL && fi1_pt->name!=NULL) {
         s0_pt->word = fi0_pt->name;
         s1_pt->word = fi1_pt->name;
         fi0_pt = fi1_pt->next;
      }
      else {
         s0_pt->word = s1_pt->word = NULL;
      }
      s0_pt->row = s1_pt->row = i;
      s0_pt->col = strlen(prompt0)+1;
      s1_pt->col = strlen(prompt1)+41;
      s0_pt++;
      s1_pt++;
   }

CC:
   flag = fedit(f_pt,ss0,ss1,prompt0,prompt1,
    "A = user input string, B = what A is mapped onto in the command line",0,
    hfile,hnum,"Continue mapping    ");
DD:
   if (flag==0) {
      switch(ask(
"This would undo your mapping changes.  Please confirm (y/n):",
      LINES-4,1,"yn",31,fthelp)) {
      case 'y':
         clear();
         return(0);
         break;
      case 'n':
         clrlns(LINES-4,LINES-4);
         goto CC;
         break;
      default:
         beep();
         goto DD;
         break;
      }
   }
   clear();
   store(stdscr,1,LINES,COLS);
   refresh();

   fi0_pt = first_cpo_pt = (struct fix*)malloc(sizeof(struct fix));
   fi0_pt->next = (struct fix *)0;
   fi0_pt->name = NULL;

   s0_pt = ss0;
   s1_pt = ss1;
   i = 5;

   for (; i<LINES-3; s0_pt++, s1_pt++, i++) {
      if (s0_pt->word!=NULL && s1_pt->word!=NULL) {
         if (fi0_pt == NULL && fi1_pt!=NULL)
             fi0_pt = fi1_pt->next = (struct fix *)malloc(sizeof(struct fix));
         fi1_pt = fi0_pt->next = (struct fix *)malloc(sizeof(struct fix));
         fi1_pt->next = NULL;
         fi0_pt->name = (char*)calloc((unsigned)(strlen(s0_pt->word)+2),sizeof(char));
         fi1_pt->name = (char*)calloc((unsigned)(strlen(s1_pt->word)+2),sizeof(char));
         strcpy(fi0_pt->name,s0_pt->word);
         strcpy(fi1_pt->name,s1_pt->word);
         fi0_pt = NULL;
      }
   }
   return(first_cpo_pt);
}




int ask(s,i,j,ok,hn,h)
char *s, *ok;
char *h;  /* help file */
int i, j;
int hn;  /* help number */
{  int c=0, test=0, oc;
   char *ok_pt;
   extern char fthelp[128];

   mvaddstr(i,j,s);
   getyx(stdscr,i,j);
   refresh();
   for (;;) {
      oc = c;
      c=getch();
      switch(c) {
      case '\015':
         if (test==1) {
            move(i,j+1);
            return(oc);
         }
         else beep();
         break;
      case CTRL(V):
      case CTRL(T):
      case CTRL(R):
      case KEY_F(2):
      case KEY_F(4):
      case KEY_F(5):
         return(c);
         break;
      case KEY_F(3):
      case CTRL(L):
         clearok(curscr,TRUE);
         wrefresh(curscr);
         break;
      case '\010':
         beep();
         break;
      case KEY_F(6):
      case CTRL(Y):
         store(stdscr,0,LINES,COLS);
         if (i<LINES-12)
            fhelp(h,hn,2,i+1);
         else
            fhelp(h,hn,2,3);
         store(stdscr,1,LINES,COLS);
         refresh();
         break;
      default:
         test = 0;
         if (!isprint(c)) beep();
         else {
            mvaddch(i,j,c);
            move(i,j);
            refresh();
            for (ok_pt=ok; *ok_pt; ok_pt++)
               if ((char)c == *ok_pt) test=1;
         }
         if (test==0) beep();
         break;
      }
   }
}

struct charstr *askstring(s,i,j,maxj,hn,h,edflag)
char *s, *h;  /* Prompt and help file name */
int i, j, maxj, hn, edflag;  /* y/x coord; maxcol; helpnum, edit permiss */
{  char *c_pt, *c0_pt, *c1_pt;
   struct charstr *cs_pt;
   char *editstr(), *dctrlch();
   int c, jj, errnum, j0, c0;
   int literal=0, out=0;

   cs_pt = (struct charstr*)malloc(sizeof(struct charstr));

   cs_pt->str = (char*)calloc((unsigned)(maxj-j+3),sizeof(char));
   *(c_pt = cs_pt->str) = null;

   mvaddstr(i,j,s);
   getyx(stdscr,i,j);
   refresh();

   while (!out) {
      c = getch();
      show_cmd("",8);
      err_rpt(0,0); /* XXX */
      if (literal) goto insert;
      switch(c) {
      case '\015':
      case CTRL(E):
      case KEY_F(7):
      case KEY_DOWN:
      /* case CTRL(V): */
      /* case KEY_F(2): */
      /* case CTRL(R): */
      /* case KEY_F(4): */
      /* case CTRL(T): */
      /* case KEY_F(5): */
         if (!literal) {
            if ((errnum=chckstr(cs_pt->str,0))==0) 
               out = 1;
            else {
               err_rpt(errnum,TRUE);
               show_cmd("",28);
            }
         }
         break;
      case KEY_F(3):
      case CTRL(L):
         clearok(curscr,TRUE);
         wrefresh(curscr);
         break;
      case CTRL(C):
         if (!literal) literal = 1;
         else {
            literal = 0;
            goto insert;
         }
         break;
      case '\010':
         if (c_pt>cs_pt->str)  {
            *c_pt-- = null; 
            *c_pt = null;
         }
         else beep();
         break;
      case CTRL(Y):
      case KEY_F(6):
         store(stdscr,0,LINES,COLS);
         if (i<LINES-12)
            fhelp(h,hn,4,i+2);
         else
            fhelp(h,hn,4,3);
         store(stdscr,1,LINES,COLS);
         refresh();
         break;
      default:
insert:
         if (literal) literal = 0;
         if (errnum=chckchar((char)c)) err_rpt(errnum,TRUE);
         else {
            *c_pt++ = c;
            *c_pt = null;
         }
         break;
      }
      for (jj=j;jj<maxj;jj++) mvaddch(i,jj,' ');
      jj = j;
      move(i,j);
      for (c0_pt=cs_pt->str; *c0_pt && jj<maxj ; c0_pt++) {
         c1_pt = dctrlch(*c0_pt);
         jj += strlen(c1_pt); 
         if (jj<maxj)
            addstr(c1_pt);
         else  {
            show_cmd("",27);
            refresh();
            while ((c0=getch()) != CTRL(M) && c0!=CTRL(J))  beep();
            while ((errnum=chckstr(cs_pt->str=editstr(cs_pt->str),0))>0) {
               err_rpt(errnum,TRUE);
               show_cmd("",16);
               for (j0=j;j0<maxj;j0++) mvaddch(i,j0,' ');
               j0 = j;
               move(i,j);
               for (c0_pt=cs_pt->str; *c0_pt && j0<maxj ; c0_pt++) {
                  c1_pt = dctrlch(*c0_pt);
                  j0 += strlen(c1_pt); 
                  if (j0<maxj)
                     addstr(c1_pt);
               }
               refresh();
               getch();
            }
            for (j0=j;j0<maxj;j0++) mvaddch(i,j0,' ');
            j0 = j;
            move(i,j);
            for (c0_pt=cs_pt->str; *c0_pt && j0<maxj ; c0_pt++) {
               c1_pt = dctrlch(*c0_pt);
               j0 += strlen(c1_pt); 
               if (j0<maxj)
                  addstr(c1_pt);
            }
            err_rpt(0,0); /* XXX */
            return(cs_pt);
         }
      }
/*    err_rpt(0,0);  XXX */
      refresh();
   }

   if (*cs_pt->str == null) mvaddstr(i,j,"<empty>");
   refresh();

   cs_pt->ch = (char)c;

   switch(c) {
   case CTRL(E):
   case KEY_F(7):
      if (!edflag) beep();
      else {
         dctrlblank(i,j,maxj-j,cs_pt->str);
         while ((errnum=chckstr(cs_pt->str=editstr(cs_pt->str),0))>0) {
            err_rpt(errnum,TRUE);
            show_cmd("",16);
            dctrlshow(i,j,78-j,cs_pt->str);
            refresh();
            getch();
         }
         err_rpt(0,0);
   /*    refresh();   XXX */
         move(i,j);
         jj = j;
         for (c_pt=cs_pt->str, jj=j; *c_pt && jj<maxj; c_pt++) {
            jj += strlen(dctrlch(*c_pt)); 
            c0_pt = dctrlch(*c_pt);
            if (jj<maxj) addstr(c0_pt);
         }
      break;
      }
   default:
      break;
   }

   return(cs_pt);
}



VOID finalize(maxs,f_pt,b,f_cpr_pt,f_cpo_pt)
struct field *f_pt;
int maxs;
int b;
struct fix *f_cpr_pt, *f_cpo_pt;
{
   if (f_pt==(struct field*)0) return(0);

   if (b != -1) f_pt->bundle = b;
   f_pt->type = 7;
   f_pt->maxfsegms = maxs;
   f_pt->first_cpr_pt = f_cpr_pt;
   f_pt->first_cpo_pt = f_cpo_pt;
}




char *view_type(f_pt,flag)   /* Returns fieldtype label */
struct field *f_pt;
{  char *OUT0, *A, *B, *x0, *x, *y, *a, *b;
   struct fix *cpr_pt, *cprA_pt, *cprB_pt;
   struct fix *cpox0_pt, *p_pt;
   static char *OUT;
   char *dctrlstr();
   int len;
   static int freeflag=0;

   if (f_pt==(struct field*)0) return(NULL);

   if (f_pt->type!=7) return("A onto A (default mapping) ");

   if (freeflag) free(OUT);
   freeflag=0;

   cpr_pt = f_pt->first_cpr_pt;

   if (cpr_pt==NULL || cpr_pt->name==NULL || strlen(cpr_pt->name)!=4) 
      return(NULL);
   cprA_pt = (cpr_pt==NULL || cpr_pt->name==NULL) ? NULL : cpr_pt->next;
   A = (cprA_pt==NULL) ? &nullchar : cprA_pt->name;
   cprB_pt = (cprA_pt==NULL) ? NULL :  cprA_pt->next;
   B = (cprB_pt==NULL) ? &nullchar : cprB_pt->name;
   cpox0_pt = f_pt->first_cpo_pt;
   x0 = (cpox0_pt==NULL || cpox0_pt->name==NULL) ? &nullchar : cpox0_pt->name;

   /* A, B, and x0 are now guaranteed not to be NULL.
      cpr_pt->name has length 4. */

   /* Catch special case #1 */
   if (*(cpr_pt->name+3) == '0' && *x0 == null) {
      OUT = (char*)calloc((unsigned)(dctrll(A)+dctrll(B)+200),sizeof(char));
      freeflag = 1;
      if (*A!=null)
         a = dctrlstr(A);
      else a="";
      if (strlen(a)>(COLS-10)/2) a="<long string>";
      sprintf(OUT,"A onto %sA",a);
      if (*B!=null)
         b = dctrlstr(B);
      else b="";
      if (strlen(b)>(COLS-10)/2) b="<long string>";
      strcat(OUT,b);
      if (flag==1) {
         strcat(OUT,"\n");
         set_spaces(f_pt->maxfsegms,cpr_pt->name,OUT);
         return(OUT);
      }
      else {
         if (strlen(OUT)<COLS) return(OUT);
         else return("A onto <long string>A");
      }
   }

   /* Catch special case #2  AND the "never" case */
   if (*(cpr_pt->name+3) == '1' && *A==null && *B==null) {
      len=0;
      for (p_pt=cpox0_pt; p_pt!=NULL && p_pt->name!=NULL; p_pt=p_pt->next)
         len+=(dctrll(p_pt->name) + 20);
      OUT = (char*)calloc((unsigned)(len+50),sizeof(char));
      freeflag = 1;
      for (p_pt=cpox0_pt; p_pt!=NULL && p_pt->name!=NULL && p_pt->next!=NULL;
           p_pt=p_pt->next) {
         x = dctrlstr(p_pt->name);
         if (strlen(x)>(COLS-10)/2) x="<long string>";
         strcat(OUT,x);
         strcat(OUT," onto ");
         y = dctrlstr(p_pt->next->name);
         if (strlen(y)>(COLS-10)/2) y="<long string>";
         strcat(OUT,y);
         if (flag==1) strcat(OUT,"\n");
         else strcat(OUT,"   ");
         p_pt=p_pt->next;
         if (flag==0 && strlen(OUT)>COLS) {
            *(OUT+COLS-1) = null;
            if (f_pt->bundle==1)  {
               *(OUT+COLS-15) = null;
               strcat(OUT,"... [bundled]");
            }
            return(OUT);
         }
      }
      if (f_pt->bundle==1) strcat(OUT," [bundled]");
      return(OUT);
   }

   /* General case */
   OUT0 = (char*)calloc((unsigned)(dctrll(A)+dctrll(B)+20),sizeof(char));
   if (*A!=null) a = dctrlstr(A);
   else a="";
   if (strlen(a)>(COLS-10)/2) a="<long string>";
   sprintf(OUT0,"A onto %sA",a);
   if (*B!=null) b = dctrlstr(B);
   else b="";
   if (strlen(b)>(COLS-10)/2) b="<long string>";
   strcat(OUT0,b);
   if (x0==null) {
      OUT = (char*)calloc((unsigned)(strlen(OUT0)+200),sizeof(char));
      freeflag = 1;
      strcpy(OUT,OUT0);
      free(OUT0);
   }
   else {
      len=0;
      for (p_pt=cpox0_pt; p_pt!=NULL && p_pt->name!=NULL && p_pt->next!=NULL;
           p_pt=p_pt->next)
         len+=(dctrll(p_pt->name) + 20);
      OUT = (char*)calloc((unsigned)(strlen(OUT0)+len+200),sizeof(char));
      freeflag = 1;
      strcpy(OUT,OUT0);
      free(OUT0);
      if (flag==1) strcat(OUT,"\n");
      else strcat(OUT," ");
      strcat(OUT,"exceptions:");
      if (flag==1) strcat(OUT,"\n");
      else strcat(OUT," ");
      for (p_pt=cpox0_pt; p_pt!=NULL && p_pt->name!=NULL && p_pt->next!=NULL;
           p_pt=p_pt->next) {
         x = dctrlstr(p_pt->name);
         if (strlen(x)>(COLS-10/2)) x="<long string>";
         strcat(OUT,x);
         strcat(OUT," onto ");
         y = dctrlstr(p_pt->next->name);
         if (strlen(y)>(COLS-10/2)) y="<long string>";
         strcat(OUT,y);
         if (flag==1) strcat(OUT,"\n");
         else strcat(OUT,"   ");
         p_pt=p_pt->next;
         if (flag==0 && strlen(OUT)>COLS) {
            *(OUT+COLS-7) = null;
            strcat(OUT," ... ");
            return(OUT);
         }
      }
   }
   if (flag==1) set_spaces(f_pt->maxfsegms,cpr_pt->name,OUT);
   return(OUT);
}

VOID set_spaces(m,name,out)
char *name, *out;
int m;
{
   strcat(out,"<strings>");
   if (*name == '1') strcat(out,"<SPACE>");
   if (m>1) {
      if (*(name+1) == '1') 
         strcat(out,"B<SPACE>B<SPACE> ... <SPACE>B");
      else strcat(out,"BB...B");
   }
   else strcat(out,"B");
   if (*(name+2) == '1') strcat(out,"<SPACE>");
   strcat(out,"<strings>");
}

clrlns(i,j)
int i,j;
{ for (;i<=j;i++) { move(i,0); clrtoeol(); } }

int show_map(f_pt,s)
struct field *f_pt;
char *s;
{  int ci, cj;
   register char *c_pt;

   clrlns(4,LINES-1);
   show_caption(f_pt,"Command Line Mapping");
   show_cmd("",7);

   mvaddstr(5,1,"Current command line mapping:");
   ci = 6;
   if ((c_pt = view_type(f_pt,1)) == NULL) return(0);
   
   move(++ci,1);
   if (*c_pt==' ') while (*++c_pt==' ');
   while (ci<LINES-6 && *c_pt) {
      if (*c_pt=='\015' || *c_pt==CTRL(J)) {
         move(++ci,1);
         while (*++c_pt==' ');
      }
      if (*c_pt!=null) addch(*c_pt++);
   }
   getyx(stdscr,ci,cj);
   if (*c_pt!=null) 
      mvaddstr(ci+1,1," [more] ");
   getyx(stdscr,ci,cj);
   switch(ask(s,ci+2,1,"yn",helpnum,fthelp)) {
      case 'y':
         addstr("es"); refresh();
         return(2);
         break;
      case 'n':
         addstr("o"); refresh();
         return(1);
         break;
      case CTRL(T):
      case KEY_F(5):
         return(0);
         break;
      case CTRL(V):
      case KEY_F(2):
         return(3);
         break;
      case CTRL(R):
      case KEY_F(4):
         return(4);
         break;
   }
}

rmslash(s)  /* Removes non-escaped ^C's  */
char *s;
{   register char *c_pt;

    if (s==NULL || *s==null) return(0);

    do {
       if (*s=='\\') 
          for (c_pt = s; *c_pt; c_pt++) 
             *c_pt = *(c_pt+1);
   }  while (*++s);
}

set_valstrs(f_pt)
struct field *f_pt;
{  struct fix *f0_pt, *f1_pt, *v0_pt;
   struct vfunc *v_pt, *addval();

   if (f_pt==(struct field*)0 || 
       (f0_pt=f_pt->first_cpr_pt)==(struct fix*)0 ||
        f0_pt->name==NULL || strlen(f0_pt->name)!=4)
      return(0);
       
   if (*(f_pt->first_cpr_pt->name+3) !='1')  {
      rmvaln(f_pt,4);
      return(0);
   }

   for (f0_pt = f_pt->first_cpo_pt;
        f0_pt != (struct fix *)0  && 
        f0_pt->name != NULL && 
        (f1_pt = f0_pt->next) != (struct fix *)0  && 
        f1_pt->name != NULL;
        f0_pt = f1_pt->next) {

      if (f0_pt == f_pt->first_cpo_pt) {  /* First string */
         rmvaln(f_pt,4);
         if ((v_pt = addval(f_pt,4)) == (struct vfunc *)0) return(0);
         v0_pt = v_pt->pr_pt = (struct fix*)malloc(sizeof(struct fix));
      }
      else v0_pt = v0_pt->next = (struct fix*)malloc(sizeof(struct fix));

      v0_pt->name = f0_pt->name;
      v0_pt->next = (struct fix *)0;
   }

   return(1);
}

int restart()
{   int ask();

qqq:
   switch(ask(
"Mapping changes will not be saved. Continue or exit mapping module (c/e):",
   LINES-4,1,"ce",helpnum,fthelp)) {
      case 'c':
         return(1);
         break;
      case 'e':
         return(0);
         break;
      default:
         beep();
         goto qqq;
   } 
}
