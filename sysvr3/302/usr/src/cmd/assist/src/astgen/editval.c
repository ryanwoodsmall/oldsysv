/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:editval.c	1.26"

#include "../forms/muse.h"
#include "vdefs.h"

/*
 *  editval is called by vedit, and has as argument:
 *    v_pt: assigned vfunc pointer, with num defined.
 *    mess: header to instruct user about meaning of pre- (post)fixes.
 *  prompt: prompt given on each line, serving as placeholder/anchor
 *    f_pt: current field pointer
 *
 *  editval is called only when a validation function has pre- and/or
 *    post-fixes.  It is called by vedit().
 *
 *  editval lists any pre-existing fixes in a form, and allows the
 *    user to edit or delete these fixes and add new ones.
 *
 *  the linked fix list originating at v_pt is updated.
 */


char *vvisible[] = {
   "<empty>",
   "^A",
   "^B",
   "^C",
   "^D",
   "^E",
   "^F",
   "^G",
   "^H",
   "<TAB>",
   "<NL>",
   "^K",
   "^L",
   "<CR>",
   "^N",
   "^O",
   "^P",
   "^Q",
   "^R",
   "^S",
   "^T",
   "^U",
   "^V",
   "^W",
   "^X",
   "^Y",
   "^Z",
   "<ESC>",
   "<FS>",
   "<GS>",
   "<RS>",
   "<US>",
   "<SPACE>",
   NULL
};

int editval(f_pt,v_pt,mess,prompt,prpo,numfs,hnum,hfile,r)
struct field *f_pt;
struct vfunc *v_pt;
char *mess, *prompt;
int prpo;
char *hfile, *r;
int hnum;
{  struct fix *fi_pt, *fi0_pt;
   static struct segment *ss=(struct segment*)0;
   struct segment *s_pt;
   int i, enumfs, flag;

   if (ss!=(struct segment*)0) {
      free(ss);
      ss = (struct segment*)0;
   }

   ss = (struct segment*)calloc((unsigned)(LINES),sizeof(struct segment));

   if (v_pt==(struct vfunc *)0) return(0);

   for (s_pt=ss+LINES-1; s_pt>ss; s_pt--) s_pt->word=NULL;

   enumfs = (numfs==0) ? LINES-8 : numfs;
   if (enumfs>12) enumfs=12;
   
   switch(prpo) {
   case 0:
      if (v_pt->pr_pt==(struct fix *)0)  {
         v_pt->pr_pt = (struct fix *)malloc(sizeof(struct fix));
         v_pt->pr_pt->name = NULL;
         v_pt->pr_pt->next = (struct fix *)0;
      }
      fi_pt = v_pt->pr_pt;
      break;
   case 1:
      if (v_pt->po_pt==(struct fix *)0) {
         v_pt->po_pt = (struct fix *)malloc(sizeof(struct fix));
         v_pt->po_pt->name = NULL;
         v_pt->po_pt->next = (struct fix *)0;
      }
      fi_pt = v_pt->po_pt;
      break;
   default:
      return(0);
      break;
   }

   s_pt = ss;

   for (fi0_pt=fi_pt, i=5; i<5+enumfs; i++) {
      if (fi0_pt!=(struct fix *)0 &&fi0_pt->name!=NULL) s_pt->word = fi0_pt->name;
      else s_pt->word = NULL;
      s_pt->row  = i;
      s_pt->col  = strlen(prompt)+1;
      s_pt++;
      if (fi0_pt!=(struct fix *)0) fi0_pt = fi0_pt->next;
   }

   refresh();

   flag = fedit(f_pt,ss,(struct segment *)0,prompt,NULL,mess,enumfs,
         hfile,hnum,r);

/*
 * Store words from ss in linked list of fixes.
 */

   fi_pt->name = NULL; /* In case use wiped out all fixes */
   fi_pt->next = (struct fix *)0; 

   for (s_pt=ss, fi0_pt=fi_pt, i=5; s_pt->word!=NULL && i<5+enumfs; 
               s_pt++, i++) {
      fi0_pt->name=(char*)calloc((unsigned)(strlen(s_pt->word)+2),sizeof(char));
      strcpy(fi0_pt->name,s_pt->word);
      if ((s_pt+1)->word != NULL) {
         fi0_pt->next = (struct fix *)malloc(sizeof(struct fix));
         fi0_pt = fi0_pt->next;
         fi0_pt->next = (struct fix *)0;
         fi0_pt->name = NULL;
      }
      else fi0_pt->next = (struct fix *)0;
   }
   return(flag);
}


char *dctrlstr(s)
char *s;
{  register char *c_pt;
   register int ll;
   static char *line=NULL;
   static int freeflag=0;

   if (s==NULL) return(&nullchar);

   if (freeflag) free(line);
   line = (char*)calloc((unsigned)(dctrll(s)+2),sizeof(char));
   freeflag = 1;

   if (*s==null) strcpy(line,"<empty>");
   else for (c_pt=s;*c_pt; c_pt++) {
      if ((int)*c_pt<=32) strcat(line,vvisible[(int)*c_pt]);
      else  {
         ll = strlen(line);
         *(line+ll) = *c_pt;
         *(line+ll+1) = null;
      }
   }
   return(line);
}


int dctrlshow(i,j,n,s)
register int i, j, n;
char *s;
{  register char *c_pt;
   register int ll, maxx;
   int flag=0;   /* 0: no editor needed; 1: editor needed */
   char *line;

   maxx = n-j-1;
   if (maxx<0 || s==NULL) return(0);

   line = (char*)calloc((unsigned)(dctrll(s)+2),sizeof(char));

   if (*s==null) strcat(line,"<empty>");
   else for (c_pt=s;*c_pt; c_pt++) {
      if ((int)*c_pt<=32) strcat(line,vvisible[(int)*c_pt]);
      else  {
         ll = strlen(line);
         *(line+ll) = *c_pt;
         *(line+ll+1) = null;
      }
   }
   move(i,j);
   if (strlen(line)>maxx-1) {
      for (c_pt=line; *c_pt && (c_pt-line)<maxx-8; c_pt++)
         addch(*c_pt);
      addstr(" [more]");
      flag=1;
   }
   else
      for (c_pt=line; *c_pt && (c_pt-line)<maxx-1; c_pt++)
         addch(*c_pt);
   free(line);
   return(flag);
}


dctrlblank(i,j,n,s)
char *s;
{  register int i0, ll;

   if (s==NULL) return(0);

   ll = dctrll(s);
   move(i,j);
   for (i0=0; i0<n && i0<=ll; i0++) addch(' ');
}

int dctrll(s)
char *s;
{  register char *c_pt;
   register int count=0;

   if (s==NULL) return(0);
   else if (*s==null) return(7);
   else 
      for (c_pt=s;*c_pt; c_pt++)
         if ((int)*c_pt<=32) count += strlen(vvisible[(int)*c_pt]);
         else count++;
   return(count);
}

char *dctrlch(ch)
char ch;
{  static char *chst;
   static int freeflag = 0;

   if (ch==null) return("<empty>");
   if ((int)ch<=32) return(vvisible[(int)ch]);

   if (freeflag) free(chst);
   freeflag = 1;
   chst = (char*)calloc(2,sizeof(char));
   *chst = ch;
   *(chst+1) = null;
   return(chst);
}


fedit(f_pt,s0,s1,p0,p1,m,numfs,hf,hn,R)
struct field *f_pt;
char *hf, *R;
int hn;
struct segment *s0, *s1; /* assumed to be arrays of LINES elements */
char *p0, *p1, *m;           /* prompts */
int numfs;
{  struct segment *s_pt, *s, *s00_pt, *s01_pt, *s10_pt, *s11_pt;
   char *scratch, *c_pt, *editstr();
   int emode, i, c, fnumfs, limit, limit0, limit1, errnum, c0;
   int literal = 0;
   int freeflag = 0;
   int err_flag;

   fnumfs = (numfs==0) ? LINES-8 : numfs;
   if (fnumfs>12) fnumfs=12;

   showscr(f_pt,s0,s1,p0,p1,m,fnumfs);

   limit0 = (p1==NULL) ? 80 : 40;
   limit1 = 80;


   s_pt = s0;
   limit = limit0;
   emode = 0;
   if (s_pt->word!=NULL) {
      if (strlen(s_pt)>1022)
          scratch = (char*)calloc((unsigned)(strlen(s_pt->word)+2),sizeof(char));
      else scratch = (char*)calloc(1024,sizeof(char));
      strcpy(scratch,s_pt->word);
   }
   else scratch = (char*)calloc(1024,sizeof(char));
   freeflag = 1;
  
   err_flag = 0;

   s = s0;

   err_rpt(0,FALSE);

   for (;;) {

      bottom_stuff(emode,R);
      if (emode==0) move(s_pt->row,s_pt->col);
      else move(s_pt->row,s_pt->col+dctrll(scratch));
      refresh();

      c = getch();
      err_rpt(0,FALSE);
      if (literal) goto insert;

      switch(c)  {
      case KEY_F(6):
      case CTRL(Y):
         store(stdscr,0,LINES,COLS);
         fhelp(hf,hn,2,2);
         store(stdscr,1,LINES,COLS);
         refresh();
         break;
      case KEY_F(4):
      case CTRL(R):
         if (emode==0) return(1);
         else beep();
         break;
      case KEY_F(5):
      case CTRL(T):
         if (emode==0) return(0);
         else beep();
         break;
      case KEY_F(3):
      case CTRL(L):
         REDRAW;
         break;
      case CTRL(J):
      case CTRL(M):
         if (s1==(struct segment *)0) goto DOWN;   /* 1-col displays: move down */
         else if (s1!=(struct segment *)0 && s==s0) goto RIGHT; /*2-col: right */
         else if (s1!=(struct segment *)0 && s==s1) {  /* 2-col: down and left */
            if (emode==1) {
               if ((err_flag=sscpy(s_pt,scratch,limit))==0) {
                  if (s_pt-s<fnumfs-1) 
                     s_pt = s0 + (int)(s_pt-s1) + 1;
                  if (freeflag) free(scratch);
                  freeflag = 0;
                  s = s0;
                  limit = limit0;
               }
               else break;
            }
            else if (emode == 0) {
               if (s_pt-s>=fnumfs-1) beep();
               else {
                  if (s_pt->word==NULL) {
                     s00_pt = s0 + (int)(s_pt - s);
                     s01_pt = s00_pt + 1;
                     if (s1!=(struct segment *)0) {
                         s10_pt = s1 + (int)(s_pt - s);
                         s11_pt = s10_pt + 1;
                     }
                     else s10_pt = s11_pt = (struct segment *)0;
   
                     if (s00_pt->word==NULL                           &&
                      s01_pt->word==NULL                              &&
                      (s10_pt==(struct segment*)0 || s10_pt->word==NULL) &&
                      (s11_pt==(struct segment*)0 || s11_pt->word==NULL))
                        beep();
                     else {
                        s_pt = s0 + (int)(s_pt-s1) + 1;
                        s = s0;
                        limit = limit0;
                     }
                  }
                  else  {
                     s_pt = s0 + (int)(s_pt-s1) + 1;
                     s = s0;
                     limit = limit0;
                  }
               }
            }
         }
         if (!err_flag) emode = 0;
         err_flag=0;
         break;
      case KEY_DOWN:
      case CTRL(N):
DOWN:    if (emode==1) {
            if ((err_flag=sscpy(s_pt,scratch,limit))==0) {
               if (s_pt-s<fnumfs-1) s_pt++;
               if (freeflag) free(scratch);
               freeflag = 0;
            }
            else break;
         }

         else if (emode == 0) {
            if (s_pt-s>=fnumfs-1) beep();
            else {
               if (s_pt->word==NULL) {
                  s00_pt = s0 + (int)(s_pt - s);
                  s01_pt = s00_pt + 1;
                  if (s1!=(struct segment *)0) {
                      s10_pt = s1 + (int)(s_pt - s);
                      s11_pt = s10_pt + 1;
                  }
                  else s10_pt = s11_pt = (struct segment *)0;

                  if (s00_pt->word==NULL                              &&
                   s01_pt->word==NULL                                 &&
                   (s10_pt==(struct segment*)0 || s10_pt->word==NULL) &&
                   (s11_pt==(struct segment*)0 || s11_pt->word==NULL))
                     beep();
                  else s_pt++;
               }
               else s_pt++;
            }
         }
         if (!err_flag) emode = 0;
         err_flag = 0;
         break;
      case KEY_UP:
      case CTRL(P):
         if (emode==1) {
            if ((err_flag=sscpy(s_pt,scratch,limit))==0) {
               if (freeflag) free(scratch);
               freeflag = 0;
            }
            else break;
         }
         if (s_pt>s) s_pt--;
         else beep();
         if (!err_flag) emode = 0;
         err_flag = 0;
         break;
      case KEY_LEFT:
      case CTRL(B):
         if (emode==1) {
            if ((err_flag=sscpy(s_pt,scratch,limit))==0) {
               if (freeflag) free(scratch);
               freeflag = 0;
            }
            else break;
         }
         if (s==s1) {
            s = s0;
            limit = limit0;
            s_pt = s0 + (int)(s_pt - s1);
         }
         else beep();
         if (!err_flag) emode = 0;
         err_flag = 0;
         break;
      case KEY_RIGHT:
      case CTRL(W):
RIGHT:   if (emode==1) {
            if ((err_flag=sscpy(s_pt,scratch,limit))==0) {
               if (freeflag) free(scratch);
               freeflag = 0;
            }
            else break;
         }
         if (s==s0 && s1!=(struct segment *)0) {
            s = s1;
            limit = limit1;
            s_pt = s1 + (int)(s_pt - s0);
         }
         else beep();
         if (!err_flag) emode = 0;
         err_flag = 0;
         break;
      case CTRL(O):
      case KEY_F(8):
         if (emode==0 && s_pt->word!=NULL) {

            move(s_pt->row,0);
            clrtoeol();

            if (s==s0 && s1!=(struct segment *)0)
               s11_pt = s1 + (int)(s_pt-s)+1;
            else s11_pt = s0 + (int)(s_pt-s)+1;

            if (s_pt-s == fnumfs-1) {  /* last segment */
               s_pt->word = NULL;
               if (s1 != (struct segment*)0)
                  (s11_pt-1)->word = NULL;
            }
            else for (s00_pt=s_pt+1, i=s_pt-s+1; i<fnumfs; 
                      i++, s00_pt++, s11_pt++) {
               move (s00_pt->row,0); clrtoeol();
               (s00_pt-1)->word = s00_pt->word;
               s00_pt->word = NULL;
               if (s1 != (struct segment*)0) {
                  (s11_pt-1)->word = s11_pt->word;
                  s11_pt->word = NULL;
               }
            }
            if (s_pt->word==NULL) {
               if (s_pt>s) s_pt--; 
               else s_pt=s;
            }
            showscr(f_pt,s0,s1,p0,p1,m,numfs);
         }
         else beep();
         break;
      case CTRL(E):
      case KEY_F(7):
         if (emode==0) {
            dctrlblank(s_pt->row,s_pt->col,limit-s_pt->col,s_pt->word);
            while ((errnum=chckstr(s_pt->word=editstr(s_pt->word),0))>0) {
               err_rpt(errnum,TRUE);
               show_cmd("",16);
               refresh();
               getch();
               err_rpt(0,FALSE);
            }
            showscr(f_pt,s0,s1,p0,p1,m,fnumfs);
            dctrlshow(s_pt->row,s_pt->col,limit,s_pt->word);
         }
         else beep();
         break;
      case CTRL(G):
      case KEY_F(1):
         if (emode==0) {
            if (s_pt->word!=NULL) 
               *s_pt->word==null;
            move (s_pt->row,s_pt->col);
            for (i=s_pt->col; i<limit; i++) mvaddch(s_pt->row,i,' ');
            emode = 1;
            c_pt = scratch = (char*)calloc(1024,sizeof(char));
            freeflag = 1;
            dctrlshow(s_pt->row,s_pt->col,limit,scratch);
            if (s==s0 && s1!=(struct segment*)0) {
               s00_pt = s1 + (int)(s_pt-s);
               if (s00_pt->word == NULL)
                 s00_pt->word = &nullchar;
               dctrlshow(s00_pt->row,s00_pt->col,limit1,s00_pt->word);
            }
            else if (s==s1) {
               s00_pt = s0 + (int)(s_pt-s);
               if (s00_pt->word == NULL)
                 s00_pt->word = &nullchar;
               dctrlshow(s00_pt->row,s00_pt->col,limit0,s00_pt->word);
            }
         }
         else beep();
         break;
      case CTRL(C):
         if (emode==0) beep();
         else {
            if (literal==0) literal=1;
            else {
               literal=0;
               goto insert;
            }
         }
         break;
      case '\010':
         if (emode==0) beep();
         else {
            if (c_pt>scratch) {
               dctrlblank(s_pt->row,s_pt->col,limit-s_pt->col,scratch);
               *c_pt-- = null;
               *c_pt = null;
               dctrlshow(s_pt->row,s_pt->col,limit,scratch);
            }
            else beep();
         }
         break;
      default:
insert:  if (emode!=1) beep();
         else {
            if (errnum=chckchar((char)c)) err_rpt(errnum,1);
            else {
               if (literal) literal=0;
               dctrlblank(s_pt->row,s_pt->col,limit-s_pt->col,scratch);
               *c_pt++ = c;
               if (dctrlshow(s_pt->row,s_pt->col,limit,scratch)) {
                  emode=0;
                  show_cmd("",27);
                  refresh();
                  while ((c0=getch()) != CTRL(M) && c0!= CTRL(J)) beep();
                  dctrlblank(s_pt->row,s_pt->col,limit-s_pt->col,scratch);
                  s_pt->word = editstr(scratch);
                  while ((errnum=chckstr(s_pt->word,0)) > 0) {
                     err_rpt(errnum,TRUE);
                     show_cmd("",16);
                     refresh();
                     getch();
                     err_rpt(0,FALSE);
                     s_pt->word=editstr(s_pt->word);
                  }
                  if (freeflag) free(scratch);
                  freeflag = 0;
                  showscr(f_pt,s0,s1,p0,p1,m,fnumfs);
                  dctrlshow(s_pt->row,s_pt->col,limit,s_pt->word);
               }
            }
         }
         break;
      }

   }
}

int sscpy(s_pt,z,limit)
struct segment *s_pt;
char *z;
{  int tmp;
   if (s_pt==NULL) return(0);

   if (z==NULL) s_pt->word==NULL;
   else {
      if (tmp=chckstr(z,0)) {
         err_rpt(tmp,TRUE);
         show_cmd("",28);
         refresh();
         getch();
         err_rpt(0,FALSE);
         show_cmd("",6);
         return(tmp);
      }
      else {
         s_pt->word = (char*)calloc((unsigned)(strlen(z)+2),sizeof(char));
         strcpy(s_pt->word,z);
         *z = null;
      }
   }
   return(0);
}


showscr(f_pt,s0,s1,p0,p1,m,numfs)
struct field *f_pt;
struct segment *s0, *s1;
char *p0, *p1, *m;
int numfs;
{  int i, limit;
   struct segment *s_pt;
   int snumfs;

   snumfs = (numfs==0) ? LINES-8 : numfs;
   if (snumfs>12) snumfs=12;

   limit = (p1==NULL) ? 80-strlen(p0) : 40-strlen(p0);

   if (f_pt!=(struct field*)0) show_caption(f_pt,m);
   else {
      mvaddstr(1,(COLS-strlen(m))/2,m);
      draw_line(3);
   }

   for (s_pt=s0, i=5; i<5+snumfs; s_pt++, i++) {
      mvprintw(i,1,"%s",p0);
      if (s_pt->word!=NULL) 
         dctrlshow(s_pt->row,s_pt->col,limit,s_pt->word);
   }

   if (p1!=NULL) {
      limit = 40-strlen(p1);
      for (s_pt=s1, i=5; i<5+snumfs; s_pt++, i++) {
         mvprintw(i,41,"%s",p1);
         if (s_pt->word!=NULL)
            dctrlshow(s_pt->row,s_pt->col,80,s_pt->word);
      }
   }
}



int edit_exit(hfile,hnum,r)
char *hfile, *r;
int hnum;
{  char **e_pt;
   struct segment ss[7], *s_pt;
   char *prompt, *m;
   int i, xnumfs = 5, flag;

   prompt = "Exit string:";
   m = "Type Exit Strings";

   s_pt = ss;

   for (e_pt = exit_strs, i=5; i<5+xnumfs; e_pt++, i++) {
      s_pt->word = *e_pt;  /* Can be NULL */
      s_pt->row  = i;
      s_pt->col  = strlen(prompt)+1;
      s_pt++;
   }

   refresh();

   flag = fedit((struct field*)0,ss,(struct segment *)0,prompt,NULL,m,xnumfs,
         hfile,hnum,r);

/*
 * Store words from ss in exit_strs[]
 */
   for (s_pt=ss, e_pt = exit_strs, i=2; s_pt->word!=NULL && i<2+xnumfs; 
               s_pt++, i++) {
      *e_pt=(char*)calloc((unsigned)(strlen(s_pt->word)+2),sizeof(char));
      strcpy(*e_pt,s_pt->word);
      if ((s_pt+1)->word != NULL)
         e_pt++;
      else *(e_pt+1) = NULL;
   }
   return(flag);
}

bottom_stuff(m,rlab)
char *rlab;
{
   if (m==0) 
      show_cmd(rlab,5);
   else  
      show_cmd("",6);
}
