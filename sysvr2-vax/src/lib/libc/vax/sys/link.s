	.file	"link.s"
#	@(#)link.s	1.5
# C library -- link

# error = link(old-file, new-file);

	.set	link,9
.globl	_link
.globl	cerror

_link:
	.word	0x0000
	MCOUNT
	chmk	$link
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
