/ @(#)start.s	1.1
.globl	start, _end, _edata, _main
start:

/ Set location 0 and 2 to catch traps and jumps to 0

	mov	$42,0		/ illegal instruction if jump
	mov	$777,2		/ trace trap at high priority if trap

/ initialize systems segments

	mov	$KISA0,r0
	mov	$KISD0,r1
	mov	$200,r4
	clr	r2
	mov	$7,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	r4,r2
	sob	r3,1b

/ initialize io segment

	mov	$IO,(r0)+
	mov	$77406,(r1)+		/ 4k rw

/ get a sp and start segmentation

	mov	$_u+[usize*64.],sp
	inc	SSR0

/ clear bss

	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$_end
	blo	1b

/ allocate memory for process 0 user block

	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	r0,-(sp)

/ test for floating point hardware

	setd
nofpp:

/ Take stuff above data out of address space

	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	$KISD0,r1
1:
	cmp	r0,$200
	bge	3f
	dec	r0
	bge	4f
	clr	(r1)
	br	3f
4:
	movb	r0,1(r1)
3:
	tst	(r1)+
	sub	$200,r0
	cmp	r1,$KISD0+[7*2]
	blo	1b

/ set up previous mode and call main
/ with address of process 0 user block
/ on return, enter user mode at 0R

	mov	$PRUSER,PS
	jsr	pc,_main
	mov	$170000,(sp)
	clr	-(sp)
	rtt
