/	@(#)uname.s	1.3
/ C library -- uname

/ uname(unixname);
/ unixname[0], ...unixname[7] contain the unixname

.globl _uname, cerror
.pwbsys = 57.
uname = 0

_uname:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.pwbsys; uname
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
