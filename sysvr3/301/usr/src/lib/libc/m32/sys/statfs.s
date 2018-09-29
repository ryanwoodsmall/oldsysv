#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/statfs.s	1.1"
# C library -- statfs

# int
# statfs(filename, sp, len, fstyp);
# char *filename;
# struct statfs *sp;
# int len, fstyp;

	.set	_statfs,35*8

	.globl	statfs
	.globl  _cerror

statfs:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_statfs,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET
