	.file	"mcount.s"
#	@(#)mcount.s	1.5
# count subroutine called during profiling

.globl	mcount
.comm	countbase,4

mcount:
	movl	(r0),r1
	jneq	.incr
	movl	countbase,r1
	jeql	.return
	addl2	$8,countbase
	movl	(sp),(r1)+
	movl	r1,(r0)
.incr:
	incl	(r1)
.return:
	rsb
