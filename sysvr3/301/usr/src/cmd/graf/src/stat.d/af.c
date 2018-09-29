/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/af.c	1.3"
/* <: t-5 d :> */
#include "stdio.h"
#include "s.h"
#include "ae.h"
char *nodename;
#define MAXPFIX 25
#define MAXOPN 20

struct options {
	int colset;	/* columns per output line */
	short verb;	/* when true, function calls are expanded */
} op = { 0, 0 };

main(argc,argv)
int argc;
char *argv[];
{
	int ac, i;
	char **av, *cp;

	struct operand n[MAXOPN];
	struct p_entry pfix[MAXPFIX];
	int col;
	char lab[MAXLABEL];
	double x, aeeval();

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			int done;
			for( done=FALSE; *cp!='\0' && !done; )
				switch(*cp++) {
				case 'c': SETINT(op.colset,cp); *av[0]='\0'; break;
				case 't': TITOUT0(stdin,lab,MAXLABEL); *av[0]='\0'; break;
				case 'v': op.verb=1; *av[0]='\0'; break;
				case ' ': case ',': break;
				default: if( cp == av[0]+2 ) argc++, done++; /* no option */
					else ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	for( ; argc>1; argc--,argv++) {
		if( *argv[1]=='\0' ) { argc++; continue; } /* skip option strings */

		if(op.colset>0) col = op.colset; /* set columnation */
		else if( (i=charcnt(argv[1],','))>=1 ) col = i+1;
		else col = DFLTCOL;

		for(i=0; i<MAXOPN; i++) n[i].name[0]='\0';
		if( aeparse(argv[1],pfix,pfix+MAXPFIX,n,n+MAXOPN,op.verb)==0 ) {
			for(i=1; getopn(n,n+MAXOPN) || i==1; i++) {
				x = aeeval(pfix,n,&i,col);
				PRINT(x,i,col);
			}
		}
		if(i%col != 1) putchar('\n');
		rmtmps(); /* remove temp files (func in aeparse) */
	}
}

getopn(n,ne) /* get operand values, put into opn list */
struct operand *n, *ne;
{
	int more=0;
	for( ; n->name[0]!='\0' && n<ne; n++)
		if(n->fd) { /* fd nonzero for file opns */
			if(getdoub(n->fd,&n->val)==EOF) return(0); /* EOF */
			more = 1;
		}
	return(more);
}

charcnt(p,c) /* return count of c in p */
char *p, c;
{
	int cc = 0;

	for( ; *p!='\0'; p++)
		if(*p==c) cc++;
	return(cc);
}
