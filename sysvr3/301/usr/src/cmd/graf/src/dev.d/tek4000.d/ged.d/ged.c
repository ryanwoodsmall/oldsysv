/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/ged.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "ged.h"
#include "setop.h"
/* #include "errpr.h" done by include ged.h */
#include "chtable"
struct control ct = { FALSE, TRUE };
struct options {
	int eopt,Ropt;  /*  erase,restricted shell opts  */
} cop = {TRUE,FALSE};

int dbuf[BUFSZ],*endbuf=dbuf;
char *nodename;
jmp_buf env; /* environment save ptr for interrupt restore */

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fpi, *rstdin=NULL, *fdopen(), *freopen();
	char *cp,**av;
	int ac;

	static int pts[4],pi=2;
	struct area ar;
	int r, interr();
	char ch, *termfp, *ttyname();
	double dx,dy;

	nodename= *argv;
	if( (termfp=ttyname(fileno(stderr))) == NULL ) {
		ERRPR0(cannot redirect stderr); exit(1);
	}
	if(ttyname(fileno(stdin))==NULL) { /*  not from terminal  */
		rstdin= fdopen(dup(fileno(stdin)),"r"); /* copy old stdin to new descriptor */
		if(freopen(termfp,"r",stdin)==NULL) { ERRPR0(cannot make terminal stdin); exit(1); }
	}
	tekinit();
	region(13,&dx,&dy);
	pts[0]=(int)dx; pts[1]=(int)dy;
	pts[2]=(int)dx+XDIM; pts[3]=(int)dy+YDIM;
	for(ac=argc-1,av=argv+1; ac>0 ;ac--,av++){
		if( *(cp=av[0])=='-' && *++cp!=EOS){
			while(*cp != EOS){
				switch(*cp++){
				case 'u':
					pts[0]=XYMIN; pts[1]=XYMIN;
					pts[2]=XYMAX; pts[3]=XYMAX;
					setwindow(pts,pi);
					break;
				case 'r':  SETINT(r,cp);
					region(r,&dx,&dy);
					pts[0]=(int)dx; pts[1]=(int)dy;
					pts[2]=(int)dx+XDIM; pts[3]=(int)dy+YDIM;
					setwindow(pts,pi);
					break;
				case 'e':  cop.eopt=FALSE; break;
				case 'R':  cop.Ropt++; break;
				case ' ': case',': break;
				default:
					cop.eopt=FALSE;
					ERRPR1(%c?,*(cp-1));
					break;
				}
			}
			--argc;
		}
	}
	if(cop.eopt) tekerase();
	while(argc-->1){
		if(strcmp(*++argv,"-")==0) fpi=rstdin;
		else if( **argv != '-') {
			if((fpi=fopen(*argv,"r"))==NULL) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( getc(fpi)==FCH ) { getc(fpi); initread(fpi); }
		else ERRPR0(input not GPS);
		fclose(fpi);
		if(fpi==stdin) fopen(ttyname(fileno(stdout)),"r");
	}

	if(endbuf>dbuf && ct.wratio==0){
		xymxmn(dbuf,endbuf,&ar);
		pts[0]=ar.lx; pts[1]=ar.ly;
		pts[2]=ar.hx; pts[3]=ar.hy;
		setwindow(pts,pi);
	}
	if(ct.wratio==0) setwindow(pts,pi);
	else dispbuf(dbuf,endbuf,table);
	ct.change = FALSE;

	setjmp(env);
	CATCHSIG;
	while( putchar('*'), (ch=getcraw())!='Q' )
		if( ch=='!' ) putchar('!'), unixesc(cop.Ropt);
		else if( cmdproc(ch,table)==1 )
			printf("?\n");
	printf("Q\n");
	exit(0);
}

initread(fpi)
FILE *fpi;
{
	int *getfile();
	endbuf = getfile(fpi,endbuf,dbuf+BUFSZ);
	fclose(fpi);
}
interr()
{
	IGNSIG;
	tekclr();
	setmode(ALPHA);
	putchar('\n');
	longjmp(env,0);
}
