	.file	"getuid.s"
#	@(#)getuid.s	1.5
# C library -- getuid

# uid = getuid();
#  returns real uid

	.set	getuid,24
.globl	_getuid

_getuid:
	.word	0x0000
	MCOUNT
	chmk	$getuid
	ret
