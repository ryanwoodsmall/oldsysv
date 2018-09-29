#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C-library sbrk
.ident	"@(#)libc-m32:sys/sbrk.s	1.6"

# oldend = sbrk(increment);

# sbrk gets increment more core, and returns a pointer
#	to the beginning of the new core area
#
# The m4 macro calls are for shared libraries.
# If SHLIB is defined, substitute end for a pointer to end (_libc__end)
#

	.globl	sbrk

sbrk:
	MCOUNT
	SAVE	%r8
_m4_ifdef_(`SHLIB',
`	ADDW3	_libc_end,0(%ap),%r8
',
`	ADDW3   nd,0(%ap),%r8
')
	PUSHW	%r8
	CALL	-4(%sp),brk
 	TSTW	%r0
 	jne	brkerr
_m4_ifdef_(`SHLIB',
`	MOVW	_libc_end,%r0
',
`	MOVW    nd,%r0
')
	SUBW2	0(%ap),%r0
brkerr:
	RESTORE	%r8
	RET

# brk(value)
# as described in man2.
# returns 0 for ok, -1 for error


	.set	_break,17*8

	.globl	brk
	.globl	_cerror

brk:
	MOVW	&4,%r0
	MOVW	&_break,%r1
	GATE
	jgeu	noerr
	JMP	_cerror
noerr:
_m4_ifdef_(`SHLIB',
`	MOVW	0(%ap),_libc_end
',
`	MOVW    0(%ap),nd
')
	CLRW	%r0
	RET
_m4_ifdef_(`SHLIB',
`',
`	.data
nd:
	.word	end
')
