/*	@(#)x25llnk.c	1.2	*/
/*
 *	This is the 5.0 version of the x25alnk, x25ilnk, x25clnk, x25dlnk,
 *	and x25hlnk subroutine interface for establishing and taking down
 *	links.  Flag values, return values, and argument order could change.
 */


/*
 *	x25ilnk is used to initialize a link, or more precisely, to start
 *	the script running for the particular line.  Thus, the KMC
 *	must have already been started (usually via vpmstart).  For the
 *	KMS, it is possible for the interpreter to be "running" in the
 *	KMC (that is, able to communicate with the VPM bottom in the main
 *	computer) without the protocol script (normally HDLC, or level 2
 *	BX.25) being active for any of the 8 lines.  It is also possible,
 *	of course, for the KMC to have the script running (level 2 running)
 *	for all 8 lines simultaneously.
 *
 *	linkid is the index in the operating system tables for the
 *	    link descriptor structure to be used.  The link index is
 *	    can be thought of as the connection between x25pvci calls and
 *	    the x25ilnk call for the physical link the channels are multi-
 *	    plexed on.  An example link index is 1.
 *	devname is the name of the  physical device running the interpreter and
 *	    script for this link, e.g., "/dev/kmc0"
 *	lineno is the physical line number (range 0-7) for a logical link
 *	    on the physical device, e.g., 4
 *	modname is the name of the synchronous modem control device.
 *	    If specified (flag must be set to use it), the standard modem
 *	    control functions performed for the line are to raise data
 *	    terminal ready and request to send. An example dmk name is
 *	    "/dev/dmk0"
 *	pksize is the packet size to be used for level 3 data packets.
 *	    pksize must be a number that is a power of 2 between 16 and
 *	    1024 inclusive. The default pksize is 128. The LNKPKT bit
 *	    flag must be raised to set a non-default size.
 *	flags specifies the options for the initialization call, e.g.,
 *	    LNKPKT | LNKMOD
 */
#include <sys/types.h>
#ifndef u3b
#include <sys/dmk.h>
#endif
#include <x25lib.h>
#include <sys/map.h>
#include <sys/csi.h>
#include <sys/x25.h>
#include <sys/nc.h>

struct ncstart ncstart, *startp;
struct ncattch ncattch, *attchp;

int
x25alnk(linkid, devname, lineno, modname, flags)
int	linkid;
char	devname[];
int	lineno;
char	modname[];
unsigned	flags;
{
	int fd,fd2;
	int rtval, ioflg;
#ifndef u3b
	struct dmkcmd dmkcmd;
#endif

	startp = &ncstart;
	attchp = &ncattch;

	startp->csiflags.options = 0;

	attchp->linkno = linkid;

	if ((rtval = mmdevno(devname)) < 0 ) return(rtval);
	attchp->dev = (unsigned short) rtval;

	if (lineno >= 0 && lineno <= 7) 
		attchp->dev |= lineno<<5; 
	else
		return(ELNKLNO);

	if (flags&LNKMOD) {
#ifdef u3b
		return(ELNKMCO);
#endif
		dmkcmd.line = lineno;
		dmkcmd.mode = DMKRTS|DMKDTR;
		if ((fd2 = open(modname,0)) < 0) {
			return(ELNKMCO);
		}
		if (ioctl(fd2, DMKSETM, &dmkcmd) < 0) {
			close(fd2);
			return(ELNKMCI);
		}
		close(fd2);
	}

	if (flags&LNKBACK)
		ioflg = NCBKATTACH;
	else
		ioflg = NCATTACH;

	if((fd = open(NCDEV,2)) <0){
		return(ELNKNCO);
	}

	if (ioctl(fd, ioflg, attchp) < 0) {
		close(fd);
		return(ELNKNCI);
	}

	close(fd);
	return(NULL);
}

int
x25ilnk(linkid,pksize,flags)
int	linkid;
int	pksize;
unsigned	flags;
{

	int fd, ioflg;

	startp = &ncstart;
	startp->csiflags.options = 0;

	startp->linkno = linkid;

	if(flags&LNKPKT){
		if(pksize%16 || pksize < 16 || pksize > 1024){
			return(ELNKPKT);
		}
		startp->pktsz = pksize;
	} else
		startp->pktsz = STDPKTSZ;


/*	is this to be station B at level two ?	  */
 
	if( flags&LNKISB ) startp->csiflags.options |= ADRB;

	/* set bit for fast speed */
	if(flags&LNKFAST)
		startp->csiflags.options |= FAST;

	if(flags&LNKBACK)
		ioflg = NCBKSTART;
	else
		ioflg = NCSTART;
 
	if((fd = open(NCDEV,2)) <0){
		return(ELNKNCO);
	}

	if(ioctl(fd, ioflg, startp) < 0){
		close(fd);
		return(ELNKNCI);
	}


	close(fd);
	return(NULL);
}


int
x25clnk(linkid)
int	linkid;
{

	int fd;

	if((fd = open(NCDEV,2)) <0){
		return(ELNKNCO);
	}

	if(ioctl(fd, NCCHNGE, linkid) < 0){
		close(fd);
		return(ELNKNCI);
	}


	close(fd);
	return(NULL);
}

int
x25dlnk(linkid, flags)
int	linkid;
unsigned	flags;
{
	int fd, ioflg;

	if (flags&LNKBACK)
		ioflg = NCBKDETACH;
	else
		ioflg = NCDETACH;

	if((fd = open(NCDEV,2)) <0){
		return(ELNKNCO);
	}

	if (ioctl(fd, ioflg, linkid) < 0) {
		close(fd);
		return(ELNKNCI);
	}


	close(fd);
	return(NULL);
}

int
x25hlnk(linkid, flags)
int	linkid;
unsigned	flags;
{
	int fd, ioflg;

	if (flags&LNKBACK)
		ioflg = NCBKSTOP;
	else
		ioflg = NCSTOP;
	if((fd = open(NCDEV,2)) <0){
		return(ELNKNCO);
	}

	if (ioctl(fd, ioflg, linkid) < 0) {
		close(fd);
		return(ELNKNCI);
	}
	close(fd);
	return(NULL);
}
