	.file	"alarm.s"
#	@(#)alarm.s	1.3
# C library - alarm

	.set	alarm,27
	.globl	_alarm

	.align	2
_alarm:
	MCOUNT
	addr	alarm,r0
	addr	4(sp),r1
	svc
	ret	0
