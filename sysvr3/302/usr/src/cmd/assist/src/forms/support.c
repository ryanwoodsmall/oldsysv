/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:support.c	1.32"
#include "muse.h"

extern int localflag;

char *visible[] = {
   "<null>",
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
   (char*)0
};


/* EXIT FUNCTION                                                                   */

int done()
{
   char err_mess[128], *file_name;
   char nc;

   nc = null;
   *err_mess = null;
   file_name= &nc;

   switch(status)   
   {
      case 0:
         sprintf(err_mess,"YOU HAVE EXITED ASSIST");
         break;
      case 1:
         printf("Usage: muse [-c] [screenname]\n");
         exit(1);
         break;
      case 2:
         file_name = lab_pt->files_name;
         strcpy(err_mess,": Not implemented.  ");
         break;
      case 3:
         strcpy(err_mess,"Bad ASSIST file");
         break;
      case 4:
         strcpy(err_mess,"Too many characters.  ");
         break;
      case 5:
         strcpy(err_mess,"Too many strings.  ");
         break;
      case 6:
         strcpy(err_mess,"Too many pre- or post-fixes.  ");
         break;
      case 7:
         strcpy(err_mess,"Too many indices.  ");
         break;
      case 8:
         strcpy(err_mess,"Too many fields.  ");
      case 9:
         strcpy(err_mess,"Generated command line too long.  ");
         break;
      case 10:
         strcpy(err_mess,"Help message too many characters.  ");
         break;
      case 11:
         strcpy(err_mess,"Incorrect shellscript delimiters.  ");
         break;
      case 12:
         printf("YOU HAVE EXITED ASSIST\n");
         exit(12);
         break;
      case 13:
         printf("ASSIST FILE NAME TOO LONG\n");
         exit(13);
         break;
      case 14:
         printf("ASSIST FILE NAME CANNOT CONTAIN BACKSLASHES\n");
         exit(14);
         break;
      case 15:
         strcpy(err_mess,"^Y-enclosed script produces too much output.  ");
         break;
      case 16:
         strcpy(err_mess,"^Y-enclosed script produces bad output.  ");
         break;
      case 17:
         strcpy(err_mess,"You need at least 24 lines on your terminal\n");
         break;
      case 18:
         strcpy(err_mess,"You need at least 80 columns on your terminal\n");
         break;
      default:
         break;
   }
   move(LINES-1,0);
   refresh();
   endwin();
   updatetty(&termbuf);
   printf("\n%s%s\n",file_name,err_mess);
   exit(status);
}



int implement(word,flag)  
char *word;
int flag;
{  
   register int c;
   register int test=0;
   register char **b, *c_pt, *other_b;
   char s[100], dumstr[100];
   struct stat buf;
   register FILE *fp0;
   register int other_loc = 0;
   char *cwd, *getcwd();

   if (word==NULL || *(word)==null)
   {
      strcpy(error_mess," THERE ARE NO COMMAND FORMS FOR \"MORE\" COMMANDS ");
      warn_mess[0] = null;
      return(0);
   }
   sprintf(dumstr,"%s%s.fs",fs_forms,word);
   if (stat(dumstr,&buf)==0)   /* fs file exists */
   {
      fp0 = fopen(dumstr,"r");
      if (fp0==NULL)         /* fs file cannot be opened for reading */
      {
         strcpy(error_mess," ASSIST FILE EXISTS BUT CANNOT BE OPENED ");
         warn_mess[0] = null;
         fclose(fp0);
         return(0);
      }

      while ((c=getc(fp0))!=CTRL(E) && c!=EOF);
      if (c==EOF)            /* bad fs file */
      {
         strcpy(error_mess," ASSIST FILE EXISTS BUT IS CORRUPTED ");
         warn_mess[0] = null;
         return(0);
      }
      c = getc(fp0);   /* Get command form vs. menu flag */
      if (c!='4')      /* Not a menu. Check whether command in $PATH */
      {
         while ('0' <= (c=getc(fp0)) && c <= '9');
         c_pt=dumstr;
         *c_pt++ = c;
         while ((c=getc(fp0))!=SPACE && c!='\n') *c_pt++ = c;
         *c_pt = null;
         fclose(fp0);
         for (c_pt=dumstr; *c_pt && *c_pt==' '; c_pt++);
         if (built_in(c_pt)) return(1);
         if (*c_pt == '/') return(1);
         test = 0;
         cwd = getcwd(NULL,512);
         for (b=binpaths; *b && test==0; b++)
         {
            if (**b!=null)
               sprintf(s,"%s/%s",*b,c_pt);
            else (strcpy(s,c_pt));

            if (access(s,01)==0)  
            {
               if (strcmp(*b,"/usr/bin")==0 || strcmp(*b,"/bin")==0
                   || (**b==null && strcmp(cwd,"/usr/bin")==0) 
                   || (**b==null && strcmp(cwd,"/bin")==0) )
               {
                  if (other_loc) test=2;
                  else test = 1;
               }
               else 
               {
                  other_loc = 1; 
                  if (**b==null) other_b = ".";
                  else other_b = *b;
               }
            }
         }

         if (other_loc && test==0) test = 1;

         switch(test)
         {
         case 0: /* Not in PATH */
            if (strlen(c_pt) <= 14)
               sprintf(error_mess,
               "ASSIST SUPPORTS %s; EXECUTABLE CMD %s NOT IN YOUR $PATH",c_pt,c_pt);
            else
               sprintf(error_mess,
               " SUPPORTED BY ASSIST, BUT NOT FOUND IN YOUR $PATH ");
            warn_mess[0] = null;
            return(0);
            break;
         case 1:
            return(1);
            break;
         case 2: /* In PATH, but in some directory 
                              preceding correct directory */
            if (2*strlen(c_pt) +strlen(other_b) <= 45)
               sprintf(error_mess,
               "%s/%s: %s CMD NOT IN EXPECTED DIRECTORY",other_b,c_pt,c_pt);
            else
               sprintf(error_mess,
               " CMD NOT IN EXPECTED DIRECTORY ");
            warn_mess[0] = null;
            return(0);
            break;
         default:
            return(0);
            break;
         }
      }

      fclose(fp0);
      return(1);   /* Was menu; no PATH checking needed */
   }

   else /* Not supported by ASSIST */
   {
      if (strlen(word)<30)
      {
         if (!localflag)
            sprintf(error_mess,
             "%s NOT SUPPORTED BY ASSIST ",word);
         else
            sprintf(error_mess,
             "%s.fs NOT ACCESSIBLE ",word);
      }
      else
      {
         if (!localflag)
            sprintf(error_mess,
             " NOT SUPPORTED BY ASSIST ");
         else
            sprintf(error_mess,
             "fs FILE NOT ACCESSIBLE ");
      }
      return(0);
   }
}



VOID copy_form_std(top_row)
int top_row;
{  
   register int i, row;
   register struct field *f_pt;
   register struct segment *s_pt;

   for (i=top_row;i<=SCRLINES;i++)
   {
      move(i,0);
      clrtoeol();
   }
   if (mode==MENU)
   for (;i<LINES;i++)
   {
      move(i,0);
      clrtoeol();
   }
   
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      row = f_pt->row-Stdscr_loc;
      if (row>top_row && row<=SCRLINES)
         mvaddstr(row,f_pt->col,f_pt->caption);
      if (f_pt->first_s_pt)
      {
         for (s_pt=f_pt->first_s_pt ; s_pt!=NULL ; s_pt=s_pt->next)
         {
            row = s_pt->row-Stdscr_loc;
            if (row>top_row && row<=SCRLINES && s_pt->word!=NULL)
               show(s_pt);
         }
      }
   }
}





dectrl(s)
register char *s;
{  register char *t_pt=showstring, *v;
   while (*s)
   {
      if ((int)*s<=27)
      {
         v = visible[(int)*s];
         while (*v) *t_pt++ = *v++;
      }
      else
         *t_pt++ = *s;
      s++;
   }
   *t_pt=null;
}


int length(s)
register char *s;
{  register int kount=0;
   while (*s)
   {
      if (*s<=27)
         kount+= strlen(visible[*s]);
      else kount++;
      s++;
   }
   return(kount);
}


VOID show(s_pt)
struct segment *s_pt;
{  register char *c_pt;
   register int j;
   move(s_pt->row-Stdscr_loc,s_pt->col);
   dectrl(s_pt->word);
   for (c_pt=showstring, j=s_pt->col; *c_pt && j<COLS-1; j++, c_pt++)
      addch(*c_pt);
}


idone()
{  int i;
   for (i=1; i<18; i++) signal(i,SIG_IGN);
   if (LINES>1) {
      attrset(0);
      (void) move(LINES-1,0);    /* to lower left corner */
      clrtoeol();                /* clear bottom line */
      (void) refresh();		/* flush out everything */
   }
   endwin();			/* curses cleanup */
   updatetty(&termbuf);
   exit(0);
}

char *built_ins[] = {
  "break",
  "cd",
  "continue",
  "eval",
  "exec",
  "exit",
  "export",
  "hash",
  "read",
  "readonly",
  "return",
  "set",
  "shift",
  "test",
  "times",
  "trap",
  "type",
  "ulimit",
  "umask",
  "unset",
  "wait",
  (char*)0
};

int built_in(s)
char *s;
{  char **b;
   for (b=built_ins; *b!=(char*)0; b++)
      if (equal(s,*b)) return(1);
   return(0);
}

updatetty(tb)
struct termio *tb;
{
   ioctl(fileno(stdin),TCSETAW,tb);
}

catchtty(tb)
struct termio *tb;
{
   ioctl(fileno(stdin),TCGETA,tb);
}


/* Routine to be placed in read_in.c that closes gaps in
 * the locations.
 */

close_gaps()
{  struct field *f_pt, *f0_pt;
   int locs[MAXFIELDS];

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++)  {
      if (f_pt->bundle!=1)  {
         f_pt->loc++;
         locs[(int)(f_pt-fields)] = 1;
      }
      else   {
         f_pt->loc = 0;
         locs[(int)(f_pt-fields)] = 0;
      }
   }

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++)
   for (f0_pt=fields; f0_pt<=last_field_pt; f0_pt++) 
      if (f_pt!=f0_pt && f_pt->loc>f0_pt->loc && f0_pt->bundle!=1) 
         locs[(int)(f_pt-fields)]++;

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++) 
      f_pt->loc = locs[(int)(f_pt-fields)];
}
