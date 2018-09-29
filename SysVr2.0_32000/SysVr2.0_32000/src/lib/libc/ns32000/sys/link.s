	.file	"link.s"
#	@(#)link.s	1.3
# C library -- link

# error = link(old-file, new-file);

	.set	link,9
	.globl	_link
	.globl	cerror

_link:
	MCOUNT
	addr	link,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
