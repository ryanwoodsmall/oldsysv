/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/vtoc.c	1.2"
/*<:t-5 d:>*/
#include <stdio.h>
#include "vtoc.h"

struct node *nd[MAXNODE];	/*  node declared in file vtoc.h  */
int lastnode;
double xsize,ysize,xspace=0.5,yspace=1.0;
int iopt = 1;		int copt = 0;
int kopt = 1;		int sopt = 1;
int dopt = 0;
char *nodename;

main(argc,argv) 
int argc;
char *argv[];
{	char *c;
	char command[NAMELEN],*tempfile="/tmp/vtoc.XXXXXX";
	FILE *fpi,*fpo=stdout,*fopen();
	double atof();
	int vcmp(),i,level_deep,level_wide;

	int ac, argf;
	char **av;
	nodename= *argv;
	for(ac=argc-1, av=argv+1; ac > 0; ac--, av++){
		if(*(c=av[0]) == '-' && *++c != '\0'){
			while (*c != '\0')
				switch (*c++) {
				case 'd' :		/* diagonal lines */
					dopt = 1;
					break;
				case 'i' :			/*  no id  */
					iopt = 0;
					break;
				case 'm' :			/*  no mark  */
					kopt = 0;
					break;
				case 'c':			/*  not all upper case  */
					copt = 1;
					break;
				case 's':			/*  enable squisher  */
					sopt = 0;
					break;
				case 'h':			/* horizontal interbox space */
					SETDOUB(xspace,c);
					xspace*=.01;
					break;
				case 'v':			/* vertical interbox space */
					SETDOUB(yspace,c);
					yspace*=.01;
					break;
				case ' ':
					break;
				case ',':
					break;
				default:
					ERRPR1(%c?,*(c-1));
					break;
				}
			argc--;
		}
	}
	if( argc==1 ) argc=2, argf=0; /* no args, use stdin */
	else argf=1;
	while (argc-- > 1){
		if(strcmp(*++argv,"-") == 0 || !argf) *argv="-";
		else if(**argv != '-') {
			if(access(*argv,0) != NULL){
				ERRPR1(cannot access %s, *argv);
				continue;
			}
		} else	{ argc++; continue; }

		mktemp(tempfile);
		sprintf(command,"cat %s|/usr/bin/graf/remcom>%s",*argv,tempfile);
		system(command);
		if((fpi=fopen(tempfile,"r")) == NULL) {
			ERRPR1(cannot open %s,tempfile);
			continue;
		}
		else	{
			if(treebldr(fpi) == FAIL){
				ERRPR0(Too much text/Too many nodes.);
				exit(1);
			}
			if(lastnode < 0){
				ERRPR0(Bad input file.);
				exit(1);
			}
			fclose(fpi);
			unlink(tempfile);
			qsort(nd,lastnode+1,sizeof(nd[0]),vcmp);
			chcknds();
			setkids();
			position();
			if(sopt){
				getmax(&level_deep,&level_wide);
				if(squish(level_deep,level_wide) == FAIL)
					ERRPR0(Too much text/Too many nodes to squish);
			}
			vmap();
			gpsfmt(fpo);
			exit(0);
		}
	}
}
