#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C return sequence which
.ident	"@(#)libc-m32:crt/cerror.s	1.4"
# sets errno, returns -1.
# errno is define in libc-m32:gen/gen_data.s

	.globl	_cerror
	.globl	errno

_cerror:
	MOVW	%r0,errno
	MNEGW	&1,%r0
	RET

