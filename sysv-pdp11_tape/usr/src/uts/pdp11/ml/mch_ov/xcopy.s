/ @(#)xcopy.s	1.1
/	bufio.s	1.1
.globl	_xgetc, _xputc
_xgetc:			/ c = xgetc( (paddr_t)addr );
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	mov	(sp)+,r0
	tst	r1
	bpl	1f
	swab	r0
1:
	bic	$!377,r0
	br	9f

_xputc:			/ xputc( (paddr_t)addr, c);
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	tst	r1
	bpl	1f
	movb	12(sp),1(sp)
	br	8f
1:
	movb	12(sp),(sp)
	br	8f

.globl	_xget, _xput
_xget:			/ w = xget( (paddr_t)addr );
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	mov	(sp)+,r0
	br	9f

_xput:			/ xput( (paddr_t)addr, w);
	mov	PS,-(sp)
	jsr	pc,7f
	mov	10(sp),-(sp)
	br	8f

.globl	_xgetl, _xputl
_xgetl:			/ l = xgetl( (paddr_t)addr );
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)+
	mfpi	(r0)
	mov	(sp)+,r1
	mov	(sp)+,r0
	br	9f

_xputl:			/ xputl( (paddr_t)addr, l);
	mov	PS,-(sp)
	jsr	pc,7f
	mov	10(sp),-(sp)
	mtpi	(r0)+
	mov	12(sp),r1
	mov	r1,-(sp)
8:
	mtpi	(r0)
	mov	10(sp),r0
9:
	mov	(sp)+,PS
	rts	pc

7:			/ first argument is a long (paddr_t)
	mov	6(sp),r0
	mov	10(sp),r1
	ashc	$10.,r0
	mov	$10340,PS
	mov	r0,SISA0+[5*2]
	mov	$1200,r0
	ashc	$5,r0
	asl	r0
	rts	pc
