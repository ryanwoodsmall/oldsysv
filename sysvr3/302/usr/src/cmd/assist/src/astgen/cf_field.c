/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:cf_field.c	1.23"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"
#define	OPT 0
#define	MULTI 1
#define	EXP 2
#define	MAP 3
#define	I_H_M 4
#define	VAL 5
#define	INC 6
#define	REQ 7
#define	LEAST 8

#define	NUMVARS	9 /*number of fields*/

/*the index to this array is the assist validation number (0-34) */
int bad[]={0,0,0,0,0,1,1,0,0,0,1,1,0,1,1,0,1,0,0,1,0,
		0,1,1,0,1,0,0,1,1,1,1,0,0,0,0}; /*1 means should be
						skipped in counting*/

char *helpfile="cf_f.help"; /*help message file*/
static int cur_cff = 0; /*current menu item number*/
struct labels fixed[] = {
	{0,5,20," Input strings are:"},
	{0,6,32," More than one input string is:"},
	{0,7,38," Shell expansion of input strings is:"},
	{0,9,4," Mapping of input to command line"},
	{0,12,20," Item help message"},
	{0,14,14," Validations"},
	{3,17,25," Incompatible fields"},
	{3,18,25," Required fields"},
	{3,19,25," At-least-one fields"},
};

VOID fprompt(f_pt)	/*display prompt that is appropriate for cur field*/
struct field *f_pt;
{
	switch(cur_cff) {
		case OPT: if (f_pt->minfsegms <= 0)
				show_cmd("change to 'required'",2);
			 else show_cmd("change to 'optional'",2);
			 break;
		case MULTI: if (f_pt->maxfsegms <= 1)
				show_cmd("change to 'allowed'",2);
			 else show_cmd("change to 'prohibited'",2);
			 break;
		case EXP: if ((f_pt->first_cpr_pt != (struct fix*)0 &&
                               f_pt->first_cpr_pt->name != (char*)0 &&
                               *(f_pt->first_cpr_pt->name + 3) == '1') 
                               || f_pt->bundle==1)
				show_cmd("",15); /*can't select*/
			  else
				if (f_pt->bundle == 2)
					show_cmd("change to 'enabled'",2);
			 	else show_cmd("change to 'disabled'",2);
			 break;
		case MAP: show_cmd("'mapping-of-input' module",2);
			break;
		case I_H_M: show_cmd("edit item help message",2);
			break;
		case INC: show_cmd("'incompatible-fields' module",2);
			break;
		case REQ: show_cmd("'required-fields' module",2);
			break;
		case LEAST: show_cmd("'at-least-one-fields' module",2);
			break;
		case VAL: show_cmd("'validation' module",2);
			break;
		default:	show_cmd("",99); /*should never reach*/
		}; /*switch*/
	hilight(fixed[cur_cff].ylab,fixed[cur_cff].xlab,fixed[cur_cff].name);
	refresh();
}; /*prompt*/
			
VOID f_toggle(f_pt) /*this routine toggles the first three fields */
register struct field *f_pt;
{	

	switch(cur_cff) {
		case OPT:	/*required or optional*/ 		
			if (f_pt->minfsegms <= 0) 	/*currently opt*/
				f_pt->minfsegms = 1;	/*make required*/
			else f_pt->minfsegms = 0;	/*make optional*/
			redisplay(f_pt);
			refresh();
			break;
		case MULTI: 		/*multiple values or single*/
			if (f_pt->maxfsegms <= 1) /*currently single*/
				f_pt->maxfsegms = 30; /*make multiple*/
			else f_pt->maxfsegms = 1;
			redisplay(f_pt);
			refresh();
			break;
		case EXP:		/*shell expansion on or off*/
		        if ((f_pt->first_cpr_pt != (struct fix*)0 &&
                               f_pt->first_cpr_pt->name != (char*)0 &&
                               *(f_pt->first_cpr_pt->name + 3) == '1') 
                               || f_pt->bundle==1)
				{
				err_rpt(36,TRUE);
				refresh();
				}
			else
				{
			switch(f_pt->bundle) {
			case 0: f_pt->bundle = 2; /*currently on*/
				break;
			case 2: f_pt->bundle = 0; /*currently off*/
				break;
			default:	break;
			} /*switch*/
			redisplay(f_pt);
			refresh();
				} /*else*/
		}; 
}

VOID fig_req(f_pt) /*figures the number of required fields*/
register struct field *f_pt;
{
	int i=0;
	struct index *v_pt;
	
	for (v_pt = f_pt->first_m_pt;v_pt != NULL;v_pt = v_pt->next)
		i++;
	if (i != 1)
		mvprintw(fixed[REQ].ylab,fixed[REQ].xval,
		"[%d fields required by current field]",i);
	else 	mvaddstr(fixed[REQ].ylab,fixed[REQ].xval,
		"[1 field required by current field]");
}

VOID fig_least(f_pt) /*figures the number of at least one fields*/
register struct field *f_pt;
{
	int i=0;
	struct index *v_pt;
	
	for (v_pt = f_pt->first_a_pt;v_pt != NULL;v_pt = v_pt->next)
		i++;
	if (i != 1)
		mvprintw(fixed[LEAST].ylab,fixed[LEAST].xval,
		"[%d fields in group]",i);
	else 	mvaddstr(fixed[LEAST].ylab,fixed[LEAST].xval,
		"[1 field in group]");
}

VOID fig_val(f_pt) /*figures the number of validations fields*/
register struct field *f_pt; /*only those validations selectable thru*/
{			     /*astgen are counted*/
	int i=0;
	struct vfunc *v_pt; 
	for (v_pt = f_pt->first_v_pt;v_pt != NULL;v_pt = v_pt->next)
		if (bad[v_pt->num] == 0) /*include in count*/
			i++;
	if (i != 1)
		mvprintw(fixed[VAL].ylab,fixed[VAL].xval,
		"[%d validations selected]",i);
	else 	mvaddstr(fixed[VAL].ylab,fixed[VAL].xval,
		"[1 validation selected]");
}

VOID fig_incomp(f_pt) /*figures the number of incompatible fields*/
register struct field *f_pt;
{
	int i=0;
	struct index *v_pt;
	
	for (v_pt = f_pt->first_i_pt;v_pt != NULL;v_pt = v_pt->next)
		i++;
	if (i != 1)
		mvprintw(fixed[INC].ylab,fixed[INC].xval,
		"[%d fields incompatible with current field]",i);
	else 	mvaddstr(fixed[INC].ylab,fixed[INC].xval,
		"[1 field incompatible with current field]");
}

VOID fig_help(f_pt) /*count lines in help message*/
struct field *f_pt;
{
	int cnt = 0;
	register char *c_pt = f_pt->help;

	move(fixed[I_H_M].ylab,fixed[I_H_M].xval);
	if (f_pt->help !=NULL)
		{
		if (strlen(c_pt)<5)
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

vedit(f_pt)	/*routine where the action is*/
register struct field *f_pt;
{
	char *editstr();
	int tmp,c;

	redisplay(f_pt);
	fprompt(f_pt); /*display context-sensitive help*/
	while (c=getch())
		{
		err_rpt(0,FALSE);	/*clear any previous error message*/
		switch (c) {
		case KEY_F(4):
		case CTRL(r):
			return(1); /*allow new field to be selected*/
		case KEY_F(5):
		case CTRL(t):
			return(0); /*all the way to TOP menu*/
		case KEY_F(1):
		case CTRL(g):
			switch(cur_cff)
			{
				case OPT:
				case MULTI:
				case EXP:
					f_toggle(f_pt);
					break;
				case MAP:
					if (field_type(f_pt) == 0)
						return(0); /*to top*/
					clear();
					redisplay(f_pt);
					refresh();
					break;
				case INC:
					if (set_icmp(f_pt) == 0)
						return(0); /*to top*/
					clear();
					redisplay(f_pt);
					refresh();
					break;
				case REQ:
					if (set_reqs(f_pt) == 0)
						return(0); /*to top*/
					clear();
					redisplay(f_pt);
					refresh();
					break;
				case LEAST:
					if (set_atlone(f_pt) == 0)
						return(0); /*to top*/
					clear();
					redisplay(f_pt);
					refresh();
					break;
				case VAL:
					if (selectval(f_pt) == 0)
						return(0); /*to top*/
					clear();
					redisplay(f_pt);
					refresh();
					break;
				case I_H_M:
					f_pt->help=editstr(f_pt->help);
					while ((tmp=helpchk(f_pt->help)) != 0)
						{
						clear();
						redisplay(f_pt);
						refresh();
						hilight(fixed[cur_cff].ylab,fixed[cur_cff].xlab,fixed[cur_cff].name);
						if (tmp!=48) show_cmd("",16);
						else show_cmd("",25);
						err_rpt(tmp,TRUE); /*too long*/
						refresh();
						getch();	   /*or wide*/
						if (tmp==48) break;
						f_pt->help=editstr(f_pt->help);
						}
					if (tmp!=48) clear();
					redisplay(f_pt);
					refresh();
					break;
				default:	/*should never reach this*/
					break;
			}; /*switch*/
			break;
		case KEY_F(3):
		case CTRL(l): /*redraw screen*/
			REDRAW;
			break;
		case KEY_F(6):
		case CTRL(y):	/*user wants help*/
				fhelp(helpfile,cur_cff,2,4);
				break;
		case KEY_DOWN:
		case '\n': /*new line*/
		case '\r': /*return*/
			mvaddstr(fixed[cur_cff].ylab,fixed[cur_cff].xlab,
			fixed[cur_cff].name);
			cur_cff=nextvar(NUMVARS,cur_cff);
			break;
		case CTRL(p):
		case KEY_UP:
			mvaddstr(fixed[cur_cff].ylab,fixed[cur_cff].xlab,
			fixed[cur_cff].name);
			cur_cff=prevvar(NUMVARS,cur_cff);
			break;
		default:
			if ((tmp=firstlet(fixed,NUMVARS,c,cur_cff)) == -1)
				flushinp(); /*no first letter match*/
			else 
				{
				mvaddstr(fixed[cur_cff].ylab,fixed[cur_cff].xlab,
				fixed[cur_cff].name);
				cur_cff = tmp; /*move to matched letter*/
				}
			break;
		} /*switch*/
		fprompt(f_pt);
	} /*while*/
return(1); /*all ok*/
} /*procedure*/

VOID redisplay(f_pt) /*display screen*/
register struct field *f_pt;
{
	int i;

	/*get current field from data structure*/
	show_caption(f_pt,"Field Attributes");
	mvaddstr(16,1,"Relationship of Current Field With Other Fields");
	for (i = 0; i < NUMVARS; i++)
		mvaddstr(fixed[i].ylab,fixed[i].xlab,fixed[i].name);
	if (f_pt->minfsegms <= 0)
		mvaddstr(fixed[OPT].ylab,fixed[OPT].xval,"optional");
	else mvaddstr(fixed[OPT].ylab,fixed[OPT].xval,"required");
	clrtoeol();
	if (f_pt->maxfsegms <= 1)
		mvaddstr(fixed[MULTI].ylab,fixed[MULTI].xval,"prohibited");
	else mvaddstr(fixed[MULTI].ylab,fixed[MULTI].xval,"allowed");
	clrtoeol();
	move(fixed[EXP].ylab,fixed[EXP].xval);
	if ((f_pt->first_cpr_pt != (struct fix*)0 &&
             f_pt->first_cpr_pt->name != (char*)0 &&
             *(f_pt->first_cpr_pt->name + 3) == '1') 
            || f_pt->bundle==1)
		addstr("disabled");
	else
		if (f_pt->bundle == 0)
			addstr("enabled");
		else addstr("disabled");
	clrtoeol();
	fig_req(f_pt); /*count required fields*/
	fig_incomp(f_pt); /*count incompatible fields*/
	fig_least(f_pt); /*count least-one fields*/
	fig_val(f_pt);  /*count validations*/
	fig_help(f_pt); /*count item help lines*/
	fig_trans(f_pt); /*display command line mapping*/
}

VOID fig_trans(f_pt) /*display command line mapping*/
struct field *f_pt;
{
	char *c_pt, *tmp, *view_type();
	int maxl;

	maxl = COLS - fixed[MAP].xval - 1;
	move(fixed[MAP].ylab + 1,fixed[MAP].xval); /*goes on next line*/
        clrtoeol();
	c_pt=tmp=view_type(f_pt,0); /*make sure not too long*/
	if (tmp!=NULL)
	{
        	if (strlen(tmp)>maxl) 
		{
			*(tmp+maxl-11) = null;
			strcat(tmp," ... [more]");
		}
        	for (;*c_pt && c_pt-tmp<maxl; c_pt++) addch(*c_pt);
	}
}
