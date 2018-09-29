/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:m_global.c	1.23"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"
#define S_N 0 /*screen name*/
#define H_M 1 /*form help message*/

char *mghelp="m_g.help"; /*help message file*/
static int cur_mf = 0; /*current item in menu*/

struct labels gmenu[] = {
	{0,5,14," Screen Name:"},
	{0,7,19," Form Help Message"},
};

VOID mgprompt()	/*display prompt that is appropriate for cur field*/
{
	if (cur_mf == 0)
		show_cmd("",21);	/*special prompt, since ^E ok*/
	else show_cmd("edit help message",22);
	hilight(gmenu[cur_mf].ylab,gmenu[cur_mf].xlab,gmenu[cur_mf].name);
	refresh();
} /*prompt*/
			
mglobal()
{
	int tmp,c;
	char *editstr(), *c_pt;
	struct charstr *askstring(); 

	clear();
	mglredisp();
	mgprompt(); /*display appropriate prompt*/
	while (c=getch()) /*forever, exit from loop is handled below*/
		{
		err_rpt(0,FALSE);	/*clear any previous error message*/
		switch (c) {
		case KEY_F(7):
		case CTRL(e):
			if (cur_mf == S_N)
				{
				c_pt = editstr(lab_pt->screen_name);
				while ((tmp=chckstr(c_pt,0)) != 0)
					{	
					move(gmenu[S_N].ylab,gmenu[S_N].xval);
					clrtoeol();
					dctrlshow(gmenu[S_N].ylab,
					gmenu[S_N].xval,79,lab_pt->screen_name);
					show_cmd("",16);
					err_rpt(tmp,TRUE); 
					refresh();
					getch();
					err_rpt(0,FALSE);
					c_pt = editstr(c_pt);
					}
				strcpy(lab_pt->screen_name,c_pt);
					/* Truncate to 17 chars */
				if (strlen(lab_pt->screen_name)>17)
					{
					*(lab_pt->screen_name+17) = null;
					err_rpt(50,TRUE);
					}
				}
			else
				{
				err_rpt(5,TRUE);
				refresh();
				}
			move(gmenu[S_N].ylab,gmenu[S_N].xval);
			clrtoeol();
			dctrlshow(gmenu[S_N].ylab,
			gmenu[S_N].xval,79,lab_pt->screen_name);
			refresh();
			break;
		case KEY_F(1):
		case CTRL(g):
			if(cur_mf==S_N)  /*screen name*/
				{
				mvaddstr(gmenu[S_N].ylab,gmenu[S_N].xval,"<empty>");
				clrtoeol();
				show_cmd("",8); /*insert mode prompts*/
				refresh();
				strcpy(lab_pt->screen_name,
				askstring("",gmenu[S_N].ylab,gmenu[S_N].xval,
					79,cur_mf,mghelp,1)->str);
				while ((tmp=chckstr(lab_pt->screen_name,0))!=0)
					{
					move(gmenu[S_N].ylab,gmenu[S_N].xval);
					clrtoeol();
					dctrlshow(gmenu[S_N].ylab,
					gmenu[S_N].xval,79,lab_pt->screen_name);
					show_cmd("",25);
					err_rpt(tmp,TRUE);
					refresh();
					getch();
					err_rpt(0,FALSE);
					show_cmd("",8);
					strcpy(lab_pt->screen_name,
					askstring("",gmenu[S_N].ylab,gmenu[S_N].xval,
					79,cur_mf,mghelp,1)->str);
					}
					/* Truncate to 17 chars */
				if (strlen(lab_pt->screen_name)>17)
					{
					*(lab_pt->screen_name+17) = null;
					err_rpt(50,TRUE);
					}
				move(gmenu[S_N].ylab,gmenu[S_N].xval);
				clrtoeol();
				dctrlshow(gmenu[S_N].ylab,
				gmenu[S_N].xval,79,lab_pt->screen_name);
				refresh();
				}
			else 	/*must be on form help item*/
				{
				form_help = editstr(form_help);
				while ((tmp=helpchk(form_help)) != 0)
					{
					clear();
					mglredisp();
					hilight(gmenu[cur_mf].ylab,gmenu[cur_mf].xlab,gmenu[cur_mf].name);
					show_cmd("",16);
					err_rpt(tmp,TRUE); /*too long or wide*/
					refresh();
					getch();
					form_help = editstr(form_help);
					}
				clear();
				mglredisp();
				}
			break;
		case KEY_F(6):
		case CTRL(y): /*user wants help*/
			fhelp(mghelp,cur_mf,2,4);
			err_rpt(0,FALSE); /*clear prompt*/
			break;
		case KEY_DOWN:
		case '\n': /*new line*/
		case '\r': /*return*/
			mvaddstr(gmenu[cur_mf].ylab,gmenu[cur_mf].xlab,
			gmenu[cur_mf].name);
			cur_mf = nextvar(2,cur_mf);
			break;
		case CTRL(p):
		case KEY_UP:
			mvaddstr(gmenu[cur_mf].ylab,gmenu[cur_mf].xlab,
			gmenu[cur_mf].name);
			cur_mf = prevvar(2,cur_mf);
			break;
		case KEY_F(5):
		case CTRL(t):
			return(0);	/*user wants to go to top*/
		case KEY_F(4):
		case CTRL(r):
			return(1); /*user wants to select new field*/
		case KEY_F(3):
		case CTRL(l): /*redraw screen*/
			REDRAW;
			break;
		default:
			if ((tmp=firstlet(gmenu,2,c,cur_mf)) == -1)
				flushinp(); /*no first letter match*/
			else 
				{
				mvaddstr(gmenu[cur_mf].ylab,gmenu[cur_mf].xlab,
				gmenu[cur_mf].name);
				cur_mf = tmp; /*go to matched letter*/
				}
			break;
		} /*switch*/
	mgprompt(); /*show proper prompt*/
	} /*while*/
} /*procedure*/


VOID mglredisp()
{
	int i;

	mvaddstr(0,31,"Global Information");
	draw_line(3);
        draw_line(LINES-3);
	for (i = 0; i <= 1; i++)
		{
		mvaddstr(gmenu[i].ylab,gmenu[i].xlab,gmenu[i].name);
		clrtoeol();
		}
	dctrlshow(gmenu[S_N].ylab,gmenu[S_N].xval,79,lab_pt->screen_name);
	clrtoeol();
	fig_mhelp_mess(); /*show number of help message lines*/
}

VOID fig_mhelp_mess() /*count number of help message lines*/
{
	int cnt = 0;
	register char *c_pt = form_help;

	move(gmenu[H_M].ylab,gmenu[H_M].xval);
	if (form_help !=NULL)
		{
		if (*(c_pt+1) == null)
			addstr("[0 lines specified]");
		else
			{
			while (*c_pt)
				if (*c_pt++ == '\n')
					cnt++;
			cnt++;
			if (cnt != 1)
				printw("[%d lines specified]",cnt);
			else addstr("[1 line specified]");
			}
		}
	else
		addstr("[0 lines specified]");
	clrtoeol();
}
