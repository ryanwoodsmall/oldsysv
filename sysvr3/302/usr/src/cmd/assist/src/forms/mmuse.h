/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:mmuse.h	1.4"

#define MENUFIELDS      10  /*  Number of popup menu items                        */
#define ESIZE          256  /* Used for regular expression validation             */
extern struct field menus[MENUFIELDS], *last_menu;
extern struct segment arrow, arrow_buff;
extern int m_length, m_lines; /* line width and #lines in popup menu */
extern char   expbuf[ESIZE];
extern char   stripes[b_COL];
extern char   command[2048];
extern char   *top_help;
extern char   *top_help0;
extern char   *menu_help;
extern char   *cf_help;
extern char   *menu_help0;
extern char   *cf_help0;
extern char   *pu_help;
extern char   *pu_help0;
extern char   *pu_menu_help;
extern char     bottom_str[512];           /* String produced by bottom_write()          */
extern char   f[9][3];                     /* Function key labels                        */

extern char   musebin[100];   /* muse[]bin/ is default; $MUSEBIN overrides      */
extern char   forms[100];     /* muselib[]forms/ is default; $FORMS overrides   */
extern char   refs[100];      /* muselib[]refs/ is default; $REFS overrides     */
extern char   tutorials[100]; /* muselib[]tutorials/ is default; $TUTORIALS 
                                 overrides                                      */
extern char   mecho[100];     /* musebin[]mecho                                 */
extern char   *cdpaths[50];
