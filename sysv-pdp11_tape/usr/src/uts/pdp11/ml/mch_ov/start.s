/ @(#)start.s	1.1
.globl	dstart, tstart, _end, _edata, _etext
.data
dstart:
	mov	$stk+4,sp
	reset
	mov	$2,0
	mov	$777,2
	mov	$PRUSER,PS

/ Initialise all kernal segmentation registers

	mov	$32.,r0		/ number of register to initialise
	mov	$KISD0,r1	/ first segmentation register
	mov	$__kisd,r2	/ first prototype
1:
	mov	(r2)+,(r1)+
	sob	r0,1b

/ 22 bit, U  and K sep

	mov	$25,SSR3
	bit	$20,SSR3
	beq	1f		/ not 11/70
	mov	$70.,_cputype
	bis	$3,*$MSCR
1:

/ set up supervisor D registers

	mov	$77406,SISD0
	mov	KISA0,SISA0
	mov	$77406,SISD0+[1*2]
	mov	KISA0+[1*2],SISA0+[1*2]
	mov	$77406,SISD0+[4*2]
	mov	$77406,SISD0+[5*2]
	mov	$77406,SISD0+[7*2]
	mov	$IO,SISA0+[7*2]

/ Now give control to text start
	jmp	tstart

.text

.globl	start, _main
start:
	mov	$_u+[usize*64.],sp		/ setup stack

/ Clear ublock

	mov	$_u,r0
/ set stack limit
	mov	r0,SL
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b

/ Test for floating point
	setd
nofpp:

/ Enter main()

	mov	KDSA0+[6*2],-(sp)		/ physical address of u_block
	mov	$PRUSER,PS
	jsr	pc,_main
	mov	$170000,(sp)
	clr	-(sp)
	rtt

.globl	__kisd, __kdsd, __kisa, __kdsa
.data

/ 17772300-17772316  kisd0-kisd7
/ 17772320-17772336  kdsd0-kdsd7
/ 17772340-17772356  kisa0-kisa7
/ 17772360-17772376  kdsa0-kdsa7

/ order of following declarations must be __kisd, __kdsd, __kisa, __kdsa

__kisd:	.=.+16.
__kdsd:	.=.+16.
__kisa:	.=.+16.
__kdsa:	.=.+16.

.text
