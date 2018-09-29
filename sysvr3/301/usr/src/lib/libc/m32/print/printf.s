#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"printf.s"
.ident	"@(#)libc-u3b:print/printf.s	1.9"

#-----------------------------------------------------------------------#
#									#
# printf for 3b20s							#
#									#
#									#
#									#
#-----------------------------------------------------------------------#


_m4_include_(print.defs)#		Shared definitions for printf family.


#
# printf(format [, arg [, arg ...]] )
#
	.globl	printf
	.align	4
printf:
	save	&.Rcnt			# Save registers.
	MCOUNT
	addw2	&.locals,%sp		# Reserve stack space for locals.
#
# The following macro call is used for shared libraries
# If SHLIB is defined, substitute iob for pointer to iob (_libc__iob)
#
_m4_ifdef_(`SHLIB',			
`	movw	&stdoutoff,iop
	addw2	_libc__iob,iop
',
`	movw	&_iob+stdoutoff,iop	# Output stream is stdout.
')
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
	addw2	&4,%ap			# Get pointer to format arguments.
	movw	iop,iophold(%sp)	# Save FILE pointer.
	movw	_ptr(iop),ptr		# ptr = iop->_ptr
	movzbw	_file(iop),%r0		# Get file number
	llsw2	&2,%r0			# and make it into word index.
#
# The following macro call is used for shared libraries
# Substitute _bufendtab for pointer to _bufendtab (_libc__bufendtab)
#
_m4_ifdef_(`SHLIB',
`	addw2	_libc__bufendtab,%r0
	movw	0(%r0),buff_end
',
`	movw	_bufendtab(%r0),buff_end# Pick up copy of buffer-end ptr.
')
	movw	&0,wcount(%sp)		# Initialize output character count.
	jmp	__doprnt		# Do the heavy work.
