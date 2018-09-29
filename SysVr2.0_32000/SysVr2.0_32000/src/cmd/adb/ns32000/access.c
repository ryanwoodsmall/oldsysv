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
/*	@(#)access.c	1.2	*/
#include "defs.h"

SCCSID(@(#)access.c	1.2);

static unsigned accfile(mode, adr, sptype, value)
    int mode;
    int adr;
    int sptype;
    int value;
{
    short pmode;
    short rd;
    int   curfile;
    int w;

    if (sptype == NSP)
        return(0);

    rd = (mode == RD);

    if (pid) {      /* Does a process exist? */
         pmode = (sptype & DSP ? (rd ? RDUSER : WDUSER) : 
                               (rd ? RIUSER : WIUSER));
		 errno = 0;
         w = ptrace(pmode, pid, adr, value);
         if (w == -1 && errno != 0)
             errflg = (sptype & DSP ? BADDAT : BADTXT);
         return(w);
    }
    w = 0;
    if (mode == WT && wtflag == O_RDONLY)
        error("not in write mode");

    if ( ! chkmap(&adr ,sptype))
        return(0);

    curfile = (sptype & DSP ? slshmap.ufd : qstmap.ufd);
    if (lseek(curfile, (long) adr&0x7fffffff, 0) == -1L ||
       (rd ? read(curfile, (char *) &w, sizeof(w)) : 
	    write(curfile, (char *) &value, sizeof(w))) == -1)
			errflg = (sptype&DSP ? BADDAT : BADTXT);
    return(w);
}

/* file handling and access routines */

void put(adr, sptype, value)
    int adr;
    int sptype;
    int value;
{
    (void) accfile(WT, adr, sptype, value);
}

unsigned get(adr, sptype)
    int adr;
    int sptype;
{
    return(accfile(RD, adr, sptype, 0));
}

unsigned chkget(n, sptype)
    int n;
    int sptype;
{
    unsigned w;

    w = get(n, sptype);
    chkerr();
    return(w);
}

unsigned bchkget(n, sptype) 
    int n;
    int sptype;
{
    return(chkget(n, sptype) & LOBYTE);
}

static int chkmap(adr, sptype)
    int *adr;
    short sptype;
{
    MAPPTR amap;

    amap = (sptype & DSP ? &slshmap : &qstmap);
    if (sptype&STAR || !within(*adr, amap->b1, amap->e1)) { 
        if (within(*adr, amap->b2, amap->e2))
            *adr += (amap->f2) - (amap->b2);
        else { 
            errflg = (sptype&DSP ? BADDAT : BADTXT); 
            return(0);
        }
    }
    else
        *adr += (amap->f1) - (amap->b1);
    return(1);
}

static int within(adr, lbd, ubd)
    int adr;
    unsigned lbd;
    unsigned ubd;
{
    return(adr >= lbd && adr < ubd);
}
