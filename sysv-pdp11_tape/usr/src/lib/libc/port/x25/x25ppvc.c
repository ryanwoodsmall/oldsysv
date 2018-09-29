/*	@(#)x25ppvc.c	1.1	*/
/*
 *	This is the 5.0 version of the x25ipvc and x25rpvc
 *	subroutine for installing and removing a PVC on a link.
 */

/*
 *	x25ipvc is used to install a PVC on a link, or to associate a
 *	level 3 interface with a level 2 interface for BX.25 in UNIX.
 *
 *	slotname is the name of a slot, or PVC, e.g., "/dev/x25s12"
 *	    (for now, a slot denotes a particular PVC on a link to a
 *	    particular remote host or network interface)
 *	chno is the BX.25 level 3 logical channel number associated with
 *	    the pvc, e.g., 3
 *	linkid is the identifier for the link data structure to be used
 *	    in the operating system. This identifier can be thought of
 *	    as the connector between x25ipvc calls and the x25alnk call
 *	    for the physical link on which the channels are multiplexed.
 *	flags contains flag values for specifying PVC install options --
 *	    see permissible PVC flag values.  An example flag
 *	    is PVCSESS.
 */

#include <sys/types.h>
#include <sys/map.h>
#include <sys/csi.h>
#include <sys/x25.h>
#include <sys/nc.h>
#include <sys/x25u.h>
#include <x25lib.h>

struct ncpvc ncpvc, *pvcp;


int
x25ipvc(slotname, chno, linkid, flags)
char	slotname[];
int	chno;
int	linkid;
unsigned	flags;
{
	int fd,rtval;
	int Rflg, Sflg, Nflg;
        pvcp = &ncpvc;


	Nflg = Rflg = Sflg = 0;

	if(flags & PVCSESS){
		Sflg++;
		pvcp->options |= PVC_SESS;
	}


	if(flags & PVCREST){
		Rflg++;
		pvcp->options |= PVC_RST;
	}


	if(flags & PVCNONE){
		Nflg++;
		pvcp->options |= PVC_NONE;
	}


	if( ((Sflg + Rflg +Nflg) > 1) || ((Sflg + Rflg + Nflg) < 1) )
	return(EPVCNP);



	if( (rtval = mdevno(slotname)) < 0 ) return(rtval);
	pvcp->slot = (unsigned short) rtval;


	pvcp->chno = (unsigned short) chno;

	pvcp->linkno = (unsigned short) linkid;

	if((fd = open(NCDEV,2)) <0){
		return(EPVCNCO);
	}

	if(ioctl(fd,NCPVCI,pvcp) < 0){
		close(fd);
		return(EPVCNCI);
	}
	close(fd);
	return(NULL);
}


/*
 *	x25rpvc is used to remove a PVC on a link.
 */


int
x25rpvc(slotname)
char	slotname[];
{
	int fd,rtval;

	if( (rtval = mdevno(slotname)) < 0 ) return(rtval);

	if((fd = open(NCDEV,2)) <0){
		return(EPVCNCO);
	}

	if(ioctl(fd,NCPVCR,rtval) < 0){
		close(fd);
		return(EPVCNCI);
	}
	close(fd);
	return(NULL);
}
