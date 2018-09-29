/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:musedefs.h	1.13"

/* musedefs.h -- header file that contains declarations and definitions
             for both mforms and tforms */

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
#include   <fcntl.h>


#define STANDOUT   01
#define FKEYS      02
#define ALTCHAR    04

/* Macro for array overflow */
#define TOOMANY(u,x,y,z) {if (x>y+z) {status=u;done();};}

/* Macro used just before popen() calls. */
#define PR_POPEN         {signal(SIGQUIT,SIG_IGN);\
                         signal(SIGINT,SIG_IGN);\
                         signal(SIGALRM,SIG_IGN);\
                         signal(SIGTERM,SIG_IGN);\
                         freopen("/dev/null","w",stderr);}

/* Macro used just after popen() calls. */
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

#define NLEVELS         50  /* Maximum number of remembered levels                      */
#define NRETS            5  /* Maximum number of different correct return codes  (used, but superfluous) */
#define MAXFIELDS      100  /*                   fields in a form                 */
#define MAXSEGMS       128  /*                   user input strings in a form     */
#define MAXINDICES    2048  /*                   validation functions in a field  */
#define CAPSIZE         80  /*                   characters in a caption          */
#define NFIX          3072  /*  Maximum number of pre and post-fixes                      */
#define NUMVFUNCS      256  /*  Maximum number of validation functions                    */

#define b_COL          100  /* Max size of blank[] and stripes[] arrays           */
#define SMART            1  /* (not used) */

                            /* MODE FLAGS  (see fill_out.c to understand usage) */
#define NEWFIELD         0  /* "Neutral" mode I                        */
#define NEWLSEGMENT      1  /* New segment created left of leftmost segment */
#define NEWRSEGMENT      2  /* New segment created right of rightmost segment */
#define NEXTSEGMENT      3  /* (not used)                */
#define MENU             4  /* Mode set when program is used for menu             */
#define INSERT           5  /* Entered when ordinary character is typed in form    */
#define INCORRECT        6  /* Fatal error; user cannot leave field                                      */
#define WARN             7  /* Invalid entry                                      */
#define GRERROR          8  /* Non-Fatal error; user can leave field but not execute form                                        */
#define POPUP            9  /* Popup menu mode                                    */

/* Flags for next_screen() routine */
#define BACK             0  /* Backtrack */
#define TOP              1  /* Go to TOP Menu */
#define NEW              2  /* ENter new screen */

#define OFF              0
#define ON               1

/* F;ags for expand() and validate() routines */
#define NO_UPDATE        0
#define UPDATE           1

typedef int              VOID;

extern char *calloc(), *malloc();

/* Used for 2-digit field indices used for multi-field valfuncs */
struct index
{
   int num;
   struct index *next;
}  indices[MAXINDICES], *last_in_pt;

/* Used to store strings used by valfuncs and command line generation */
struct fix
{
   char *name;
   struct fix *next;
}  fixes[NFIX], *last_fi_pt;

/* Used to store selectables (menus) or user input (forms) */
struct segment
{
   int row;                 /* Absolute row of segment in formscr                */
   int col;                 /*          col of first character in formscr        */
   struct segment *next;
                            /* Pointer to next segment of field (NULL for first)   */
   struct segment *previous;
                            /* Pointer to previous segment of field (NULL for last)*/
   char *word;              /* Segment string                                      */
} *last_segm_pt, segments[MAXSEGMS];
                            /* Storage for all segments in a form                  */

/* Used to store validation functions */
struct vfunc
{
   int num;            /* Index of actual validation function         */
   struct vfunc *next; /* Pointer to next valfunc in current field    */
   struct fix *pr_pt;  /* Pointer to first prefix for valfunc         */
   struct fix *po_pt;  /* Pointer to first postfix for valfunc        */
} vfuncs[NUMVFUNCS], *last_v_pt;

/* The central data structure.  Has pointers to all of the above linked lists */
struct field
{
   int type;             /* 0: heading only;  not a continuation.
                            1 - 4: obsolete.
                            7: general, non-header field
                            9: first k-1 lines of a 
                                 k-line caption */
   int row;                        /* Row of field in formscr                    */
   int col;                        /* Column of first character of caption       */
   int last_row;                   /* Row of last segment   (see update())          */
   int group;                      /* (not used)*/
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
   struct fix *first_pr_pt;          /* Used in obsolete parts of source code          */
   struct fix *first_po_pt;          /* Used in obsolete parts of source codets to post-fixes such as ".a"          */
   struct fix *first_cpr_pt;         /* Points to command line pre-fixes           */
   struct fix *first_cpo_pt;         /* Points to command line post-fixes          */
   char *op_name;                    /* Obsolete */
   char *def_arg;                    /* Obsolete */
   int loc;                        /* 0:Relative location on command line */
   int bundle;                     /* 1 means that single-letter option can be
                                        bundled.                                   */
}  fields[MAXFIELDS], *last_field_pt;
                                     /* Storage for all fields in a form           */


                    /* A couple of self-explanatory declarations                   */
FILE   *fp, *fopen();
char comment[1024];   /* Storage of  #ident line */
int    rets[NRETS];   /* Used but ignored */
int idone();
char   error_mess[80], warn_mess[80];
char   init_cmd[200]; /* Storage of "COMMAND LINE: <cmd>" */
char   dum_cmd_name[50]; /* Storage for <cmd>" */
char   *form_help;
char   *exit_mess;

/* VARIABLES STARTING WITH CAPS CONTAIN CURRENT, GLOBAL VALUES                      */
int    Row;                       /* Current row coordinate of cursor in formscr*/
int    Col;                       /* Current col coordinate of cursor in formscr*/
struct segment *Segm_pt;            /* Pointer to current segment in current field*/
struct field *Field_pt;             /* Pointer to current field                   */
int    Stdscr_loc;               /* Location of first row of stdscr.
                                    NOTE: paging and positioning uses
                                    2 coordinate systems: that of the
                                    stdscr window and that of a non-existing
                                    window that displays the entire form
                                    or menu.  WHen you page, the stdscr
                                    is moved down relative to this
                                    hypothetical window.  The location of
                                    the top row of stdscr relative to
                                    this window is Stdscr_loc.  Since
                                    the paging involves the main body only,
                                    and the first two lines of the stdscr
                                    are not part of  the main body,
                                    Stdscr_loc = -2 + (page-1)*(SCRLINES-2) */

int      status;                    /* Exit status                                */
int    mode;                      /* NEWFIELD vs. NEWSEGMENT vs. INSERT         */
int    cmd_col;                   /* First column of generated command line               */
int    clt_code;                  /* 00 means command that the command line
                                        generation can be handled by ordinary
                                        means, 01-99 refers to special
                                        command() sub-functions.  Current
                                        special cases: ar, umask, chmod   */
                                        
int    SCRLINES;                  /* Number of lines in main body of form or menu       */
int    term;    /* Terminal identifier */

/* Used to store information for backtracking */
struct label
{
   char *files_name;
   int memory;
   char screen_name[50];
}  labels[NLEVELS], *lab_pt;


char   hor, ver, tl, tr, bl, br;    /* Terminal-dependent boxing characters       */
char   HOR, VER, TL, TR, BL, BR;  
int  page;                          /* Current page in form or menu.              */
int  maxpage;                       /* Total number of pages in form or menu.     */
char   showstring[256];             /* User input string as physically shown 
                                       on screen after control character 
                                       conversion */
char   *buffer, *buf_pt;       /* Central storage for all character input in
                                        segments, fields, options, and 
                                        form_help.  */
int exit_field;                     /* Number of field that has to be checked
                                       to decide whether exit message should 
                                       be given */
char *exit_strs[5], **exit_strs_pt; /* Strings against which input of field
                                       exit_field must be checked */
char   fs_forms[100];  /* Name of directory where .fs files are stored */
char   *binpaths[50];  /* $PATH parsed into separate strings */
int BUFSIZE;      /* Upper bound to number of chars needed to store fs file */
char muse[100], muselib[100];  /* Paths used for fs file directory */
struct termio termbuf;
int localflag;  /* 1 iff -c option is used */
