	.file	"exit.s"
#	@(#)exit.s	1.3
# C library -- _exit

# _exit(code)
# code is return in r0 to system
# Same as plain exit, for user who want to define their own exit.

	.set	sysexit,1
	.globl	__exit

	.align	2
__exit:
	MCOUNT
	addr	sysexit,r0
	addr	4(sp),r1
	svc
	bpt
