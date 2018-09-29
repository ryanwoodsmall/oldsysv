	.file	"locore.s"
	.set	UDOT,0x810000
	.set	USIZE,2
	.set	USTACK,UDOT+(USIZE*1024)

	.set	PG_V,1
	.set	PG_KW,2
	.set	PG_UW,6
