/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:cf_global.c	1.25"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"

#define NEVER	4	/*num fields with no exit message*/
#define	UNCOND	5	/*num fields with uncond exit message*/
#define	COND	7	/*num fields with cond exit message*/
#define C_N	0 /*command name*/
#define C_L_O	1 /*command line order*/
#define C_F_H	2 /*command form help*/
#define	W_E_M	3 /*want exit message?*/
#define E_M	4 /*exit message*/
#define C_F	5 /*conditional field*/
#define C_E_V	6 /*conditional values*/

char *ghelp="cf_g.help"; /*help message file*/
				/*need to handle paths later*/
int numfields;
static int cur_cfg = 0; /*current item number*/
static int gexit_vals = 0; /*num exit values*/
static int gexit_lines = 0; /*num exit message lines*/

struct labels gnames[] = {	/*used for displaying items*/
	{0,4,18," Name of Command:"},
	{0,6,26," Ordering of Command Line"},
	{0,8,27," Command Form Help Message"},
	{0,10,20," Give Exit Message:"},
	{0,12,15," Exit Message"},
	{0,14,20," Conditional Field:"},
	{0,16,26," Conditional Exit Values"},
};

VOID gprompt()	/*display prompt that is appropriate for cur field*/
{
	switch(cur_cfg) {
		case C_N:	show_cmd("",21);
			break;
		case C_L_O:	show_cmd("'ordering' module",22);
			break;
		case C_F_H:	show_cmd("edit help message",22);
			break;
		case W_E_M: 	show_cmd("next value",22);
				break;
		case E_M:	show_cmd("edit exit message",22);
			break;
		case C_F:	show_cmd("select new exit field",22);
			break;
		case C_E_V:	show_cmd("edit exit values",22);
			break;
		default:
			break;
	} /*switch*/
	hilight(gnames[cur_cfg].ylab,gnames[cur_cfg].xlab,gnames[cur_cfg].name);
	refresh();
} /*prompt*/
			
int verify_ex()	/*see if data structure is incomplete*/
{
	int c;

	glredisp();
	if (numfields==NEVER)
		exit_mess = NULL;
	if (numfields!=COND)
		exit_field = -1;
	if (numfields==COND && exit_field==-1) /*no exit field picked yet*/
		{
		err_rpt(37,TRUE); 
		show_cmd("",18);
		move(gnames[cur_cfg].ylab,gnames[cur_cfg].xlab);
		hilight(gnames[cur_cfg].ylab, gnames[cur_cfg].xlab,
		gnames[cur_cfg].name);
		refresh();
		if ((c=getch())==CTRL(r) ||
		c == KEY_F(4))
			return(0); /*top*/
		err_rpt(0,FALSE);
		return(1);
		}
	if (numfields != NEVER && gexit_lines == 0) /*no exit message*/
		{
		err_rpt(38,TRUE); 
		show_cmd("",18);
		move(gnames[cur_cfg].ylab,gnames[cur_cfg].xlab);
		hilight(gnames[cur_cfg].ylab, gnames[cur_cfg].xlab,
		gnames[cur_cfg].name);
		refresh();
		if ((c=getch())==CTRL(r) ||
		c == KEY_F(4))
			return(0); /*top*/
		err_rpt(0,FALSE);
		return(1);
		}
	if (numfields == COND && gexit_vals == 0) /*no exit values*/
		{
		err_rpt(39,TRUE); 
		show_cmd("",18);
		move(gnames[cur_cfg].ylab,gnames[cur_cfg].xlab);
		hilight(gnames[cur_cfg].ylab, gnames[cur_cfg].xlab,
		gnames[cur_cfg].name);
		refresh();
		if ((c=getch())==CTRL(r) ||
		c == KEY_F(4))
			return(0); /*top*/
		err_rpt(0,FALSE);
		return(1);
		}
	return(0);	/*user wants to go to top*/
}

toggle_want()	/*toggles exit message from never/uncond/cond*/
{
	switch(numfields)	{
		case NEVER:	numfields = UNCOND;
				break;
		case UNCOND:	numfields = COND;
				break;
		case COND:	numfields = NEVER;
				break;
		default:	break;
	}; /*switch*/
}

get_efield() /*this routine allows user to select conditional exit field*/
{
	struct fieldlink *fl, *select();

		if (exit_field > -1) 
			Field_pt = fields + exit_field;
		else
			Field_pt = fields;
         	fl = select((struct fieldlink *)NULL,
                 " Select field on which exit message is conditional",3);
         	if (fl !=NULL && (fl->f_pt)!=NULL)
			exit_field = (int)(fl->f_pt - fields);
		else return(0);	/*user wants top menu*/
	clear();
	glredisp();
	refresh();
	return(1);
}

global()	/*main global screen routine*/
{
	int tmp,tmp1,c;
	char *editstr(), *c_pt;
	struct charstr *askstring(); 

	clear();
	if (exit_field != -1)
		numfields = COND;	/*conditional stuff*/
	else if (exit_mess != NULL)
		numfields = UNCOND;	/*no conditional stuff*/
	     else numfields = NEVER;	/*no exit message*/
	glredisp();
	gprompt(); /*display appropriate prompt*/
	while (c=getch())
		{
		err_rpt(0,FALSE);	/*clear any previous error message*/
		switch (c) {
		case KEY_F(7):
		case CTRL(e):
			if (cur_cfg == C_N)
				{
				c_pt = editstr(lab_pt->screen_name);
				while ((tmp1=chckstr(c_pt,0)) != 0 || 
				(tmp=lcheck(c_pt))!=0)
				{
					move(gnames[C_N].ylab,gnames[C_N].xval);
					clrtoeol();
					dctrlshow(gnames[C_N].ylab,gnames[C_N].xval,79,c_pt);
					show_cmd("",16);
					switch(tmp)	{
					case 1: /*found 'cd'*/
						err_rpt(51,TRUE);
						break;
					case 2: /*found 'umask'*/
						err_rpt(52,TRUE);
						break;
					default: /*must be bad ctrl chars*/
						err_rpt(tmp1,TRUE);
						break;
					}; /*switch*/
					refresh();
					getch();
					err_rpt(0,FALSE);
					c_pt = editstr(c_pt);
				}
				strcpy(lab_pt->screen_name,c_pt);
				if (strlen(lab_pt->screen_name)>17)
					{
					*(lab_pt->screen_name+17) = null;
					err_rpt(50,TRUE);
					}
				move(gnames[C_N].ylab,gnames[C_N].xval);
				clrtoeol();
				dctrlshow(gnames[C_N].ylab,gnames[C_N].xval,
				79,lab_pt->screen_name);
				refresh();
				}
			else
				{
				err_rpt(5,TRUE);
				refresh();
				}
			break;
		case KEY_F(1):
		case CTRL(g):
			switch(cur_cfg) {
			case C_N:	/*edit command name*/ 
				show_cmd("",8); /*insert prompt*/
				mvaddstr(gnames[C_N].ylab,gnames[C_N].xval,"<empty>");
				clrtoeol();
				c_pt = askstring("",gnames[C_N].ylab,gnames[C_N].xval,79,cur_cfg,ghelp,1)->str;
                while ((tmp1=chckstr(c_pt,0))!=0 || (tmp=lcheck(c_pt))!=0)
					{
					move(gnames[C_N].ylab,gnames[C_N].xval);
					clrtoeol();
					dctrlshow(gnames[C_N].ylab,gnames[C_N].xval,79,c_pt);
					show_cmd("",25);
					switch(tmp)	{
						case 1: /*found 'cd'*/
							err_rpt(51,TRUE);
							break;
						case 2: /*found 'umask'*/
							err_rpt(52,TRUE);
							break;
						default:	/*all ok*/
							err_rpt(tmp1,TRUE);
							break;
						}; /*switch*/
					refresh();
					getch();
					move(gnames[C_N].ylab,gnames[C_N].xval);
					clrtoeol();
					move(gnames[C_N].ylab,gnames[C_N].xval);
					addstr("<empty>");
                   	err_rpt(0,FALSE);
				show_cmd("",8); /*insert prompt*/
					c_pt = askstring("",gnames[C_N].ylab,gnames[C_N].xval,79,cur_cfg,ghelp,1)->str;
			}
				strcpy(lab_pt->screen_name,c_pt);
				if (strlen(lab_pt->screen_name)>17)
					{
					*(lab_pt->screen_name+17) = null;
					err_rpt(50,TRUE);
					}
				move(gnames[C_N].ylab,gnames[C_N].xval);
				clrtoeol();
				dctrlshow(gnames[C_N].ylab,gnames[C_N].xval,
				79,lab_pt->screen_name);
		          	show_cmd("",21);
				refresh();
				break;
			case W_E_M:	/*want exit mess NEVER/UNCOND/COND*/
					toggle_want(); 
					glredisp();
					refresh();
					break;
			case C_F: if (get_efield()== 0)
					{
					clear();
					if (verify_ex() == 0)
						return(0); /*top*/
					}
				  else glredisp();
				  refresh();
				   break;
			case C_E_V: 	/*edit exit values*/
				clear();
				if (edit_exit(ghelp,cur_cfg,"'global'") == 0)
					{
					clear();
					if (verify_ex() ==0)
						return(0); /*top*/
					}
				else
					{
					clear();
					glredisp();
					}
				refresh();
				break;
			case C_L_O: /*order command line*/
				if (location() == 0)
					{
					clear();
					if (verify_ex() == 0)
						return(0); /*top*/
					}
				else
					{
					clear();
					glredisp();
					}
				refresh();
				break;
			case C_F_H: /*form help*/
				form_help = editstr(form_help);
				while ((tmp=helpchk(form_help)) != 0)
					{
					clear();
					glredisp();
					hilight(gnames[cur_cfg].ylab,
					gnames[cur_cfg].xlab,gnames[cur_cfg].name);
					if (tmp!=48) show_cmd("",16); /*hit any key..*/
					else show_cmd("",25);
					err_rpt(tmp,TRUE); /*error mess >78*/
					refresh();
					getch();	/*chars or >12 lines*/
					if (tmp==48) break;
					form_help = editstr(form_help);
					}
				if (tmp!=48) clear();
				glredisp();
				refresh();
				break;
			case E_M: /*exit message*/
				exit_mess = editstr(exit_mess);
				while ((tmp=helpchk(exit_mess)) != 0)
					{
					clear();
					glredisp();
					hilight(gnames[cur_cfg].ylab,
				gnames[cur_cfg].xlab,gnames[cur_cfg].name);
					if (tmp!=48) show_cmd("",16); /*hit any key..*/
					else show_cmd("",25);
					err_rpt(tmp,TRUE); /*bad exit mess*/
					refresh();
					getch(); /*try again*/
					if (tmp==48) break;
					exit_mess = editstr(exit_mess);
					}
				if (tmp!=48) clear();
				glredisp();
				refresh();
				break;
			default: /*shouldn't get here*/
				err_rpt(5,TRUE);
				break;
			}; /*switch*/
			break;
		case KEY_F(6):	/*user wants item help*/
		case CTRL(y):
				fhelp(ghelp,cur_cfg,2,4);
				err_rpt(0,FALSE); /*clear*/
			break;
		case KEY_DOWN:
		case '\n': /*new line*/
		case '\r': /*return*/
			mvaddstr(gnames[cur_cfg].ylab,
			gnames[cur_cfg].xlab,gnames[cur_cfg].name);
			cur_cfg = nextvar(numfields,cur_cfg);
			break;
		case KEY_UP:
		case CTRL(p):
			mvaddstr(gnames[cur_cfg].ylab,
			gnames[cur_cfg].xlab,gnames[cur_cfg].name);
			cur_cfg = prevvar(numfields,cur_cfg);
			break;
		case KEY_F(3):
		case CTRL(l): /*redraw screen*/
			REDRAW;
			break;
		case KEY_F(5):
		case CTRL(t):
			if (verify_ex() == 0)
				return(0); /*top*/
			break;
		case KEY_F(4):
		case CTRL(r):
			if (verify_ex() == 0)
				return(0); /*top*/
			break;
		default:
			if ((tmp=firstlet(gnames,numfields,c,cur_cfg)) == -1)
				flushinp(); /*no first letter match*/
			else 		 /*set location to letter*/
				{
				mvaddstr(gnames[cur_cfg].ylab,
				gnames[cur_cfg].xlab,gnames[cur_cfg].name);
				cur_cfg = tmp;
				}
			break;
		} /*switch*/
	gprompt(); /*show proper prompt*/
	} /*while*/
} /*procedure*/

glredisp()
{
	int i;
	struct field *f_pt;
	char *tmp, *c_pt;

	mvaddstr(0,31,"Global Information");
	draw_line(3);
        draw_line(LINES-3);
	for (i = 0; i < numfields; i++)
		{
		mvaddstr(gnames[i].ylab,gnames[i].xlab,gnames[i].name);
		clrtoeol();
		}
	switch(numfields)	{
	case NEVER:
		for (i=gnames[E_M].ylab;i<=LINES-4;i++)
			{
			move(i,0);
			clrtoeol(); /*so don't display it*/
			}
		mvaddstr(gnames[W_E_M].ylab,gnames[W_E_M].xval, "never");
		clrtoeol();
		break;
	case UNCOND:
		for (i=gnames[C_F].ylab;i<=LINES-4;i++) /*clear rest*/
			{
			move(i,0);
			clrtoeol(); /*so don't display it*/
			}
		mvaddstr(gnames[W_E_M].ylab,gnames[W_E_M].xval,
			"unconditionally");
		clrtoeol();
		break;
	case COND:
		move(gnames[C_F].ylab,gnames[C_F].xval);
       		clrtoeol();
		if (exit_field != -1)
			{
			f_pt = fields; /*find conditional exit field*/
			for (i=0;f_pt<=last_field_pt && i<exit_field;i++,f_pt++)
				;
			c_pt = tmp = f_pt->caption; /*make sure it fits*/
        		for (;*c_pt && c_pt-tmp<50; c_pt++) addch(*c_pt);
			}
		else addstr("<none defined yet>");
		mvaddstr(gnames[W_E_M].ylab,gnames[W_E_M].xval,"conditionally");
		clrtoeol();
		break;
	default: break;
	};/*switch*/
	dctrlshow(gnames[C_N].ylab,gnames[C_N].xval,79,lab_pt->screen_name);
	clrtoeol();
	fig_exit_vals(); /*count number of exit values*/
	fig_exit_mess(); /*count number of lines of exit message*/
	fig_help_mess(); /*count number of lines of help message*/
}

VOID fig_exit_vals()	/*count number of conditional exit values*/
{
	int cnt = 0;
	char **e_pt;

	if (numfields == COND)
		{
		move(gnames[C_E_V].ylab,gnames[C_E_V].xval);
		for (e_pt = exit_strs; *e_pt != NULL; e_pt++)
			cnt++;
		if (cnt != 1)	/*pluralize*/
			printw("(%d strings defined)",cnt);
		else addstr("(1 string defined)");
		clrtoeol();
		}
	gexit_vals = cnt;	/*global, used for exit warning*/
}

VOID fig_exit_mess() /*count number of exit message lines*/
{
	int cnt = 0;
	register char *c_pt = exit_mess;

	if (numfields != NEVER)
	{
		move(gnames[E_M].ylab,gnames[E_M].xval);
		if (exit_mess !=NULL)
			{
			if (*(c_pt+1) == null)
				addstr("(0 lines specified)");
			else	/*at least something there*/
				{
				while (*c_pt)
					if (*c_pt++ == '\n')
						cnt++;
				cnt++;
				if (cnt != 1)
					printw("(%d lines specified)",cnt);
				else addstr("(1 line specified)");
				}
			}
		else addstr("(0 lines specified)");
		clrtoeol();
	}
	gexit_lines=cnt; /*global variable used for exit*/
}

VOID fig_help_mess() /*count number of help message lines*/
{
	int cnt = 0;
	register char *c_pt = form_help;

	move(gnames[C_F_H].ylab,gnames[C_F_H].xval);
	if (form_help !=NULL)
		{
		if (*(c_pt+1) == null)
			addstr("(0 lines specified)");
		else
			{
			while (*c_pt)
				if (*c_pt++ == '\n')
					cnt++;
			cnt++;
			if (cnt != 1)
				printw("(%d lines specified)",cnt);
			else addstr("(1 line specified)");
			}
		}
	else
		addstr("(0 lines specified)");
	clrtoeol();
}

/*This routine is used to determine that the name of an assist screen
is not the "cd" or "umask" command names.*/
int lcheck(str)
char *str;	/*string to be checked*/
{

	if (strncmp(str,"cd ",3)==0 ||
	(strlen(str)==2 && strncmp(str,"cd",2)==0))
		return(1);	/*can't use 'cd ' as screen name*/
	if ((strncmp(str,"umask ",6)==0) || 
	(strlen(str)==5 && strncmp(str,"umask",5)==0))
		return(2);	/*can't use 'umask' as screen name*/
	return(0);	/*no error detected*/
}


