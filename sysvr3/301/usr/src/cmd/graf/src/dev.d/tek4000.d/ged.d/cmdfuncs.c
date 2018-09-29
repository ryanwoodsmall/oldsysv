/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/cmdfuncs.c	1.5"
/* <: t-5 d :> */
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "ged.h"
#include "termodes.h"
int universe[] = {XYMIN,XYMIN,XYMAX,XYMAX};


extern int interr();
extern int *readhelp();
extern char *nodename;
extern int dbuf[], *endbuf;
extern struct control ct;

extern struct command cd;

static
wtcode(mnu) /* return code for line weight */
int mnu;
{
	switch(mnu) {
	case 'b': return(BOLD);
	case 'm': return(MEDIUM);
	case 'n': return(NARROW);
	default: return(NARROW);
	}
}

static
stcode(mnu) /* return code for line style */
int mnu;
{
	switch(mnu) {
	case 'd'+256*'a': return(DASHED);
	case 'd'+256*'d': return(DOTDASH);
	case 'd'+256*'o': return(DOTTED);
	case 'l'+256*'d': return(LONGDASH);
	case 's'+256*'o': return(SOLID);
	default: return(SOLID);
	}
}
cmdlines(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{

	cd.cmd = LINES;
	cd.cnt = (pi==1 ? 2:pi)*2+2;
	cd.weight = wtcode(OP('w'));
	cd.style = stcode(OP('s'));
	cd.aptr = pts;

	putbufe(&cd);
	if(OP('e')) { teklines(&cd); outbuf(); }
}
cmdarcs(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{
	cd.cmd = ARCS;
	cd.cnt = (pi==2 ? 3:pi)*2+2;
	cd.weight = wtcode(OP('w'));
	cd.style = stcode(OP('s'));
	cd.aptr = pts;

	putbufe(&cd);
	if(OP('e')) { tekarc(&cd); outbuf(); }
}
cmdtext(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	char c;
	int *p;

	cd.cmd = TEXT;
	cd.cnt = (strlen(text)/2)+1+5;
	cd.trot = OP('a');
	cd.tsiz = OP('h');
	cd.font = wtcode(OP('w'))<<4; /* weight is upper 4 bits of font */
	cd.aptr = pts;
	cd.tptr = text;
	c = OP('m') ? 'm'
		: OP('r') ? 'r'
		: 'l';

	p=endbuf; /* save start of command */
	if( c!='l' ) textmr(&cd,c);
	else gedtext(&cd);
	if(OP('e')) {
		c = ct.textf; /* save current textf */
		ct.textf = OP('t'); /* get local textf */
		dispbuf(p,endbuf,table);
		ct.textf = c; /* restore */
	}
}
cmdhard(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{
	cd.cmd = ALPHA;
	cd.cnt = (strlen(text)/2)+1+3;
	cd.aptr = pts;
	cd.tptr = text;

	putbufe(&cd);
	if(OP('e')) { tekalpha(&cd); restoremode(); }
}
cmdbox(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{
	int p[10];

	if( pi>=1 ) {
		cd.cmd = LINES;
		cd.cnt = 12;
		cd.weight = wtcode(OP('w'));
		cd.style = stcode(OP('s'));
		cd.aptr = p;
	
		p[8] = p[6] = p[0] = pts[0];
		p[9] = p[3] = p[1] = pts[1];
		p[4] = p[2] = pts[ pi==1 ? 2 : 2*pi-2 ];
		p[7] = p[5] = pts[ pi==1 ? 3 : 2*pi-1 ];
	
		putbufe(&cd);
		if(OP('e')) { teklines(&cd); outbuf(); }
	} else ERRPR0(two points needed);
}
cmdcirc(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{
	int p[6];

	if( pi>=1 ) {
		cd.cmd = ARCS;
		cd.cnt = 8;
		cd.weight = wtcode(OP('w'));
		cd.style = stcode(OP('s'));
		cd.aptr = p;

		p[4] = p[0] = pts[ pi==1 ? 2 : 2*pi-2 ];
		p[5] = p[1] = pts[ pi==1 ? 3 : 2*pi-1 ];
		p[2] = p[0] - 2*(p[0]-pts[0]);
		p[3] = p[1] - 2*(p[1]-pts[1]);

		putbufe(&cd);
		if(OP('e')) { tekarc(&cd); outbuf(); }
	} else ERRPR0(two points needed);
}

cmdread(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	FILE *fp;
	char *cp, *cp1;
	int *p, *getfile(), c;

	for( cp=args; isspace(*cp)||*cp=='-'; )
		if( *cp++=='-' ) while( !isspace(*cp)&&*cp!='\0' ) cp++;
	for( cp1=cp+1; !isspace(*cp1)&&cp1!='\0'; cp1++ ) ;
	*cp1 = '\0';

	if( (fp=fopen(cp,"r"))!=NULL ) {
		if( (c=getc(fp))==FCH ) { /* null comment */
			getc(fp);
			IGNSIG;
			ct.change=TRUE;
			endbuf = getfile(fp,p=endbuf,dbuf+BUFSZ);
			CATCHSIG;
			if(OP('e')) dispbuf(p,endbuf,table);
		} else if( !feof(fp) && isascii(c) ) { /* text file */
			char *p;
			short sp;				/*for EOF compare (on 3B)*/
			for( p=TEXTBUF,*p++=c; p<(char *)dbuf+BUFSZ && (sp=getc(fp))!=EOF; *p=(char)sp, p++) ;
			*p = '\0'; /* unused part of dbuf buffers text */
			pts = readhelp(); /* get help from cmdproc */
			cmdtext(clo,pts,1,TEXTBUF,args,table);
		} else ERRPR1(%s not GPS or text,cp);
	} else ERRPR1(cannot read from %s,cp);
	return;
}
cmdwrite(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	FILE *fp;
	char *cp;
	int i,j;

	for( cp=args; isspace(*cp); cp++ ) ;
	if( (fp=fopen(cp,"w"))!=NULL ) {
		putc(037,fp); /* null */
		putc(0,fp); /* comment */
		j=(char *)endbuf - (char *)dbuf;
		if ((i=fwrite((char *)dbuf,sizeof(char),(unsigned)j,fp)) != j) {
			ERRPR1(output truncated to %d characters,i);
			perror(nodename);
		} else ct.change=0; /* flag buffer save */
		fclose(fp);
	} else ERRPR1(cannot write to %s,cp);
	return;
}
cmdnew(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	tekerase();
	dispbuf(dbuf,endbuf,table);
}
cmdpt(clo,pts,pi)
int clo[][NUMOPS], pts[], pi;
{
	struct area ara;
	if( OP('l') ) echolpts(); /* echo labelled points */
	else if( OP('u')||OP('v') ) points(&ct.w,dbuf,endbuf);
	else {
		window(pts,pi,&ara);
		points(&ara,dbuf,endbuf);
	}
}
cmdobj(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ara;
	if( OP('u')||OP('v') ) objects(&ct.w,dbuf,endbuf);
	else {
		window(pts,pi,&ara);
		objects(&ara,dbuf,endbuf);
	}
}
cmdzoom(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	if(OP('o')) outwindow(pts,pi);
	else setwindow(pts,pi);
	tekerase(); dispbuf(dbuf,endbuf,table);
}
cmdquit()
{
	if( ct.change && endbuf>dbuf ) { /* first time */
		printf(" ?\n");
		ct.change=0;
	} else {
		putchar('\n');
		exit(0);
	}
}
cmdcoord(clo,pts,pi,text,args)
int clo[][NUMOPS], pts[], pi;
char text[], args[];
{
	int x, y, i;

		printf("\nPT:\tUNIVERSE\tSCREEN\n\n");
		for( i=0; i<pi; i++ ) {
			x=pts[2*i]; y=pts[2*i+1];
			printf("%d:\t%d,%d\t",i,x,y);
			unscaleoff(&x,&y);
			printf("%d,%d\n",x,y);
		}
}
cmdview(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	int ar[4], dx, dy;
	double lx, ly;
	struct area ara;

	if(OP('u')) {
		setwindow(universe,2);
	} else if(OP('h')) {
		xymxmn(dbuf,endbuf,&ara);
		ar[0] = ara.lx;
		ar[1] = ara.ly;
		ar[2] = ara.hx;
		ar[3] = ara.hy;
		setwindow(ar,2);
	} else if( OP('r')>0 && OP('r')<26 ) { /* valid region */
		region(OP('r'),&lx,&ly);
		ar[0] = (int)lx;
		ar[1] = (int)ly;
		ar[2] = (int)lx+XDIM;
		ar[3] = (int)ly+YDIM;
		setwindow(ar,2);
	} else {
		dx = pts[pi+2]-pts[pi];
		dy = pts[pi+3]-pts[pi+1];

		ar[0] = clipu((long)ct.w.lx-dx);
		ar[1] = clipu((long)ct.w.ly-dy);
		ar[2] = clipu((long)ct.w.hx-dx);
		ar[3] = clipu((long)ct.w.hy-dy);
		setwindow(ar,2);
	}
	tekerase();
	dispbuf(dbuf,endbuf,table);
}
cmddel(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ar;

	if( OP('u') ) {
		tekerase();
		endbuf = dbuf;
	} else if( OP('v') ) {
		tekerase();
		deleteobj(&ct.w);
	} else {
		window(pts,pi,&ar);
		deleteobj(&ar);
	}
}
cmdcopy(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	int i, dx, dy;
	struct area ar;

	i = 2*pi;
	dx = pts[i+2] - pts[i]; i++;
	dy = pts[i+2] - pts[i];
	window(pts,pi,&ar);
	copy(dx,dy,&ar,table,(OP('p')?'p':'o'),OP('e'));

	for( i=0; i<pi; i++ ) {
		pts[i*2] = clipu(pts[i*2]+(long)dx);
		pts[i*2+1] = clipu(pts[i*2+1]+(long)dy);
	}
}
cmdmove(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	int i, dx, dy;
	struct area ar;

	i = 2*pi;
	dx = pts[i+2] - pts[i]; i++;
	dy = pts[i+2] - pts[i];
	window(pts,pi,&ar);
	move(dx,dy,&ar,table,(OP('p')?'p':'o'),OP('e'));

	for( i=0; i<pi; i++ ) {
		pts[i*2] = clipu(pts[i*2]+(long)dx);
		pts[i*2+1] = clipu(pts[i*2+1]+(long)dy);
	}
}
cmdscale(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ar;
	int *p, px, py;
	double f;

	window(pts,pi,&ar);
	px=pts[2*pi];
	py=pts[2*pi+1];
	f=OP('f')/100.;
	scalearea(px,py,&ar,f,OP('k'),OP('e'),table);
	f -= 1;

	for( p=pts; p<pts+2*pi; ) {
		*p = clipu((long)(*p+(*p-px)*f)); p++;
		*p = clipu((long)(*p+(*p-py)*f)); p++;
	}
}
cmdrot(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ar;
	int a, *p, px, py;

	window(pts,pi,&ar);
	rotarea(px=pts[2*pi],py=pts[2*pi+1],&ar,a=OP('a'),OP('k'),OP('e'),table);

	for( p=pts; p<pts+2*pi; ) {
		double x,y;
		x=p[0]; y=p[1];

		gslswing((double)px,(double)py,&x,&y,a);
		*p++ = clipu((long)x);
		*p++ = clipu((long)y);
	}
}
cmdedit(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ar, *arp = &ar;
	struct command cd;
	int *bptr=dbuf, *eptr;
	int *getgeds();

	if( OP('v') ) arp = &ct.w;
	else if( OP('u') ) arp = (struct area *) universe;
	else window(pts,pi,&ar);
	for( ; (eptr=getgeds(bptr,endbuf,&cd))!=NULL; bptr=eptr ) {
		if( inarea(*cd.aptr,*(cd.aptr+1),arp) ) {
			if(OPF('a')) cd.trot=OP('a');
			if(OPF('h')) cd.tsiz=OP('h');
			if(OPF('s')) cd.style=stcode(OP('s'));
			if(OPF('w')) { cd.weight=wtcode(OP('w')); cd.font=cd.weight<<4; }
			putbuf(bptr,eptr,&cd);
			if(OP('e')) dispbuf(bptr,eptr,table);
		}
	}
}
cmdx(clo,pts,pi,text,args,table)
int clo[][NUMOPS], pts[], pi;
char text[], args[], **table;
{
	struct area ar;

	if(OP('v')) drx(&ct.w,0,NARROW,DOTTED);
	else {
		if( pi==1 ) pi=2; /* use <cr> as 2nd point */
		if( window(pts,pi,&ar) )
			drx(&ar,0,NARROW,DOTTED);
		else ERRPR0(two points needed);
	}
}
