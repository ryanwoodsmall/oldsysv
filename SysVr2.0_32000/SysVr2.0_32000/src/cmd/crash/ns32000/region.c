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
/* @(#)region.c	1.1 */
#include	"crash.h"
#include	<sys/page.h>
#include	<sys/region.h>

prregion(c)
register  int  c;
{
	struct region regbuf;
	char *type;
	short r_flags;

	if(c == -1)
		return;
	if(c >= v.v_region || c < 0)
		if((unsigned)c >= Region->n_value &&
		(unsigned)c < (Region->n_value+v.v_region*sizeof(regbuf))) {
			c = ((unsigned)c - Region->n_value)/sizeof(regbuf);
		} else {
			printf("%3x out of range\n", c);
			printf("v_region=%x  Region->n_value=%x sizeof(regbuf)=%x\n",
				v.v_region, Region->n_value, sizeof(regbuf));
			return;
		}
	if(readmem(&regbuf, (long)SYM_VALUE(Region) + c * sizeof regbuf,
		sizeof regbuf) != sizeof regbuf) {
		printf("%3d read error\n");
		return;
	}
	printf("%5d %5d %5d %4d %8lx %8lx %8lx", regbuf.r_pgoff, regbuf.r_pgsz, 
		regbuf.r_nvalid, regbuf.r_refcnt, regbuf.r_list,
		regbuf.r_iptr, regbuf.r_forw);

	switch (regbuf.r_type) {
	case RT_UNUSED:
		type = "unused";
		break;
	case RT_PRIVATE:
		type = "private";
		break;
	case RT_STEXT:
		type = "shared text";
		break;
	case RT_SHMEM:
		type = "shared mem";
		break;
	}
	printf("%12.11s", type);
	
	r_flags = regbuf.r_flags;
	printf("%s%s%s%s%s%s%s\n",
		r_flags & RG_NOSWAP ? " noswap" : "",
		r_flags & RG_DEMAND ? " demand" : "",
		r_flags & RG_LOCK ? " locked" : "",
		r_flags & RG_WANTED ? " wanted" : "",
		r_flags & RG_NOFREE ? " nofree" : "",
		r_flags & RG_DONE ? " done" : "",
		r_flags & RG_WAITING ? " waiting" : "");
}
