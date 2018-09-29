/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/cmdproc.c	1.4"
/* <: t-5 d :> */
#include <stdio.h>
#include "setop.h"
#include "termodes.h"
#include "ged.h"

#define AREAF 010
#define TEXTF 04
#define POINTF 02
#define PIVOTF 01
#define DELIM 1
#define MAXPTS 30
#define MAXARGL 80
#define SPW 5. /* single point window */


extern int cmdlines(), cmdarcs(), cmdtext(), cmdhard(), cmdcirc(), cmdbox();
extern int cmdread(), cmdwrite();
extern int cmdnew(), cmdobj(), cmdpt(), cmdzoom(), cmdview();
extern int cmddel(), cmdcopy(), cmdmove(), cmdscale(), cmdrot(), cmdedit();
extern int cmdquit(), cmdcoord(), cmdx();
extern int tekerase();
extern int cmdcmd(), cmdset();
char *getline();

static struct cmnd {
	char *name,*op,*opty;
	short flags;	/* tells whether cmd takes TEXT, POINTS, AREA, PIVOT */
	int (*func)();
} cds[] = {
"\1construct-commands:", "", "", 0, 0,
"Arc", "esw", "fcc", POINTF, cmdarcs,
"Box", "esw", "fcc", POINTF, cmdbox,
"Circle", "esw", "fcc", POINTF, cmdcirc,
"Hardware", "e", "f", TEXTF|POINTF, cmdhard,
"Lines", "esw", "fcc", POINTF, cmdlines,
"Text", "aehmrtw", "ififffc", TEXTF|POINTF, cmdtext,

"\1\12edit-commands:", "", "", 0, 0,
"Delete", "uv", "ff", AREAF|POINTF, cmddel,
"Edit", "aehsuvw", "ificffc", AREAF|POINTF, cmdedit,
"Kopy", "epx", "fff", AREAF|POINTF|PIVOTF, cmdcopy,
"Move", "epx", "fff", AREAF|POINTF|PIVOTF, cmdmove,
"Rotate", "aekx", "ifff", AREAF|POINTF|PIVOTF, cmdrot,
"Scale", "efkx", "fiff", AREAF|POINTF|PIVOTF, cmdscale,

"\1\12viewing-commands:", "", "", 0, 0,
"coordinates", "", "", POINTF, cmdcoord,
"erase", "", "", 0, tekerase,
"new", "", "", 0, cmdnew,
"objects", "uv", "ff", AREAF|POINTF, cmdobj,
"points", "luv", "fff", AREAF|POINTF, cmdpt,
"view", "hrux", "fiff", PIVOTF, cmdview,
"x", "v", "f", POINTF, cmdx,
"zoom", "o", "f", AREAF|POINTF, cmdzoom,

"\1\12other commands:", "", "", 0, 0,
"quit", "", "", 0, cmdquit,
"Quit", "", "", 0, NULL,
"read", "aehmrtw", "ififffc", 0, cmdread,
"set", "adefhkmoprstwx", "iffiifffffcfcf", 0, cmdset,
"write", "", "", 0, cmdwrite,
"?", "", "", 0, cmdcmd,
NULL,
};
static struct option {
	char let,type;
	int val;	/* initial value for setable options */
} op [] = {
'a', 'i', 0,
'f', 'i', 200,
'h', 'i', 200,
'r', 'i', -1,
's', 'c', 's'+256*'o',
'w', 'c', 'm',

'd', 'f', 0,
'e', 'f', 1,
'k', 'f', 0,
'm', 'f', 0,
'o', 'f', 0,
'p', 'f', 0,
'r', 'f', 0,
't', 'f', 1,
'x', 'f', 0,
0,
};
static int clo[2][NUMOPS]; /* command line options array: value, explicit set flag */
static int pts[2][MAXPTS*2+4]; /* points array, double buffered */
static int px=0, pis[2]={ -1,-1 }; /* pts index, max index each buffer */
static int labpt[2]['Z'-'A']; /* array of labeled points */

extern struct tekterm tm;
extern struct control ct;
extern int dbuf[], *endbuf;
extern char *nodename;

cmdproc(ch,table) /* command processor */
char ch, *table[];
{
	struct cmnd *cp, *cptr();
	char args[MAXARGL], *o, *t, *c;
	int i, pi=0, *pt, x, y;

	if( (cp=cptr(ch))==NULL ) return(1);
	printf("%s ",cp->name);
	nodename = cp->name;

	getline(stdin,args,MAXARGL);

/* install default options */
	for( i=0; i<NUMOPS; i++ ) clo[1][i] = clo[0][i] = 0; /* reset flags and options */

	for( o=cp->op,t=cp->opty; *o!='\0'; o++,t++ )
		OP(*o) = dfltop(*o,*t);
	OP('d') = dfltop('d','f');

/* get options from command line */
	for( c=args; *c!='\0'; )
		if( *c++=='-' ) {
			while( *c!='\0' && !isspace(*c) ) {
				char s[4];
				char ch;
				switch( type(cp,(ch= *c++)) ) {
				case 'i':  SETSINT(OP(ch),c); OPF(ch)=1; break;
				case 'c':  SETSTR(s,c,3); OP(ch) = (int)s[0]+256*(int)s[1]; OPF(ch)=1; break;
				case 'f':
					if( *c=='-' ) OP(ch) = 0, c++;
					else OP(ch)=1;
					OPF(ch)=1;
					break;
				case DELIM: break;
				default: ERRPR1(%c?,ch);
				}
			}
		}

/* get arguments for command */
	if( cp->flags&TEXTF ) getline(stdin,TEXTBUF,(char *)dbuf+BUFSZ-TEXTBUF);

	if( cp->flags&POINTF || cp->flags&PIVOTF ) {
		pt = pts[px^=01]; /* px is an alternate action switch, values 0,1.  Done in readhelp() also */
		pis[px] = 0;
	}

	if( cp->flags&POINTF && !(OPF('l')||OPF('u')||OPF('v')) ) {
		char ch;

		for(pi=0; pi<MAXPTS && getpoint(&x,&y,&ch) && ch!='\r'; pi++ ) {
			switch(ch) {
			case '.':
				pis[px] = pi-1; /* save current ptr */
				px ^= 01; /* get previous state */
				pi = pis[px]; /* set previous ptr */
				pt = pts[px]; /* set previous buffer start */
				continue;
			case '>': {
				char s[2], c;

				putchar('>');
				pi--;
				getline(stdin,s,2);
				if( !isalpha(s[0]) ) printf("?\n");
				else {
					c = toupper(s[0]);
					labpt[0][c-'A'] = pt[2*pi];
					labpt[1][c-'A'] = pt[2*pi+1];
				}
				getacur(&tm.alphax,&tm.alphay);
				continue;
			}
			case '$':
				if( !getlabpt(&x,&y,pi) ) {
					pi--; continue;
				} else break;
			case '@': pi = -1; continue;
			case '#': pi = (pi>1 ? pi-2 : -1); continue;
			case '=': echopts(pt,--pi); continue;
			case ' ': i=pi; break;
			default: pi--; continue;
			}
			pt[2*pi] = x;
			pt[2*pi+1] = y;
			unscaleoff(&x,&y); /* convert to screen units */
			putdraw(pi,x,y);
		}
		if (pi==2 && strcmp(cp->name,"Arc")==0) { /* Don't let <cr> */
			pt[2*pi] = pt[2*(pi-1)];       /* become third point */
			pt[2*pi+1] = pt[2*(pi-1)+1];	/* of arc */
			pi++;
		}
		pis[px] = pi-1; /* store end of buf */
		pt[2*pi]=x; pt[2*pi+1]=y; /* store loc of <cr> */
	}

/* get defined area */
	if( cp->flags&AREAF && !(OPF('l')||OPF('u')||OPF('v')) ) {
		struct area ar;

		if(pi==1) pi=2, pis[px]=1; /* use <cr> as 2nd pt */
		if( !window(pt,pi,&ar) ) { /* single point window */
			double offset = SPW*ct.wratio;
			pt[0] = ar.lx = pt[0]-offset;
			pt[1] = ar.ly = pt[1]-offset;
			pt[2] = ar.hx = pt[0]+2*offset;
			pt[3] = ar.hy = pt[1]+2*offset;
			pi=2; pis[px]=1;
		}
		drbox(&ar,0,NARROW,DOTTED);
		if( OP('x') ) drx(&ar,0,NARROW,DOTTED);

		if( !(cp->flags&PIVOTF) ) getline(stdin,args,1); /* confirm area */
	}
/* get pivot point */
	if( cp->flags&PIVOTF && !(OPF('h')||OPF('r')||OPF('u')||OPF('v')) ) {
		if( !(cp->flags&AREAF) && OP('x') ) drx(&ct.w,0,NARROW,DOTTED);

		i = 2*pi;
		while( getpoint(&x,&y,&ch), (ch=='$' && !getlabpt(&x,&y,pi)) ) ;
		pt[i++]=x; pt[i++]=y;
		unscaleoff(&x,&y);
		putcraw('*',x,y);

		while( getpoint(&x,&y,&ch), (ch=='$' && !getlabpt(&x,&y,pi)) ) ;
		pt[i++]=x; pt[i++]=y;
	}

/* transfer control to command function */
	if( OP('d') ) { /* DEBUG option */
		char *c;
		int i;
		printf("OPTIONS\n");
		for( c=cp->op; *c!='\0'; c++ )
			printf("%c= %o\n",*c,OP(*c));
		printf("POINTS (pi=%d), last two are PIVOT, DEST\n",pi);
		for( i=0; i<2*(pi+2); i+=2 )
			printf("%d: x= %d, y= %d\n",i/2,pt[i],pt[i+1]);
		getacur(&tm.alphax,&tm.alphay);
	}
	if( cp->func!=NULL )
		(*cp->func)(clo,pt,pi,TEXTBUF,args,table);
	return(0);
}

struct cmnd *cptr(ch) /* return pointer to command instance */
char ch;
{
	struct cmnd *p;

	for( p=cds; p->name!=NULL; p++ )
		if( *p->name==ch ) return(p);
	return(NULL);
}

type(cp,ch) /* return type of command line option */
struct cmnd *cp;
char ch;
{
	char *o,*tp;

	for( o=cp->op,tp=cp->opty; *o!='\0'; o++,tp++ )
		if( ch==*o ) return(*tp);
		else if( ch==' '||ch=='-'||ch==','||ch=='+' )
			return(DELIM);
	if( ch=='d' ) return('f');
	else return(0);
}
dfltop(ch,ty) /* return default value for op ch*/
char ch,ty;
{
	struct option *p;

	for( p=op; p->let!=NULL; p++ )
		if( p->let==ch && p->type==ty ) return(p->val);
	if( ty=='f' ) return(0);
	else ERRPR0(cmdproc: option not found in default list);
	return(-1);
}
echopts(pt,pi)
int pt[],pi;
{
	int i, x, y;

	for( i=0; i<=pi; i++ ) {
		x=pt[i*2]; y=pt[i*2+1];
		unscaleoff(&x,&y);
		putdraw(i,x,y);
	}
}
echolpts()
{
	int i, x, y;

	for( i=0; i<('Z'-'A'); i++ )
		if( (x=labpt[0][i])!=0 && (y=labpt[1][i])!=0 ) /* 0,0 means unset */
			if( inarea(x,y,&ct.w) ) {
				unscaleoff(&x,&y);
				putcraw((char)(i+'A'),x,y);
			}
}

cmdcmd() /* print cmnd table */
{
	struct cmnd *p;

	printf("\nNAME\tOPTION,TYPE(c=char,f=flag,i=int)\n\n");
	for( p=cds; p->name!=NULL; p++ ) {
		printf("%s\t%s, %s\n",p->name,p->op,p->opty);
	}
}
cmdset(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], *table;
{
	struct option *p;

	if( args[0]!='\0' ) { /* set defaults */
		for( p=op; p->let!='\0'; p++ ) 
			p->val = OP(p->let);
		ct.textf = OP('t');
	} else { /* print defaults */
		char s[3]; s[2]=0; /* temp for char to string convert */
		printf("\nOPTION VALUE\n");
		for( p=op; p->let!='\0'; p++ )
			switch( p->type ) {
			case 'c': s[0] = p->val%256; s[1] = p->val/256;
				printf("   %c = %s\n",p->let,s);
				break;
			case 'f': printf("   %c = %c\n",p->let,(p->val)?'+':'-');
				break;
			default: printf("   %c = %d\n",p->let,p->val);
			}
	}
	return;
}

int *
readhelp() /* get point for cmdread */
{
	char ch;
	int *pt;

	pt = pts[px^=01];					/*Done above in cmdproc.c, line 143*/
	pis[px] = 0;
	while( getpoint(&pt[0],&pt[1],&ch), (ch=='$' && !getlabpt(&pt[0],&pt[1],pis[px]+1)) ) ;
	return(pt);
}
getlabpt(x,y,pi) /* get labelled point */
int *x,*y,pi;
{
	char s[5], *sp=s, c;
	int i, px1=px;

	putchar('$');
	getline(stdin,s,4);
	if( isalpha(*sp) ) { /* labeled point */
		c=toupper(*sp);
		if( (*x=labpt[0][c-'A']) && (*y=labpt[1][c-'A']) ) {
			getacur(&tm.alphax,&tm.alphay);
			return(TRUE);
		}
	} else { /* numbered point */
		if( *sp=='.' ) px1^=01, sp++; /* previous environ */
		if( (i=atoi(sp)) < (*s!='.' ? pi:pis[px1]+1) && i>=0 ) {
			*x = pts[px1][2*i];
			*y = pts[px1][2*i+1];
			getacur(&tm.alphax,&tm.alphay);
			return(TRUE);
		}
	}
	printf("?\n"); /* no match */
	getacur(&tm.alphax,&tm.alphay);
	return(FALSE);
}
