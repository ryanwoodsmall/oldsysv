/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/aeparse.c	1.4"
/* <: t-5 d :> */

#include "s.h"
#include "stdio.h"
#include "ae.h"
#include <sys/types.h>
#include <sys/stat.h>
#define MAXS 50
#define MAXBUF 25
#define NAMELEN 13	/* /tmp/afXXXXXX */
#define MAXSOURCE (256+NAMELEN)

char *template = "/tmp/afXXXXXX";

struct precedence {
	char op;
	int forw;
	int back;
} ops[11] = { /* number of ops plus 1 */
	',',1,8,
	')',2,8,
	'+',3,3,
	'-',3,3,
	'*',4,4,
	'|',4,4,
	'%',4,4,
	'^',6,5,
	'~',6,5,
	'(',8,1,
	EOS,0,0,
};
int nops=10;

aeparse(in,p,pe,n,ne,verb) /* parse input from in and build postfix execution pgm */
char *in;
struct p_entry p[], *pe;
struct operand n[], *ne;
short verb;
{
	int stdinf; /* stdin assigned flag */
	int s[MAXS], *sp=s, t=0, next=0;
	char buf[MAXBUF], source[MAXSOURCE+1];
	char *strncpy();
	struct operand *nstore();

	in = strncpy(source,in,MAXSOURCE); in[MAXSOURCE]='\0';
	prescan(in,verb); /* make all operators unique, execute functions */
	*sp = EOS; /* init stack */
	stdinf = 0; /* stdin not assigned */
	for(;;) {
		if(!t) t=nexttoken(buf,&in,FALSE);
		if( t&OPN ) { /* buf contains operand */
			if( (p->opn=nstore(buf,n,ne,next,&stdinf))!=NULL )
				p++->opr = OPN;
			t = 0; next = 0;
		}
		else { /* buf contains operator */
			if( buf[0]=='\'' ) next++, t=0; /* duplicate calls to next opn */
			else if( !p_ge(sp,buf) ) {
				if( !pushi(&sp,s+MAXS,*buf) ) {
					ERRPR0(too many tokens in expression);
				}
				t = 0;
			}
			else if( reduce(s,&sp,&p,pe)==EOF ) return(0);
		}
	}
}

prescan(bp,verb) /* change unary minus to ~, execute functions */
char *bp;
short verb;
{
	char buf[MAXBUF], *lastp, *p;
	int change, t;

	for( p=bp; *p!=EOS; p++ ) /* use | for division */
		switch(*p) {
		case '\\': if(*++p==EOS) p--;
		case '/': *p='|';
		}

	for( change=TRUE, lastp=bp;
	 (t=nexttoken(buf,&bp,TRUE))!=EOS; lastp=bp)
		switch(t) {
		case OPR: if( *buf=='-' && change ) *(bp-1)='~';
			else if( *buf==')' ) change=FALSE;
			else change=TRUE;
			break;
		case IDENT: {
			char *p=bp;
			nexttoken(buf,&p,TRUE); /* look ahead */
			if( *buf=='(' ) bp = p - 1, getandgo(lastp,&bp,verb);
		} /* fall through */
		default: change=FALSE;
		}
}
reduce(s,sp,pp,pe) /* reduce nonterminal on stack to code on pp */
int *s, **sp;
struct p_entry **pp, *pe;
{
	switch(**sp) {
	case ')': while( --*sp>s && **sp!='(' ) ;
		if( **sp=='(' ) --*sp;
		else ERRPR0(missing paren assumed);
		break;
	case '(': ERRPR0(missing paren assumed);
		--*sp; break;
	case EOS: return EOF;
	default: (*pp)->opr = **sp;
		if( *pp<pe ) ++*pp;
		else ERRPR0(too many tokens in expression);
		--*sp;
	}
	return(0);
}

struct operand *
nstore(name,n,ne,next,stdinf) /* put name in opn list */
char *name;
struct operand *n, *ne;
int next, *stdinf;
{
	FILE *tfd=NULL;
	struct stat statbuf;

	for( ; n<ne; n++)
		if( strcmp(name,n->name)==0 ) { /* operand in list */
			if( next--<=0 ) return(n);
			else tfd = n->fd; /* save fd for duplication */
		}
		else if( n->name[0]=='\0' ) { /* not in list */
			if( type(*name)==IDENT ) { /* opn is filename */
				if( tfd!=NULL ) n->fd = tfd; /* dup pointer */
				else if( (n->fd=fopen(name,"r") ) == NULL)
					if(!(*stdinf)++) { /* assign stdin */
						n->fd = stdin;
						ERRPR1(cannot open %s -- using standard input,name);
					} else {
						ERRPR1(cannot open %s,name);
						return(NULL);
				} else {
					stat(name, &statbuf);
					if((statbuf.st_mode & S_IFMT) == S_IFDIR){
						ERRPR1(argument %s should not be a directory, name);
/*						return(NULL);	*/
					}
				}
				strcpy(n->name,name);
			}
			else { /* opn is constant */
				n->fd = NULL;
				strcpy(n->name,name);
				n->val = atof(name);
			}
			return(n);
		}
	ERRPR1(too many operands in expression\, %s ignored,name);
	return(NULL);
}

nexttoken(buf,p,silent) /* put next token pointed to by p into buf */
char *buf, **p;
{
	int i;

	while( isspace(**p) ) ++*p; /* skip over spaces */
	switch( i=type(**p) ) {
	case EOS: *buf++ = EOS; break;
	case OPR: *buf++ = *(*p)++; break;
	case IDENT: while( ISID(**p) || **p=='/' ) *buf++ = *(*p)++;
			break;
	case CONST: while( ISNUM(**p) ) *buf++ = *(*p)++;
			break;
	default: if(!silent) ERRPR1(? %c,*(*p)++); break;
	}
	*buf = '\0';
	return(i);
}

type(c) /* return type of char */
char c;
{
	if( c<' ' ) return EOS;
	switch(c) {
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '|':
		case '\'':
		case '^':
		case '~':
		case '%': return OPR;
		case '.': return CONST;
		case '_': return IDENT;
		case '/': return IDENT;
		}
	if( c>='0' && c<='9' ) return CONST;
	if( c>='A' && c<='Z' ) return IDENT;
	if( c>='a' && c<='z' ) return IDENT;
	return UNKNOWN;
}

p_ge(sp,next) /* precedence greater than test between top operator
		on stack and next operator from input */
int *sp;
char *next;
{
	int s, n;

	for( s=0; ops[s].op!=*sp && s<nops; s++ ) ;
	for( n=0; ops[n].op!=*next && n<nops; n++ ) ;
	return (ops[s].back>=ops[n].forw);
}

pushi(sp,limit,val) /* put val on stack at sp */
int val, **sp, *limit;
{
	if( *sp<limit-1 ) { *++*sp=val; return(1); }
	else return(0);
}

getandgo(np,ap,verbose) /* get and execute a function operand */
char *np, **ap;
short verbose;
{
	char xbuf[2*MAXBUF], *p, name[NAMELEN+1];
	char *move();
	int open, i, code;

	for( i=0; np+i<*ap; i++ ) xbuf[i]=np[i]; /* copy name */
	xbuf[i]=' ';

	for( open=1,p= *ap+1; open>0; p++ ) { /* copy arguments */
		switch(*p) {
		case '(': open++; break;
		case ')': open--; break;
		case EOS: open=0; break;
		}
		xbuf[++i] = *p;
	}

	xbuf[i++] = '>'; xbuf[i]=EOS;
	strcpy(name,template); mktemp(name);
	strcat(xbuf,name);
	if(verbose) fprintf(stderr,"%s\n",xbuf);
	if( (code=system(xbuf))!=0 )
		ERRPR2(%s  error code %d returned,xbuf,code);

	*ap = move(p,NAMELEN-(p-np)); /* make room for temp name */
	for( i=0; name[i]!=EOS; i++ ) *np++ = name[i]; /* replace function by temp file */
}
static char *
move(start,amt) /* move string at p by amt */
char *start;
int amt;
{
	char *p=start;

	if( amt>0 ) { /* move to the right */
		while( *p != EOS ) p++;
		for( ; p>=start; p-- ) p[amt] = p[0];
	} else { /* move to the left */
		for( ; *p!=EOS; p++ ) p[amt] = p[0];
		p[amt]=EOS;
	}
	return(start+amt);
}
rmtmps() /* remove temp files with this processes id */
{
	char name[NAMELEN+1], remove[NAMELEN+4] ;
	char s[6];

	strcpy(name,"/tmp/af000000");
	strcpy(remove,"rm ");

	sprintf(s,"%d",getpid());
	strcpy(name+8+(5-strlen(s)),s);
	if(fopen(name,"r")) { 
		name[7] = '?';
		strcat(remove,name);
		system(remove);
	}
}
