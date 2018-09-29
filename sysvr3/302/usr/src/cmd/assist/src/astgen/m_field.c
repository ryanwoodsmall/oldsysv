/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:m_field.c	1.18"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"
#define I_H_M 0
#define ACT 1
#define NAME 2
#define	NUM_MLINES	3

char *action; /*temporary store for action.  need to add ^X as needed*/
char *uprompt=" UNIX System Command:"; /*put in mfld, depending on state*/
char *mprompt=" ASSIST Screen Name: ";
char *mhelpfile="m_f.help"; /*help message file*/
static int cur_mf = 0; /*current menu item number*/
int unix_action; /*when true, action is a unix command*/


struct labels mfld[] = {
	{0,4,20," Item Help Message"},
	{0,6,22," Action on Selection:"},
	{0,8,22," ASSIST Screen Name: "},
};

VOID set_act(f_pt) /*figures out whether action is a screen or command*/
struct field *f_pt;
{
	char *action_pt;

	unix_action = FALSE; /*default*/
	if ((action_pt = f_pt->action) != NULL && *action_pt) /*not empty*/
		{
		action = (char*)calloc((unsigned)(strlen(action_pt)+2),sizeof(char));
		if (*action_pt != CTRL(X)) /*^X is an assist screen*/
			{
			mfld[2].name = uprompt; /*use unix label*/
			unix_action = TRUE;
			}
		else 	/*must be an assist screen name*/
			{
			action_pt++;	/*step past ^X, rest is screen name*/
			mfld[2].name = mprompt; /*use assist label*/
			unix_action = FALSE;
			}
		strcpy(action,action_pt);	/*action used to display*/
		}
	else 		/*no action defined, so assume assist screen*/
		{
                action = &nullchar;	/*null out action*/
		mfld[2].name = mprompt;
		unix_action = FALSE;
		}
}

VOID mfprompt()	/*display prompt that is appropriate for cur field*/
{

	switch(cur_mf) {
		case ACT: if (unix_action == FALSE)
				show_cmd("change to 'UNIX system command'",2);
			 else show_cmd("change to 'ASSIST screen'",2);
			 break;
		case NAME: show_cmd("",23);
			 break;
		case I_H_M: show_cmd("edit help message",2);
			break;
		}; /*switch*/
	hilight(mfld[cur_mf].ylab,mfld[cur_mf].xlab,mfld[cur_mf].name);
	refresh();
}; /*prompt*/
			
VOID mfig_help(f_pt)	/*count help lines*/
struct field *f_pt;
{
	int cnt = 0;
	register char *c_pt = f_pt->help;

	move(mfld[0].ylab,mfld[0].xval);
	if (f_pt->help !=NULL)
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

VOID cleanup(f_pt)	/*called to put back ^X, if needed*/
struct field *f_pt;
{
	f_pt->action = (char*)calloc((unsigned)(strlen(action)+3),sizeof(char));

	if (unix_action == FALSE) {	/*assist screen, so need ^X*/
		*f_pt->action = CTRL(X);	/*add it*/
		strcat(f_pt->action+1,action);
	}
        else	/*must be unix command, so don't need to add ^X*/
      		strcpy(f_pt->action,action);
}

mvedit(f_pt)	/*where the action is*/
register struct field *f_pt;
{
	char *editstr();
	struct charstr *askstring();
	int tmp,c;

	set_act(f_pt);	/*figure out type of action*/
	mredisplay(f_pt);
	mfprompt(); /*display context-sensitive prompt*/
	while (c=getch())	/*forever.  loop exited below*/
		{
		err_rpt(0,FALSE);	/*clear any previous error message*/
		switch (c) {
		case KEY_F(4):
		case CTRL(r):	/*user wants new field*/
			cleanup(f_pt);	/*add ^X to action, if needed*/
			return(1); /*allow new field to be selected*/
		case KEY_F(5):
		case CTRL(t):	/*user wants top menu*/
			cleanup(f_pt);	/*add ^X to action, if needed*/
			return(0); /*all the way back*/
		case KEY_F(7):
		case CTRL(e):	if (cur_mf == NAME)
					{
					action=editstr(action);
					mredisplay(f_pt);
					}
				else
					{
					err_rpt(5,TRUE);
					/*refresh();*/
					}
			break;
		case KEY_F(1):
		case CTRL(g):
			switch(cur_mf)
			{
				case ACT:
					switch_act(); /*toggle action*/
					mredisplay(f_pt);
					break;
				case NAME:
					mvaddstr(mfld[NAME].ylab,mfld[NAME].xval,"<empty>");
					clrtoeol();
					show_cmd("",8);
					refresh();
					action =
			askstring("",mfld[NAME].ylab,mfld[NAME].xval,79,cur_mf,mhelpfile,1)->str;
					break;
				case I_H_M:
					f_pt->help=editstr(f_pt->help);
					while ((tmp=helpchk(f_pt->help)) != 0)
						{
						clear();
						mredisplay(f_pt);
						hilight(mfld[cur_mf].ylab,mfld[cur_mf].xlab,mfld[cur_mf].name);
						if (tmp!=48) show_cmd("",16);
						else show_cmd("",25);
						err_rpt(tmp,TRUE); /*too long*/
						refresh();
						getch();	  /*or wide*/
						if (tmp==48) break;
						f_pt->help=editstr(f_pt->help);
						}
					if (tmp!=48) clear();
					mredisplay(f_pt);
					break;
				default:	/*should not get here*/
					break;
			}; /*switch*/
			break;
		case KEY_F(6):
		case CTRL(y): /*user wants help*/
			fhelp(mhelpfile,cur_mf,2,4);
			err_rpt(0,FALSE); /*clear*/
			break;
		case KEY_DOWN:
		case '\n': /*new line*/
		case '\r': /*return*/
			mvaddstr(mfld[cur_mf].ylab,mfld[cur_mf].xlab,
			mfld[cur_mf].name);
			cur_mf=nextvar(NUM_MLINES,cur_mf);
			break;
		case CTRL(p):
		case KEY_UP:
			mvaddstr(mfld[cur_mf].ylab,mfld[cur_mf].xlab,
			mfld[cur_mf].name);
			cur_mf=prevvar(NUM_MLINES,cur_mf);
			break;
		case KEY_F(3):
		case CTRL(l):	/*redraw screen*/
			REDRAW;
			break;
		default:
			if ((tmp=firstlet(mfld,NUM_MLINES,c,cur_mf)) == -1)
				flushinp();	/*no first letter match*/
			else 
				{
				mvaddstr(mfld[cur_mf].ylab,mfld[cur_mf].xlab,
				mfld[cur_mf].name);
				cur_mf = tmp; /*move to matched letter*/
				}
			break;
		} /*switch*/
	mfprompt(); /*show proper prompt*/
	} /*while*/
return(1); /*all ok*/
} /*procedure*/

VOID switch_act() /*toggle from unix command to assist screen*/
{

		if (unix_action == FALSE) /*currently assist screen*/
			{
			unix_action = TRUE; /*make unix command*/
			mfld[2].name = uprompt; /*use proper label*/
			}
		else 
			{
			unix_action = FALSE;
			mfld[2].name = mprompt;
			}
}

VOID mredisplay(f_pt)
register struct field *f_pt;
{
	int i;
	struct first_second *find_f_s();

	/*get current field from data structure*/
	mvaddstr(0,33,"Field Attributes");
	mvaddstr(1,0,"Current Field: "); /*need to combine segments*/
	printw("%s%s",find_f_s(f_pt)->first,find_f_s(f_pt)->second);
        draw_line(3);
        draw_line(LINES-3);
	mfig_help(f_pt);
	for (i=0; i < NUM_MLINES; i++)
		mvaddstr(mfld[i].ylab,mfld[i].xlab,mfld[i].name);
	if (unix_action == TRUE)
		mvaddstr(mfld[ACT].ylab,mfld[ACT].xval,"execute UNIX system command");
	else
		mvaddstr(mfld[ACT].ylab,mfld[ACT].xval,"'assist' menu or command form");
	clrtoeol();
	if (action != NULL)
		dctrlshow(mfld[NAME].ylab,mfld[NAME].xval,79,action);
	else mvaddstr(mfld[NAME].ylab,mfld[NAME].xval,"[undefined]");
	clrtoeol();
}
