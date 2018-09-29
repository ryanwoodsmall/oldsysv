/calls[ 	]*$2,_mtpr/c\
\	mtpr 4(sp),(sp)\
\	addl2 $8,sp
/calls[ 	]*$1,_mfpr/c\
\	mfpr (sp)+,r0
/calls[ 	]*$1,_fubyte/{
	i\
	\	movl (sp)+,r1\
	\	mnegl $1,r0\
	\	prober $3,$1,(r1)\
	\	beql	1f\
	\	movzbl	(r1), r0\
	\1:
	d
}
/calls[ 	]*$1,_fuword/{
	i\
	\	movl	(sp)+,r1\
	\	mnegl	$1,r0\
	\	prober $3,$4,(r1)\
	\	beql	1f\
	\	movl	(r1), r0\
	\1:
	d
}
/calls[ 	]*$3,_bcopy/c\
\	movc3	8(sp),*(sp),*4(sp)\
\	addl2	$12,sp
/calls[ 	]*$2,_bzero/c\
\	movc5	$0,(r0),$0,4(sp),*(sp)\
\	addl2	$8,sp

/ @(#)calls.sed	1.1 /s/ / /
