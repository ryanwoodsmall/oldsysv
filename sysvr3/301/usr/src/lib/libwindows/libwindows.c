/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:hostagent.c	2.10"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/tty.h>
#include	<sys/jioctl.h>
#include	<sys/xtproto.h>
#undef		NULL

#undef		exit()
#undef		sleep(n)
#undef		strlen()		/* ugh ick ugh!!!!! */
#include	<windows.h>
#undef		NULL
#define		NULL	0

struct 		bagent cmd;
union		bbuf	{
		struct	agentrect	ar;
		char	buf[MAXPKTDSIZE];
};
union		bbuf ret;

int
openagent()
{
	char	*cp;
	int	cntlfd;
	char	*ttyname();
	static	char	cntlf[]	= "/dev/xt/000";

	if ((cp = ttyname(fileno (stdin))) == NULL)
		return(-1);
	cntlf[9] = cp[strlen(cp)-2];
	cntlf[8] = cp[strlen(cp)-3];
	return ( open (cntlf, O_RDONLY) );
}

int
openchan(chan)
int chan;
{
	char	*cp;
	char	*ttyname();
	static	char	blitfile[]	= "/dev/xt/000";

	if ((cp = ttyname(fileno (stdin))) == NULL)
		return(-1);
	blitfile[8] = cp[strlen(cp)-3];
	blitfile[9] = cp[strlen(cp)-2];
	blitfile[10] = chan + '0';
	return ( open (blitfile, O_RDWR) );
}

int
Newlayer(fd, x1, y1, x2, y2)
int	fd;
int	x1, y1, x2, y2;
{
	if( ioctlagent(fd, A_NEWLAYER, x1, y1, x2, y2, 0) )
		return(-1);
	return (ret.ar.chan);
}

int
New(fd, x1, y1, x2, y2)
int	fd;
int	x1, y1, x2, y2;
{
	if( ioctlagent(fd, A_NEW, x1, y1, x2, y2, 0) )
		return(-1);
	return (ret.ar.chan);
}

int
Current(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_CURRENT, 0, 0, 0, 0, chan) );
}

int
Delete(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_DELETE, 0, 0, 0, 0, chan) );
}

int
Top(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_TOP, 0, 0, 0, 0, chan) );
}

int
Bottom(fd, chan)
int	fd;
int	chan;
{
	return ( ioctlagent(fd, A_BOTTOM, 0, 0, 0, 0, chan) );
}

int
Move(fd, chan, x1, y1)
int	fd;
int	chan;
int	x1, y1;
{
	return ( ioctlagent(fd, A_MOVE, x1, y1, 0, 0, chan) );
}

int
Reshape(fd, chan, x1, y1, x2, y2)
int	fd;
int	chan;
int	x1, y1, x2, y2;
{
	return ( ioctlagent(fd, A_RESHAPE, x1, y1, x2, y2, chan) );
}

int
Exit(fd)
int fd;
{
	return ( ioctlagent(fd, A_EXIT, 0, 0, 0, 0, 0) );
}


int
Runlayer(chan, command)
int	chan;
char	*command;
{

	int fdc;
	char cb[TTYHOG+1];
	int rv;

	if ( (fdc = openchan(chan)) == -1 )
		return(-1);
	if ( strlen(command) > TTYHOG )
		return(-1);
	cb[0] = (char )chan;
	cb[1] = '\0';
	strcat(cb,command);
	rv = ioctl(fdc,JTRUN,cb);
	close(fdc);
	return(rv);
}



ioctlagent(fd, command, x1, y1, x2, y2, chan)
int	fd;
int	command;
int	x1, y1, x2, y2;
short	chan;
{
	union	bbuf	arbuf;
	int	size;

	arbuf.ar.command = command;
	arbuf.ar.chan = chan;
	arbuf.ar.r.origin.x = x1;
	arbuf.ar.r.origin.y = y1;
	arbuf.ar.r.corner.x = x2;
	arbuf.ar.r.corner.y = y2;
#ifdef 	vax
	swab(arbuf.buf, arbuf.buf, sizeof(struct agentrect));
#endif	vax
	cmd.size = sizeof(struct agentrect);
	cmd.src = arbuf.buf;
	cmd.dest = ret.buf;
	if( (size = ioctl(fd, JAGENT, &cmd) ) != sizeof(struct agentrect) )
		return(-1);
#ifdef 	vax
	swab(ret.buf, ret.buf, sizeof(struct agentrect));
#endif	vax
	if(ret.ar.command == (-1))
		return(-1);
	return (0);
}

