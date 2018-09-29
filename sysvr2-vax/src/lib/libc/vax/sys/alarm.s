	.file	"alarm.s"
#	@(#)alarm.s	1.5
# C library - alarm

	.set	alarm,27
.globl	_alarm

	.align	2
_alarm:
	.word	0x0000
	MCOUNT
	chmk	$alarm
	ret
