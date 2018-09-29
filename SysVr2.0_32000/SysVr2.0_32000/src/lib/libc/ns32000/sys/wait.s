	.file	"wait.s"
#	@(#)wait.s	1.3
# C library -- wait

# pid = wait(0);
#   or,
# pid = wait(&status);

# pid == -1 if error
# status indicates fate of process, if given

	.set	syswait,7
	.globl	_wait
	.globl	cerror

	.align	2
_wait:
	MCOUNT
	addr	syswait,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	cmpqd	0,4(sp)		# status desired?
	beq	nostatus	# no
	movd	r1,0(4(sp))	# store child's status
nostatus:
	ret	0
