#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libg:m32/libg.s	1.6"
	.file	"libg.s"
##	for SDB on 3B2/3B5/3B15
##	warning--functions returning structures are not treated.
##	To implement calling a USER subroutine from sdb
##	[setup and called from xeq.c/doscall()]
##	[checked as return point in machdep.c/prfrx()]
##	This becomes libg.a, which gets loaded when cc is given -g flag
##
	.globl	__dbargs
	.bss	__dbargs,512,4
##		sdb doscall() sets up _dbargs bytes as follows:
#				0->3:	address
#				4->7:	number of arguments, max=14
#				8->11:	first word of arguments
#				 ... :	more words of arguments
#				60->63:	last word of arguments
#				64->511:	space for strings
##  sdb writes args to __dbargs.  __dbsubc copies onto the user stack.
	.text
	.globl	__dbsubc		# entry point for sdb
	.globl	__dbsubn		# on return to sdb,PC=__dbsubn
__dbsubc:
	movw	&0,%r1
dbs1:
	cmpw	%r1,__dbargs+4		# %r1 >= num args ?
	jge	dbcall			# if so, exit loop
	llsw3	&2,%r1,%r0		# words to bytes
	pushw	__dbargs+8(%r0)		# first arg at __dbargs+8
	addw2	&1,%r1			# increment arg counter
	jmp	dbs1			# loop
# on 3B5/3B15 the CALL instruction does not need an immediate operand.
# modified code, so sdb does not need to overwrite #_args.
dbcall:					# leaving loop,r1=#_args_pushed
	llsw3	&2,%r1,%r0		# r0<- #_bytes_pushed
	subw3	%r0,%sp,%r0		# r0<-"where AP should point"
	CALL	0(%r0),*$__dbargs	# non-IS25 form of call
__dbsubn:		# immediately after call instr; get back to sdb
	BPT		# BPT is non-IS25 (takes no flag)
dbl:
	jmp	dbl	# should never get here !
