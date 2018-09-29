/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:msupport.c	1.27"


#include "muse.h"
#include "mmuse.h"


char *leftmenu = " CURRENT MENU: ";
char *leftform = " CURRENT FORM: ";
char *chelp = "HELP: ^A";
char *fchelp = "HELP: f8 OR ^A";

VOID show_header()
{  char center[128], right[128], left[128];
   int xcenter, llength, clength, lrlength;

   if (equal((lab_pt-1)->screen_name,"UNIX"))  {
      strcpy(center,"[ACCESSED FROM UNIX SYSTEM]");
      lrlength = 31;
   }
   else {
      sprintf(center,"PREVIOUS MENU OR FORM: %s",(lab_pt-1)->screen_name);
      lrlength = (strlen(center)>28) ? 32 : (strlen(center)+2);
   }

   if (term & FKEYS) strcpy(right,fchelp);
   else strcpy(right,chelp);

   if (mode==MENU)
      strcpy(left,leftmenu);
   else
      strcpy(left,leftform);
   strcat(left,lab_pt->screen_name);

   if (strlen(right)+strlen(left) > COLS-lrlength) {
      llength = COLS-lrlength-strlen(right);
      if (llength>4 && llength-4<length(left)) {
        *(left+llength-4) = null;
         strcat(left," ...");
      }
   }

   if (strlen(left) + strlen(center) + strlen(right) > COLS-6) {
      clength = COLS-6-strlen(left)-strlen(right);
      if (clength>4 && clength-4<length(center)) {
         *(center+clength-4) = null;
         strcat(center," ...");
      }
   }

   xcenter = (strlen(left) + COLS-strlen(right) - strlen(center)) / 2;

   move(0,0); clrtoeol();
   mvaddstr(0,0,left);
   mvaddstr(0,xcenter,center);
   mvaddstr(0,COLS-strlen(right),right);

   ALT;
   mvaddstr(1,0,stripes);
   NALT;
}



VOID curses(flag)
int flag;
{  extern int idone();

   status=0;

   switch(flag)
   {
   case ON:
      initscr();
      nonl();
      cbreak();
      noecho();
      idlok(stdscr,FALSE);
      keypad(stdscr,TRUE);
      intrflush(stdscr,TRUE);
      signal(SIGINT,idone);
      signal(SIGTERM,idone);
      signal(SIGQUIT,idone);
      signal(SIGALRM,SIG_IGN);
      break;
   case OFF:
      clear();
      refresh();
      endwin();
      break;
   }
}


int exec_cmd(k,str,flag)
char *str;
int k, flag;
{  register int c;
   char key[5], *getenv();

   if (k==0) strcpy(key,f[4]);
   else 
   {
      if (k==1) strcpy(key,"^R");
      else strcpy(key,"f4");
   }
   clear();
   move (0,0); 
   refresh(); 
   if (flag)
   {
      printw(" EXECUTING: %s\n\r\n\r",str);
      refresh();
   }
   saveterm(); resetterm();
   updatetty(&termbuf);

   if ((equal(str,"cd") || equaln(str,"cd ",3)) && flag!=2)  
   {
      if (strlen(str)>3) 
         mchdir(str+3);
      else chdir((char*)getenv("HOME"));
   }
   else if (strlen(str) >= 9 && equaln(str,"umask ",6) && flag!=2) /* see ^E */
      umask((int)strtol(str+6,(char**)0,8)); /* umask ooo */
   else
      system(str);


   if (flag)
   {
      printf("\n\r\n\rSTRIKE RETURN TO GO BACK TO ");
      printf("%s",lab_pt->screen_name);
      printf(" SCREEN,");
      printf(" %s",key);
      if (equaln((lab_pt-1)->files_name,"UNIX",4))
         printf(" TO EXIT ASSIST\n\r");
      else
      {
         printf(" TO GO TO ");
         printf("%s",(lab_pt-1)->screen_name);
         printf(" SCREEN\n\r");
      }
      fflush(stdout);
      catchtty(&termbuf);
      fixterm();
      refresh();
      c=getch();
      clear();
      if (c==CTRL(E)) return(c);
      if (c==CTRL(D))
      {
         status=0;
         done();
      }
      clearok(curscr,TRUE);
   
      if (c==CTRL(R) || c==KEY_F(4)) next_screen("",BACK);
   
      if (equaln(lab_pt->files_name,"UNIX",4))
      {
         curses(OFF); status=0; done();
      }
   }
   else
   {
      clearok(curscr,TRUE);
      catchtty(&termbuf);
      fixterm();
   }
   return(c);
}




int find_field(up_down,first_f_pt,last_f_pt)
register int up_down;
register struct field *first_f_pt, *last_f_pt;
{  
   register int found;
   Field_pt += up_down;
   if (Field_pt<first_f_pt)        Field_pt = last_f_pt;
   if (Field_pt>last_f_pt) Field_pt = first_f_pt;


   if ((Field_pt->type==0 || Field_pt->type==9) && up_down==0)
      found=0;
   else
   {
      while (Field_pt->type==0 || Field_pt->type==9)
      {
         Field_pt+=up_down;
         if (Field_pt<first_f_pt)        Field_pt = last_f_pt;
         if (Field_pt>last_f_pt) Field_pt = first_f_pt;
      }
      found=1;
   }

   Segm_pt = Field_pt->first_s_pt;
   Row = Segm_pt->row;
   Col = Segm_pt->col;
   return(found);
}


int gotofield(letter,first_f_pt,last_f_pt)
register struct field *first_f_pt, *last_f_pt;
char letter;
{  
   register int found=0, i;
   register struct field *f_pt;
   register struct segment *s_pt;
   register char letter0, *c_pt;

   i = last_f_pt-first_f_pt+1;
   if ((f_pt=Field_pt+1) > last_f_pt) f_pt=first_f_pt;
   letter |= 040;

   while (i-- && found==0)
   {
      if (f_pt->type!=0 && f_pt->type!=9 &&
          (s_pt=f_pt->first_s_pt)!=NULL &&
           s_pt->word!=NULL && *(c_pt=s_pt->word)!=null)
      {
         letter0 =  *c_pt;
         if (letter0==SPACE || letter0=='*')
            while ((letter0 = *++c_pt) == SPACE || letter0 == '*');
         letter0 |= 040;
         if (letter==letter0) found=1;
      }
      if (++f_pt>last_f_pt) f_pt=first_f_pt;
   }

   if (found)
   {
      if ((Field_pt=f_pt-1) < first_f_pt) Field_pt=last_f_pt;
      Segm_pt = Field_pt->first_s_pt;
      Row = Segm_pt->row;
      Col = Segm_pt->col;
   }
   return(found);
}




VOID highlight(s_pt,on_off,f_pt)
struct segment *s_pt;
struct field *f_pt;
int on_off;
{  register struct field *f1_pt;
   register struct segment *s0_pt;
   if (on_off)
   {
      REV;
      if (mode==MENU && s_pt!=NULL)
      {
         arrow_buff.row = arrow.row = s_pt->row;
         arrow_buff.col = arrow.col = s_pt->col-1;
      }
      else if (mode!=MENU && f_pt!=NULL)
      {
         arrow_buff.row = arrow.row = f_pt->row;
         arrow_buff.col = arrow.col = f_pt->col-1;
      }
      else if (mode==POPUP && s_pt!=NULL)
      {
         arrow_buff.row = arrow.row = s_pt->row;
         arrow_buff.col = arrow.col = s_pt->col-1;
      }
      if (arrow.row-Stdscr_loc>1 && arrow.row-Stdscr_loc<=SCRLINES) {
         *arrow_buff.word = mvinch(arrow.row-Stdscr_loc,arrow.col);
         *(arrow_buff.word+1) = null;
      }
      else
         *arrow_buff.word = null;
      show((&arrow));
      for (s0_pt=s_pt; s0_pt!=NULL; s0_pt=s0_pt->next)
         if (s0_pt->word!=NULL && *(s0_pt->word)!=null) show(s0_pt);
      if (f_pt!=NULL)
      {
         mvaddstr(f_pt->row-Stdscr_loc,f_pt->col,f_pt->caption);
         if ((f1_pt=f_pt)>fields)
         while (--f1_pt>=fields && f1_pt->type==0 && 
           f1_pt->caption!=NULL && f1_pt->row==(f1_pt+1)->row-1)
             mvaddstr(f1_pt->row-Stdscr_loc,f1_pt->col,f1_pt->caption);
      }
      NREV;
   }
   else
   {
      if (arrow_buff.row-Stdscr_loc>1 && 
          arrow_buff.row-Stdscr_loc<=SCRLINES)
         show((&arrow_buff));
      for (s0_pt=s_pt; s0_pt!=NULL; s0_pt=s0_pt->next)
         if (s0_pt->word!=NULL && *(s0_pt->word)!=null) show(s0_pt);
      if (f_pt!=NULL)
      {
         mvaddstr(f_pt->row-Stdscr_loc,f_pt->col,f_pt->caption);
         if ((f1_pt=f_pt)>fields)
         while (--f1_pt>=fields && f1_pt->type==0 && 
           f1_pt->caption!=NULL && f1_pt->row==(f1_pt+1)->row-1)
             mvaddstr(f1_pt->row-Stdscr_loc,f1_pt->col,f1_pt->caption);
      }
   }
}


VOID show_cmd(s)
register char *s;
{  
   register char *c_pt, *b_pt, *t_pt;
   register int i = 0;
   register int j = 0;
   char temp[10][CAPSIZE];

   move(SCRLINES+1,0); clrtoeol();
   SCRLINES = LINES-5;

   move(LINES-3,0); clrtoeol();
   mvaddstr(LINES-3,0,init_cmd);

   if (*s!=null) 
   {

      c_pt = b_pt = s;
      t_pt = temp[i=0];
      while (*c_pt && i<9)
      {
         if (j+cmd_col==COLS-3)
         {
            while (--c_pt > b_pt) *(t_pt--) = null;
            *t_pt++ = '\\';
            *t_pt = null;
            t_pt = temp[++i];
            SCRLINES--;
            j = 0;
         }
         else
         {
            *(t_pt++) = *c_pt;
            *t_pt = null;
            j++;
            if (*c_pt == SPACE) b_pt = c_pt;
            c_pt++;
         }
         if (i==9)
         {
            strcpy(t_pt," [MORE] ");
         }
      }
   }

   else
   {
      i=0;
      temp[0][0]=null;
   }
   
   move(SCRLINES+2,0); clrtoeol();
   mvaddstr(SCRLINES+2,0,init_cmd);
   mvaddstr(SCRLINES+2,cmd_col,temp[0]);
   while (i>=1)
   {
      move(SCRLINES+i+2,0); clrtoeol();
      mvaddstr(SCRLINES+i+2,cmd_col,temp[i--]);
   }


   ALT;
   mvaddstr(LINES-2,0,stripes);
   mvaddstr(SCRLINES+1,0,stripes);
   NALT;
   maxpage = (last_field_pt->last_row-1)/(SCRLINES-2) + 1;
   if (maxpage>1)
      mvprintw(SCRLINES+1,COLS-13," PAGE %1d OF %1d ", page,maxpage);
   move(Std_row,Std_col);
}


VOID next_screen(c_pt,flag)
char *c_pt;
int flag;
{
   register struct label *lab;

   lab_pt->memory = Field_pt-fields;

   switch(flag)
   {
   case BACK:
      lab_pt--;
      break;
   case TOP:
      lab_pt = labels+1;
      if (diffn(lab_pt->files_name,"TOP",3))   /* First visit to the top   */
      {
         lab_pt->memory = 0;
         lab_pt->files_name = (char *)calloc(5,sizeof(char));
         strcpy(lab_pt->files_name,"TOP");
         strcpy(lab_pt->screen_name,"TOP");
      }
      break;
   case NEW:
      if (lab_pt < labels + NLEVELS - 1) lab_pt++;
      else
         for (lab=labels;lab<lab_pt;lab++)
         {
            free(lab_pt->files_name);
            lab_pt->files_name = (char*)calloc(
               (unsigned)(strlen((lab_pt+1)->files_name)+2),
                sizeof(char));
            strcpy(lab->files_name,(lab+1)->files_name);
            strcpy(lab->screen_name,(lab+1)->screen_name);
            lab->memory = (lab+1)->memory;
         }
      lab_pt->files_name = (char *)calloc((unsigned)(strlen(c_pt)+2),
                                          sizeof(char));
      strcpy(lab_pt->files_name,c_pt);
      lab_pt->memory = 0;
      break;
   }
}

VOID next_segment(c_move)
int c_move;
{  
   register struct segment *s_pt;

/*
 * Completely disallow between-segment moves when only one segment 
 * is allowed.
 */
   if (Field_pt->maxfsegms==1)
   {
      strcpy(error_mess," ONE STRING ONLY ");
      return(0);
   }


/*
 * First deal with "normal" cases: there is a next (previous) segment.
 */
   if (c_move==1 && Segm_pt->next!=NULL)
   {
      Segm_pt = Segm_pt->next;
      Col = Segm_pt->col;
      Row = Segm_pt->row;
      if (mode==INSERT) mode=NEWFIELD;
      return(1);
   }

   if (c_move== -1 && Segm_pt->previous!=NULL)
   {
      Segm_pt = Segm_pt->previous;
      Col = Segm_pt->col;
      Row = Segm_pt->row;
      if (mode==INSERT) mode=NEWFIELD;
      return(1);
   }

/*
 * Now we know that a new segment has to be created, either before
 * first or after last segment.
 */
   if (c_move == -1)
   {
      if (mode==MENU)
      {
         strcpy(error_mess," INVALID COMMAND ");
         return(0);
      }
      mode = NEWLSEGMENT;
      s_pt = last_segm_pt++;
      Segm_pt->previous = s_pt;
      s_pt->next = Segm_pt;
      Field_pt->first_s_pt = Segm_pt = s_pt;
      Segm_pt->word = buf_pt;
      TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
      *buf_pt++ = null;
      update(Field_pt->last_row);
      return(1);
   }

   if (c_move==1)
   {
      if (mode==MENU)
      {
         strcpy(error_mess," INVALID COMMAND ");
         return(0);
      }
      mode = NEWRSEGMENT;
      s_pt = last_segm_pt++;
      Segm_pt->next = s_pt;
      s_pt->previous = Segm_pt;
      Segm_pt = s_pt;
      Segm_pt->word = buf_pt;
      *buf_pt = null;
      update(Field_pt->last_row);
      return(1);
   }
   return(0);  /* Should never get here */
}





VOID wipe_out()
{  register char *c_pt;
   register int j;
   char *restore();

   if ((c_pt=Segm_pt->word)!=NULL && *c_pt)
   {
      mvaddch(Std_row,Segm_pt->col,SPACE);
      j = length(c_pt);
      while (--j>0) addch(SPACE);
   }

   if ((c_pt=restore(Field_pt)) == NULL || *c_pt==null)
      Segm_pt->word = NULL;
   else if (Segm_pt->word!=NULL && diff(Segm_pt->word,c_pt))
      Segm_pt->word = c_pt;

   Row = Segm_pt->row;
   Col = Segm_pt->col;
}



fkeys()
{  char fk[500];
   strcpy(fk,"echo \"\033F@!\007!\"\n");
   strcat(fk,"echo \"\033FA!\026!\"\n");
   strcat(fk,"echo \"\033FB!\014!\"\n");
   strcat(fk,"echo \"\033FC!\022!\"\n");
   strcat(fk,"echo \"\033FD!\006!\"\n");
   strcat(fk,"echo \"\033FE!\031!\"\n");
   strcat(fk,"echo \"\033FF!\017!\"\n");
   strcat(fk,"echo \"\033FG!\001!\"\n");
   system(fk);
}



VOID write_file()
{  char s[500];

   if (access(".",02)!=0)
   {
      sprintf(error_mess," CURRENT DIRECTORY DOES NOT HAVE WRITE PERMISSION ");
      return;
   }
   sprintf(s,"%s.assist",lab_pt->files_name);

   if (access(s,00)==0 && access(s,02)!=0)
   {
      sprintf(error_mess," %s DOES NOT HAVE WRITE PERMISSION ",s);
      return;
   }
   if (access(s,00)==0)
   {
      REV;
      mvprintw(LINES-1,40," %s EXISTS; REPEAT ^K TO CONFIRM ",s);
      NREV; move(Std_row,Std_col); refresh();
      move(LINES-1,0); clrtoeol();
      if (getch() != CTRL(K)) return;
   }
   
   fp=fopen(s,"w");  /*Do not have to check write permission any more */
   fprintf(fp,"%s%s\n",dum_cmd_name,command);
   fclose(fp);
   sprintf(warn_mess," COMMAND LINE WRITTEN IN FILE %s ",s);
   sprintf(s,"chmod u+x %s.assist",lab_pt->files_name);
   system(s);
}


#define NEUTRAL   0
#define S_QUOTE   1
#define D_QUOTE   2
#define SKIP      3
#define BACKSL    4

rmblnks(s)
register char *s;
{
   register char *out_pt=s;
   register int state=0;

   if (s==NULL || *s==null) return;

   switch(*s)
   {
   case '\\':
      state = BACKSL;
      break;
   case '\042':                /* Double quote     */
      state = D_QUOTE;
      break;
   case '\047':                /* Single quote     */
      state = S_QUOTE;
      break;
   default:
      break;
   }

   *out_pt++ = *s;

   while (*++s)
      switch(*s)
      {
      case SPACE:
      case TAB:
         switch(state)
         {
         case NEUTRAL:
            state=SKIP;
            *out_pt++ = SPACE;
            break;
         case S_QUOTE:
         case D_QUOTE:
            *out_pt++ = *s;
            break;
         case BACKSL:
            *out_pt++ = *s;
            state = NEUTRAL;
            break;
         case SKIP:
            break;
         }
         break;
      case '\047':                /* Single quote     */
         switch(state)
         {
         case NEUTRAL:
         case SKIP:
            state=S_QUOTE;
            *out_pt++ = *s;
            break;
         case S_QUOTE:
            state=NEUTRAL;
            *out_pt++ = *s;
            break;
         case BACKSL:
           *out_pt++ = *s;
           break;
         case D_QUOTE:
           *out_pt++ = *s;
            break;
         }
         break;
      case '\042':                /* Double quote     */
         switch(state)
         {
         case NEUTRAL:
         case SKIP:
            state=D_QUOTE;
            *out_pt++ = *s;
            break;
         case S_QUOTE:
           *out_pt++ = *s;
            break;
         case BACKSL:
           *out_pt++ = *s;
           break;
         case D_QUOTE:
            state=NEUTRAL;
            *out_pt++ = *s;
            break;
         }
         break;
/*  Backspace interpretation not needed anymore -- no fs files.
      case '\010':
         switch(state)
         {
         case NEUTRAL:
         case SKIP:
            if ((*(s-1)!=TAB && *(s-1)!=SPACE) ||
                (*(s-2)!=TAB && *(s-2)!=SPACE) )
               *--out_pt = null;
            if (*(out_pt-1)!=TAB && *(out_pt-1)!=SPACE) state=NEUTRAL;
            break;
         case S_QUOTE:
         case D_QUOTE:
            *out_pt++ = *s;
            break;
         case BACKSL:
            *out_pt++ = *s;
            state = NEUTRAL;
            break;
         }
         break;
*/
      case '\\':
         switch(state)
         {
         case BACKSL:
            *out_pt++ = *s;
            state = NEUTRAL;
            break;
         case NEUTRAL:
         case SKIP:
            *out_pt++ = *s;
            state = BACKSL;
            break;
         case S_QUOTE:
         case D_QUOTE:
            *out_pt++ = *s;
            break;
         }
         break;
      default:
         if (state==SKIP || state==BACKSL) state=NEUTRAL;
         *out_pt++ = *s;
         break;
      }

   *out_pt-- = null;
   while (*out_pt==SPACE || *out_pt==TAB) *out_pt-- = null;
}


int isset(f_pt,s_pt)
register struct field *f_pt;
register struct segment *s_pt;
{  register int test;
   register struct fix *p0_pt, *p1_pt;
   register char *c_pt;

   if (f_pt==NULL || s_pt==NULL) return(0);

   switch(f_pt->type)
   {
      case 7:
         /* Never considered as set when empty */
         if (s_pt->word==NULL || *(s_pt->word)==null) 
            return(0);

         /* Definitely (not) set if explicitly mapped onto null (not-null) */
         test=0;
         for (p0_pt=f_pt->first_cpo_pt; 
            test==0 && p0_pt!=NULL; p0_pt=p0_pt->next) {
            p1_pt = p0_pt->next;
            if ((c_pt=p0_pt->name)!=NULL && equal(s_pt->word,c_pt)) 
            {  
               test = 1;
               if ((c_pt = p1_pt->name)==NULL || *c_pt==null) 
                  return(0);
               else if ((c_pt = p1_pt->name)!=NULL && *c_pt!=null) 
                  return(1);
            }
            p0_pt = p1_pt;
         }

         /* Not explicitly mapped ; check if it should have been */
         if (f_pt->first_cpr_pt!=NULL && *(f_pt->first_cpr_pt->name+3)=='0')
            return(1);
         else
            return(0);
         break;
      case 6:  /* Check whether string is NOT mapped onto null string */
         p1_pt = f_pt->first_cpr_pt;
         for (p0_pt=f_pt->first_pr_pt; p0_pt!=NULL; p0_pt=p0_pt->next)
         {
            if (s_pt->word!=NULL &&
                  equal(s_pt->word,p0_pt->name) && *(p1_pt->name)) 
               return(1);
            p1_pt = p1_pt->next;
         }
         return(0);
         break;
      case 4:
         if (s_pt->word==NULL) return(0);
         return(equaln(s_pt->word,"y",1) ||  equaln(s_pt->word,"Y",1));
         break;
      case 3:
         if (s_pt->word==NULL) return(0);
         return(s_pt->word!=NULL && diff(s_pt->word,f_pt->def_arg));
         break;
      case 5:
      case 2:
      case 1:
         return(s_pt->word!=NULL && *(s_pt->word)!=null);
         break;
      default:
         return(0);
         break;
   }
   return(0);  /* Should never get here */
}


/*
 * copies quoted "interior" of s into v.  E.g., if s is
 * "'"xxx"'" then v will be xxx.
 */
rmquotes(s,v)
char *s, *v;
{  register char *c_pt=s, *v_pt=v, *c_last, *c0_last;
   register int slflag = 0, i;

   /* blank out v */
   for (i=0; i<128; i++) v[i] = null;
   v_pt = v;

   if (s == NULL) return(0);

   if (strlen(s)<2)
   {
      *v = *s;
      return(0);
   }

   /* Point c_pt and c_last to first and last character of "interior".
      Backslash-escaped quotes are not counted */
   c_last = s + strlen(s) - 1;
   while (*c_pt == *c_last && (*c_pt == '\'' || *c_pt == '\"'))
   {
      slflag = 0;
      if (c_last>c_pt)
         for (c0_last=c_last-1; c0_last > c_pt && *c0_last =='\\'; 
              c0_last--) 
            slflag = 1-slflag;
      if (!slflag)
      {
         c_last--;
         c_pt++;
      }
      else break;
   }

   /* if interior of s is equal to s*/
   if (c_pt==s)
   {
      strcpy(v,s);
      return(0);
   }

   /* copy interior of s onto v */
   for (;c_pt<=c_last;c_pt++)
         *v_pt++ = *c_pt;

   *v_pt = null;
}


   
char *restore(f_pt)
register struct field *f_pt;
{  register struct fix *p0_pt, *p1_pt;
   register char *c_pt;

   switch(f_pt->type)
   {
   case 7:
      for (p0_pt=f_pt->first_cpo_pt; p0_pt!=NULL; p0_pt=p0_pt->next) {
            p1_pt = p0_pt->next;
            if ((c_pt = p1_pt->name)==NULL || *c_pt==null) break;
            p0_pt = p1_pt;
      }
      if (p0_pt==NULL) return(NULL);
      else return(p0_pt->name);
      break;
   case 6:  /* Check whether string is NOT mapped onto null string */
      for (p0_pt=f_pt->first_pr_pt; p0_pt!=NULL; p0_pt=p0_pt->next)
      {
         p1_pt = p0_pt->next;
         if ((c_pt = p1_pt->name)==NULL || *c_pt==null) break;
         p0_pt = p1_pt;
      }
      if (p0_pt==NULL) return(NULL);
      else return(p0_pt->name);
      break;
   case 3:
      return(f_pt->def_arg);
      break;
   default:
      return(NULL);
      break;
   }
   return(NULL); /* Should never get here */
}

int mchdir(s)
char *s;
{  register char **b;
   char *cdpath;
   int test = 0, L;
   struct stat buf;

   shexp(s);
   
   if (stat(s,&buf)==0 && buf.st_mode&S_IFDIR && 
      access(s,01)==0)
      return(chdir(s));

   L = strlen(s) + 3;

   for (b=cdpaths; *b && test==0; b++)
   {
      cdpath = (char*)calloc((unsigned)(strlen(*b)+L),sizeof(char));
      if (**b!=null)
         sprintf(cdpath,"%s/%s",*b,s);
      else (strcpy(cdpath,s));
      if (stat(cdpath,&buf)==0 && buf.st_mode & S_IFDIR
          && access(cdpath,01)==0)
         return(chdir(cdpath));
   }
   return(-1);
}


int pop_exit()
{  int c;
   register int test=0;   /* Flag whether exit message must be given */
   register char **e_pt;
   register struct segment *s_pt;
   int help();

   if (exit_mess==NULL || *exit_mess==null) return(1);
   if (exit_field == -1)
       /* First case: no exit field given; then always
          give exit message */
      test=1;
   else
   {
       /* Second case: field is blank: then either there are no
          exit strings or the null exit string is the first
          acceptable one: */
       if ((s_pt=(fields+exit_field)->first_s_pt) == NULL ||
          s_pt->word == NULL || *(s_pt->word) == null)
       {
          for (e_pt = exit_strs; *e_pt != NULL; e_pt++)
             if (**e_pt == null) test=1;  /* check is null is str */
          if (*exit_strs==NULL) test=1;  /* also msg if no strs at all */
       }
       else
       /* Third case: field is not blank: then it must match
          one of the exit strings: */
       {
          for (e_pt = exit_strs; *e_pt != NULL; e_pt++)
             if (equal(s_pt->word,*e_pt))
                test=1;
       }
   }
   
   if (test==0) return(1);

   help(exit_mess,Std_row);
   refresh();

   REV;
   mvprintw(LINES-1,23,
    " TYPE %s TO CONFIRM, %s TO CONTINUE WITH COMMAND FORM ",f[1],f[2]);
   NREV; 
   move(Std_row,Std_col); 
   refresh();
   if ((c=getch()) == CTRL(G) || c==KEY_F(1))
   {
      move(LINES-1,0); clrtoeol();
      return(1);
   }
   else 
   {
      move(LINES-1,0); clrtoeol();
      return(0);
   }
}

int help(help_text,help_row)           /* 0: help message empty */
                                         /* 1: fits in dynamic window */
                                         /* 2: does not fit in dynamic 
                                               window but fits in screen */
char *help_text;
int help_row;
{  
   register int c;
   int flag=1, tflag=0;
   int A_flag=0;
   register int i, j, lines, maxlength, row0, row1, col0, col1;
   register char *c_pt = help_text;

   lines = maxlength = j = 0 ;

   if (help_text!=NULL)
   {
      while (*c_pt)
      {
         if (*c_pt!=CTRL(A) && *c_pt!=CTRL(B)) j++;
         if (*c_pt++ == '\n')
         {
            if (maxlength<j) maxlength=j;
            j = 0;
            lines++;
         }
      }
      if (maxlength<j) maxlength=j;
   }

   if (maxlength<=5 || help_text==NULL)
   {
      strcpy(error_mess," NO HELP AVAILABLE; ");
      return(0);
   }

   col0 = 1;
   if (maxlength<=75)
      col1 = col0 + maxlength + 2;
   else
   {
      maxlength = 75;
      col1 = col0 + 77;
   }
   if (help_row == -1)
   {
      row0 = 2;
      flag=2;
   }
   else
   {

      if (lines+2 > help_row)              /* No room above   */
      {
         if (help_row+lines+3 > SCRLINES)  /* No room beneath */
         {
            if (lines < LINES-5)        /* Fits on total screen */
            {
               flag = 2;
               row0 = 0;
            }
            else                           /* Truncate */
            {
               lines = LINES-6;
               tflag = 1;
               flag = 2;
               row0 = 0;
            }
         }
         else                              /* Fits beneath    */
            row0 = help_row+1;
      }
      else                                 /* Fits above      */
      {
         if (help_row+lines+3 > SCRLINES)  /* No room beneath */
            row0 = help_row - lines - 2;  
         else 
            row0 = help_row - lines - 2;
      }
   }

   row1 = row0 + lines + 1;

   for (i=row0;i<=row1;i++)
   {
      move(i,j=col0-1);
      for (;j<=col1+1;j++) addch(SPACE);
   }

   ALT;
   mvaddch(row0,col0,TL);
   mvaddch(row0,col1,TR);
   mvaddch(row1,col0,BL);
   mvaddch(row1,col1,BR);
   for (i=row0+1;i<=row1-1;i++)
   {
      mvaddch(i,col0,VER);
      mvaddch(i,col1,VER);
   }
   move(row0,j=col0+1);
   for (;j<=col1-1;j++) addch(HOR);
   move(row1,j=col0+1);
   for (;j<=col1-1;j++) addch(HOR);
   NALT;
   move(i=row0+1,col0+2);
   j = 0;
   if (tflag) 
      strcpy(error_mess," MESSAGE TRUNCATED; ");
   while ((c = *(help_text++)) != null)
   {
      switch(c)
      {
      case '\n':
         move(++i,col0+2);
         if (i>row1-1 && *help_text!=null)
            return(3);
         j = 0;
         break;
      case CTRL(A):
         A_flag=1;
         REV;
         break;
      case CTRL(B):
         A_flag = 1-A_flag;
         if (A_flag) {REV} else {NREV};
         break;
      default:
         if (j++ < maxlength) addch(c);
         break;
      }
   }

   return(flag);
}


VOID shexp(s)
char *s;
{  char *s1, *c_pt;
   int done();
   register FILE *fp0;
   int k;

   for (c_pt=s;*c_pt;c_pt++)
      if (*c_pt=='*' || *c_pt=='$' || *c_pt=='[' || *c_pt==']' || *c_pt=='?')
         break;

   if (*c_pt==null) return;

   s1 = (char*)calloc((unsigned)(strlen(mecho)+strlen(s)+5),sizeof(char));
   sprintf(s1,"%s %s",mecho,s);

   PR_POPEN;
   fp0 = popen(s1,"r");   c_pt = s;
   while ((k=getc(fp0))!=EOF) *c_pt++ = k;   *--c_pt = null;    pclose(fp0);
   PO_POPEN;
}
