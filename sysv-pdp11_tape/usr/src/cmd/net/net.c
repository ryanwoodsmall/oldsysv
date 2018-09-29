/* @(#)net.c	1.2 */
/*
 *	net - execute remote command through PCL-11B
 *		net system [ command [ args ... ] ]
 *	default command is /bin/sh
 */

#include	"sys/types.h"
#include	"sys/pcl.h"
#include	"sys/utsname.h"
#include	"signal.h"
#include	"errno.h"
#include	"pwd.h"
#include	"net.h"


char		dev[24]	= "/dev/pcl/";
int		cm[2];		/* vector for control message info */
char		name[17];
int		Fd;
struct utsname	utsname;

extern	int	errno;
extern char *	strcpy();
#define	Strcpy	(void)strcpy
extern char *	strcat();
#define	Strcat	(void)strcat
extern char *	strncpy();
#define	Strncpy	(void)strncpy
#define	Signal	(void)signal
extern struct passwd *	getpwuid();



main(argc, argv)
	int		argc;
	char *		argv[];
{
	register int	n, fd;
	register char *	cp;
	int		child;
	int		uid;
	char		c;
	char *		chan;
	struct passwd *	pwp;
	extern void	(* catch())();

	Strcpy(name, *argv++);
	Strcat(name, ": ");

	if(--argc == 0)
		fatal("args: system [ command [ args ... ] ]");
	--argc;

	Strcat(dev, *argv++);

	if ( (pwp = getpwuid(uid = getuid())) == 0 )
		fatal("uid unknown");
	endpwent();

	Strcpy(buf, pwp->pw_name);
	Strcat(buf, " ");

	uname(&utsname);
	Strcat(buf, utsname.nodename);
	Strcat(buf, " 0 ");

	cp = &buf[strlen(buf)];
	chan = cp - 2;	/* channel number */

	if(argc == 0)
	{
		Strcpy(cp, "/bin/sh -i ");
		cm[1] = strlen(buf) & ~1;
	}
	else
	{
		while(argc--) {
			while(*cp++ = *(*argv)++)
				if(cp > &buf[PCLBSZ - 2])
					fatal("command list too long");
			*--cp = ' ';
			cp++;
			argv++;
		}
		cm[1] = (cp - buf) & ~01;
	}

	cm[0] = (int) buf;
	catch(SIGALRM);
	n = strlen(dev);

	for(c = '0'; c <= '7'; c++) {

		char	rbuf[REPLYSIZE];

		dev[n] = c;
		if((Fd = fd = open(dev, 2)) < 0)
			continue;

		*chan = c;
		alarm(20);

		if ( ioctl(fd, CTRL, cm) < 0
			|| ioctl(fd, WAIT, 0) < 0
			|| ioctl(fd, RSTR, 0) < 0
			|| read(fd, rbuf, REPLYSIZE) != REPLYSIZE
			|| strncmp(rbuf, ack, REPLYSIZE))
		{
			if ( strncmp(rbuf, unk, REPLYSIZE) == 0 )
				fatal("user is unknown at remote");
			if ( strncmp(rbuf, ill, REPLYSIZE) == 0 )
				fatal("user is denied access at remote");
			if ( strncmp(rbuf, dir, REPLYSIZE) == 0 )
				fatal("home directory inaccessible");
			close(fd);
			fd = -1;
			continue;
		}

		alarm(0);
		break;
	}

	if(fd < 0)
		fatal("cannot open channel to system");

	setuid(uid);	/* net shouldn't be a setuid program, but to be safe... */

	if((child = fork()) == 0) {	/* child - pcl writing process */

		Signal(SIGINT, SIG_IGN);

		while ( (n = read(0, buf, PCLBSZ)) >= 0 )
			while(write(fd, buf, n) != n)
				if(errno != EIO) {
					kill(getppid(), SIGKILL);
					fatal("connection broken");
				} else
					mesg("write error");

	} else {			/* parent - pcl reading process */

		if(child == -1)
			fatal("cannot fork reader process");

		if(signal(SIGINT, catch) == SIG_IGN)
			Signal(SIGINT, SIG_IGN);
		catch(SIGPIPE);

		for(;;)
			if((n = read(fd, buf, PCLBSZ)) >= 0)
				write(1, buf, n);
			else if(errno != EINTR)
				break;

		kill(child, SIGKILL);
	}

	return 0;
}

fatal(s)
	char *s;
{
	mesg(s);
	_exit(1);
}

mesg(s)
	char *s;
{
	wrt(name);
	wrt(s);
	wrt("\n");
}

wrt(s)
	char *	s;
{
	write(2, s, strlen(s));
}

void
(* catch(sig))()
{
	Signal(sig, catch);
	if(sig == SIGINT)
		(void)ioctl(Fd, SIG, sig);
}

exit(){}
