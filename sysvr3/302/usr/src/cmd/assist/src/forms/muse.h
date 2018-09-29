/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:muse.h	1.17"

#include   <stdio.h> 
#include   <curses.h>
#include   <ctype.h>
#include   <math.h>
#include   <search.h>
#include   <string.h>
#include   <signal.h>
#include   <sys/types.h>
#include   <sys/stat.h>
#include   <errno.h>
#include   <malloc.h>
#include   <fcntl.h>


#define STANDOUT   01
#define FKEYS      02
#define ALTCHAR    04

#define TOOMANY(u,x,y,z) {if (x>y+z) {status=u;done();};}
#define PR_POPEN         {signal(SIGQUIT,SIG_IGN);\
                         signal(SIGINT,SIG_IGN);\
                         signal(SIGALRM,SIG_IGN);\
                         signal(SIGTERM,SIG_IGN);\
                         freopen("/dev/null","w",stderr);}
#define PO_POPEN         {signal(SIGQUIT,idone);\
                         signal(SIGINT,idone);\
                         signal(SIGALRM,SIG_IGN);\
                         signal(SIGTERM,idone);\
                         freopen("/dev/tty","w",stderr);}
#define equal(a,b)     (strcmp(a,b)==0)
#define equaln(a,b,n)  (strncmp(a,b,n)==0)
#define diff(a,b)      (strcmp(a,b)!=0)
#define diffn(a,b,n)   (strncmp(a,b,n)!=0)
#define REV            attrset(A_STANDOUT);
#define NREV           attroff(A_STANDOUT);
#define ALT            if (term & ALTCHAR) attrset(A_ALTCHARSET);
#define NALT           if (term & ALTCHAR) attroff(A_ALTCHARSET);
#define CTRL(c)        ('c' & 037)
#define null           '\0'
#define TAB            '\011'
#define SPACE          ' '
#define Std_row        Row - Stdscr_loc      /* Current row of cursor in stdscr */
#define Std_col        Col                   /* Current col of cursor in stdscr */

#define NLEVELS         50  /* Maximum number of menu levels                      */
#define NRETS            5  /* Maximum number of different correct return codes   */
#define MAXFIELDS      100  /*                   fields in a form                 */
#define MAXSEGMS       128  /*                   user input strings in a form     */
#define MAXINDICES    2048  /*                   validation functions in a field  */
#define CAPSIZE         80  /*                   characters in a caption          */
#define NFIX          3072  /*  Number of pre and post-fixes                      */
#define NUMVFUNCS      256  /*  Number of validation functions                    */

#define b_COL          100  /* Max size of blank[] and stripes[] arrays           */
#define SMART            1

                            /* MODE FLAGS                                         */
#define NEWFIELD         0  /* Enter from start, ^P, or ^I                        */
#define NEWLSEGMENT      1  /* Enter from ' ' or left/right arrows                */
#define NEWRSEGMENT      2  /* Enter from ' ' or left/right arrows                */
#define NEXTSEGMENT      3  /* Enter from ' ' or left/right arrows                */
#define MENU             4  /* Mode set when program is used for menu             */
#define INSERT           5  /* Enter from first new character added to segment    */
#define INCORRECT        6  /* Invalid entry                                      */
#define WARN             7  /* Invalid entry                                      */
#define GRERROR          8  /* Group error                                        */
#define POPUP            9  /* Popup menu mode                                    */

#define BACK             0
#define TOP              1
#define NEW              2

#define OFF              0
#define ON               1

#define NO_UPDATE        0
#define UPDATE           1

typedef int              VOID;

extern char *calloc(), *malloc();

struct index
{
   int num;
   struct index *next;
};
extern struct index indices[MAXINDICES], *last_in_pt;

struct fix
{
   char *name;
   struct fix *next;
};
extern struct fix fixes[NFIX], *last_fi_pt;

struct segment
{
   int row;                 /* Absolute row of segment in formscr                */
   int col;                 /*          col of first character in formscr        */
   struct segment *next;
                            /* Pointer to next segment of field (NULL for first)   */
   struct segment *previous;
                            /* Pointer to previous segment of field (NULL for last)*/
   char *word;              /* Segment string                                      */
};
extern struct segment *last_segm_pt, segments[MAXSEGMS];
                            /* Storage for all segments in a form                  */

struct vfunc
{
   int num;            /* Index of actual validation function         */
   struct vfunc *next; /* Pointer to next valfunc in current field    */
   struct fix *pr_pt;  /* Pointer to first prefix for valfunc         */
   struct fix *po_pt;  /* Pointer to first postfix for valfunc        */
};
extern struct vfunc vfuncs[NUMVFUNCS], *last_v_pt;

struct field
{
   int type;                       /* 0: heading only; 
                                        1: strings, copied directly to command line;
                                        2: strings ==> option; 
                                        3: string!=def_arg ==> option;
                                        4: yes/no;                                 */
   int row;                        /* Row of field in formscr                    */
   int col;                        /* Column of first character of caption       */
   int last_row;                   /* Row of last segment                        */
   int group;                      /* Group label for "group"-type errors        */
   char *caption;                    /* Caption                                    */
   struct segment *first_s_pt;       /* Pointer to first segment                   */
   int minfsegms;                  /* Typically 0, 1 or MAXFSEGMS                */
   int maxfsegms;                  /* Typically 1 or MAXFSEGMS                   */
   struct vfunc  *first_v_pt;        /* Pointer to first validation function       */
   struct index  *first_i_pt;        /* Pointer to first incompat. field           */
   struct index  *first_m_pt;        /* Pointer to first condit. mandat. field     */
   struct index  *first_a_pt;        /* Pointer to first at-least-one field        */
   char *help;                       /* Five lines of 40 char help info            */
   char *action;                     /* Action taken when menu selection is made   */
                                     /* If action[] is empty, program looks for
                                        _segm_pt->word.files.                      */
   struct fix *first_pr_pt;          /* Points to post-fixes such as ".a"          */
   struct fix *first_po_pt;          /* Points to post-fixes such as ".a"          */
   struct fix *first_cpr_pt;         /* Points to command line pre-fixes           */
   struct fix *first_cpo_pt;         /* Points to command line post-fixes          */
   char *op_name;                    /* Option name, including delimiter character */
   char *def_arg;                    /* Default arg., to detect if option is absent*/
   int loc;                        /* 0:First group                              */
   int bundle;                     /* 1 means that single-letter option can be
                                        bundled.                                   */
};
extern struct field fields[MAXFIELDS], *last_field_pt;
                                     /* Storage for all fields in a form           */


                    /* A couple of self-explanatory declarations                   */
extern FILE   *fp, *fopen();
extern int    rets[NRETS];
extern char comment[1024];
extern int idone();
extern char   error_mess[80], warn_mess[80];
extern char   init_cmd[200];
extern char   dum_cmd_name[50];
extern char   *form_help;
extern char   *exit_mess;

/* VARIABLES STARTING WITH "_" CONTAIN CURRENT, GLOBAL VALUES                      */
extern int    Row;                       /* Current row coordinate of cursor in formscr*/
extern int    Col;                       /* Current col coordinate of cursor in formscr*/
extern struct segment *Segm_pt;            /* Pointer to current segment in current field*/
extern struct field *Field_pt;             /* Pointer to current field                   */
extern int    Stdscr_loc;                /* Location of first row of stdscr in formscr */

extern int      status;                    /* Exit status                                */
extern int    mode;                      /* NEWFIELD vs. NEWSEGMENT vs. INSERT         */
extern int    cmd_col;                   /* First column of command line               */
extern int    clt_code;                  /* 00 means command that can be handled
                                        by ordinary means, 01-99 refers to special
                                        command() sub-functions.                   */
extern int    SCRLINES;                  /* Number of lines in scrollable region       */
extern int    term;

struct label
{
   char *files_name;
   int memory;
   char screen_name[50];
};
extern struct label labels[NLEVELS], *lab_pt;


extern char   hor, ver, tl, tr, bl, br;    /* Terminal-dependent boxing characters       */
extern char   HOR, VER, TL, TR, BL, BR;  
extern int  page;                          /* Current page in form or menu.              */
extern int  maxpage;                       /* Total number of pages in form or menu.     */
extern char   showstring[256];             /* String as phsyically shown on screen 
                                       after control character conversion */
extern char   *buffer, *buf_pt;       /* Central storage for all character input in
                                        segments, fields, options, and form_help.
                                        Not user in muse.c.                        */
extern int exit_field;                     /* Number of field that has to be checked
                                       to decide whether exit message should be
                                       given */
extern char *exit_strs[5], **exit_strs_pt; /* Strings against which input of field
                                       exit_field must be checked */
extern char   fs_forms[100];  /* Typically==lmforms, but=="" w/ -c option       */
extern char   *binpaths[50];
extern char muse[100], muselib[100];
extern int BUFSIZE;
extern struct termio termbuf;
