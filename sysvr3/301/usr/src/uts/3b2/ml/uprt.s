#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


	.ident	"@(#)kern-port:ml/uprt.s	10.3"
#
# This is a run time startup routine for the portion of the
# UNIX boot which runs in physical mode.
#
	.globl	pstart_s
	.globl	pstart
	.globl	phys_pcb

	.set	slb,0xc		# offset to stack-lower-bound in pcb
	.set	sub,0x10	# offset to stack-upper-bound in pcb
	.set	u_pcbp,0x6c	# offset to pcb pointer in u-block.
	.set	u_kpcb2,0x7c	# offset of u.u_kpcb:

	.text

pstart_s:
	MOVW	0(%ap),autoconfig
	MOVAW	pstack,%sp
	MOVW	%sp,%ap
	MOVW	%sp,%fp

	MOVAW	phys_pcb,%pcbp	# initialize physical pcb
	MOVW	%sp,slb(%pcbp)
	MOVAW	1024(%sp),sub(%pcbp)

	CALL	0(%sp),pstart
pstart_loop:
	WAIT
	BRB	pstart_loop

	.data
	.align	4
pstack:
	.zero	1024


#	pinset(pclkno) 
#
# We have been called by pstart.
# At this time, the general purpose registers are loaded with values
# to set up the process 0 environment in virtual mode.
#
# The following parameters are passed to vstart_s.
#	r8	Physical click nbr of next free page.
#	r7	The autoconfig flag from mcp.
#

	.globl  u
	.globl  userstack
	.globl  krnl_isp
	.globl	vstart_s
	.globl	pinset

	.text

pinset:	
	MOVW	0(%ap),%r8		# pclkno
	MOVW	autoconfig,%r7
	MOVAW	u+u_kpcb2,%pcbp		# Set pointer to kernel pcb.
	MOVAW	krnl_isp,%isp

	MOVAW	vstart_s,%r0
	ENBVJMP	


#
# The flag passed by lboot is stored here.  Zero means "an auto
# config boot was not done", and non-zero means "an auto-config
# boot was done".
#
	.data
	.align  4
autoconfig:
	.word   0
