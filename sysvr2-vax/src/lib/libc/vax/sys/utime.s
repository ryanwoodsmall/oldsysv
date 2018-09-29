	.file	"utime.s"
#	@(#)utime.s	1.5
# C library -- utime
 
#  error = utime(string,timev);
 
	.set	utime,30
.globl	_utime
.globl	cerror
 
_utime:
	.word	0x0000
	MCOUNT
	chmk	$utime
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
