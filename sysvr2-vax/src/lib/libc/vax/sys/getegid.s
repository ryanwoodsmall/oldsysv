	.file	"getegid.s"
#	@(#)getegid.s	1.5
# C library -- getegid

# gid = getegid();
# returns effective gid

	.set	getgid,47
.globl	_getegid

_getegid:
	.word	0x0000
	MCOUNT
	chmk	$getgid
	movl	r1,r0
	ret
