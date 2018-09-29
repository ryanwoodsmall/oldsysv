/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:dhelp.c	1.8"

/*	this routine opens the specified file, looks for a line that
*	begins with the specified item number, and prints the following
*	lines, up to the next field.  It assumes that the file is found
*	in "muselib" (which has been set by set_path()
*	Starting x,y (upper left corner of box) is last two args
*/

#include "tools.h"
#include "../forms/muse.h"

fhelp(fname,num,x,y)
char *fname; /*pointer to file name*/
int x,y,num; /*item number*/
{
        register char *c_pt;
        register int c;
	WINDOW *hm;
	int i;
	int found = FALSE;
	int done = FALSE;
	char buf[128];
        char bbuf[2048];
	char fullname[128]; /*used to hold full pathname*/
	int xwin = 0; /*staring x loc*/
	int ywin = 0;
	int winsize; /*num lines */
        int maxl, len, winwidth;

	sprintf(fullname,"%s%s",muselib,fname);

	if ((fp = fopen(fullname,"r")) == NULL)
		{
		err_rpt(4,TRUE);
		return(-1); /*couldn't find/open help file*/
		}
	while (found == FALSE && fgets(buf,sizeof buf, fp) != NULL)
		{
		buf[strlen(buf)-1] = '\0';
		if (atoi(buf) == num)/*start printing next line*/
			found = TRUE;
		}
	if (found == FALSE) /*hit eof and didn't find item help*/
		{
		err_rpt(7,TRUE);
		return(-1); 
		}

                      /* Compute required window size  */
        winsize = 2;
        maxl = 0;
        c_pt = buf;
        *bbuf = null;
	while (done == FALSE && (c=getc(fp)) != EOF)
		{
                *c_pt++ = c;
                *c_pt = null;
                if (c == '\n') {
                   len = (int)(c_pt-buf);
                   if (maxl<len) maxl = len;
                   winsize++;
                   len = 0;
                   strcat(bbuf,buf);
                   *(c_pt = buf) = null;
                }
		if (atoi(buf) == num+1)/*thats all*/
			done = TRUE;
		}

        fclose(fp);
        winwidth = maxl+3;
	hm=newwin(winsize,winwidth,y,x);
        idlok(hm,TRUE);
        keypad(hm,TRUE);
        intrflush(hm,TRUE);
	if (term & ALTCHAR) wattron(hm,A_ALTCHARSET);
	for (i=xwin+1;i<winwidth-1;i++) mvwaddch(hm,ywin,i,HOR);
	for (i=xwin+1;i<winwidth-1;i++) mvwaddch(hm,winsize-1,i,HOR);
	for (i=ywin+1;i<winsize;i++)
		{
			mvwaddch(hm,i,xwin,VER);
			mvwaddch(hm,i,winwidth-1,VER);
		}
        mvwaddch(hm,ywin,xwin,TL);
        mvwaddch(hm,ywin,winwidth-1,TR);
        mvwaddch(hm,winsize-1,xwin,BL);
        mvwaddch(hm,winsize-1,winwidth-1,BR);
	if (term & ALTCHAR) wattroff(hm,A_ALTCHARSET);
        ywin+=1;
        wmove(hm,ywin++,xwin+2);
        c_pt = bbuf;
	while (*c_pt)
		{
		if (*c_pt =='\n') wmove(hm,ywin++,xwin+2);
                else waddch(hm,*c_pt);
                c_pt++;
		}
        wstandout(hm);
        mvwaddstr(hm,winsize-1,xwin+(winwidth-25)/2,
           " Hit any key to continue ");
        wstandend(hm);
	touchwin(hm);
	wrefresh(hm);
        flushinp();
	wgetch(hm);	/*wait until user is ready before clearing help*/
	touchwin(stdscr);
	return(0); /*all done*/
}
