	.file	"cerror.s"
#	@(#)cerror.s	1.4
# C return sequence which
# sets errno, returns -1.

.globl	cerror
.globl	_errno

	.data
_errno:	.long	0

	.text
cerror:
	movl	r0,_errno
	mnegl	$1,r0
	ret
