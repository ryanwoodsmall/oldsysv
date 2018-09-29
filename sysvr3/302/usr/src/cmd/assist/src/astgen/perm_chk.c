/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:perm_chk.c	1.7"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"
#define F_T 0
#define F_T_M 1
#define RD 2
#define RD_M 3
#define WR 4
#define WR_M 5
#define EX 6
#define EX_M 7

char *phelp="perm.help"; /*help message file*/
int pfields = 8;
static int cur_p = 0; /*current menu item*/


struct labels pnames[] = {
	{0,4,12," File type:"},
	{0,5,10," Message:"},
	{0,7,37," User should have 'read' permission:"},
	{0,8,10," Message:"},
	{0,10,39," User should have 'write' permission:"},
	{0,11,10," Message:"},
	{0,13,41," User should have 'execute' permission:"},
	{0,14,10," Message:"},
};

VOID pprompt(f_pt,locp)	/*display prompt that is appropriate for cur field*/
struct field *f_pt;
char *locp;
{
	switch(cur_p) {
		case F_T:	
		case RD:
		case WR:
		case EX: show_cmd("for next value",17);
			break;
		case F_T_M:
		case RD_M:
		case WR_M:
		case EX_M: if (*(locp+cur_p) == '0')
				show_cmd("to change to 'warning'",17);
			else show_cmd("to change to 'error'",17);
			break;
		default:
			break;
	} /*switch*/
	clrtoeol();
	standend();
	pdisplay(f_pt,locp);
	hilight(pnames[cur_p].ylab,pnames[cur_p].xlab,pnames[cur_p].name);
	refresh();
} /*prompt*/
			
set_perms(f_pt,v_pt)
struct field *f_pt;
struct vfunc *v_pt;
{
	int tmp,c;

	clear();
	if (v_pt == (struct vfunc*)0) return(0);

	if (v_pt->pr_pt == (struct fix *)0)
		{
		v_pt->pr_pt = (struct fix *)malloc(sizeof(struct fix));
		v_pt->pr_pt->next = (struct fix *)0;
		v_pt->pr_pt->name = (char *)calloc(10,sizeof(char));
		strcpy(v_pt->pr_pt->name,"10101010");/*default values*/
		}
	pprompt(f_pt,v_pt->pr_pt->name); /*display appropriate prompt*/
	leaveok(stdscr,0);
	while (c=getch())
		{
		err_rpt(0,FALSE);	/*clear any previous error message*/
		switch (c) {
		case KEY_F(1):
		case CTRL(g):
			switch(cur_p) {
			case F_T: 
			case RD:
			case WR:
			case EX:
				switch (*(v_pt->pr_pt->name+cur_p))	{
				case '0':	*(v_pt->pr_pt->name+cur_p) = '1';
						break;
				case '1':	*(v_pt->pr_pt->name+cur_p) = '2';
						break;
				case '2':	*(v_pt->pr_pt->name+cur_p) = '0';
						break;
				default:	break;
				} /*switch*/
				break;
			case F_T_M:
			case RD_M:
			case WR_M:
			case EX_M:
				if (*(v_pt->pr_pt->name+cur_p) == '0')
					*(v_pt->pr_pt->name+cur_p) = '1';
				else *(v_pt->pr_pt->name+cur_p) = '0';
				break;
			default:
				break;
			}; /*switch*/
			break;
		case KEY_F(6):
		case CTRL(y):
				fhelp(phelp,cur_p,2,4);
				err_rpt(0,FALSE);
			break;
		case KEY_DOWN:
		case '\n': /*new line*/
		case '\r': /*return*/
			mvaddstr(pnames[cur_p].ylab,pnames[cur_p].xlab,
			pnames[cur_p].name);
			cur_p = nextvar(pfields,cur_p);
			break;
		case KEY_UP:
		case CTRL(p):
			mvaddstr(pnames[cur_p].ylab,pnames[cur_p].xlab,
			pnames[cur_p].name);
			cur_p = prevvar(pfields,cur_p);
			break;
		case KEY_F(3):
		case CTRL(l):
			REDRAW;
			break;
		case KEY_F(5):
		case CTRL(t):
			clear();
			return(0);	/*user wants to go to top*/
		case KEY_F(4):
		case CTRL(r):
			clear();
			return(1); /*user wants new field*/
		default:
			if ((tmp=firstlet(pnames,pfields,c,cur_p)) == -1)
				flushinp();
			else 
				{
				mvaddstr(pnames[cur_p].ylab,pnames[cur_p].xlab,
				pnames[cur_p].name);
				cur_p = tmp;
				}
			break;
		} /*switch*/
	pprompt(f_pt,v_pt->pr_pt->name); /*show proper prompt*/
	} /*while*/
} /*procedure*/


pdisplay(f_pt,locp)
struct field *f_pt;
char *locp;
{
	int i;

	show_caption(f_pt,"File Types and Permissions");
	for (i = 0; i < pfields; i++)
		{
		mvaddstr(pnames[i].ylab,pnames[i].xlab,pnames[i].name);
		clrtoeol();
		}
	move(pnames[F_T].ylab,pnames[F_T].xval);
	switch(*locp)	{
		case '0':	addstr("directory");
			break;
		case '1':	addstr("file or directory");
			break;
		default:	addstr("file");
		} /*switch*/
	clrtoeol();
	for (i=1;i<pfields;i+=2)
		if (*(locp+i) == '0')
			mvaddstr(pnames[i].ylab,pnames[i].xval,"error");
		else mvaddstr(pnames[i].ylab,pnames[i].xval,"warning");
	clrtoeol();
	for (i=2;i<pfields;i+=2)
		{
		switch(*(locp+i))	{
		case '0':	mvaddstr(pnames[i].ylab,pnames[i].xval,"yes");
			break;
		case '1':	mvaddstr(pnames[i].ylab,pnames[i].xval,"doesn't matter");
			break;
		default:	mvaddstr(pnames[i].ylab,pnames[i].xval,"no");
		} /*switch*/
		} /*for*/
	clrtoeol();
	refresh();
}
