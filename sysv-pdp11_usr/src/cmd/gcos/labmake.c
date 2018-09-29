/*	@(#)labmake.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*  lablist and labmake commands */

#include <stdio.h>

#define	SPTYPE	GCOSB
#define FIRSTCHAR 'l'-1
#define	NTEMP	2
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*NAME;

#include	"spool.c"

FILE *fin;

#define NO	0
#define YES	1
#define FIRST 12
int lablist ;

struct par{
	char *name;
	char *value;
}  pars[20];



main(agc, agv)
int agc;
char *agv[];
{
	int i, unit;
	char *p, *q, *val();

	argc = agc;    argv = agv;
	setup();
	remotsw = ONL;
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '2';
	for(p=q=argv[0] ; *p!= '\0' ; ++p)
		if(*p == '/') q = p+1;
	lablist = ! strcmp(q, "lablist");

	setval("copies", "1");
	setval("lines", "6");
	setval("forms", "avery");
	setval("each", "1");
	setval("sel", "");
	fin = stdin;

	for(i=1; i<argc; ++i)
		if((arg = argv[i])[0]=='-'  &&  argv[i][1]!='\0'){
		    if(!comopt(arg[1]))
			switch(arg[1]){
			case 'l':
				lablist = YES;
				break;

			default:
				errout("Bad flag %s", arg);
			}
			argv[i] = 0;
			if (lablist == YES)
				remotsw = ONL;
		}

		else{
			for(p = argv[i] ; *p!='\0' ; ++p)
				if(*p == '='){
					*p = '\0';
					setval(argv[i], p+1);
					argv[i] = 0;
					break;
				}
		}

	gcos1();

	card('L', "$	OPTION	FORTRAN");
	card('L', "$	SELECT	./LAB.OBJ");

	if(lablist)
		card('L', "$	EXECUTE	ON1");
	else
		card('L', "$	EXECUTE");

	card('L', "$	LIMITS	10,12K");
	card('L', "$	FILE	P*,NULL");
	card('L', "$	SYSOUT	06");
	if(lablist)
		card('L', "$	SYSOUT	42");
	else{
		card('L', "$	MASS	42,X20S,10L");
		card('L', "$	FFILE	42,NOSLEW");
	}

	card('L', "$	DATA	05");
	card('L', "%s-lines", val("lines"));
	card('L', "%s", val("sel") );

	unit = FIRST;
	for(i=1 ; i<argc ; ++i)
		if(argv[i] != 0)
			card('L', "*%d %s", unit++ , argv[i]);

	if(unit == FIRST)
		card('L', "*%d standard input", FIRST);

	unit = FIRST;
	for(i=1 ; i<argc; ++i)
		if(argv[i] != 0){
			card('L', "$	DATA	%d", unit++);
			if(argv[i][0]=='-' && argv[i][1]=='\0')
				fin = stdin;
			else if( (fin = fopen(argv[i],"r")) == NULL)
				errout("Cannot open file %s", argv[i]);

			copyfile(fin, argv[i]);
			fclose(fin);
		}

	if(unit == FIRST){
		card('L', "$	DATA	%d", FIRST);
		copyfile(stdin, "standard input");
	}

	if(!lablist){
		if( (p = val("nc")) || (p = val("copy")) )
			setval("copies", p);

		card('L', "$	SELECT	CC/LABEL1");

		card('L', "%s-each  %s  %s-lines  %s-copies",
			val("each"), val("forms"),
			val("lines"), val("copies") );

		card('L', "$	MASS	IN,X20D");
		card('L', "$	SELECT	CC/LABEL2");
	}

	gcos2();
}


char *val(s)
register char *s;
{
register struct par *p;

	for(p=pars; p->name!=0 ; ++p)
		if(! strcmp(p->name, s))
			return(p->value);

	return(0);
}


setval(n,v)
char *n, *v;
{
register struct par *p;

	for(p=pars ; p->name!=0 && strcmp(p->name,n) ; ++p)
		;

	p->name = n;
	p->value = v;
}
