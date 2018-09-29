#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"iotrapget.s"
.ident	"@(#)libc-m32:fp/iotrapget.s	1.3"
# return the present status of the integer overflow trap
	.globl	iotrapget
iotrapget:
	MCOUNT
	EXTFW	&0,&22,%psw,%r0	#pull one bit out of the PSW
	RET
