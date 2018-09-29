#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.ident	"@(#)libc-m32:fp/iotrapset.s	1.4"
	.file	"iotrapset.s"

# IOTRAPSET( newOE ) -- enable and disable integer overflow trapping
#
# USAGE:
#	#include	"p754.h"
#	int	old_io, iotrapset();
#	old_io = iotrapset( FP_ENABLE  );
#	old_io = iotrapset( FP_DISABLE );
#
# DESIGN:
#	Changing the trap setting requires modifying the OE bit in
#	the PSW register, which is not writable at user level.
#	The return-from-gate instruction can modify the PSW,
#	but has a restartability problem on paging systems.
#	The following solution gets around the problem by always
#	RETGing to an instruction on the same (2k-byte) page as the
#	RETG instruction itself.
#	RETG is a macro-ROM (two byte) opcode, and if the opcode
#	crosses a page boundary, it is safe to use the RET on the
#	page containing the first byte of the RETG, since the RETG
#	will not succeed until both pages are available.
#
# CREDIT:
#	This elegant solution was suggested by B.K.Strelioff.
#
# PSEUDOCODE:
# int			/* return previous value of OE		*/
# iotrapset( newOE )	/* set PSW OE (overflow enable) bit	*/
#	int	newOE;
# [1]	{
# [2]	newOE &= 0x01;	/* discard all but bottom bit of newOE */
# [3]	%r0 = current value of PSW<OE> (to return to caller);
# [4]	push &.RET1;
# [5]	push PSW with new value of OE;
# [6]	stacked-PSW<IO> = 0; /* clear IO to prevent a trap	*/
# [7]	if ( .RET1 and .RETG are on different pages )
#		/* on 32001 chips, RETG may not be restartable */
#		change the stacked PC to point to &.RET2;
# [10]	goto .RETG;
#
#	.RET1:
# [13]		RET
#	.RETG:
# [15]		RETG
#	.RET2:
# [17]		RET
#	}
#
# OPTIMIZATIONS?:
#	If newOE == oldOE, we could return immediately,
#	    at a cost of 1 word of text, and a jump usually not taken.
#	    However, this code is more straightforward.
#	If .RET1 and .RETG are on the same page, they stay that way.
#	    We could set a one-time flag on first call, and then do
#	    the right thing immediately on each subsequent entry.
#	    It is not clear that we would gain much, and this routine
#	    should not be a "high runner" function, so why complicate
#	    things?
#
	.align	4
	.globl	iotrapset
	.def	iotrapset; .val iotrapset; .scl 2;   .type 044; .endef
	.def	.bf;       .val ..0;       .scl 101; .line 67;  .endef
	.def	.ef;       .val ..1;       .scl 101; .line 17;  .endef
iotrapset:
	# no need for a 'save' since we only use r0, r1, and r2.
	MCOUNT		# for profiling
..0:

	# extract the current value of OE
	#  by extracting into %r0, we have our return value in place
	#  only the bottom bit of 'newOE' is significant
	.ln	2
	andw2	&0x1,0(%ap)
	.ln	3
	extzv	&22,&1,%psw,%r0

	# create a GATE stack frame assuming we 'RETG' to .RET1
	#  copy the current PSW, and then insert the new OE value
	.ln	4
	pushw	&.RET1			# RETG PC
	.ln	5
	pushw	%psw			# RETG PSW
	insv	0(%ap),&22,&1,-4(%sp)

	# turn off the stacked PSW's V bit (bit 19) to avoid getting
	# a trap if we are enabling integer overflow trapping.
	# This may not be necessary, but it is safe.
	.ln	6
	andw2	&0xfff7ffff,-4(%sp)

	# check to see if .RET1 and .RETG are on the same page,
	#  if not, replace the .RET1 in the gate frame with .RET2
	.ln	7
	andw3	&0xfffff800,&.RET1,%r1
	andw3	&0xfffff800,&.RETG,%r2
	cmpw	%r1,%r2
	je	.RETG
	movw	&.RET2,-8(%sp)

	# now, through the magic of hardware, change the PSW<OE> bit
	.ln	10
	jmp	.RETG

	# for restartability, the RETG and its target must both be
	# present in memory; this is accomplished by using a target
	# which is on the same page as the RETG.  One or the other
	# 'RET' will fulfill this requirement.
	.ln	13
.RET1:
	RET
	.ln	15
.RETG:
	RETG
	.ln	17
..1:
.RET2:
	RET

	.def	iotrapset; .val .; .scl -1; .endef
