/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)line.c	1.6 */
#include "uucp.h"
#include <sys/types.h>



#ifndef RT
#include <termio.h>

#define D_ACU	 1
#define D_DIRECT 2

struct sg_spds {int sp_val, sp_name;} spds[] = {
	{   0,     0},
	{ 300,  B300},
	{1200, B1200},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
	{-1, -1} };
static struct termio Savettyb;
/*
 * set speed/echo/mode...
 *	tty 	-> terminal name
 *	spwant 	-> speed
 *	type	-> type
 * return:  
 *	none
 */
fixline(tty, spwant,type)
int tty, spwant, type;
{
	register struct sg_spds *ps;
	struct termio ttbuf;
	int speed = -1;
	int ret;

	if(!isatty(tty)) return(0);
	for (ps = spds; ps->sp_val >= 0; ps++)
		if (ps->sp_val == spwant)
			speed = ps->sp_name;
	DEBUG(4, "fixline - speed= %d\n", speed);
	ASSERT(speed >= 0, "BAD SPEED", "", speed);
	ioctl(tty, TCGETA, &ttbuf);
	ttbuf.c_iflag = (ushort)0;
	ttbuf.c_oflag = (ushort)0;
	ttbuf.c_lflag = (ushort)0;
	if (speed) ttbuf.c_cflag = speed;
	else ttbuf.c_cflag &= CBAUD;

/*   The following if statement should be used if using direct lines that
 *   do not have the status lines (DTR, DCD) implemented.  Using the if
 *   prevents uucp fron knowing whether the other system is really out
 *   there until the login sequence times out.
 */

/*
 *	if (type == D_DIRECT) {
 *		DEBUG(4,"fixline - direct%s\n","");
 *		ttbuf.c_cflag |= CLOCAL;
 *	}
 *	else
 *		ttbuf.c_cflag &= ~CLOCAL;
 */

	ttbuf.c_cflag |= (CS8|CREAD| (speed ? HUPCL : 0));
	ttbuf.c_cc[VMIN] = 6;
	ttbuf.c_cc[VTIME] = 1;
	ret = ioctl(tty, TCSETA, &ttbuf);
	ASSERT(ret >= 0, "RETURN FROM fixline", "", ret);
	return;
}

sethup(dcf)
int dcf;
{
	struct termio ttbuf;

	if(!isatty(dcf)) return(0);
	ioctl(dcf, TCGETA, &ttbuf);
	if(!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		ioctl(dcf, TCSETA, &ttbuf);
	}
}

genbrk(fn)
register int fn;
{
	if(isatty(fn)) ioctl(fn, TCSBRK, 0);
}

/*
 * optimize line setting for sending or receiving files
 * return:
 *	none
 */
#define PACKSIZE	64
#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

setline(type)
register char type;
{
	static struct termio tbuf, sbuf;
	static int set = 0;

	if(!isatty(Ifn)) return(0);
	DEBUG(2, "setline - %c\n", type);
	switch(type) {
	case SNDFILE:
		break;
	case RCVFILE:
		ioctl(Ifn, TCGETA, &tbuf);
		sbuf = tbuf;
		tbuf.c_cc[VMIN] = PACKSIZE;
		ioctl(Ifn, TCSETAW, &tbuf);
		set++;
		break;
	case RESET:
		if (set == 0) break;
		set = 0;
		ioctl(Ifn, TCSETAW, &sbuf);
		break;
	}
}

savline()
{
	int ret;

	ret = ioctl(0, TCGETA, &Savettyb);
	Savettyb.c_cflag = (Savettyb.c_cflag & ~CS8) | CS7;
	Savettyb.c_oflag |= OPOST;
	Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	return(ret);
}

restline()
{
	return( ioctl(0, TCSETA, &Savettyb) );
}
#else
#include <sgtty.h>
struct sgttyb Savettyb;
struct sg_spds {int sp_val, sp_name;} spds[] = {
	{ 300,  B300},
	{1200, B1200},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
	{0, 0} };

/***
 *	fixline(tty, spwant, type)	set speed/echo/mode...
 *	int tty, spwant, type;
 *
 *	return codes:  none
 */

fixline(tty, spwant, type)
int tty, spwant, type;
{
	struct sgttyb ttbuf;
	struct sg_spds *ps;
	int speed = -1;
	int ret;

	if(spwant == 0){
		fixmode(tty);
		return;
	}
	for (ps = spds; ps->sp_val; ps++)
		if (ps->sp_val == spwant)
			speed = ps->sp_name;
	ASSERT(speed >= 0, "BAD SPEED", "", speed);
	ioctl(tty, TIOCGETP, &ttbuf);
	ttbuf.sg_flags =(ANYP|RAW);
	ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
	ret = ioctl(tty, TIOCSETP, &ttbuf);
	ASSERT(ret >= 0, "RETURN FROM STTY", "", ret);
	ioctl(tty, TIOCHPCL, STBNULL);
	ioctl(tty, TIOCEXCL, STBNULL);
	return;
}
/***
 *	fixmode(tty)	fix kill/echo/raw on line
 *
 *	return codes:  none
 */

fixmode(tty)
int tty;
{
	struct sgttyb ttbuf;
	int ret;

	ioctl(tty, TIOCGETP, &ttbuf);
	ttbuf.sg_flags = (ANYP | RAW);
	ret = ioctl(tty, TIOCSETP, &ttbuf);
	ASSERT(ret >= 0, "STTY FAILED", "", ret);
	ioctl(tty, TIOCEXCL, STBNULL);
	return;
}
sethup(dcf)
{

	ioctl(dcf, TIOCHPCL, STBNULL);
}
#define BSPEED B150

/***
 *	genbrk		send a break
 *
 *	return codes;  none
 */

genbrk(fn)
{
	struct sgttyb ttbuf;
	int ret, sospeed;
	int	bnulls;

	bnulls = 1;
	ret = ioctl(fn, TIOCGETP, &ttbuf);
	sospeed = ttbuf.sg_ospeed;
	ttbuf.sg_ospeed = BSPEED;
	ret = ioctl(fn, TIOCSETP, &ttbuf);
	ret = write(fn, "\0\0\0\0\0\0\0\0\0\0\0\0", bnulls);
	ASSERT(ret > 0, "BAD WRITE genbrk", "", ret);
	ttbuf.sg_ospeed = sospeed;
	ret = ioctl(fn, TIOCSETP, &ttbuf);
	ret = write(fn, "@", 1);
	ASSERT(ret > 0, "BAD WRITE genbrk", "", ret);
	DEBUG(4, "sent BREAK nulls - %d\n", bnulls);
	return;
}
setline()
{
}
savline()
{
	int	ret;

	ret = ioctl(0, TIOCGETP, &Savettyb);
	Savettyb.sg_flags |= ECHO;
	Savettyb.sg_flags &= ~RAW;
	return(ret);
}
restline()
{

		return(ioctl(0, TIOCSETP, &Savettyb));
}
#endif
