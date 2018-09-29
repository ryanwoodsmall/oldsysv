#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/fstatfs.s	1.1"
# C library -- fstatfs

# int
# fstatfs(fd, sp, len, fstyp);
# int fd;
# struct statfs *sp;
# int len, fstyp;

	.set	_fstatfs,38*8

	.globl	fstatfs
	.globl  _cerror

fstatfs:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_fstatfs,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET
