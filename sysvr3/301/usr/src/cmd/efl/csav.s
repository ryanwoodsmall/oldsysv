#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

/ C register save and restore -- version 7/79-B
/
/ calling procedure:
/ prog:	jsr	r0,csav
/	...
/	jmp	cret
/
/ appearance of stack during execution of prog:
/	arg_n
/	...
/	arg_0
/	return address
/	old r5		(r5 -> old r5)
/	old r4
/	old r3
/	old r2
/

.globl	csav
	.ident	"@(#)efl:csav.s	1.3"

csav:
	mov	r5,(sp)
	mov	sp,r5		/ context switch!
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	tst	-(sp)
	mov	r0,pc
