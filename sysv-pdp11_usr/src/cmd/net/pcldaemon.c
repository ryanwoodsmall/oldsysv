/* @(#)pcldaemon.c	1.2 */
/*
 *	pcldaemon - monitors pcl control channel for all systems
 *	linked to this pcl.  Exec's shell and commands for remote.
 */

#include	"sys/types.h"
#include	"sys/pcl.h"
#include	"signal.h"
#include 	"stdio.h"
#include 	"setjmp.h"
#include	"fcntl.h"
#include 	"pwd.h"
#include	"net.h"

/*
**	Compile-time options
*/

#define	NO_ROOTS	1	/* 1 if super-user access is to be denied */

int		log;
char		name[17];

extern char *		ctime();
extern struct passwd *	getpwnam();
extern char *		strtok();
extern char *		strcat();
#define		Strcat	(void)strcat
extern char *		strcpy();
#define		Strcpy	(void)strcpy


main(argc, argv)
	int		argc;
	char *		argv[];
{
	register int	n, ctrl;

	Strcpy(name, *argv++);
	Strcat(name, ": ");

	catch(SIGALRM);

	while ( argc > 1 )
	{
		argc--;
		argv++;

		alarm(2);

		if ( (ctrl = open(*argv, O_RDWR)) >= 0 )
			close(ctrl);

		alarm(0);
	}

	if((ctrl = open("/dev/pcl/ctrl", O_RDWR)) < 0)
		fatal("cannot open pcl control channel\n");

	switch ( fork() )
	{
	 case -1:	fatal("cannot fork\n");
	 default:	_exit(0);
	 case 0:	;
	}

	if ( (log = open("/usr/adm/pcllog", O_CREAT|O_APPEND|O_WRONLY, 0644)) == -1 )
		fatal("cannot open/create log file\n");

	write(log, "\n", 1);
	logmsg("*** RESTARTED ***", 0);

	setpgrp();
	chdir("/");

	for(;;) {
		if((n = read(ctrl, buf, PCLBSZ)) < 0) {
			logmsg("ctrl read error", 0);
			continue;
		}
		logmsg(buf, n);
		buf[n] = 0;

		switch ( fork() )
		{
		 case 0:
			close(ctrl);
			connect();
			_exit(2);

		 case -1:
			logmsg("fork error", 0);
			alarm(10);
			pause();
			continue;
		}

		alarm(2);
		while(wait(0) >= 0);
		alarm(0);
	}
}

catch(sig)
	int	sig;
{
	signal(sig, catch);
}

connect()
{
	register char *	channel;
	register char *	uid;
	register char *	cmd;
	register char *	mc_id;
	struct passwd *	pwp;
	char		dev[20];
	char **		envp;
	extern char **	setvars();

	uid = strtok(buf, " ");
	mc_id = strtok(0, " ");
	channel = strtok(0, " ");
	cmd = channel + strlen(channel) + 1;

	setpgrp();
	close(0); close(1); close(2);

	do
	{
		Strcpy(dev, "/dev/pcl/");
		Strcat(dev, mc_id);
		Strcat(dev, channel);

		if ( open(dev, O_RDWR) == 0 )
			break;
	}
	while
		( *++mc_id );

	if ( *mc_id == '\0' ) {
		logmsg("ctrl assigned channel open error", 0);
		return;
	}

	dup(0); dup(0);

	ioctl(0, WAIT, 0);

	if ( (pwp = getpwnam(uid)) == 0 )
	{
		logmsg(unk, 0);
		write(0, unk, REPLYSIZE);
		return;
	}
	endpwent();

#	if	NO_ROOTS == 1
	if ( pwp->pw_uid == 0 )
	{
		logmsg(ill, 0);
		write(0, ill, REPLYSIZE);
		return;
	}
#	endif	NO_ROOTS

	if ( chdir(pwp->pw_dir) < 0 )
	{
		logmsg(dir, 0);
		write(0, dir, REPLYSIZE);
		return;
	}

	setgid(pwp->pw_gid);
	setuid(pwp->pw_uid);

	if(write(0, ack, REPLYSIZE) != REPLYSIZE) {
		logmsg("ctrl ack write error", 0);
		return;
	}

	close(log);
	ioctl(0, RSTR, 0);

	if ( (channel = pwp->pw_shell) == 0 || *channel == '\0' )
		channel = "/bin/sh";

	if ( envp = setvars(pwp->pw_dir, ":/bin:/usr/bin") )
		execle(channel, "sh", "-c", cmd, 0, envp);
	else
		execl(channel, "sh", "-c", cmd, 0);
}

fatal(s)
	char	*s;
{
	write(2, name, strlen(name));
	write(2, s, strlen(s));
	_exit(1);
}

logmsg(s, n)
	char	*s;
{
	long	clock;

	time(&clock);
	write(log, ctime(&clock)+4, 16);
	write(log, s, n ? n : strlen(s));
	write(log, "\n", 1);
}

exit(){}


/*
**	Setvars -- extract environment from ".profile".
*/

static char strbuf[256];
static char *strp=strbuf;
static FILE *profile;

static char **remember();
static int nvars;

extern char *	strchr();
extern char *	strcpy();
extern char *	strcat();
extern char *	malloc();
extern int	strlen();
extern void	longjmp();


char **
setvars(homedir, path)
	char *	homedir;
	char *	path;
{
	nvars=0;
	setup("HOME", homedir);
	setup("PATH", path);
	if((profile=fopen(".profile", "r")) != NULL){
		look();
		fclose(profile);
	}
	return remember();
}


static jmp_buf env;
static char quotechar;
static struct var{
	char *str;
	struct var *next;
}*var, *this;

/* states */
#define	SLOOK	0	/* looking for start of token */
#define	SBUILD	1	/* building token string */
#define	SSKIP	2	/* skipping input until specific word */
#define	SCOPY	3	/* skip input, but copy to output */

static int state;

/* char types */
#define	UNK	0
#define	ALF	1
#define	NUM	2
#define	EQL	3
#define	QUOT	4
#define	ESC	5
#define	SEP	6
#define	TERM	7
#define	COM	8
#define	EOFC	9

static char chartype[128]={
    /*  nul	 soh	 stx	 etx	 eot	 enq	 ack	 bel */
	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,
    /*  bs 	 ht 	 nl 	 vt 	 np 	 cr 	 so 	 si  */
	UNK,	SEP,	TERM,	UNK,	UNK,	UNK,	UNK,	UNK,
    /*  dle	 dc1	 dc2	 dc3	 dc4	 nak	 syn	 etb */
	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,
    /*  can	 em 	 sub	 esc	 fs 	 gs 	 rs 	 us  */
	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,
    /*  sp 	  ! 	  " 	  # 	  $ 	  % 	  & 	  '  */
	SEP,	UNK,	QUOT,	COM,	UNK,	UNK,	TERM,	QUOT,
    /*   ( 	  ) 	  * 	  + 	  , 	  - 	  . 	  /  */
	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,	UNK,
    /*   0 	  1 	  2 	  3 	  4 	  5 	  6 	  7  */
	NUM,	NUM,	NUM,	NUM,	NUM,	NUM,	NUM,	NUM,
    /*   8 	  9 	  : 	  ; 	  < 	  = 	  > 	  ?  */
	NUM,	NUM,	UNK,	TERM,	UNK,	EQL,	UNK,	UNK,
    /*   @ 	  A 	  B 	  C 	  D 	  E 	  F 	  G  */
	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   H 	  I 	  J 	  K 	  L 	  M 	  N 	  O  */
	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   P 	  Q 	  R 	  S 	  T 	  U 	  V 	  W  */
	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   X 	  Y 	  Z 	  [ 	  \ 	  ] 	  ^ 	  _  */
	ALF,	ALF,	ALF,	UNK,	ESC,	UNK,	UNK,	ALF,
    /*   ` 	  a 	  b 	  c 	  d 	  e 	  f 	  g  */
	QUOT,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   h 	  i 	  j 	  k 	  l 	  m 	  n 	  o  */
	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   p 	  q 	  r 	  s 	  t 	  u 	  v 	  w  */
	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,	ALF,
    /*   x 	  y 	  z 	  { 	  | 	  } 	  ~ 	 del */
	ALF,	ALF,	ALF,	ALF,	TERM,	ALF,	ALF,	EOFC,
};

/*ARGSUSED*/
static
null(c){
}

static
build(c){
	strp=strbuf;
	copy(c);
	state=SBUILD;
}

static
quote(c){
	quotechar=c;
	state=SSKIP;
}

static
ret(c){
	if(state==SCOPY)
		emit(c);
	longjmp(env, 0);
}

static
copy(c){
	*strp++=c;
}

static
dovar(c){
	copy(c);
	state=SCOPY;
}

/*ARGSUSED*/
static
lookst(c){
	state=SLOOK;
}

/*ARGSUSED*/
static
skipnl(c){
	strp=strbuf;
	quote('\n');
}

static
chkqt(c){
	if(quotechar==0)
		quotechar=c;
	else if(quotechar==c){
		quotechar=0;
		state=SLOOK;
	}
}

static
esc(c){
	copy(c);
	copy(getc(profile));
}

static
cpyqt(c){
	if(quotechar==0)
		quotechar=c;
	else if(quotechar==c)
		quotechar=0;
}

/*ARGSUSED*/
static
emit(c){
	if(strp!=strbuf){
		*strp++='\0';
		savevar(strbuf);
	}
	state=SLOOK;
	strp=strbuf;
}

static
sep(c){
	if(quotechar)
		copy(c);
	else
		emit(c);
}

static
coment(c){
	if(quotechar==0)
		skipnl(c);
}

static int (*acttab[SCOPY+1][EOFC+1])()={
/*	UNK	ALF	NUM	EQL	QUOT	ESC	SEP	TERM	COM	EOFC */
/*LOOK*/ null,	build,	null,	null,	quote,	skipnl,	null,	null,	skipnl,	ret,
/*BLD*/	skipnl,	copy,	copy,	dovar,	quote,	skipnl,	skipnl,	lookst,	skipnl,	ret,
/*SKIP*/null,	null,	null,	null,	chkqt,	null,	null,	chkqt,	coment,	ret,
/*COPY*/copy,	copy,	copy,	copy,	cpyqt,	esc,	sep,	sep,	copy,	ret,
};

static
look(){
	int around=0;
	int c;
	setjmp(env);
	if(around++)
		return;
	state=SLOOK;
	while(c=(getc(profile)&0177))
		(*acttab[state][chartype[c]])(c);
}

static
savevar(s)
	register char *s;
{
	register struct var *n;
	if((n=(struct var *)malloc(sizeof (struct var)))==NULL
	   || (n->str=(char *)malloc(strlen(s)+1))==NULL)
		return;		/* Just give up */
	strcpy(n->str, s);
	n->next=0;
	if(this)	/* already had a var */
		this->next=n;
	else
		var=n;
	this=n;
	nvars++;
}

static char *
cpstr(s, t)
	register char *s, *t;
{
	do;
	 while(*s++ = *t++);
	return(--s);
}

static char *
match(s, t)
	register char *s, *t;
{
	/*
	 * Assumes t is of form x=value
	 */
	do; while(*s++ == *t++);
	if(*--s==0)
		return(t);
	return(0);
}

static char *
getvar(s)
	register char *s;
{
	register struct var *vp;
	register char *t;
	for(vp=var; vp; vp=vp->next)
		if(t=match(s, vp->str))
			return(t);
	return("");
}

#define	isalnum(C)	(('a'<=(C) && (C)<='z')||('A'<=(C) && (C)<='Z')||('0'<=(C) && (C)<='9')||((C)=='_'))

static
setup(parm, val)
	register char *parm, *val;
{
	char string[256];
	register char *p;
	p=cpstr(string, parm);
	*p++='=';
	cpstr(p, val);
	savevar(string);
}

static char *
expand(str)
	register char *str;
{
	register char *s, *t, *p;
	char parm[128], word[128], val[128];
	register brace, op;
	if(strchr(str, '$')==0)
		return(str);
	p=strbuf;
	s=str;
	while(*p = *s++){
		if(*p++=='$'){
			if(*s=='{'){
				brace=1;
				s++;
			}else{
				brace=0;
				if(!isalnum(*s))
					continue;
			}
			t=parm;	/* start of variable name */
			while(isalnum(*s))
				*t++= *s++;
			*t=0;
			strcpy(val, getvar(parm));
			if(brace){
				if(*s==':')
					s++;
				op= *s++;
				if(op!='}'){
					t=word;
					while(*s!='}')
						*t++= *s++;
					*t=0, s++;
				}
				if(val[0]){
					if(op=='+')
						strcpy(val, word);
				}else if(op=='-' || op=='='){
					if(op=='=')
						setup(parm, word);
					strcpy(val, word);
				}	/* '+' happens for free */
			}
			p=cpstr(--p, val);
		}
	}
	p=(char *)malloc(strlen(strbuf)+1);
	if(p==NULL)
		return(str);
	strcpy(p, strbuf);
	return(p);
}

static char **
remember(){
	register char **	ep;
	register struct var *	vp;
	register char **	envp;

	if ( envp = (char **)malloc((sizeof (char **))*(nvars+1)) )
	{
		for(vp=var, ep=envp; vp; vp=vp->next)
			*ep++=expand(vp->str);
		*ep++=0;
	}
	return envp;
}
