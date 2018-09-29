/	@(#)setpgrp.s	1.3
/	C library s/getpgrp

.globl	_setpgrp, _getpgrp
.setpgrp =	39.

_setpgrp:
	MCOUNT
	mov	$1,r0
	jbr	1f
	
_getpgrp:
	MCOUNT
	clr	r0
	
1:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.setpgrp
	mov	(sp)+,r5
	rts	pc
