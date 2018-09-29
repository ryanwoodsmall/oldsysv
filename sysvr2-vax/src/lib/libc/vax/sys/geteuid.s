	.file	"geteuid.s"
#	@(#)geteuid.s	1.5
# C library -- geteuid

# uid = geteuid();
#  returns effective uid

	.set	getuid,24
.globl	_geteuid

_geteuid:
	.word	0x0000
	MCOUNT
	chmk	$getuid
	movl	r1,r0
	ret
