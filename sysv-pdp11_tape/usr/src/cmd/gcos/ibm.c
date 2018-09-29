/*	@(#)ibm.c	1.2	*/
/*
 *	ibm -- off line submit job to HO 370
 */
#include <stdio.h>

#define	SPTYPE	GCOSB
#define FIRSTCHAR 'i'-1
#define	NTEMP	2
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*NAME;

#include	"spool.c"

int	card1 = 1;
int	jcl = 0;
char	*pid;

main(agc, agv)
int agc;
char *agv[];
{
	int ibox;
	FILE	*f;

	argc = agc;    argv = agv;
	setup();
	remotsw = TOSS;
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '1';
	pid = &Xfname[0][INCHAR+1];

	while (argc>1 && (arg = argv[1])[0]=='-') {
	    if(!comopt(arg[1]))
		switch (arg[1]) {
		case 'j':
			jcl++;
			break;

		default:
			err("Unrecognized option %s", arg);
			break;
		}
		argc--;
		argv++;
	}
	gcos1();

	card('L', "$	select	cc/asprdr");
	card('L', "$	data	01,ncksum,copy");
	if (jcl==0) {
		idcard[iname-1] = '\0';
		ibox = iacct;
		while(idcard[ibox++] != ',') ;
		card('L', "//%s%s job (%s),%s",
		   &idcard[iname], pid, &idcard[iacct], &idcard[iname]);
		card('L', "//*send dest=mh,bin=%s", &idcard[ibox]);
		card('L', "//*main failure=restart,lines=1");
		card('L', "//*main grade=x,time=1");
	}
	if(argc == 1) {
		copy(stdin, "pipe.end");
	}
	while(--argc) {
		arg = *++argv;
		if((f = fopen(arg, "r")) == NULL)
			errout("Cannot open %s", arg);
		copy(f, arg);
		fclose(f);
	}
	card('L', "/*");
	card('L', "$      endcopy");
	printf("job %s%s\n", &idcard[iname], pid);

	gcos2();
}

copy(f, p)
FILE	*f;
char	*p;
{
	char *bp;

	if(card1){
		if((fgets(cardbuf, CARDSIZE, f) == NULL) ||
			((bp = strchr(cardbuf, '\n')) == NULL))
			errout("First line too long in file %s.", p);
		*bp = 0;
		if((bp = strchr(cardbuf, '?')) == NULL)
			card('L', cardbuf);
		else{
			*bp++ = '\0';
			card('L', "%s%s%s", cardbuf, pid, bp);
		}
		card1 = 0;
	}
	copyfile(f, p);
}
