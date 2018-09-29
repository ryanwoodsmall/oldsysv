	.file	"profil.s"
#	@(#)profil.s	1.3
# profil

	.set	prof,44
	.globl	_profil

_profil:
	MCOUNT
	addr	prof,r0
	addr	4(sp),r1
	svc
	ret	0
