/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:control.c	1.33"

/*
 * control.c is the main() routine of vforms -- command form
 * generation tool for ASSIST.
 *
 * Usage: vforms [<command or menu name>]
 *
 * It performs the following steps:
 *
 * 1. Initialization.
 * 2. If argc==2: read <name>.fs file into datastructure.
 *    If argc==1: prompt user for type (menu vs. form) and <name>
 * 3. Create temp file to be used for template.  The same temp
 *    file is used throughout the program.
 * 4. If argc==1, dump user into vi/toolong/copy_file_ds cycle.
 * 5. Enter the main switch:
 *    e -- edit: copy_form_file: copy datastructure into temp file.
 *               layout(): vi temp file
 *                         check filecontents w/ toolong; 
 *                         copy_file_form: file to datastructure
 *                         prompt(0), reorder: clean up datastructure
 *    g -- global: change/add global attributes: exit mess,
 *                 form/menu help.
 *    q -- quit: check first if wrote into file
 *    s -- select new field; call vedit() to change attributes
 *    w -- write into file
 *
 */

#include "../forms/musedefs.h"
#include "tools.h"

#define MINFS(X) {if (strlen(X)>3 && strcmp(X+strlen(X)-3,".fs")==0)\
                  *(X+strlen(X)-3) = null;}

#define   ADD         0
#define   DEL         1

#define   MINFSEGMS   0
#define   MAXFSEGMS   1
#define   HELP        2
#define   OP_NAME     3
#define   DEFAULT     4
#define   ACTION      5
#define   BUNDLE      6
#define   LOC         7
#define   SHELL_EXP   8
#define   REDRAW      {clearok(curscr,TRUE); wrefresh(curscr);}

char editor[16];
char fthelp[128];
char tf_name[128];

struct fieldlink {
   struct field *f_pt;
   struct fieldlink *next;
};

struct two_index {
   struct index *pr_pt;
   struct index *po_pt;
};

char nullchar;

char *calloc(), *malloc();
struct charstr {
   char *str;
   char ch;
};

struct first_second {
   char first[1024];
   char second[1024];
};


#define WRITE_OUT       {PR_POPEN;\
                         if (mode!=MENU) write_out(tf_pt);\
                         else mwrite_out(tf_pt);\
                         PO_POPEN;}


int vdone();
int tdone();
VOID read_in();
int t_cur;

struct labels t_lab [] = {
   {10,5,10," Edit layout"},
   {10,6,10," Select field"},
   {10,7,10," Global info"},
   {10,8,10," Try ASSIST"},
   {10,9,10," Write"},
   {10,10,10," Quit"},
}; /*t_lab*/

main(argc,argv)
int argc;
char *argv[];
{  
   register int rsp, tmp, i;
   char *thelpfile = "top.help";
   register char *c_pt;
   char menuinfo[128], cfinfo[128];
   char *getenv();
   int ask();
   struct charstr *askstring();
   extern int helpnum;
   char mforms[128];
   FILE *tf_pt;
   char s[256], edstr[256];
   struct field *f_pt;
   struct fieldlink  *fl, *select();
   int writeflag = 1;
   int new = 0;
   struct stat buf;

   signal(SIGINT,SIG_IGN);
   signal(SIGQUIT,SIG_IGN);
   signal(SIGALRM,SIG_IGN);
   signal(SIGTERM,SIG_IGN);

   if (argc!=2) {
      fprintf(stderr,"Usage: astgen screen-name\n");
      exit(1);
   }

   if (access(".",02) != 0) {
      fprintf(stderr,"Current directory does not have write permission\n");
      exit(1);
   }


   if (strcmp(argv[1],".fs") == 0) {
      fprintf(stderr,"ASSIST file name %s not allowed \n",argv[1]);
      exit(1);
   }

   MINFS(argv[1]);

   c_pt = argv[1] + strlen(argv[1]) - 1;
   while (c_pt>argv[1] && *c_pt!= '/') c_pt--;
   if (*c_pt=='/') c_pt++;
   
   if (strlen(c_pt) > 11) {
      fprintf(stderr,"%s has more than 11 characters\n",c_pt);
      exit(1);
   }
   
   if (strlen(c_pt) == 0) {
      fprintf(stderr,"Filename has zero length\n");
      exit(1);
   }
   
   if (strlen(argv[1]) > 128) {
      fprintf(stderr,"%s has more than 128 characters\n",argv[1]);
      exit(1);
   }

   sprintf(s,"%s.fs",argv[1]);
   if (access(s,00)==0) {
      if (access(s,04)!=0) {
         fprintf(stderr,"%s has no read permission \n",s);
         exit(1);
      }
      else if (access(s,02)!=0) {
         fprintf(stderr,"%s has no write permission \n",s);
         exit(1);
      }
      stat(s,&buf);
      if (buf.st_mode & S_IFDIR) {
         fprintf(stderr,"%s is a directory \n",s);
         exit(1);
      }
      new = 0;
   }
   else new = 1;

   lab_pt = labels;
   lab_pt->memory = 0;
   lab_pt->files_name = (char*)calloc((unsigned)(strlen(argv[1])+2),
                                       sizeof(char));
   strcpy(lab_pt->files_name,argv[1]);
   strcpy(lab_pt->screen_name,c_pt); /* c_pt shorter than 11 chars */

   get_path();

   if (strlen(muselib)>100) {
      fprintf(stderr,"%s has more than 100 characters \n",muselib);
      exit(1);
   }
   sprintf(menuinfo,"%smenuinfo",muselib);
   sprintf(cfinfo,"%scfinfo",muselib);

   strcpy(fthelp,"ftype.help");

   if ((c_pt=getenv("EDITOR"))!=(char*)0 && *c_pt && strlen(c_pt)<16)
      strcpy(editor,c_pt);
   else strcpy(editor,"vi");

   if (!isinpath(editor)) {
      fprintf(stderr,"%s is not in your path or is not executable \n",editor);
      exit(1);
   }

   termstuff();

   nullchar = null;

   catchtty(&termbuf);

   initscr();
   nonl();
   cbreak();
   noecho();
   idlok(stdscr,FALSE);
   keypad(stdscr,TRUE);
   intrflush(stdscr,TRUE);
   signal(SIGINT,tdone);
   signal(SIGQUIT,tdone);
   signal(SIGTERM,tdone);

   buf_pt = buffer = (char*)0;

   clear();

   if (new==0) {
      /* Needed to hook up right with read_in.c */
      last_field_pt = fields+MAXFIELDS-1;
      last_segm_pt = segments+MAXSEGMS-1;
      last_fi_pt = fixes+NFIX-1;
      last_in_pt = indices+MAXINDICES-1;
      last_v_pt = vfuncs+NUMVFUNCS-1;
      read_in(0);   /* Read fs file into data structure */
   }
   else if (new==1) {
      last_field_pt = fields;
      last_segm_pt = segments;
      last_fi_pt = fixes;
      last_in_pt = indices;
      last_v_pt = vfuncs;
      exit_field = -1;
      form_help = exit_mess = (char*)0;
      maxpage = page = 1;

      helpnum = 20;
AA:   
      switch(ask("Are you creating a menu or a command form (m/c):",1,1,"mc",helpnum,fthelp)) {
      case 'm':
         addstr("enu");
         mode=4;
         break;
      case 'c':
         addstr("ommand form");
         mode=0;
         break;
      default:
         beep();
         goto AA;
         break;
      }
   }

   /*
 * Create and open temp file to contain template.
 */
   c_pt = tmpnam((char*)0);
   if (c_pt==(char*)0) {
      status = 0;
      vdone();
   }
   if (strlen(c_pt)>128 || strlen(c_pt) + strlen(editor) > 256) {
      status = 1;
      vdone();
   }
   strcpy(tf_name,c_pt);
   sprintf(edstr,"%s %s",editor,tf_name);

   /*
 * If creating a new file, MUST first design layout.
 */

   if (new==1) {
      writeflag=0;
      if (mode==4) {
         mvprintw(3,1, "You have chosen to create a new menu.");
         mvprintw(5,1, "The layout of the menu is designed");
      }
      else {
         mvprintw(3,1, "You have chosen to create a new command form.");
         mvprintw(5,1, "The layout of the command form is designed");
      }

      if (strlen(editor)<40)
         mvprintw(6,1, "in a temporary file with an editor, %s.",editor);
      else
         mvprintw(6,1, "in a temporary file with an editor.");

      helpnum = 21;
BB:   
      switch(ask(
          "Do you want this file to contain instructions and a sample layout (y/n):",
          8,1,"yn",helpnum,fthelp)) {
      case 'y':
         addstr("es");
         move(LINES-1,0);
         refresh();
         if (mode==MENU) intro(tf_name,menuinfo);
         else intro(tf_name,cfinfo);
         break;
      case 'n':
         addstr("o");
         move(LINES-1,0);
         refresh();
         if ((tf_pt = fopen(tf_name,"w")) == NULL) {
            status = 2;
            vdone();
         }
         putc('\n',tf_pt);
         fclose(tf_pt);
         break;
      default:
         beep();
         goto BB;
         break;
      }
      clear();
      move(0,0);
      refresh();
      layout(tf_name,1,edstr);
      i = 1;
      for (f_pt = fields; f_pt<=last_field_pt; f_pt++)
         if (f_pt->type==7) f_pt->loc= i++;
   }

   clear();
   helpnum = 22;

   t_cur = 0;
   display_menu();
   storeline(0,LINES-3);
   for (;;)
   {
      rsp = getch();
      err_rpt(0,FALSE);
      switch(rsp) {
      case KEY_F(3):
      case CTRL(l):
         REDRAW;
         break;
      case CTRL(M):
      case CTRL(N):
      case CTRL(J):
      case KEY_DOWN:
         mvaddstr(t_lab[t_cur].ylab,t_lab[t_cur].xlab,t_lab[t_cur].name);
         t_cur = nextvar(6,t_cur);
         break;
      case CTRL(P):
      case KEY_UP:
         mvaddstr(t_lab[t_cur].ylab,t_lab[t_cur].xlab,t_lab[t_cur].name);
         t_cur = prevvar(6,t_cur);
         break;
      case CTRL(Y):
      case KEY_F(6):
         fhelp(thelpfile,t_cur,1,10);
         err_rpt(0,0);
         break;
      case CTRL(g):
      case KEY_F(1):
         switch(t_cur) {
         case 0:
            clear();
            move(0,0);
            refresh();
            copy_form_file(tf_name);
            layout(tf_name,2,edstr);
            writeflag=0;
            display_menu();
            break;
         case 3:
            if ((i=chkwrite())!=0) {
               err_rpt(i,TRUE);
               break;
            }
            if (getenv("ASSISTBIN")==(char*)0) {
               err_rpt(1,TRUE);
               break;
            }
            if (strlen(c_pt = getenv("ASSISTBIN"))>75) {
               err_rpt(1,TRUE);
               break;
            }
            sprintf(mforms,"%s/mforms",c_pt);
            if (access(mforms,04)!=0) {
               err_rpt(2,TRUE);
               break;
            }
            strcat(mforms," -c temp");
            if ((tf_pt = fopen("temp.fs","w"))==0) {
               err_rpt(9,TRUE);
               break;
            }
            WRITE_OUT;
            fclose(tf_pt);
            if (access("temp.fs",02)!=0) {
               err_rpt(10,TRUE);
               break;
            }
            clear();
            refresh();
            saveterm(); 
            resetterm();
            updatetty(&termbuf);
            system(mforms);
            catchtty(&termbuf);
            fixterm();
            unlink("temp.fs");
            mvaddstr(LINES-1,0," Type any key to continue with astgen.");
            refresh();
            getch();
            display_menu();
            break;
         case 2:
            if (mode == MENU)
               mglobal();
            else global();
            writeflag=0;
            display_menu();
            break;
         case 5:
            if (writeflag==0) {
               helpnum = 22;
DD:
               switch(ask(
                   "Write into file before quitting (y/n)?",
                   LINES-5,1,"yn",helpnum,fthelp)) {
               case 'y':
                  if ((i=chkwrite())==0) {
                     sprintf(s,"%s.fs",lab_pt->files_name);
                     tf_pt = fopen(s,"w");
                     WRITE_OUT;
                     if (strlen(s)>65) {
                        *(s+60) = null;
                        strcat(s," ...");
                     }
                     mvprintw(LINES-5,1,"Written to %s\n",s);
                     fclose(tf_pt);
                  }
                  else {
                     err_rpt(i,TRUE);
                     break;
                  }
                  break;
               case 'n':
                  break;
               default:
                  goto DD;
                  break;
               }

            }
            unlink(tf_name);
            refresh();
            endwin();
            updatetty(&termbuf);
            printf("\n");
            exit(1);
            break;
         case 1:
            fl = select((struct fieldlink *)0,
                " Select Field Whose Attributes You Want to Set ",0);
            if (mode == MENU)
            {
               while (fl!=(struct fieldlink *)0 && (f_pt=fl->f_pt)!=(struct field *)0 && 
                   mvedit(f_pt)) {
                  fl = select((struct fieldlink *)0,
                      " Select Field whose Attributes You Want to Set ",0);
                  writeflag=0;
               }
            }
            else
            {
               while (fl !=(struct fieldlink *)0 && (f_pt=fl->f_pt)!=(struct field *)0 && 
                   vedit(f_pt))
               {
                  fl = select((struct fieldlink *)0,
                      " Select Field Whose Attributes You Want To Set ",0);
                  writeflag=0;
               }
            }
            display_menu();
            writeflag=0;
            break;
         case 4:
            writeflag=1;
            sprintf(s,"%s.fs",lab_pt->files_name);
            if ((tf_pt = fopen(s,"w")) != NULL) {
               if ((i=chkwrite())==0)  {
                  WRITE_OUT;
                  if (strlen(s)>65) {
                     *(s+60) = null;
                     strcat(s," ...");
                  }
                  mvprintw(LINES-5,1,"Written to %s",s);
               }
               else {
                  err_rpt(i,TRUE);
                  break;
               }
               fclose(tf_pt);
            }
            else
               mvprintw(LINES-5,1,"Could not open %s",s);
            break;
         default:
            beep();
         } /*switch t_cur*/
         break; /*for f1 switch*/
      default:
         if ((tmp=firstlet(t_lab,6,rsp,t_cur)) == -1)
            flushinp();
         else 
         {
            mvaddstr(t_lab[t_cur].ylab,t_lab[t_cur].xlab,t_lab[t_cur].name);
            t_cur = tmp;
         }
         break;
      } /*switch rsp*/
      REV;
      mvaddstr(t_lab[t_cur].ylab,t_lab[t_cur].xlab,t_lab[t_cur].name);
      mvaddch(t_lab[t_cur].ylab,t_lab[t_cur].xlab,'>');
      attrset(0);
      move(t_lab[t_cur].ylab,t_lab[t_cur].xlab);
      refresh();
   } /*for */
} /*main*/


/* EXIT FUNCTION                                                                   */

int vdone()
{
   char err_mess[128], file_name[128];

   *err_mess = null;
   *file_name= null;

   switch(status)
   {
   case 0:
      sprintf(err_mess,"Cannot create temp file\n");
      break;
   case 1:
      sprintf(err_mess,"Layout file name or $EDITOR too many characters\n");
      break;
   case 2:
      sprintf(err_mess,"Layout file does not have write permission\n");
      break;
   case 3:
      sprintf(err_mess,"Sample layout file does not have read permission\n");
      break;
   case 4:
      sprintf(err_mess,"Layout file does not have read permission\n");
      break;
   case 5:
      sprintf(err_mess,"Illegal data structure: field has empty caption and segment\n");
      break;
   case 6:
      sprintf(err_mess,"Layout file checker failure\n");
      break;
   case 7:
      sprintf(err_mess,"temp file does not have write permission\n");
      break;
   case 8:
      sprintf(err_mess,"temp file does not have write permission\n");
      break;
   default:
      sprintf(err_mess,"Fatal error\n");
      break;
   }
   move(LINES-1,0);
   attrset(0);
   refresh();
   endwin();
   updatetty(&termbuf);
   printf("\n%s%s\n",file_name,err_mess);
   exit(status);
}


intro(out_name,in_name)
register char *in_name, *out_name;
{  
   register FILE *in_pt, *out_pt;
   register int c;
   if ((in_pt=fopen(in_name,"r"))==NULL) {
      status = 3;
      vdone();
   }
   if ((out_pt=fopen(out_name,"w"))==NULL) {
      status = 2;
      vdone();
   }
   while ((c=getc(in_pt))!=EOF) putc(c,out_pt);
   fclose(in_pt);
   fclose(out_pt);
}


VOID display_menu()
{
   clear();
   mvaddstr(1,30,"TOP astgen MENU");
   mvaddstr(t_lab[0].ylab,t_lab[0].xlab,t_lab[0].name);
   addstr(" ........... specify or edit screen layout");
   mvaddstr(t_lab[1].ylab,t_lab[1].xlab,t_lab[1].name);
   addstr(" .......... set or change the attributes of a field");
   mvaddstr(t_lab[2].ylab,t_lab[2].xlab,t_lab[2].name);
   addstr(" ........... set or change screen-wide attributes");
   mvaddstr(t_lab[3].ylab,t_lab[3].xlab,t_lab[3].name);
   addstr(" ............ try ASSIST");
   mvaddstr(t_lab[4].ylab,t_lab[4].xlab,t_lab[4].name);
   addstr(" ................. write into the data file");
   mvaddstr(t_lab[5].ylab,t_lab[5].xlab,t_lab[5].name);
   addstr(" .................. terminate this session");
   REV;
   mvaddstr(t_lab[t_cur].ylab,t_lab[t_cur].xlab,t_lab[t_cur].name);
   mvaddch(t_lab[t_cur].ylab,t_lab[t_cur].xlab,'>');
   NREV;
   draw_line(3);
   draw_line(LINES-3);
   show_cmd("",0);
   move(t_lab[t_cur].ylab,t_lab[t_cur].xlab);
   refresh();
}

layout(tf_n,flag,s)
register char *tf_n;
register int flag;
register char *s;
{  
   register int i;
   register FILE *tf_pt;
   saveterm(); 
   resetterm();
   updatetty(&termbuf);

   prompt((struct field *)0,3);  /* Reset taken[] */

   if (access(tf_n,04) != 0) {
      fprintf(stderr,"Layout file has no read permission\n",s);
      exit(4);
   }

   do {
      if (system(s)==256) {
         fprintf(stderr,"Editor in \"%s\" not found\n",s);
         exit(9);
      }
   }   while (toolong(tf_n) == 0);

   catchtty(&termbuf);
   fixterm();

   i=0;
   do {
      clear();
      if ((tf_pt = fopen(tf_n,"r")) == NULL) {
         status = 4;
         vdone();
      }
      i=copy_file_form(tf_pt,flag);
      fclose(tf_pt);
   }  while (i==0);

   prompt((struct field *)0,flag);
   reorder();
}

