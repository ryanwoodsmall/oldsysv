#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


	.file	"sprintf.s"
	.ident	"@(#)kern-port:debug/sprintf.s	10.2"

#-----------------------------------------------------------------------#
#									#
# sprintf for 3b20s							#
#									#
#									#
#									#
#-----------------------------------------------------------------------#


_m4_include_(print.defs)#		Shared definitions for printf family.


#
# sprintf(destination buffer, format [, arg [, arg ...]] )
#
	.globl	sprintf
	.align	4
sprintf:
	save	&.Rcnt			# Save registers.
	MCOUNT
	addw2	&.locals,%sp		# Reserve stack space for locals.
	movw	0(%ap),ptr		# First parameter--destination.
	movw	4(%ap),fmt		# Second parameter--format string.
	movw	%ap,apsave(%sp)		# Save %ap.
	addw2	&8,%ap			# Get pointer to format arguments.
	mnegw	&1,buff_end		# Establish high buffer end pointer.
	movw	ptr,outstart(%sp)	# Remember where output started.
	jmp	__doprnt		# Do the heavy work.
