/*	@(#)dpr.c	1.3	*/
#include	<stdio.h>

/*
 *  dpr -- spooler for off line print via dataphone daemon to GCOS
 */

#define	SPTYPE	GCOSA
#define FIRSTCHAR 'A'-1
#define	NTEMP	4
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't', 'c', 'l'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*cfname = Xfname[2];
char	*lfname = Xfname[3];
char	*NAME;
#define	BF	'F'
#define	GCATSW	0
#define	MAXCOPY	204800L

#include	"spool.c"

int	indent	= 8;		/*amount to indent line on printer. MRW*/
int	portrait = 0;		/* flag for xerox portrait mode */

main(agc, agv)
int agc;
char *agv[];
{
	argc = agc;    argv = agv;
	setup();
	remotsw = ONL;
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '2';

	while (argc>1 && (arg = argv[1])[0]=='-') {
		if(arg[1] != 's' && arg[2] && !arg[3]){
			remote[12] = arg[1];
			remote[13] = arg[2];
			remote[14] = 0;
		} else if(comopt(arg[1])) ;
		else switch (arg[1]) {

		case '-':
			remote[12] = 'r';
			remote[13] = '1';
			remote[14] = '\0';
			break;

		case '0':		/*allow indent of 0. MRW*/
			indent = 0;
			break;

		case 'p':		/* portrait mode for xerox printers */
			portrait = 1;
			break;

		default:
			err("Unrecognized option: %s", arg);
			break;
		}
		argc--;
		argv++;
	}

	gcos1();		/*setup initial GCOS cards.*/
	if (portrait) {
		card('L', "$	select	cc/command/sx9700");
		card('L', "sx9700 mode=portrait");
	}
	if(indent == 0)		/*set GCOS switch 32 for no indent. MRW*/
		card('L', "$	set	32");
	if(gcdebug)
		card('L', "$	select	cc/test/dpr");
	else
		card('L', "$	select	cc/unixsupport/dpr");
	card('L', "$	data	i*,ncksum,copy");

	filargs();		/*process file arguments.*/
	card('L', "$	endcopy");

	gcos2();		/*put out final cards, and spool job.*/
}


archive()
{
}


nuact()
{
}
