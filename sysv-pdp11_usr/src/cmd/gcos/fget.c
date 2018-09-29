/*	@(#)fget.c	1.2	*/
/*	3.0 SID #	1.2	*/
#include	<stdio.h>

/*
 *  fget -- spooler to get file from GCOS
 */

#define	SPTYPE	FGET
#define FIRSTCHAR 't'-1
#define	NTEMP	4
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't', 'z', 'u'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*zfname = Xfname[2];
char	*ufname = Xfname[3];
char	*NAME;

#include	"spool.c"

extern	char	*fget_id;	/*FGET_ID must be defined in gcos.h*/
char	dname[] = "FGET";
char	*fzf;
FILE	*pfdes;
char	wdir[BUFSIZ];
FILE	*uff;
char	filname[CARDSIZE];		/*unix file name*/
int	uucpsw = 0;			/*retrieved file to be sent via uucp*/
int	namef = 0;
char	*fmode;
char	*strrchr(), *strchr();

main(agc, agv)
int agc;
char *agv[];
{
	extern FILE *popen();
	int fi, i, j, nslash, c;
	unsigned time;
	int poff;
	char *p, *ofilname;

	argc = agc;    
	argv = agv;
	if(config())
		errout("Error from config().");
	setup();
	remotsw = TOSS;		/*default is toss online output*/
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '1';
	fmode = "a";
	fi = 0;
	poff = 0;

	while (argc>1 && (arg = argv[1])[0]=='-') {
		if(!comopt(c = arg[1]))
			switch (arg[1]) {

			case 'a':
				fmode = "a";
				break;

			case 'b':
				fmode = "b";
				break;

			case 'd':
				arg = getarg('d');
				if ((fi = strlen(arg)+1) >= sizeof(filname)-2)
					errout("Name too long: -d %s", arg);
				strcpy(filname, arg);
				strcat(filname, "/");
				break;

			case 'u':
				strncpy(gcosid, getarg('u'), 12);
				gcosid[12] = 0;
				uidf = 1;
				break;

			default:
				err("Unrecognized option: %s", arg);
				break;
			}
		if(c == 'f')
			namef = 1;
		argc--;
		argv++;
	}

	gcos1();		/*setup initial GCOS cards.*/
	card('L',"$	select	cc/unixsupport/access");
	if(gcdebug)
		card('L',"$	select	cc/test/fget");
	else
		card('L',"$	select	cc/unixsupport/fget");
	card('L',fget_id);
	while(--argc) {
		arg = *++argv;
		if(mailfile == 0)
			mailfile = arg;
		/* extract a unix file name */

		if((p = strrchr(arg, '/')) == NULL){
			nslash = 0;
			p = arg;
		}else
			nslash = (int)p++;
		if(namef){
			namef = 0;
			p = mailfile;
		}
		i = sizeof(filname) - fi - 1;
		strncpy(filname+fi, p, i);
		if(strlen(p) > i)
			err("Truncated unix file name: %s", filname);
		if(strchr(filname, '!')){
			uucpsw = 1;
			if(ufname[INCHAR] < 'z')
				(ofilname = ufname)[INCHAR]++;
			else{
				err("Too many files; %s not fetched", filname);
				continue;
			}
			if(p = strchr(filname, '~'))
				*p = '^';	/*'~' is not BCD*/
		}else{
			uucpsw = 0;
			ofilname = filname;
		}

		/* create the ultimate output file */

		if(debug == 0 || access(ofilname, 0)){
			if((uff = fopen(ofilname, "w")) == NULL) {
				err("Cannot create %s",ofilname);
				continue;
			}
			chmod(ofilname,0666);	/*insure file writeable. MRW*/
			chown(ofilname, getuid(), getgid());
		}

		/* now make a link for the dpd */

		if(zfname[INCHAR] < 'z')    /*don't create file names past 'z'. MRW*/
			zfname[INCHAR]++;
		fzf = zfname;
		if(uucpsw || link(ofilname, fzf) != 0){
			if( *ofilname == '/'){
				fzf = ofilname; 
			}

			else{
				if(poff == 0){
					if(getcwd(wdir, sizeof(wdir)-3) == 0
						|| wdir[0] != '/')  
						errout("Cannot find cwd");
					strcat(wdir, "/");
					poff = strlen(wdir);
				}
				strcpy(wdir+poff, filname);
				if(link(wdir, fzf) != 0) fzf = wdir;
			}
		}

		sprintf(cardbuf, "%s %s%s%s %s%s%s",
				fmode,
				(nslash == 0 || *arg == '/') ? gcosid : "",
				(nslash == 0) ? "/" : "",
				arg,
				fzf,
				uucpsw ? ">" : "" ,
				uucpsw ? filname : "" );
		card('L', cardbuf);

		if(debug == 0){
			i = 2;
			while((putc(cardbuf[i++], uff)) != '\0') ;
			c = putc( '\n', uff );
				if(c == EOF && ferror(uff)){
					err("Error writing %s", ofilname);
					break;
				}
			fclose(uff);
		}
		nact++;

	}

	fflush(tff);
	if(fork() != 0){
		gcos2();		/*put out final cards, and spool job.*/
	}

	if(debug)
		exit(0);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	dfname[INCHAR]++;
	for( time = 4 ; time < 200 ; time += time ) {
		sleep(time);
		if(access(dfname, 0) != 0) break;
	}

	execl("/usr/lib/fget.demon", "fget.demon", 0);
	execl("/etc/fget.demon", "fget.demon", 0);
	err("Execl of fget.demon failed.");
	exit(1);
}
