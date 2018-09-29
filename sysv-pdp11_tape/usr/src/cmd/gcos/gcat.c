/*	@(#)gcat.c	1.2	*/
#include	<stdio.h>

/*
 *  gcat -- spooler to send troff output to GCOS phototypesetter
 */

#define	SPTYPE	GCOSA
#define FIRSTCHAR 'G'-1
#define	NTEMP	4
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't', 'c', 'l'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*cfname = Xfname[2];
char	*lfname = Xfname[3];
char	*NAME;
#define	BF	'B'
#define	GCATSW	GCAT
#define	MAXCOPY	409600L

#include	"spool.c"

int photo, stare, text, dummy;
char *string, *str;

main(agc, agv)
int agc;
char *agv[];
{

	argc = agc;    argv = agv;
	setup();
	remotsw = TOSS;		/*default is to throw out accounting. MRW*/
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '1';

	while (argc>1 && (arg = argv[1])[0]=='-') {
		if(comopt(arg[1])) ;
		else switch (arg[1]) {

		case 'd':
			if( arg[2]=='u' ) { dummy=1;  remotsw = ONL; }
			else goto unknown;
			break;

		case 'p':
			if( arg[2]=='h' ) photo=1;
			else goto unknown;
			break;

		case 's':
			if( arg[2]=='t' ) stare=1;
			else goto unknown;
			break;

		case 't':
			if( arg[2]=='x' ) { text=1;  remotsw = ONL; }
			else if(!arg[2])
				remotsw = TOSS;
			    else goto unknown;
			break;

		case 'u':
			identf = getarg(arg[1]);
			break;

unknown:
		default:
			err("Unrecognized option: %s", arg);
			break;
		}
		argc--;
		argv++;
	}

	gcos1();		/*setup initial GCOS cards.*/

	if(gcdebug)
		card('L', "$	select	cc/test/gcat");
	else
		card('L', "$	select	cc/unixsupport/gcat");
	filargs();		/*process file arguments.*/

	if( photo || stare || text || dummy ) ;
	else photo=1;

	if(gcdebug)
		card('L', "$	select	cc/test/troff");
	else
		card('L', "$	select	cc/troff/select");
	card('L', "$	file	ot,x1r");
	card('L', "$	data	d*");
	str = string = " :trview ot*  -xx -xx -xx -xx";
	str += 15;
	if( photo ) {
		*str = 'p'; *(str+1) = 'h';
		str += 4;
	}
	if( stare ) {
		*str = 's'; *(str+1) = 't';
		str += 4;
	}
	if( text ) {
		*str = 't'; *(str+1) = 'x';
		str += 4;
	}
	if( dummy ) {
		*str = 'd'; *(str+1) = 'u';
		str += 4;
	}
	*(str-2) = '\0';
	card('L', string);

	gcos2();		/*put out final cards, and spool job.*/
}

nuact(name, temp)
char	*name, *temp;
{
	FILE *f;
	int c;

	if((f = fopen(temp, "r")) == NULL)
		errout("Can't open file %s.", temp);
	if((c = getc(f)) != 0)
		errout("Bad troff output on file %s.", name);
	fclose(f);
}


archive()
{
}
