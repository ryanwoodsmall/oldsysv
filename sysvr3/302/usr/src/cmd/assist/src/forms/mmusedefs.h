/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:mmusedefs.h	1.1"

#define MENUFIELDS      10  /*  Number of popup menu items                        */
#define ESIZE          256  /* Used for regular expression validation             */
struct field menus[MENUFIELDS], *last_menu;
struct segment arrow, arrow_buff;
int m_length, m_lines; /* line width and #lines in popup menu */
char   expbuf[ESIZE];
char   stripes[b_COL];
char   command[2048];
char   *top_help;
char   *top_help0;
char   *menu_help;
char   *cf_help;
char   *menu_help0;
char   *cf_help0;
char   *pu_help;
char   *pu_help0;
char   *pu_menu_help;
char     bottom_str[512];           /* String produced by bottom_write()          */
char   f[9][3];                     /* Function key labels                        */

char   musebin[100];   /* muse[]bin/ is default; $MUSEBIN overrides      */
char   forms[100];     /* muselib[]forms/ is default; $FORMS overrides   */
char   refs[100];      /* muselib[]refs/ is default; $REFS overrides     */
char   tutorials[100]; /* muselib[]tutorials/ is default; $TUTORIALS 
                          overrides                                      */
char   mecho[100];     /* musebin[]mecho                                 */
char   *cdpaths[50];
