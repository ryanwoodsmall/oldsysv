/*	@(#)sysvad.c	1.2	*/
#include	"crash.h"
#ifdef	vax
int Sbrptr;	/* start of page table in memory */

vaddrinit()
{
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
}


sysvad(vad)
unsigned vad;
{
	int	i;
	int	ww, spte;
	int	binp; /* byte in page */

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
}
#endif

readmem(buf, vad, len)
char *buf;
long vad;
int len;
{
#ifdef	vax
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
#ifdef	vax
	binp = vad & 0x1ff;
	start = 0;
	while (len) {
		physaddr = sysvad(vad);
		get = (binp + len > 0x200) ? 0x200 - binp : len;
		if (lseek(mem, physaddr, 0) == -1)
			return(0);
		if (read(mem, &buf[start], get) != get)
			return(0);
		start += get;
		len -= get;
		vad = (vad + 0x1ff) & ~0x1ff;
		binp = 0;
	}
#ifdef DEBUG
	printf("start = %d\n", start);
#endif
	return(start);
#endif
}
