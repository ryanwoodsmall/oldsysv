/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/gtop.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "debug.h"
#include "errpr.h"
#include "setop.h"
#include "util.h"
#include "gsl.h"
#include "gpl.h"
#define MAP(x)   (x/2 + 16383)  /*  plot space requires mapping \
			into 32k by 32k space  */
#define TRANS(cd)    if(cd.cmd==LINES || cd.cmd==ARCS) cnt=cd.cnt-2; \
  else cnt=2;  if(cd.cmd==COMMENT)cnt=0; for(lptr=cd.aptr;cnt>0;cnt-=2,lptr+=2){\
  *lptr=MAP(*lptr); *(lptr+1)=MAP(*(lptr+1));}
char *styles[] ={	"solid",
				"dotted",
				"dotdashed",
				"shortdashed",
				"longdashed"
};
struct area ar={0,0,0,0};
char string[TEXLEN];
char * nodename;

main(argc,argv)
int argc;
char *argv[]; 
{	int ac,argf,i;
	char **av,*cp;
	FILE *fpi;
	int reg;
	double dx,dy;

/*
 *  gtop translates ged___ graphic files to plot format
 */
	nodename = *argv;
	erase();
	ar.lx=ar.ly=ar.hx=ar.hy=0;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!=EOS ) {
			for(; *cp!=EOS; --argc )
				switch(*cp++) {
				case 'r':
					SETINT(reg,cp);
					region(reg,&dx,&dy);
					ar.lx=dx;  ar.hx=dx + XDIM;
					ar.ly=dy;  ar.hy=dy + YDIM;
					break;
				case 'u':
					ar.lx=ar.ly=XYMIN;
					ar.hx=ar.hy=XYMAX;
					break;
				default: ERRPR1(%c?,*(cp-1));
				}
		}

	if(argc <= 1) { argc=2; argf=0; } /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fpi=stdin;
		else if( **argv!='-' ) {
			if( (fpi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( getc(fpi)==FCH ) {
			getc(fpi);
			gtop(fpi);
		} else ERRPR0(input not GPS);
		fclose(fpi);
	}
}
gtop(fpi)
FILE *fpi; 
{	FILE *windowon();
	struct command cd;
	int style,*lptr,cnt;
	int clipseg[4], moved, i;

	if(ar.lx==0 && ar.ly==0 && ar.hx==0 && ar.hy==0){
		fpi=windowon(fpi,&ar);
	}
	ar.lx=MAP(ar.lx); ar.ly=MAP(ar.ly);
	ar.hx=MAP(ar.hx); ar.hy=MAP(ar.hy);
	setspace(&ar);
	space(ar.lx,ar.ly,ar.hx,ar.hy);
	move(ar.lx,ar.ly);
	/*label("\033\073");*/		/*  small text on 4014  */
	for(style=0;getgedf(fpi,&cd) != EOF;){
		TRANS(cd);
			
		switch (cd.cmd) {

/**********************************************/
/**********************************************/

		case ARCS:
if(cd.cnt==8 )
 {
  if( inarea(*cd.aptr, *(cd.aptr+1), &ar))
   {
    lstyle( &cd, &style );
    plotarc(*cd.aptr,*(cd.aptr+1),
    *(cd.aptr+2),*(cd.aptr+3),*(cd.aptr+4),*(cd.aptr+5));
   }
  break;
 }

/***********************************************/
/***********************************************/

		case LINES:

for( i=0, moved=0; !moved && i<cd.cnt-4; i+=2 )
 {
  moved=clipper( cd.aptr+i, ar.lx, ar.hx, ar.ly, ar.hy, clipseg );
 }

if( moved )
 {
  lstyle( &cd, &style );
  move( clipseg[0], clipseg[1] );
  cont( clipseg[2], clipseg[3] );

  for( ; i<cd.cnt-4; i+=2 )
   {
    if( clipper( cd.aptr+i, ar.lx, ar.hx, ar.ly, ar.hy, clipseg ))
     {
      if( *(cd.aptr+i)!=clipseg[0] || *(cd.aptr+i+1)!=clipseg[1] )
       {
        move( clipseg[0], clipseg[1] );
       }
      cont( clipseg[2], clipseg[3] );
     }
   }
  }
			break;

/***********************************************/
/***********************************************/

		case TEXT:
		case ALPHA:
			if(inarea(*cd.aptr,*(cd.aptr+1),&ar)){
				lstyle( &cd, &style );
				move(*cd.aptr,*(cd.aptr+1));
				label(cd.tptr);
			}
			break;
		}
	}
	move(ar.lx,ar.ly);
}

lstyle( cd, oldstyle )

struct command *cd;
int            *oldstyle;

{
if((unsigned) cd->style > LONGDASH) cd->style = SOLID;
if(cd->style != *oldstyle) {
#if u370 | u3b | u3b5 | u3b2
  linemod(styles[cd->style]);
#else
  linemod(styles[cd->style]);
#endif
  *oldstyle=cd->style;
 }
}
