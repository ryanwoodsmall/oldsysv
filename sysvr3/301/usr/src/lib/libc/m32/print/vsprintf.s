#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"vsprintf.s"
.ident	"@(#)libc-u3b:print/vsprintf.s	1.4"

#---------------------------------------------------------------#
#								#
# vsprintf for 3b20s						#
#								#
#								#
#								#
#---------------------------------------------------------------#


_m4_include_(print.defs)#	Shared definitions for printf family.


#
# vsprintf(destination buffer, format [, arg [, arg ...]] )
#
	.globl	vsprintf
	.align	4
vsprintf:
	save	&.Rcnt			# Save registers.
	MCOUNT
	addw2	&.locals,%sp		# Reserve stack space for locals.
	movw	0(%ap),ptr		# First parameter--destination.
	movw	4(%ap),fmt		# Second parameter--format string.
	movw	%ap,apsave(%sp)		# Save %ap.
	addw2	&8,%ap			# Get varargs pointer
	movw	0(%ap),%ap		# Get ptr to arguments
	mnegw	&1,buff_end		# Establish high buffer end pointer.
	movw	ptr,outstart(%sp)	# Remember where output started.
	jmp	__doprnt		# Do the heavy work.
