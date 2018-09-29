	.file	"profil.s"
#	@(#)profil.s	1.5
# profil

	.set	prof,44
.globl	_profil

_profil:
	.word	0x0000
	MCOUNT
	chmk	$prof
	ret
