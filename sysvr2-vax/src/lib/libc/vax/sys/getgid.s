	.file	"getgid.s"
#	@(#)getgid.s	1.5
# C library -- getgid

# gid = getgid();
# returns real gid

	.set	getgid,47
.globl	_getgid

_getgid:
	.word	0x0000
	MCOUNT
	chmk	$getgid
	ret
