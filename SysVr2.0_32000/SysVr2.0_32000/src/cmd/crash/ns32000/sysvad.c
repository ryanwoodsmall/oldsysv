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
/*	@(#)sysvad.c	1.3	*/
#include	"crash.h"
#if vax || ns32000
#ifdef vax
int Sbrptr;	/* start of page table in memory */
#endif
#ifdef ns32000
int Kpt2ptr;	/* address of 2nd level page table in memory */
#endif


vaddrinit()
{

#ifdef vax
	register i;

	if(lseek(mem,(long)(SYM_VALUE(Sbrpte)),0) == -1) {
		error("seek error on sbrpte");
		return(-1);
	}
	if(read(mem,(char *)&Sbrptr, sizeof Sbrptr) == -1) {
		error("read error on Sbrptr");
		return(-1);
	}
	Sbrptr &= VIRT_MEM;

	/*for (i = 0; i < (sizeof Pte/ sizeof Pte[0]); i += 128) {
		if(lseek(mem, Sbrptr + i, 0) == -1)
			error("seek error on memory page");
		if(read(mem, (char *)&Pte[i], 512) != 512)
			error("read error on memory page");
	}*/
#endif
#ifdef ns32000
#ifdef	DEBUG
	printf("Seeking to 0x%x for Kpt2\n", SYM_VALUE(Kpte2));
#endif
	if(lseek(mem,(long)(SYM_VALUE(Kpte2)),0) == -1) {
		error("seek error on kpte2");
		return(-1);
	}
	if(read(mem,(char *)&Kpt2ptr, sizeof(Kpt2ptr)) == -1) {
		error("read error on Kpt2ptr");
		return(-1);
	}
#ifdef	DEBUG
	printf("Kpt2ptr = 0x%x\n", Kpt2ptr);
#endif
#endif

}


sysvad(vad)
unsigned vad;
{
	int	i;
	int	ww, spte;
	int	binp; /* byte in page */

#ifdef vax
	vad &= VIRT_MEM;
	binp = vad & 0x1ff;
	ww = vad >> 9;
	/*if (ww < 1024)
		return(((Pte[ww]&0x1fffff) << 9) | binp);*/
#ifdef DEBUG
	printf("addr %x out of Pte\n", ww);
#endif
	ww = ww * 4 + Sbrptr;
	lseek(mem, (long)ww, 0);
	read(mem, &spte, sizeof spte);
	return(((spte&0x1fffff) << 9) | binp);
#endif
#ifdef ns32000
	binp = vad & 0x1ff;
	ww = (vad >> 9)*4 + Kpt2ptr;
	lseek(mem, (long)ww, 0);
	read(mem, &spte, sizeof spte);
	return((spte & ~0x1ff) | binp);	/* mask off pte flag bits (0-8) */
#endif
}
#endif

readmem(buf, vad, len)
char *buf;
long vad;
int len;
{
#if vax || ns32000
	int	physaddr,
		i,
		binp,
		get,
		start;
#endif
#ifdef	pdp11
#ifdef DEBUG
	printf("seeking to %lo\n", vad);
#endif
	if (lseek(mem, (long)vad, 0) == -1)
		return(0);
	if (read(mem, &buf[0], len) != len)
		return(0);
	return(len);
#endif
#if vax || ns32000
	binp = vad & 0x1ff;
	start = 0;
	while (len) {
		physaddr = sysvad(vad);
#ifdef	DEBUG
		printf("Sysvad: len = %d, vaddr = 0x%x, phyaddr = 0x%x\n",
			len, vad, physaddr);
#endif
		get = (binp + len > 0x200) ? 0x200 - binp : len;
		if (lseek(mem, physaddr, 0) == -1)
			return(0);
		if (read(mem, &buf[start], get) != get)
			return(0);
		start += get;
		len -= get;
		vad = (vad + 0x200) & ~0x1ff;
		binp = 0;
	}
#ifdef DEBUG
	printf("start = %d\n", start);
#endif
	return(start);
#endif
}
