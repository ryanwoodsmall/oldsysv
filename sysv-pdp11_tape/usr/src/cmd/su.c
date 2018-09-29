/*	@(#)su.c	1.1	*/
/*
 *	su [-] [name [arg ...]] change userid, `-' changes environment
 *	if SULOG is defined, all attemts to su to uid 0 are logged there.
 *	if CONSOLE is defined, all successful attempts are also logged there.
 */
#include <stdio.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>
#define SULOG	"/usr/adm/sulog"
#define PATH	"PATH=:/bin:/usr/bin"
#define SUPATH	"PATH=/bin:/etc:/usr/bin"
#define ELIM 128
long time();
void pause();
struct	passwd *pwd, *getpwnam();
struct	tm *localtime();
char	*malloc(), *strcpy();
char	*getpass(), *ttyname(), *strrchr();
char	*shell = "/bin/sh";
char	su[16] = "su";
char	homedir[64] = "HOME=";
char	logname[20] = "LOGNAME=";
char	*path = PATH;
char	*supath = SUPATH;
char	*envinit[ELIM];
extern	char **environ;
char *ttyn;

main(argc, argv)
char	**argv;
{
	char *nptr, *password;
	char	*pshell = shell;
	int badsw = 0;
	int eflag = 0;
	int uid, gid;
	char *dir, *shprog, *name;

	if (argc > 1 && *argv[1] == '-') {
		eflag++;
		argv++;
		argc--;
	}
	nptr = (argc > 1)? argv[1]: "root";
	if((pwd = getpwnam(nptr)) == NULL) {
		fprintf(stderr,"Unknown id: %s\n",nptr);
		exit(1);
	}
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;
	dir = strcpy(malloc(strlen(pwd->pw_dir)+1),pwd->pw_dir);
	shprog = strcpy(malloc(strlen(pwd->pw_shell)+1),pwd->pw_shell);
	name = strcpy(malloc(strlen(pwd->pw_name)+1),pwd->pw_name);
	if((ttyn=ttyname(0))==NULL)
		if((ttyn=ttyname(1))==NULL)
			if((ttyn=ttyname(2))==NULL)
				ttyn="/dev/tty??";
	if(pwd->pw_passwd[0] == '\0' || getuid() == 0 )
		goto ok;
	password = getpass("Password:");
	if(badsw || (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0)) {
#ifdef SULOG
		log(SULOG, nptr, 0);
#endif
		fprintf(stderr,"Sorry\n");
		exit(2);
	}
ok:
	endpwent();
#ifdef SULOG
		log(SULOG, nptr, 1);
#endif
	if((setgid(gid) != 0) || (setuid(uid) != 0)) {
		printf("Invalid ID\n");
		exit(2);
	}
	if (eflag) {
		strcat(homedir, dir);
		strcat(logname, name);
		envinit[2] = logname;
		chdir(dir);
		envinit[0] = homedir;
		if (uid == 0)
			envinit[1] = supath;
		else	envinit[1] = path;
		envinit[3] = NULL;
		environ = envinit;
		strcpy(su, "-su");
	}

	if (uid == 0)
	{
#ifdef CONSOLE
		if(strcmp(ttyn, CONSOLE) != 0) {
			signal(SIGALRM, to);
			alarm(30);
			log(CONSOLE, nptr, 1);
			alarm(0);
		}
#endif
		if (!eflag) envalt();
	}
	if (argc > 2) {
		if(shprog[0] != '\0') {
			fprintf(stderr,"Non-standard shell - denied\n");
			exit(2);
		}
		argv[1] = su;
		execv(shell, &argv[1]);
	} else {
		if(shprog[0] != '\0') {
			pshell = shprog;
			strcpy(su, eflag ? "-" : "");
			strcat(su, strrchr(pshell,'/') + 1);
		}
		execl(pshell, su, 0);
	}
	fprintf(stderr,"No shell\n");
	exit(3);
}

envalt()
{
int i, pset=0, j=0;
char **eptr=environ;

	for(i=0;i<130;i++)
	{
		if(*eptr == (char *)0) break;
		if(strncmp(*eptr,"PATH=",5)==0)
		{
			envinit[i-j]=supath;
			pset++;
		}
		else if(strncmp(*eptr,"PS1=",4)==0)
			j++;
		else
  			envinit[i-j] = *eptr;

		eptr++;
	}

	if(!pset)
	{
		i = (i < ELIM-2) ? i : ELIM-2;
		envinit[i++]=supath;
	}
	envinit[((i < ELIM-1) ? i : ELIM-1)]=(char *)0;

	environ = envinit;
}
log(where, towho, how)
char *where, *towho;
int how;
{
	FILE *logf;
	long now;
	struct tm *tmp;

	now = time(0);
	tmp = localtime(&now);
	if((logf=fopen(where,"a")) == NULL) return;
	fprintf(logf,"SU %.2d/%.2d %.2d:%.2d %c %s %s-%s\n",
		tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,
		how?'+':'-',(strrchr(ttyn,'/')+1),cuserid((char *)0),towho);
	fclose(logf);
}
void to(){}
