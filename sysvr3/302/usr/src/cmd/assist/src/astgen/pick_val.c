/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:pick_val.c	1.34"
#include "../forms/muse.h"
#include "tools.h"
#include "vdefs.h"

#define NUMVALS	17
#define PERMS	0
#define	EXIST	1
#define	NEXIST	2
#define EXSTR	3
#define OVERW	4
#define OWNFILE	5
#define NOTSAME 6
#define SHELLVAL	7
#define VERS	8
#define MATCH	9
#define NMATCH	10
#define LENGTH	11
#define FORMAT	12
#define REGEXP	13
#define	BEGEND	14
#define	NBEGEND	15
#define QUOTE	16

char *vhelpfile="vals.help";
int b_t_TOP = 1; /* Wants-to-go-back-to-TOP flag (if 0) */

/*indices to "map" are assist validation numbers.  Values are */
/*local validation numbers used in these routines*/
int map[] = {-1,LENGTH,EXIST,OVERW,MATCH,-1,-1,NOTSAME,FORMAT,OWNFILE,
		-1,-1,NEXIST,-1,-1,EXSTR,-1,BEGEND,NBEGEND,-1,-1,-1,
		-1,-1,NMATCH,-1,REGEXP,QUOTE,-1,-1,-1,-1,SHELLVAL,-1,PERMS,
		VERS};
		
		/*array with indices of muse validation numbers
			that contains local number, or -1 if not used (including
			incomp, requires, etc. */
struct coord {
	int left;
	int right;
	int up;
	int down;
};
struct coord dirs[] = {	/*for each val, where to go for L, R, U, and D*/
	{VERS,MATCH,VERS,EXIST},	/*PERMS*/
	{MATCH,NMATCH,PERMS,NEXIST},	/*EXIST*/
	{NMATCH,LENGTH,EXIST,EXSTR},	/*NEXIST*/
	{LENGTH,FORMAT,NEXIST,OVERW},	/*EXSTR*/
	{FORMAT,REGEXP,EXSTR,OWNFILE},	/*OVERW*/
	{REGEXP,BEGEND,OVERW,NOTSAME},	/*OWNFILE*/
	{BEGEND,NBEGEND,OWNFILE,MATCH},	/*NOTSAME*/
	{QUOTE,VERS,QUOTE,VERS},	/*SHELLVAL*/
	{SHELLVAL,PERMS,SHELLVAL,PERMS},	/*VERS*/
	{PERMS,EXIST,NOTSAME,NMATCH},	/*MATCH*/
	{EXIST,NEXIST,MATCH,LENGTH},	/*NMATCH*/
	{NEXIST,EXSTR,NMATCH,FORMAT},	/*LENGTH*/
	{EXSTR,OVERW,LENGTH,REGEXP},	/*FORMAT*/
	{OVERW,OWNFILE,FORMAT,BEGEND},	/*REGEXP*/
	{OWNFILE,NOTSAME,REGEXP,NBEGEND},	/*BEGEND*/
	{NOTSAME,QUOTE,BEGEND,QUOTE},	/*NBEGEND*/
	{NBEGEND,SHELLVAL,NBEGEND,SHELLVAL},	/*QUOTE*/
};

struct val_info {		/*info about each validation*/
	int map; /*number known to muse*/
	int prefix; /*num pref: 0=max, 1=1, -1=none*/
	int suffix; /*num post: 0=max, 1=1, -1=none*/
	int picked; /*selected: 0=not, 1=selected*/
	char *pre;  /*title for prefix screen*/
	char *post; /*title for suffix screen*/
}; /*val_info*/


struct vfunc *v_sel[NUMVALS];	/*info about selected validations*/

struct labels vals[] = {
	{4,6,4," Have specified permissions"},
	{4,7,4," Exist"},
	{4,8,4," Not exist"},
	{4,9,4," Be readable or match string(s)"},
	{4,10,4," Be writable or can be created"},
	{4,11,4," Be owned by user"},
	{4,12,4," Differ from file in last field"},
	{30,16,30," Run a shell script"},
	{30,17,30," Version check"},
	{41,6,41," Match specified string(s)"},
	{41,7,41," Not match specified string(s)"},
	{41,8,41," Be no longer than a specified length"},
	{41,9,41," Match specified format(s)"},
	{41,10,41," Be a regular expression"},
	{41,11,41," Start/end with specified string(s)"},
	{41,12,41," Not start/end with specified string(s)"},
	{41,13,41," Be quoted if it has specified char(s)"}
}; 

struct val_info info[] = {	/*info about validations*/
	{34,1,1,0,"",""},	/*permissions-call special routine*/
	{2,-1,-1,0,"",""},	/*file exists*/
	{12,-1,-1,0,"",""},	/*file must not exist*/
	{15,0,-1,0,"Acceptable User Strings",""}, /*exist or match str*/
	{3,-1,-1,0,"",""},	/*file is writable or creatable*/
	{9,-1,-1,0,"",""},	/*ownfile*/
	{7,-1,-1,0,"",""},				/*notsame*/
	{32,1,-1,0,"Shell Validation Script",""},	/*shellval*/
	{35,-1,-1,0,"",""},				/*version check*/
	{4,0,-1,0,"Acceptable User Strings",""},	/*matchlist*/
	{24,0,-1,0,"Unacceptable User Strings",""},	/*not match list*/
	{1,1,-1,0,"Maximum Length Allowed",""},		/*length*/
	{8,0,1,0,"Acceptable Formats","Characters Replacing ^P"}, /*format*/
	{26,-1,-1,0,"",""},	/*regexp*/
	{17,0,0,0,"Acceptable Prefixes","Acceptable Suffixes"},	/*prepo*/
	{18,0,0,0,"Unacceptable Prefixes","Unacceptable Suffixes"}, /*no_prepo*/
	{27,0,0,0,"Characters That Must Be Quoted","Acceptable Quoting Characters"},	/*mustquote*/
	{0,0,0,0,"",""},
};

static int cur_val = 0;	/*current local validation number*/


want_more()	/*figures out whether user wants to try again, after*/
{		/*an error in specifying arguments to a validation*/
	int rsp;

	if ((rsp=getch()) == CTRL(o) || rsp == KEY_F(8))
		{
		err_rpt(0,FALSE); /*clear old message*/
		return(FALSE);	/*user gives up*/
		}
	else
		{
		err_rpt(0,FALSE); /*clear old message*/
		return(TRUE);	/*user wants to try again*/
		}
}
		
check_pre(f_pt)	/*figures if args to validations are ok*/
struct field *f_pt;
{
	int tmp;

	switch(cur_val)	{
		case QUOTE:	/*these all require at least one prefix*/
		case MATCH:
		case FORMAT:
		case NMATCH:
		case EXSTR:
		case SHELLVAL:	tmp = haspre(v_sel[cur_val]); /*0 if all ok*/
				break;
		case LENGTH:	if (haspre(v_sel[cur_val]) == 0) /*needs arg*/
					tmp = length_chk(v_sel[cur_val]);
				else tmp = 15; /*must be integer*/
				break;
		default:	tmp = 0; /*others don't require prefixes*/
		} /*switch*/
	if (tmp == 0)	/*no errors detected*/
		return(1); /*all is well*/
	buildscreen(f_pt);	/*show screen*/
	switch(cur_val)	{	/*display prefix error message*/
		case QUOTE:	/*need list of characters*/
			err_rpt(27,TRUE);
			break;
		case EXSTR:
		case MATCH:
		case NMATCH:	/*no strings entered*/
			err_rpt(30,TRUE);
			break;
		case LENGTH:	/*need length*/
			err_rpt(15,TRUE);
			break;
		case FORMAT:	/*nothing entered*/
			err_rpt(31,TRUE);
			break;
		case SHELLVAL:	/*nothing entered*/
			err_rpt(29,TRUE);
			break;
		default:	break;
	} /*switch*/
	show_cmd("",24);	/*allow user to try again*/
	hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
	refresh();
	if (want_more() == FALSE) /*delete validation*/
		return(2); /*means forget it*/
	else return(3); /*try again*/
}

check_post()	/*figures if all ok for post*/
{

	if (haspost(v_sel[cur_val]) != 0) /*0 means suffix entered by user */
		{
		hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
		switch(cur_val)	{
			case QUOTE:  /*no quote characters supplied*/
				err_rpt(28,TRUE);
				break;
			case FORMAT: /*no postfix, and ^P used*/
				err_rpt(12,TRUE);
				break;
			/*case LIST: no list entered
				err_rpt(30,TRUE);
				break; */
			default:	break;
		} /*switch*/
		show_cmd("",24);	/*allow user to try again*/
		hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
		refresh();
		if (want_more() == FALSE) /*delete validation*/
			return(2); /*means forget it*/
		else return(3); /*try again*/
		}
	else return(1); /*all is well*/
}


get_pref(v_pt,f_pt)		/*specify prefixes, if val takes any*/
struct vfunc *v_pt;
struct field *f_pt;
{
	char *ret_msg;


	while(TRUE)
		{
		clear();
		if (info[cur_val].suffix == -1)
			ret_msg="'validation'"; /*takes no suffixes*/
		else ret_msg="'suffix'"; /*takes suffixes*/
		b_t_TOP = editval(f_pt,v_pt,info[cur_val].pre,"value: ",0,info[cur_val].prefix,
		cur_val,vhelpfile,ret_msg);

		clear();
		switch(check_pre(f_pt))	{
			case 1: return(b_t_TOP);	/*all is well*/
			case 2: delval(f_pt);	/*user wants to delete*/
				return(2);
			default: break;		/*user will try again*/
			} /*switch*/
		} /*while*/
}

get_post(v_pt,f_pt)
struct vfunc *v_pt;
struct field *f_pt;
{

	clear();
	b_t_TOP = editval(f_pt,v_pt,info[cur_val].post,"value: ",1,info[cur_val].suffix,
	cur_val,vhelpfile,"'validations'");
	clear();
	buildscreen(f_pt);
	hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
	refresh();
	if (cur_val == QUOTE || (cur_val == FORMAT && chckformat(v_pt,1) !=0)) 
		/*QUOTE always needs suffix, FORMAT does if ^P used*/
	{
	while(TRUE)
		{
		switch(check_post())	{
			case 1: return(b_t_TOP); /*all is well*/
			case 2: delval(f_pt);	/*user wants to delete*/
				return(1);
			default: break;		/*user wants to try again*/
			} /*switch*/
		clear();
		b_t_TOP = editval(f_pt,v_pt,info[cur_val].post,"value: ",1,info[cur_val].suffix,
		cur_val,vhelpfile,"'validations'");
		clear();
		buildscreen(f_pt);
		hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
		refresh();
		} /*while*/
	} /*if */
	switch(cur_val)	{ /*see if this is an 'either/or' validation*/
		case BEGEND:
		case NBEGEND:	if (haspre(v_sel[cur_val]) != 0 &&
				haspost(v_sel[cur_val]) != 0)
					{	/*delete validation*/
					delval(f_pt);
					err_rpt(32,TRUE);
					show_cmd("",25);
					refresh();
					getch();
					b_t_TOP = 1;
					}
				return(2);
		default:	return(1);
		} /*switch*/

}

getvargs(f_pt,v_pt)	/*figures out what is needed for the validation*/
struct field *f_pt;
struct vfunc *v_pt;
{
	switch(cur_val)	{
		case PERMS:
			b_t_TOP = set_perms(f_pt,v_pt);
			if (b_t_TOP == 0) return(b_t_TOP);
			break;
		case FORMAT:
			switch(get_pref(v_pt,f_pt))	{
				case 0: b_t_TOP = 0;
					return(1);
				case 2:	b_t_TOP = 1;
					return(1);	/*ok*/
				default: if (chckformat(v_pt,0) == 1) /*^P?*/
					 {
						if (get_post(v_pt,f_pt) == 0)
							b_t_TOP = 0;
						else
							b_t_TOP = 1;
					 }
					 else b_t_TOP = 1;
					 return(1);
			} /*switch*/
			break;
		default:
			if (info[cur_val].prefix != -1) /*-1 = no args*/
				{
				switch(get_pref(v_pt,f_pt))	{
					case 0: b_t_TOP = 0;
						return(0);
					case 2:	b_t_TOP = 1;
						return(1); /*continue*/
					default:	break;
				} /*switch*/
				} /*if*/
			if (info[cur_val].suffix != -1) /*-1 = no suffix*/
				{
				switch(get_post(v_pt,f_pt))	{
					case 0: b_t_TOP = 0;
						return(0);
					case 2:	b_t_TOP = 1;
						return(1);
					default:	break;
				} /*switch*/
				}
			return(1);
		} /*switch*/
	return(1);
} /*getvargs*/

VOID vprompt()	/*display prompt that is appropriate for cur_val field*/
{
	if (info[cur_val].picked != 0 && info[cur_val].prefix != -1)
		show_cmd("",4); /*allow delete or edit*/
	else 
		{
		if (info[cur_val].picked == 0 )
			show_cmd("",14); /*not yet selected*/
		else show_cmd("",13); /*delete but no edit*/
		}
	hilight(vals[cur_val].ylab,vals[cur_val].xlab,vals[cur_val].name);
	refresh();
} /*prompt*/

VOID delval(f_pt)  /*delete validation corresponding to current item*/
struct field *f_pt;
{

	rmval(f_pt,v_sel[cur_val]); /*remove from linked list*/
	info[cur_val].picked = 0;   /*remove "picked" flag*/
	v_sel[cur_val] = NULL;     /*NULL local linked list*/
	mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab-4,"    ");
				/*erase arrow next to validation*/
}

d_addval(f_pt)
struct field *f_pt;
{
	struct vfunc *addval(), *v_pt;

	v_pt = v_sel[cur_val] = addval(f_pt,info[cur_val].map);
	info[cur_val].picked = 1; /*set "picked" flag*/
	do_arrow(vals[cur_val].ylab,vals[cur_val].xlab-4);/*mark picked*/
	b_t_TOP = getvargs(f_pt,v_pt); /*see if it takes arguments*/
	buildscreen(f_pt);

	switch(cur_val)	{	/*now see if this validation is incompatible*/
	case EXIST: 		/*with any previously selected ones.  If so,*/
	case EXSTR:	if (info[NEXIST].picked == 1)	/*warn user only*/
				err_rpt(13,TRUE);
			break;
	case NEXIST:	if (info[EXIST].picked == 1 || info[EXSTR].picked == 1)
				err_rpt(13,TRUE);
			break;
	default:	
			if (b_t_TOP==1) 
				refresh();
			break;
	} /*switch*/
	return(b_t_TOP);
}

VOID do_arrow(y,x)
int x,y;
{
	int i;

	move(y,x);
	if (term & ALTCHAR) attron(A_ALTCHARSET);
	for (i=0;i<=2;i++) addch(HOR);
	if (term & ALTCHAR) attroff(A_ALTCHARSET);
	addch('>');
}

VOID show_sel(f_pt)	/*look in data structure, to figure out which*/
struct field *f_pt;	/*validations have been selected*/
{
	struct vfunc *v_pt;
	int local; /*maps muse number to local number*/
	
	for (v_pt = f_pt->first_v_pt;v_pt != NULL;v_pt = v_pt->next)
		{
		if ((local = map[v_pt->num]) != -1) /*-1 means ignore*/
			{
			do_arrow(vals[local].ylab,vals[local].xlab-4);
			info[local].picked = 1; /*set flag*/
			v_sel[local] = v_pt; /*set local linked list*/
			}		     /*needed when val is deleted*/
		}
/*	refresh();	*/
}

selectval(f_pt)
struct field *f_pt;
{	
int i,c,tmp;

clear();
for (i=0;i<=NUMVALS;i++) info[i].picked = 0;
buildscreen(f_pt);
vprompt(); /*context appropriate command prompt*/
while (c=getch())
	{
	err_rpt(0,FALSE); /*clear old error messages*/
	switch (c) {
	case KEY_F(5):
	case CTRL(t):	return(0);	/*to top menu*/
	case KEY_F(4):
	case CTRL(r):	return(1);	/*to field attribute screen*/
	case KEY_F(8):		/*delete validation*/
	case CTRL(o):	if (info[cur_val].picked == 0) /*want to delete*/
				err_rpt(14,TRUE); /*not picked*/
			else delval(f_pt);	/*delete validation*/
			break;
	case KEY_F(1):	/*select or edit validation*/
	case CTRL(g):
		if (info[cur_val].picked == 0) /*not yet selected*/
			{
			d_addval(f_pt);
			if (b_t_TOP == 0) return(0);/*add */
			}
		else if (info[cur_val].prefix == -1) /*takes no args*/
				err_rpt(11,TRUE);	/*so complain*/
		else 
		{	getvargs(f_pt,v_sel[cur_val]);
			if (b_t_TOP==0) return(0);
			else buildscreen(f_pt);
			refresh();
		}
		break;
	case KEY_F(6):
	case CTRL(y):	/*user wants help*/
		fhelp(vhelpfile,cur_val,1,5);
		break;
	case '\r':
	case '\n':
	case KEY_DOWN:
		mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab,
			vals[cur_val].name);
		cur_val = dirs[cur_val].down;
		break;
	case CTRL(b):
	case KEY_LEFT:
		mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab,
			vals[cur_val].name);
		cur_val = dirs[cur_val].left;
		break;
	case CTRL(w):
	case KEY_RIGHT:
		mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab,
			vals[cur_val].name);
		cur_val = dirs[cur_val].right;
		break;
	case CTRL(p):
	case KEY_UP:
		mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab,
			vals[cur_val].name);
		cur_val = dirs[cur_val].up;
		break;
	case KEY_F(3):
	case CTRL(l):
		REDRAW;
		break;
	default:
		if ((tmp=firstlet(vals,NUMVALS,c,cur_val)) == -1)
			flushinp();
		else
			{
			mvaddstr(vals[cur_val].ylab,vals[cur_val].xlab,
				vals[cur_val].name);
			cur_val=tmp;
			}
		break;
	} /*switch*/
	vprompt();
	} /*while*/
} /*procedure*/

buildscreen(f_pt)
struct field *f_pt;
{
	int i;
	show_caption(f_pt,"Validations"); /*format screen*/
	mvaddstr(4,11,"File Must:");
	mvaddstr(4,45,"String or File Name Must:");
	for (i = 0; i < NUMVALS; i++)
		mvaddstr(vals[i].ylab,vals[i].xlab,vals[i].name);
	show_sel(f_pt);	/*show selected validations*/
}
