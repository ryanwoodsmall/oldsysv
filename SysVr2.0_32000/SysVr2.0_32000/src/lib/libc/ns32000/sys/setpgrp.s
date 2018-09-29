	.file	"setpgrp.s"
#	@(#)setpgrp.s	1.3
# C library -- setpgrp, getpgrp

	.set	setpgrp,39
	.globl	_setpgrp
	.globl	_getpgrp

_setpgrp:
	MCOUNT
	movqd	1,tos
	bsr	pgrp
	adjspb	$-4
	ret	0

_getpgrp:
	movqd	0,tos
	bsr	pgrp
	adjspb	$-4
	ret	0

pgrp:
	addr	setpgrp,r0
	addr	4(sp),r1
	svc
	ret	0
