	.file	"move.s"
.globl _move
_move:
	movd 12(sp),r0
	movd 4(sp),r1
	movd 8(sp),r2
	movsb
	ret	0
