	.file	"signal.s"
#	@(#)signal.s	1.5
# C library -- signal

# signal(n, 0); /* default action on signal(n) */
# signal(n, odd); /* ignore signal(n) */
# signal(n, label); /* goto label on signal(n) */
# returns old label, only one level.

	.set	signal,48
.globl	_signal
.globl  cerror

	.align	2
_signal:
	.word	0x0000
	MCOUNT
	chmk	$signal
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
