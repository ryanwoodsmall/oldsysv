	.file	"cerror.s"
#	@(#)cerror.s	1.3
# C return sequence which
# sets errno, returns -1.

	.globl	cerror
	.globl	_errno

	.data
_errno:	.double	0

	.text
cerror:
	movd	r0,_errno
	movqd	-1,r0
	ret	0
