/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:mmusedefs.h	1.2"

/* mmuse.h -- header file that contains declarations and definitions
             for mforms only */
#define MENUFIELDS      10  /*  Number of popup menu items         */
#define ESIZE          256  /* Used for regular expression valfunc */
struct field menus[MENUFIELDS], *last_menu; /* popup menu storage */
struct segment arrow, arrow_buff;   /* ">" pointing char */
int m_length, m_lines; /* line width and #lines in popup menu */
char   expbuf[ESIZE];   /* Used in regular expression valfunc */
char   stripes[b_COL];  /* Storage for horizontal lines */
char   command[2048];   /* Generated and executed command line */
char   *top_help;   /* Used, but ignored */
char   *top_help0; /* Used, but ignored */
char   *menu_help; /* f8 help for menus */
char   *cf_help;   /* f8 help for command forms */
char   *menu_help0; /* ^A help for menus */
char   *cf_help0;   /* ^A help for command forms */
char   *pu_help;  /* f8 help for popup menu */
char   *pu_help0;  /* ^A help for popup menu */
char   *pu_menu_help;  /* Menu help for popup menu */
char   bottom_str[512];       /* String produced by bottom_write()          */
char   f[9][3];               /* Function key labels                        */

/* Various paths */
char   musebin[100];   /* muse[]bin/ is default; $MUSEBIN overrides      */
char   forms[100];     /* muselib[]forms/ is default; $FORMS overrides   */
char   refs[100];      /* muselib[]refs/ is default; $REFS overrides     */
char   tutorials[100]; /* muselib[]tutorials/ is default; $TUTORIALS 
                          overrides                                      */
char   mecho[100];     /* musebin[]mecho                                 */
char   *cdpaths[50];   /* $CDPATH parsed into separate strings */
