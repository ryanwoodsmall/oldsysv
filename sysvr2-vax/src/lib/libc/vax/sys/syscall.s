	.file	"syscall.s"
#	@(#)syscall.s	1.5
# C library -- syscall

#  Interpret a given system call

.globl	_syscall
.globl	cerror

_syscall:
	.word	0x0000
	MCOUNT
	movl	4(ap),r0	# syscall number
	subl3	$1,(ap)+,(ap)	# one fewer arguments
	chmk	r0		# do it
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
