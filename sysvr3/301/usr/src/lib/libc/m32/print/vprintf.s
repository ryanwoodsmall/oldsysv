#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"vprintf.s"
.ident	"@(#)libc-u3b:print/vprintf.s	1.9"

#---------------------------------------------------------------#
#								#
# vprintf for 3b20s						#
#								#
#								#
#---------------------------------------------------------------#


_m4_include_(print.defs)#	Shared definitions for printf family.


#
# vprintf(format [, arg [, arg ...]] )
#
	.globl	vprintf
	.align	4
vprintf:
	save	&.Rcnt			# Save registers.
	MCOUNT
	addw2	&.locals,%sp		# Reserve stack space for locals.
	movw	&_iob+stdoutoff,iop	# Output stream is stdout.
	bitb	&_IOWRT,_flag(iop)	# Check that write flag set
	jnz	.wset
	bitb	&_IORW,_flag(iop)	# Check that read-write flag set
	jnz	.rwset
	mnegw	&EOFN,%r0		# Set error return
	ret	&.Rcnt
.rwset:	orb2	&_IOWRT,_flag(iop)	# Set write bit
.wset:
	movw	0(%ap),fmt		# First parameter--format string.
	movw	%ap,apsave(%sp)		# Save %ap.
	addw2	&4,%ap			# Get var_arg pointer
	movw	0(%ap),%ap		# Get pointer to start of args
	movw	iop,iophold(%sp)	# Save FILE pointer.
	movw	_ptr(iop),ptr		# ptr = iop->_ptr
	movzbw	_file(iop),%r0		# Get file number
	llsw2	&2,%r0			# and make it into word index.
#
 	movw	_bufendtab(%r0),buff_end # Pick up copy of buffer-end ptr.
	movw	&0,wcount(%sp)		# Initialize output character count.
	jmp	__doprnt		# Do the heavy work.
