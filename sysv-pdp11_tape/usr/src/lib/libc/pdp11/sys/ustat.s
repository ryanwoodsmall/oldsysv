/	@(#)ustat.s	1.3
/ C library -- ustat - get superblock info

/ ustat(dev, info)
/ char *info;

.globl _ustat, cerror
.pwbsys = 57.
ustat = 2

_ustat:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r1
	mov	6(r5),r0
	sys	.pwbsys; ustat
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
