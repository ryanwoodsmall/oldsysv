#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fprintf.s"
.ident	"@(#)libc-u3b:print/fprintf.s	1.9"

#-----------------------------------------------------------------------#
#									#
# fprintf for 3b20s							#
#									#
#									#
#									#
#-----------------------------------------------------------------------#


_m4_include_(print.defs)#		Shared definitions for printf family.


#
# fprintf(stream, format [, arg [, arg ...]] )
#
	.globl	fprintf
	.align	4
fprintf:
	save	&.Rcnt			# Save registers.
	MCOUNT
	addw2	&.locals,%sp		# Reserve stack space for locals.
	movw	0(%ap),iop		# First parameter--stream (FILE ptr).
	movw	4(%ap),fmt		# Second parameter--format string.
	movw	%ap,apsave(%sp)		# Save %ap.
	addw2	&8,%ap			# Get pointer to format arguments.
	movw	iop,iophold(%sp)	# Save FILE pointer.
	bitb	&_IOWRT,_flag(iop)	# Check that write flag set
	jnz	.wset
	bitb	&_IORW,_flag(iop)	# Check that write flag set
	jnz	.rwset
	movw	apsave(%sp),%ap		# Restore trashed ptrs
	mnegw	&EOFN,%r0		# Set error return
	ret	&.Rcnt
.rwset:	orb2	&_IOWRT,_flag(iop)	# Set write bit
.wset:
	movw	_ptr(iop),ptr		# ptr = iop->_ptr
	movzbw	_file(iop),%r0		# Get file number
	llsw2	&2,%r0			# and make it into word index.
#
# The following macro call is used for shared libraries
# If SHLIB is defined, substitute _bufendtab for pointer to _bufendtab 
# (_libc__bufendtab)
#
_m4_ifdef_(`SHLIB',
`	addw2	_libc__bufendtab,%r0
	movw	0(%r0),buff_end
',
`	movw	_bufendtab(%r0),buff_end# Pick up copy of buffer-end ptr.
')
	movw	&0,wcount(%sp)		# Initialize output character count.
	jmp	__doprnt		# Do the heavy work.
