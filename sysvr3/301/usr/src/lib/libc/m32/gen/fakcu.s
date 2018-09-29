#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
.ident	"@(#)libc-m32:gen/fakcu.s	1.3"
# dummy cleanup routine if none supplied by user.

	.globl	_cleanup

_cleanup:
	RET
