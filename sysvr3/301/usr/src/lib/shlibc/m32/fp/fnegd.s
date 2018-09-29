#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fnegd.s"
.ident	"@(#)libc-m32:fp/fnegd.s	1.3"
#	double _fnegd(srcD)
#	double srcD;

	.text
	.align	4
	.def	_fnegd;	.val	_fnegd;	.scl	2;	.type	047;	.endef
	.globl	_fnegd
_fnegd:
	MCOUNT
	xorw3	&0x80000000,0(%ap),%r0
	movw	4(%ap),%r1
	RET
	.def	_fnegd;	.val	.;	.scl	-1;	.endef

