#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fptrap.s"
	.ident	"@(#)libc-m32:gen/fptrap.s	1.2"

	.globl	fptrap
fptrap:
        SAVE    %r3
	MCOUNT
        ADDW2   &localvar,%sp
fpreent:			    # this is the point we return to 
				    # when we are executing the n+1th
				    # floating point instruction in a
				    # contiguous sequence of floating
				    # point instructions (floating
				    # pointlessly forever?)
        MOVW    0(%ap),regsave      # register save area address
	MOVW	0(%ap),%r0
        MOVW    60(%r0),pc          # program counter
        MOVW    pc,cpc              # current program counter = pc
        jsb     fetch_opcode
        jsb     set_globals
        MOVW    %fp,%r7
        CLRH    fetchmd             # src
        jsb     get_operand
        CMPH    &9,op2
        je      two_operands
        CMPH    &0xf,op1
        je      get_source
        CMPH    &7,op1
        jgeu    one_operand
two_operands:
        CMPH    &9,op2
        je      get_source
        MOVH    &1,fetchmd          # src/dest
get_source:
        ADDW3   &12,%fp,%r7
        MOVW    fdt(%fp),fdt(%r7)   # fdt & fdc(fp) moved
        MOVW    fdu(%fp),fdu(%r7)
        MOVW    fdl(%fp),fdl(%r7)
        MOVW    %fp,%r7
        jsb     get_admode
        jsb     get_operand
        ADDW3   &12,%fp,%r7
        MOVW    %r3,dest
one_operand:
        jsb     compute_result
        CMPH    &0xf,op1
        je      fp_ret
        CMPH    &9,op2
        je      get_dest
        CMPH    &7,op1
        jlu     store_result
get_dest:
        MOVH    &2,fetchmd          # dest
        jsb     get_admode
        jsb     get_operand
        MOVW    %r3,dest
store_result:
	CMPH	&1,nostore	# was a nan or infinity being converted to integer?
	je	fp_ret
        MOVW    dest,%r4
        CMPH    &0xb,op1
        jl     word_store
        CMPH    &0xc,op1
        jg     word_store
# see if it is a register we are storing
	CMPH	&4,admode
	je	strreg	# if so, branch
# store in memory
        MOVH    fdu(%fp),{shalf}0(%r4)
        jmp     fp_ret
# store halfword in register
strreg:	MOVH	fdu(%fp),{sword}0(%r4)
	jmp	fp_ret
word_store:
        MOVW    fdu(%fp),0(%r4)
        CMPH    mode_precision,&single
        je     fp_ret
	CMPH	&0xd,op1	# see if double to fw converts
	je	fp_ret
	CMPH	&0xe,op1
	je	fp_ret
        MOVW    fdl(%fp),4(%r4)
fp_ret:
        MOVW    regsave,%r0
update_psw:
        INSFH   &3,&18,pswflgs,{uword}44(%r0)   # update PSW
new_pc:
        MOVW    cpc,60(%r0)                     # update pc
# a check to see if the trace enable bit is on in the PSW
# If on, skip the check for a floating point opcode in the next
# instruction.  
	BITW	&0x00020000,44(%r0)
	jne	re_store
# now, to make things just a little more efficient we check to see 
# if the next instruction has a floating point opcode. If it does
# then we fudge up the registers correctly and branch up to the 
# top of this routine to interpret that instruction, and so on.
# This relies on a table called _fpopcode which is located in fcrt0.s 
# and fmcrt0.s
	.globl	_fpopcode
	MOVB	*cpc,%r0			# get next opcode
	TSTB	_fpopcode(%r0)			# look opcode up in table
	jne	fpreent				# if 1, this is fp opcode
re_store:
        RESTORE %r3
        RET
        .data
        .set    fdt,0                           # Tag offset
        .set    fdc,2
        .set    fdu,4                           # Upper operand offset
        .set    fdl,8                           # Lower operand offset
#
#       Instruction decoding
#
        .bss    op1,2,2
        .bss    op2,2,2
        .bss    admode,2,2
        .bss    regno,2,2
        .bss    lit,2,2
#
#       Mode of operation
#
        .bss    mode_round,2,2
        .bss    mode_precision,2,2
        .bss    mode_denorm,2,2
        .bss    mode_inf,2,2
#
#       Error flags
#
        .bss    err_inexact,2,2
        .bss    err_invalid,2,2
        .bss    err_div_zero,2,2
        .bss    err_oflow,2,2
        .bss    err_uflow,2,2
        .bss    err_denorm_op,2,2
#       .bss    err_trap_nan,2,2
        .bss    err_int_fault,2,2
#
#       Destination structure
#
        .bss    unnorm_ok,2,2
        .bss    denorm_ok,2,2
        .bss    othresh,2,2
        .bss    uthresh,2,2
#
        .bss    fetchmd,2,2
        .bss    pswflgs,2,2
        .bss    subflg,2,2
        .bss    divflg,2,2
#
        .set    localvar,24
#
#       Rounding mode
#
        .set    to_nearest,0
        .set    toward_0,1
        .set    toward_plus,2
        .set    toward_minus,3
#
#       Rounding precision control
#
        .set    extended,0
        .set    single,1
        .set    double,2
#
#       Interpretations of denormalised numbers
#
        .set    warning,0
        .set    normalising,1
#
#       Interpretations of infinities
#
        .set    projective,0
        .set    affine,1
#
#       Tag types
#
        .set    zerotag,0
        .set    inftag,1
        .set    t_nantag,2
        .set    n_nantag,3
        .set    numtag,4
#
        .bss    shiftcnt,2,2
        .bss    maxshft,2,2
        .bss    maxexp,2,2
        .bss    exponent,2,2
        .bss    bias,2,2
        .bss    bias2,2,2
        .bss    exp,2,2
        .bss    expw,2,2
        .bss    mant,2,2
        .bss    cond,2,2
        .bss    isnan,2,2
        .bss    stky,2,2
        .bss    size,2,2
        .bss    nostore,2,2
        .align  4
        .bss    pc,4,4
        .bss    cpc,4,4
        .bss    regsave,4,4
        .bss    temp1,4,4
        .bss    temp2,4,4
        .bss    dest,4,4
        .bss    rstky,4,4
	.bss	tempvar,4,4
#
        .text
######################################################################
#
# This routine fetches the opcode of the floating point instruction
# to be executed.
#
fetch_opcode:
       MOVW    regsave,%r0           # r0 = regsave area
       MOVW    cpc,%r1               # r1 = pc
       MOVB    0(%r1),%r2            # 1st byte of instruction
       EXTFB   &3,&4,%r2,{uhalf}op1
       EXTFB   &3,&0,%r2,{uhalf}op2
       INCW    cpc                   # pc bumped up by 1
get_admode:
       MOVW    cpc,%r1
       MOVB    0(%r1),%r2
       EXTFB   &3,&4,%r2,{uhalf}admode
       EXTFB   &3,&0,%r2,%r3         # r3 = regno or literal (mode 6 or 7)
       INCW    cpc                   # pc bumped up by 1
       RSB
#####################################################################
#
# This initialisation routine collects state information.
#
set_globals:
        jsb    fetch_mode
        jsb    clear_error
        jsb    set_dest
	CLRH	nostore		# reset store result flag - used in converts to indicate
				# if a nan or infinity to integer convert was
				# attempted. If one was attempted, no store of
				# result will occur. 0 indicates that result
				# should be stored
        RSB
fetch_mode:
        MOVH    &to_nearest,mode_round
        CMPH    &9,op2
        je     precision
        CMPH    &7,op1
        jlu    precision
        MOVH    &toward_0,mode_round
precision:
        MOVH    &double,mode_precision
        CMPH    &1,op2
        jne    op2chk
        MOVH    &single,mode_precision
        jmp     other_modes
op2chk:
        CMPH    &9,op2
        jne    other_modes
        CMPH    &7,op1
        jgeu   other_modes
        MOVH    &single,mode_precision
other_modes:
        MOVH    &normalising,mode_denorm
        MOVH    &affine,mode_inf
        RSB
clear_error:
        CLRH    pswflgs
        CLRH    subflg
        CLRH    err_inexact
        CLRH    err_invalid
        CLRH    err_div_zero
        CLRH    err_oflow
        CLRH    err_uflow
        CLRH    err_denorm_op
        CLRH    err_int_fault
        RSB
set_dest:
        CLRH    unnorm_ok
        MOVH    &1,denorm_ok
        CMPH    mode_precision,&double
        jne     sngl_precis
dbl_precis:
        MOVH    &55,maxshft
        MOVH    &52,maxexp
        MOVH    &1023,bias
        MOVH    &2047,bias2
        MOVH    &20,exp
        MOVH    &10,expw
        MOVH    &19,mant
        MOVH    &1023,othresh
        MOVH    &-1022,uthresh
        RSB
sngl_precis:
        MOVH    &26,maxshft
        MOVH    &23,maxexp
        MOVH    &127,bias
        MOVH    &255,bias2
        MOVH    &23,exp
        MOVH    &7,expw
        MOVH    &22,mant
        MOVH    &127,othresh
        MOVH    &-126,uthresh
        RSB
#########################################################################
#
# This routine fetches the next operand
#
get_operand:
       CMPH    &2,fetchmd			# check to see if dest fetch
       je     get_op_dest			# if so skip next step
       CLRW    fdl(%r7)			# clear lower half location
						# for fetch data just in case
						# it is a single
get_op_dest:
       CMPH    &3,admode
       jleu   litmode
       CMPH    &14,admode
       jgu    litmode
       je     absdef
       CMPH    &5,admode
       jleu   regmode
       CMPH    &7,admode
       jleu   offsmod
       CMPH    &12,admode
       jgeu   bdispl
       jmp     displ
litmode:
       CMPH    fetchmd,&0
       jne    fpabort
       CMPH    &9,op2
       je     fpabort
       CMPH    &7,op1
       jlu    fpabort
       CMPH    &8,op1
       jgu    fpabort
       CMPH    &15,admode
       je     neglit
       EXTFB   &5,&0,%r2,{uword}fdu(%fp)
       RSB
neglit:
       MOVW    &0xffffffff,fdu(%fp)
       EXTFB   &5,&0,%r2,%r2
       INSFB   &5,&0,%r2,{uword}fdu(%fp)
       RSB
regmode:
       CMPB    %r3,&15
       je     immediate
       MULW2   &4,%r3               # r3 = offset into register save area
       ADDW2   regsave,%r3          # r3 = address of operand or operand's EA
       CMPH    &4,admode
       jne     deffer
# see if the operand in the register is a halfword. If so,
# then increment address by 2 to point to the correct halfword
# in the register
	CMPH	&9,op2
	je	operand
	CMPH	fetchmd,&2	## source or dest
	je	rmod_ret
#source operand, see if halfword source
	CMPH	&7,op1
	jne	operand
	ADDW2	&2,%r3	# is halfword, so point to right half
	jmp	operand
# destination operand, see if halfword destination
# register deferred
deffer:	MOVW    0(%r3),%r3           # r3 = address of operand
operand:
       CMPH    fetchmd,&2           # check if destination
       je     rmod_ret
       CMPH    &9,op2
       je     get_word
       CMPH    &7,op1
       jne    get_word
       MOVH    0(%r3),{sword}fdu(%fp)
       jmp     rmod_ret
get_word:
       MOVW    0(%r3),fdu(%r7)      # get operand
       CMPH    mode_precision,&single
       je     rmod_ret
       MOVW    4(%r3),fdl(%r7)
rmod_ret:
       RSB
immediate:
       CMPH    fetchmd,&0          # check if source only
       jne    fpabort             # an immediate is an invalid destination
       jsb    himm
       MOVH    {shalf}%r2,{sword}fdu(%r7)
       CMPH    &5,admode
       je     rmod_ret
       jsb    himm
       INSFW   &15,&16,%r2,fdu(%r7)
       RSB
himm:
       MOVW    cpc,%r8
       MOVB    1(%r8),{uword}%r2
       LLSW3   &8,%r2,%r2
       ADDB2   0(%r8),{uword}%r2
       ADDW2   &2,cpc                    # pc bumped up by 2
       RSB
offsmod:
       MOVB    %r3,{shalf}lit         # literal to be added to c(fp) or c(ap)
       CMPH    lit,&15
       je     abschk
       ADDH2   &3,admode              # admode 6 = r9 = fp, admode 7 = r10 = ap
       MULW3   &4,{uhalf}admode,%r3     
       ADDW2   regsave,%r3            # address of contents of fp or ap
       ADDW3   {shalf}lit,{uword}0(%r3),%r3   # r3 = address of operand
       jmp     operand
abschk:
       CMPH    &6,admode
       jne    absmod
       CMPH    &0,fetchmd
       jne    fpabort
       MOVB    {sbyte}*cpc,{sword}fdu(%r7)
       INCW    cpc
       RSB
absmod:
       jsb    himm
       MOVW    %r2,%r3
       jsb    himm
       INSFW   &15,&16,%r2,%r3
       jmp     operand
absdef:
       CMPB    %r3,&15
       jne    fpabort
       jsb    himm
       MOVW    %r2,%r3
       jsb    himm
       INSFW   &15,&16,%r2,%r3
       MOVW    0(%r3),%r3                # defer
       jmp     operand
bdispl:
       CMPH    &12,admode
       jne    get_offset
       CMPB    %r3,&15
       je     dimm
get_offset:
       MULW2   &4,%r3               # r3 = offset from regsave area
       ADDW2   regsave,%r3          # r3 = address of the register
       MOVW    cpc,%r8
       MOVB    {sbyte}0(%r8),{sword}%r4
       INCW    cpc
       ADDW3   0(%r3),%r4,%r3
       CMPH    &13,admode
       jne    operand
       MOVW    0(%r3),%r3
       jmp     operand
dimm:
       jsb    immediate
       MOVW    fdu(%r7),%r3
       jsb    immediate
       MOVW    fdu(%r7),fdl(%r7)
       MOVW    %r3,fdu(%r7)
       RSB
displ:
       MULW2   &4,%r3                # r3 = offset from regsave
       ADDW2   regsave,%r3           # r3 = address of register
       jsb    himm
       CMPH    &10,admode
       jlu    word_disp
       MOVH    {shalf}%r2,{sword}%r4
       jmp     dsp_sum
word_disp:
       MOVW    %r2,%r4
       jsb    himm
       INSFW   &15,&16,%r2,%r4
dsp_sum:
       ADDW3   0(%r3),%r4,%r3        # r3 = operand or operand's EA
       EXTFH   &0,&0,admode,%r2     # check if admode odd (i.e., mode 7 or 9)
       CMPH    %r2,&1
       jne    operand
       MOVW    0(%r3),%r3
       jmp     operand
#######################################################################
#
# Determine what operation is to be performed on the operand(s).
#
compute_result:
       CMPH    &0xf,op1
       je     compare
       CMPH    &0xa,op1
       jgu    flt_int
       CMPH    &4,op1
       jlu    add
       je     subtract
       CMPH    &6,op1
       jlu    multiply
       je     divide
       CMPH    &9,op2
       je     arithmetic
       CMPH    &9,op1
       jlu    int_flt
       ADDH2   op1,op2
       CMPH    &0xb,op2
       je     sngl_dbl
       CMPH    &0xe,op2
       je     dbl_sngl
# we have movss or movdd---set flags and store result
	EXTFW	&0,&31,fdu(%fp),tempvar	# get sign of source
	je	tstzero
	ORH2	&0x8,pswflgs		# set N if sign is negative
tstzero:
	EXTFW	&30,&0,fdu(%fp),tempvar	# get first half to test for 0
	ORW2	fdl(%fp),tempvar	# or in second half
	jne	retcomp
	ORH2	&0x4,pswflgs		# set Z if equal to 0
retcomp:
       RSB                    
arithmetic:
       CMPH    &8,op1
       jlu    add
       je     subtract
       CMPH    &0xa,op1
       jlu    multiply
       jmp     divide
######################################################################
#
# Unpack a number, num.  It is assumed initially that num is a typical
# number.  All special cases are then checked.
#
unpack:
        MOVH    &numtag,fdt(%r7)
        EXTFW   {uhalf}mant,&0,{uword}fdu(%r7),%r0
        MOVW    fdl(%r7),%r1
        MOVH    expw,shiftcnt
        MOVH    &99,exponent
        jsb    shift_left
        EXTFH   expw,exp,{uword}fdu(%r7),{uhalf}fdc(%r7)
        MOVH    fdc(%r7),%r4
        SUBH2   bias,fdc(%r7)
        ORW3    %r0,%r1,%r5          # r0,r1 is checked for fraction = 0
#                                         prior to inserting the implied 1
        ORW2    &0x40000000,%r0      # leading 1 inserted
        CMPH    %r4,&0
        jne    inf_or_nan
        CMPW    %r5,&0
        jne    denormalised
normal_0:
        MOVH    &zerotag,fdt(%r7)
        RSB
denormalised:
        MOVB    &1,err_denorm_op
        INSFW   &0,&30,&0,%r0         # denormalised numbers have a leading 0
        INCH    fdc(%r7)
#       CMPH    mode_denorm,&normalising
#       jne    finish
        jsb    norm
        RSB
inf_or_nan:
        CMPH    %r4,bias2
        jne    finish
        MOVH    &inftag,fdt(%r7)
        CMPW    %r5,&0
        je     finish
nan:
        MOVH    &n_nantag,fdt(%r7)
finish:
        RSB
########################################################################
#
# This routine normalises the number in r0,r1.
#
norm:
        ORW3    %r1,%r0,%r8
        jne    norm_loop
        MOVH    &zerotag,fdt(%r7)              # special case of normal zero
        RSB
norm_loop:
        EXTFH   &0,&30,{uword}%r0,%r8          # bit lost in shift
        CMPB    %r8,&0
        jne    norm_end
        EXTFW   &0,&31,%r1,%r8
        LLSW3   &1,%r1,%r1
        LLSW3   &1,%r0,%r0
        ORW2    %r8,%r0
        DECH    fdc(%r7)                       # reduce exponent by 1
        jmp     norm_loop
norm_end:
        RSB
#######################################################################
#
# Check for nans
#
nanchk:
        CLRH    isnan
        CMPH    fdt(%fp),&n_nantag
        je     found_nan
        CMPH    fdt(%r7),&n_nantag
        je     found_nan
        RSB
found_nan:
        MOVH    &1,isnan
        RSB
##########################################################################
#
# This routine converts a half or a word to floating point internal
# format and assigns that value to a single or double precision
# destination.
#
int_flt:
        EXTFW   &30,&0,fdu(%fp),%r0
        CMPW    %r0,&0
        jne    flt_nonzero

# we have either a zero or the maximum negative number. Check the 
# sign to determine which we have
	EXTFW	&0,&31,fdu(%fp),%r2
	je	zeroint			# branch if zero
	MOVH	&31,fdc(%fp)		# set exponent and
	MOVH	&numtag,fdt(%fp)	# number tag and
	MOVW	&0x40000000,%r0		# mantissa of max neg number
	CLRW	%r1
	jsb	pack
	RSB
zeroint:
        MOVH    &zerotag,fdt(%fp)
        jsb    pack
        RSB
flt_nonzero:
        MOVH    &numtag,fdt(%fp)
        CLRW    %r1
        EXTFW   &0,&31,fdu(%fp),%r2                    # r2 = sign
        je     get_exp
        MNEGW   %r0,%r0
get_exp:
        MOVH    &30,fdc(%fp)
        jsb    norm
        jsb    trim_result
        jsb    pack
        RSB
#######################################################################
#
# This routine converts a single precision number to double precision.
#
sngl_dbl:
        jsb     unpack
        MOVH    &double,mode_precision
        jsb     set_dest
	CMPH	fdt(%fp),&n_nantag
	BEB	sing_nan
        jsb     pack
	RSB
sing_nan:
	EXTFW	&22,&0,fdu(%fp),%r2              # extract the pc from the nan
	INSFW	&3,&28,%r2,fdl(%fp)              # insert part of the pc into the double
	LRSW3	&4,%r2,%r2
	INSFW	&18,&0,%r2,fdu(%fp)
	INSFH	expw,exp,bias2,{uword}fdu(%fp)   # insert the rest of the pc
        RSB
#########################################################################
#
# This routine converts a double precision number to single precision.
#
dbl_sngl:
	jsb	unpack
        MOVH    &single,mode_precision
        jsb	set_dest
	CMPH	fdt(%fp),&n_nantag
	jne	trim
#
#       We have a NaN.  Must extract the pc from the double NaN. 
#
	EXTFW	&3,&28,fdl(%fp),%r2
	EXTFW	&18,&0,fdu(%fp),%r3
	LLSW3	&4,%r3,%r3
	MOVW	&0x7f800000,fdu(%fp)        # Put in single NaN exponent.
	ORW2	%r2,%r3
	BNEB	stnan
#
#	We have an artificially produced NaN, i.e., one that has been
#	generated by hand rather than by this package.  This will occur
#	usually when tests of this package are written and NaN handling 
#	is to be tested.  The low 23 bits of the second word will most
#	likely contain the nonzero number for these cases.
#
	EXTFW	&22,&0,fdl(%fp),%r3
	BNEB	stnan
#
#       In the case of a NaN, if the upper part of the mantissa is 0, the
#       single precision result will be 0 with an exponent of all ones. 
#       The result will look like an infinity.  To preserve the NaN concept,
#       a 1 is inserted in the lowest bit of the single precision number.
#	This case of an artificially produced NaN will rarely, if ever, occur.
#
	MOVW	&1,%r3
stnan:
	ORW2	%r3,fdu(%fp)
	RSB
#
# We have a zero, an infinity, or a number.
#
trim:
        CMPW    %r1,&0
        je     smove
        ORW2    &0x1,%r0       # sticky bits from r1 included in r0
smove:
        jsb    trim_result
        jsb    pack
        RSB
#########################################################################
#
# This routine converts a single or a double precision number to
# an internal format and assigns that value to a halfword or word.
#
flt_int:
        jsb    unpack
        CMPH    fdt(%fp),&zerotag
        je     flt_A
        CMPH    fdt(%fp),&numtag
        je     flt_B
        jmp     flt_C
#
#       0
#
flt_A:
        CLRW    fdu(%fp)
	ORH2	&0x4,pswflgs
	ANDH2	&0xf7,pswflgs		#zap N flag, if set
        RSB                                        # int(0) is itself
#
#       Typical case of a finite number
#
# if we have a rounding mode, go round the number here before we later
# truncate the lower unused bits
flt_B:
        CMPH    &0xc,op1
        je     mantchk
        CMPH    &0xe,op1
        je     mantchk
        jsb    round_int	# go round number to an integer

# check if the exponent is negative. If so, go return 0 and
# set the inexact flag.
mantchk:
        CMPH    &0,fdc(%fp)
	jge	pos_exp
        ORH2    &0x1,pswflgs                     # C in PSW set
	jmp	flt_A

#here we have a positive exponent. Test to see if the sign of
# the result is to be positive or negative and set the N flag accordingly
pos_exp:
        EXTFW   &0,&31,fdu(%fp),%r4
        je     pos_num
        ORH2    &0x8,pswflgs
# now see if the positive exponent is in range to represent the
# number in a halfword or fullword. If a halfword, then
# the exponent must be <15. If a fullword, then the exponent
# must be <31. Set size to value according to the operation.
pos_num:
        MOVH    &31,size
        CMPH    &0xc,op1
        jg     float_I 
# we have a float to halfword instruction
        MOVH    &15,size
	CMPH	size,fdc(%fp)
	jl	flt_right	# branch if exponent is in range

# check for the special case of the maximum negative number. This
# occurs when the exponent=15, mantissa=8000 (after truncation or
# rounding), and the sign is negative
	CMPH	size,fdc(%fp)	# check for exp=15
	jg	flt_left	# jump if not
	EXTFW	&14,&15,%r0,tempvar	# check for mantissa=8000
	jne	flt_left	# jump if not
	CMPH	&0,%r4		# check for negative sign
	je	flt_left	# jump if not
	MOVW	&0x80000000,fdu(%fp)	# special case of max neg number
# check for imprecise result
	EXTFW	&14,&0,%r0,tempvar
	ORW2	%r1,tempvar
	je	reti
	ORH2	&0x1,pswflgs
reti:	RSB
float_I:
        CMPH    size,fdc(%fp)
        jl     flt_right	# branch if in range
	CMPH	size,fdc(%fp)	# check for exponent=15
	jg	flt_left
        CMPW    &0x40000000,%r0
        jne    flt_left
	EXTFW	&0,&31,%r1,tempvar	# test last bit of mantissa for special case
	jne	flt_left	# if one, not special case, so branch
        CMPH    &0,%r4		# test for negative sign
        je     flt_left
        MOVW    &0x80000000,fdu(%fp)
# check for imprecise result
	MOVW	%r1,tempvar
	je	reti
	ORH2	&0x1,pswflgs
        RSB

# for this section, we have an exponent that is too big, so overflow
# has occurred. Now, we need to set the V flag and determine what
# bits are left after we left shift the mantissa according to
# the exponent
flt_left:
	ORH2	&0x2,pswflgs
# first, see if any bits are left at all, ie if exponent is > maxexp
        ADDW2   size,maxexp
	CMPH	maxexp,fdc(%fp)
	jg	flt_A		# all bits are rotated out, so store zero
	jl	convert		# some bits are left, determine which ones
# test for special case where mantissa last bit =1 and exponent=maxexp
# and sign is negative
        CMPH    mode_precision,&single
	jne	dbl_flt
        EXTFW   &0,&7,%r0,%r4	# test for last bit =1
        je     flt_A		# if not, go store zero
        jmp     weird
dbl_flt:
        EXTFW   &0,&10,%r1,%r4	# test for last bit =1
        je     flt_A		# if not, go store zero
weird:
	EXTFW	&0,&31,fdu(%fp),%r4
	je	flt_A
        MOVW    &0x80000000,fdu(%fp)
        RSB
# at this point, we have a number with exponent in the range
# 15 <= exp < maxexp. We need to shift left the mantissa
# exponent-size+1 times
convert:
        SUBH3   size,fdc(%fp),shiftcnt           # shiftcnt = exponent - size
	INCH	shiftcnt
        jsb    shift_left
# check for inexact result, ie if we truncate bits off the right side of
# the mantissa
	CMPH	&15,size
	je	hwinx
	CMPW	&0,%r1
	je	exactr
	ORH2	&0x1,pswflgs
	jmp	exactr
hwinx:
	EXTFW	&15,&0,%r0,tempvar	# look at rest of bits in r0
	ORW2	%r1,tempvar		# and in r1
	je	exactr
	ORH2	&0x1,pswflgs		# set c flag for inexact

# test for negative number
exactr:
	EXTFW	&0,&31,fdu(%fp),%r4	# test sign
	je	clr31
# test for special case of 8000 or 80000000
	CMPH	&15,size
	je	checkhw
	CMPW	%r0,&0x80000000
	je	flt_ret
clr31:
	EXTFW	&30,&0,%r0,tempvar	# check for zero
	je	flt_A
	jmp	set_sign
checkhw:
	EXTFW	&15,&16,%r0,tempvar
	CMPW	&0x8000,tempvar
	je	flt_ret
	EXTFW	&14,&16,%r0,tempvar
	je	flt_A
	jmp	set_sign
flt_right:
        SUBH3   &1,size,%r6
        SUBH3   fdc(%fp),%r6,shiftcnt          # shiftcnt = 31 - exponent
        jsb    shift_right

# test for imprecise result. This occurs when bits in R1 are nonzero
# or for halfwords, when the low 16 bits of r0 are non-zero
        ORW2    rstky,%r1
	jne	setimp
	CMPH	size,&15
	jne	set_sign	# if fullword, go store result
	EXTFW	&15,&0,%r0,tempvar	# see if low half of r0 is 0
	je	set_sign
setimp:
	ORH2	&0x1,pswflgs	# set imprecise flag
set_sign:
        INSFW   &0,&31,&0,%r0
        je     flt_ret
        EXTFW   &0,&31,fdu(%fp),%r2
        je     flt_ret
        MNEGW   %r0,%r0
flt_ret:
        CMPH    &0xc,op1
        jg     word_ret
	MOVW	%r0,fdu(%fp)
        RSB
word_ret:
	MOVW	%r0,fdu(%fp)
        RSB
#
#       NaN or infinity
#
flt_C:
#       CMPH    fdt(%fp),&n_nantag
#       jne    nantrap
        ORH2    &0x2,pswflgs                # V in PSW set
	MOVH	&1,nostore			# indicate that no store of the
						# destination is to take place
        RSB                                        # result = itself




#---------------------------------------------
# round to integer subroutine for float to int converts
#
round_int:
# subroutine that rounds a number to an integer. Is called by
# the float to integer conversions. %r0,%r1 has mmantissa,
# fdc(%fp) has the exponent

# see if exp <-1 or > maxexp. If so, return since no significant
# bits will be left
	CMPH	&-1,fdc(%fp)
	jl	rndintret	# return if less than -1
	CMPH	maxexp,fdc(%fp)
	jg	rndintret	# return if greater than maxexp

# now see if exponent is in range -1<=exp<=30 or range 31<=exp<=52.
# this will tell us if LSB is in %r0 or %r1, respectively
	CMPH	&31,fdc(%fp)
	jge	largeexp	# if second range, branch

# exponent is in the range -1<=exp<=30. There are three cases here:
# LSB in bit 0, LSB is in bit 1, or LSB is in any other bit.
# handle each case separately, since location of G and Sticky
# vary for each case
	MOVW	&30,%r4
	SUBH2	fdc(%fp),%r4	# r4 is 30-exp = LSB bit #
	CMPW	&1,%r4
	jg	gencase		# branch if general case
	je	nxtcase		# branch if LSB is bit 1

# we have lsb is bit 0 in %r0, G is bit 31 in %r1, and Sticky is bits
# 0-30 of %r1
	EXTFW	&0,&31,%r1,tempvar	# G bit
	je	rndintret		# return if zero
	EXTFW	&30,&0,%r1,tempvar	# S bits
	jne	shroundup		# go round up if G=1,S=1
	EXTFW	&0,&0,%r0,tempvar	# LSB
	jne	shroundup		# go round up if G=1,S=0,LSB=1
	RSB

# we have lsb is bit 1 in r0, G is bit 0 in R0 and sticky is R1
nxtcase:
	EXTFW	&0,&0,%r0,tempvar	# G
	je	rndintret		# return if zero
	CMPW	&0,%r1			# S
	jne	shroundup		# go round if G=1,S=1
	EXTFW	&0,&1,%r0,tempvar	# LSB
	jne	shroundup		# go round if G=1,S=0,LSB=1
rndintret:
	RSB

# general case: LSB is in r0 bit 30-exp=r4, r4-1 is G bit # in r0
# r4-2 is beginning of sticky in r0,r1 is all sticky
gencase:
	DECW	%r4
	EXTFW	&0,%r4,%r0,tempvar	# get G
	je	rndintret		# return if 0
	DECW	%r4
	EXTFW	%r4,&0,%r0,tempvar	# get first half of S
	ORW2	%r1,tempvar		# get rest of S
	jne	Toshroundup		# round up if G=1,S=1
	ADDW2	&2,%r4			# point to LSB
	EXTFW	&0,%r4,%r0,tempvar
	je	rndintret		# Chop if G=1,LSB=0
	jmp	shroundup
Toshroundup:
	ADDW2	&2,%r4			# point to LSB
# round up if G=1,S=1 or G=1,S=0,LSB=1 %r4 is LSB # in R0
shroundup:
	MOVW	&1,tempvar		# used to generate one in correct
	LLSW3	%r4,tempvar,tempvar	# position to round up
	ADDW2	tempvar,%r0		#
	ANDW3	&0x80000000,%r0,tempvar	# see if overflowed into sign bit
	je	rndintret		# return if not

# we overflowed, so shift mantissa right one and increment exponent by 1
ovround:
	LRSW3	&1,%r1,%r1		# shift right lower half
	EXTFW	&0,&0,%r0,tempvar	# get last bit of 1st half
	je	shift0
	ORW2	&0x80000000,%r1		# if 1, move into low half
shift0:
	LRSW3	&1,%r0,%r0		# shift right upper half
	INCH	fdc(%fp)		# and increment exponent
	RSB


# 31<=exp<=52, so 31-(exp-31) is LSB # in r1
#		  31-(exp-31)-1 is G # in r1
#		  31-(exp-31)-2 is first S bit # in r1
largeexp:
	MOVW	&61,%r4
	SUBH2	fdc(%fp),%r4		# r4 = 31-(exp-31)-1
	EXTFW	&0,%r4,%r1,tempvar	# get G
	je	rndintret		# return if 0
	DECW	%r4			# point to S
	EXTFW	%r4,&0,%r1,tempvar	# get S
	jne	Tolnroundup		# round up if G=1,S=1
	ADDW2	&2,%r4			# point to LSB
	EXTFW	&0,%r4,	%r1,tempvar	# get LSB
	je	rndintret		# go chop if G=1,LSB=0,S=0
	jmp	lnroundup
Tolnroundup:
	ADDW2	&2,%r4			# point to LSB

# round up if G=1,S=1 or G=1,S=0,LSB=1     r4 is LSB # in R1
lnroundup:
	MOVW	&1,tempvar		# used to generate 1 in correct
	LLSW3	%r4,tempvar,tempvar	# position to round up
	ADDW2	{uword}tempvar,{uword}%r1	# round up
	jgeu	rndintret		# if no carry, return
	INCW	%r0			# propagate carry into r0
	ANDW3	&0x80000000,%r0,tempvar	# see if overflowed into sign bit
	jne	ovround
	RSB				# return if no overflow
nantrap:
#
#       If trap is disabled, set result to a nontrapping NaN
#
#       RSB
###########################################################################
#
# Set the result equal to op1 multiplied by op2
#
#                      Case Table
#
#                 |  0    NUM   INF   NAN
#            ______________________________
#              0  |  A     A     C     G
#             NUM |  A     B     E     G
#             INF |  C     D     F     G
#             NAN |  G     G     G     G
#
multiply:
        jsb    unpack          
        MOVW    %r0,%r2
        MOVW    %r1,%r3         # op2 in r2,r3
        MOVW    %fp,%r7
        jsb    unpack          # op1 in r0,r1
        ADDW3   &12,%fp,%r7
        jsb    nanchk
        CMPH    &1,isnan
        je     mpy_G
        EXTFW   &0,&31,fdu(%fp),%r4
        EXTFW   &0,&31,fdu(%r7),%r5
        INSFW   &0,&31,&0,fdu(%fp)
        CMPB    %r4,%r5
        je     is_zero
        INSFW   &0,&31,&1,fdu(%fp)
is_zero:
        CMPH    fdt(%fp),&zerotag
        jne    minfchk
        CMPH    fdt(%r7),&inftag
        je     mpy_C
        jmp     mpy_A
minfchk:
        CMPH    fdt(%fp),&inftag
        jne    mnumchk
        CMPH    fdt(%r7),&zerotag
        je     mpy_C
        CMPH    fdt(%r7),&numtag
        je     mpy_D
        jmp     mpy_F
mnumchk:
        CMPH    fdt(%r7),&zerotag
        je     mpy_A
        CMPH    fdt(%r7),&numtag
        je     mpy_B
        jmp     mpy_E
#
#       0*0 or 0*NUM or NUM*0
#
mpy_A:
        MOVH    &zerotag,fdt(%fp)
        jsb    pack
        RSB
#
#       NUM*NUM
#
mpy_B:
        ADDH2   fdc(%r7),fdc(%fp)
        CMPH    mode_precision,&single
        jne    mpy_double
        EXTFW   &9,&21,%r2,%r4                   # BU
        EXTFW   &13,&7,%r2,%r5                   # BL
        EXTFW   &9,&21,%r0,%r2                   # AU
        EXTFW   &13,&7,%r0,%r3                   # AL
        jsb    mpy
        MOVH    &16,shiftcnt
        jsb    shift_left             # b45 -> b61 for round format
mul_fin:
        EXTFW   &0,&31,%r0,%r2                     # check 1st bit
        je     mpy_stky
        MOVH    &1,shiftcnt
        INCH    fdc(%fp)                         # adjust exponent
        jsb    shift_right                      # shift right by 1
mpy_stky:
        CMPH    mode_precision,&double
        je     mul_trm
        CMPW    %r1,&0
        je     mul_trm
        ORW2    &0x1,%r0               # sticky from r1 included in r0
mul_trm:
        jsb    trim_result
        jsb    pack
        RSB
mpy_double:
        MOVH    &6,shiftcnt
        jsb    shift_right
        INSFW   &30,&0,%r0,fdu(%fp)
        MOVW    %r1,fdl(%fp)
        MOVH    &6,shiftcnt
        MOVW    %r2,%r0
        MOVW    %r3,%r1
        jsb    shift_right
        MOVW    %r0,fdu(%r7)
        MOVW    %r1,fdl(%r7)
        EXTFW   &13,&18,fdl(%fp),%r2             # AL1
        EXTFW   &13,&4,fdl(%fp),%r3              # AL2
        EXTFW   &13,&18,fdl(%r7),%r4             # BL1
        EXTFW   &13,&4,fdl(%r7),%r5              # BL2
        jsb    mpy                              # ALBL
d_AUBL:
        MOVH    &28,shiftcnt
        jsb    shift_right
        MOVW    rstky,{uhalf}stky
        MOVW    %r0,temp1
        MOVW    %r1,temp2
        EXTFW   &10,&14,fdu(%fp),%r2             # AU1
        ANDW3   &0x3fff,fdu(%fp),%r3              # AU2
        jsb    mpy                              # AUBL
        ADDW2   {uword}%r1,temp2
        jgeu    add1
        INCW    temp1
add1:
        ADDW2   %r0,temp1
        EXTFW   &13,&18,fdl(%fp),%r2             # AL1
        EXTFW   &13,&4,fdl(%fp),%r3              # AL2
        EXTFW   &10,&14,fdu(%r7),%r4             # BU1
        ANDW3   &0x3fff,fdu(%r7),%r5             # BU2
        jsb    mpy                              # ALBU
        ADDW2   {uword}temp2,%r1
        jgeu    add2
        INCW    %r0
add2:
        ADDW2   temp1,%r0
d_AUBU:
        MOVH    &22,shiftcnt
        jsb    shift_right
        ORW2    rstky,{uhalf}stky
        MOVW    %r0,temp1
        MOVW    %r1,temp2
        EXTFW   &10,&14,fdu(%fp),%r2           # AU1
        ANDW3   &0x3fff,fdu(%fp),%r3           # AU2
        jsb    mpy                            # AUBU
        MOVH    &6,shiftcnt
        jsb    shift_left
        ADDW2   {uword}temp2,%r1
        jgeu    add3
        INCW    %r0
add3:
        ADDW2   temp1,%r0
        ORH2    stky,{uword}%r1
        MOVH    &8,shiftcnt
        jsb    shift_left
        jmp     mul_fin
############################################################################
#
# This procedure multiplies two 28 bit numbers
#
mpy:
	CLRW	%r0
        MULW3   %r2,%r4,%r1        # AUBU
mpy_AUBL:
        MOVH    &14,shiftcnt
        jsb    shift_left         # r0,r1 = AUBU * 2**14
        MULW3   %r2,%r5,%r6        # AUBL
        MULW3   %r3,%r4,%r8        # ALBU
        ADDW2   %r8,%r6
        ADDW2   {uword}%r6,%r1     # r0,r1 = AUBU * 2**14 + (AUBL + ALBU)
        jgeu    mpy_ALBL
        INCW    %r0                # add in carry
mpy_ALBL:
        MOVH    &14,shiftcnt
        jsb    shift_left         # r0,r1 = AUBU * 2**28 + (AUBL + ALBU)
#                                             * 2**14
        MULW3   %r3,%r5,%r6        # ALBL
        ADDW2   {uword}%r6,%r1
        jgeu    mpy_ret
        INCW    %r0                # r0,r1 = AUBU * 2**28 + (AUBL + ALBU)
#                                             * 2**14 + ALBL * 2**0
mpy_ret:
        RSB
#
#       0*INF or INF*0
#
mpy_C:
        ORH2    &0x2,pswflgs                # V in PSW set
        jsb    nangen
        RSB
#
#       INF*NUM
#
mpy_D:
        MOVH    &inftag,fdt(%fp)
        jsb    pack
        RSB
#
#       NUM*INF
#
mpy_E:
        MOVH    &inftag,fdt(%fp)
        jsb    pack
        RSB
#
#       INF*INF
#
mpy_F:
        MOVH    &inftag,fdt(%fp)
        jsb    pack
        RSB
#
#       NAN*ANYTHING or ANYTHING*NAN
#
mpy_G:
        jsb    two_nans
        RSB
##########################################################################
#
# Set the result to the quotient op1/op2
#
#                     Case Table
#
#                |  0    NUM   INF   NAN
#           ______________________________
#             0  |  A     B     B     F
#            NUM |  C     D     B     F
#            INF |  E     E     A     F
#            NAN |  F     F     F     F
#
divide:
        jsb    unpack          
        MOVW    %r0,%r2
        MOVW    %r1,%r3         # op2 in r2,r3
        MOVW    %fp,%r7
        jsb    unpack          # op1 in r0,r1
        ADDW3   &12,%fp,%r7
        jsb    nanchk
        CMPH    &1,isnan
        je     div_F
        EXTFW   &0,&31,fdu(%fp),%r4     # sign of op2
        EXTFW   &0,&31,fdu(%r7),%r5     # sign of op1
        INSFW   &0,&31,&0,fdu(%fp)      # tentative positive sign
        CMPW    %r4,%r5
        je     dv_p1
        INSFW   &0,&31,&1,fdu(%fp)      # result is negative
dv_p1:
        CMPH    fdt(%fp),&zerotag
        jne    dinfchk
        CMPH    fdt(%r7),&zerotag
        je     div_A
        jmp     div_B
dinfchk:
        CMPH    fdt(%fp),&inftag
        jne    dnumchk
        CMPH    fdt(%r7),&inftag
        je     div_A
        jmp     div_E
dnumchk:
        CMPH    fdt(%r7),&zerotag
        je     div_C
        CMPH    fdt(%r7),&numtag
        je     div_D
        jmp     div_B
div_ret:
        jsb    trim_result
        jsb    pack
        RSB
#
#       0/0 or INF/INF
#
div_A:
        ORH2    &0x2,pswflgs             # V in PSW set
        jsb    nangen
        jmp     div_ret
#
#       0/NUM or 0/INF or NUM/INF
#
div_B:
        MOVH    &zerotag,fdt(%fp)
        jmp     div_ret
#
#       NUM/0
#
div_C:
        MOVH    &inftag,fdt(%fp)
        ORH2    &0x2,pswflgs             # V in PSW set
        jmp     div_ret
#
#       NUM/NUM
#
div_D:
        SUBH2   fdc(%r7),fdc(%fp)
        CLRH    divflg                # 1st time through
        CMPH    mode_precision,&single
        jne    d_divide
        jsb    s_divide
        EXTFW   &0,&3,%r4,%r5         # last bit of quotient
        jne    srem_chk
        ADDW2   %r2,%r0              # add divisor to remainder if last bit = 0
srem_chk:
        CMPW    %r0,&0
        je     mov_quo
        ORW2    &0x1,%r4               # sticky info included if remainder != 0
mov_quo:
        MOVW    %r4,%r0
        jmp     div_ret
d_divide:
        jsb    s_divide             # compute upper 28 bits of quotient
        MOVH    &1,divflg            # signal second time through
        MOVW    %r4,%r5
        EXTFW   &0,&3,%r4,%r6        # get last bit of quotient
        jne    uvg_p1
        ADDW2   {uword}%r3,%r1
        jgeu    uvg_p2
        INCW    %r0
uvg_p2:
        ADDW2   %r2,%r0              # add divisor to remainder if last bit = 0
uvg_p1:
	jsb	shift_R
        jsb    s_divide             # compute lower 28 bits of quotient
        EXTFW   &0,&3,%r4,%r6        # get last bit of quotient
        jne    drem_chk
        ADDW2   {uword}%r3,%r1
        jgeu    uadd3
        INCW    %r0
uadd3:
        ADDW2   %r2,%r0              # add divisor to remainder if last bit = 0
drem_chk:
        ORW2    %r0,%r1		     # all zips?
        je     fix_quo
        ORW2    &0x1,%r4            # sticky info added if remainder != 0
fix_quo:
        MOVW    %r5,%r0
        MOVW    %r4,%r1
        EXTFW   &2,&28,%r1,%r4
        LLSW3   &4,%r1,%r1
        ORW2    %r4,%r0
        jmp     div_ret
##########################################################################
#
# This procedure divides a double dividend by a double divisor to get a 
#  28-bit quotient.  Two calls to this procedure are required for a
#  double precision divide of 56 bits.
#
#  %r0,%r1 = dividend/remainder
#  %r2,%r3 = divisor
#  %r4 = quotient
#
s_divide:
        MOVW    &0x10,%r4            # bit 4 (usedtobe 3) of quotient = 1
        SUBW2   {uword}%r3,%r1
        jgeu    usub1
        DECW    %r0
usub1:
        SUBW2   %r2,%r0              # subtract divisor
	TSTW	%r0		     # the SUB doesn't give us good flags
        jge    squo1
#
#######   1st time through    #######
#
#       remainder < 0
sq0:
        CMPH    divflg,&1
        je     squo0
        DECH    fdc(%fp)
        jsb    shift_R
        ADDW2   {uword}%r3,%r1
        jgeu    uadd1
        INCW    %r0
uadd1:
        ADDW2   %r2,%r0                            # add in divisor
	TSTW	%r0		     # the ADD doesn't give us good flags
        jge    squo1
#
#######   inner loop   #######
#
squo0:
        ADDW2   {uword}%r4,%r4              # fix for ALSW3 glitch on overflow
        BVSB    sdiv_end
        jsb    shift_R
        ADDW2   {uword}%r3,%r1
        jgeu    uadd2
        INCW    %r0
uadd2:
        ADDW2   %r2,%r0                    # add in divisor
	TSTW	%r0		     # the ADD doesn't give us good flags
        jl     squo0
squo1:
        ORW2    &0x4,%r4               # insert 1 in quotient (lead zero here)
        ADDW2	{uword}%r4,%r4        # needed due to ALSW3 glitch on overflow
        BVSB    sdiv_end
        jsb    shift_R
        SUBW2   {uword}%r3,%r1
        jgeu    usub2
        DECW    %r0
usub2:
        SUBW2   %r2,%r0                            # subtract divisor
	TSTW	%r0		     # the SUB doesn't give us good flags
        jge    squo1
        jmp     squo0
sdiv_end:
        RSB
#
#       INF/NUM or INF/0
#
div_E:
        MOVH    &inftag,fdt(%fp)
        jmp     div_ret
#
#       NAN/ANYTHING or ANYTHING/NAN
#
div_F:
        jsb    two_nans
        RSB
#######################################################################
#
# Compare op1 and op2 and return the condition EQUAL, LESS THAN, GREATER
# THAN, or UNORDERED.
#
#                           Case Table
#
#                       |  0    NUM   INF   NAN
#                   -----------------------------
#                    0  |  A     B     E     H
#                   NUM |  C     D     E     H
#                   INF |  F     F     G     H
#                   NAN |  H     H     H     H
#
compare:
        jsb    unpack
        MOVW    %r0,%r2
        MOVW    %r1,%r3        # op2 in r2,r3
        MOVW    %fp,%r7
        jsb    unpack         # op1 in r0,r1
        ADDW3   &12,%fp,%r7
        jsb    nanchk
        CMPH    &1,isnan
        je     comp_H
        EXTFW   &0,&31,fdu(%fp),%r4
        CMPH    fdt(%fp),&zerotag
        jne    numchk
#
# Cases where op1 = 0
#
        CMPH    fdt(%r7),&zerotag
        je     comp_A
        CMPH    fdt(%r7),&numtag
        je     comp_B
        jmp     comp_E
numchk:
        CMPH    fdt(%fp),&numtag
        jne    infchk
#
# Cases where op1 is a finite, non-zero number
#
        CMPH    fdt(%r7),&zerotag
        je     comp_C
        CMPH    fdt(%r7),&numtag
        je     comp_D
        jmp     comp_E
infchk:
        CMPH    fdt(%r7),&inftag
        je     comp_G
        jmp     comp_F
#
#       0 vs 0
#
comp_A:
        ORH2    &0x4,pswflgs            # cond = EQUAL, Z in PSW is set
        jmp     raise_flag
#
#       0 vs finite
#
comp_B:
        EXTFW   &0,&31,fdu(%r7),%r5
        je     b_greater
        ORH2    &0x9,pswflgs            # cond = LESS, N & C in PSW are set
b_greater:
        jmp     raise_flag
#
#       Finite vs 0
#
comp_C:
        CMPW    &0,%r4
        je     c_less
        jmp     raise_flag
c_less:
        ORH2    &0x9,pswflgs            # cond = LESS, N & C in PSW are set
        jmp     raise_flag
#
#       Typical case of two finite, non-zero numbers
#
comp_D:
        EXTFW   &0,&31,fdu(%r7),%r5
        CMPW    %r4,%r5
        jne    ncomp
        CMPW    %r4,&0
        je     ncomp
        CMPW    {uword}fdu(%r7),fdu(%fp)
        jlu    c_less
        jgu    raise_flag
        jmp     wdcomp
ncomp:
        CMPW    fdu(%r7),fdu(%fp)
        jg     c_less
        jl     raise_flag
wdcomp:
        CMPW    {uword}fdl(%r7),fdl(%fp)
        jgu    comp_C
        jlu    comp_B
        ORH2    &0x4,pswflgs            # cond = EQUAL, Z in PSW set
        jmp     raise_flag
#
#       Finite vs infinite
#
comp_E:
        CMPH    mode_denorm,&affine
        je     comp_B
#       ORH2    &0x2,pswflgs            # cond = UNORDERED, V in PSW set
#       jmp     raise_flag
#
#       Infinity vs finite
#
comp_F:
        CMPH    mode_denorm,&affine
        je     comp_C
#       ORH2    &0x2,pswflgs            # cond = UNORDERED, V in PSW set
#       jmp     raise_flag
#
#       Infinity vs infinity
#
comp_G:
        CMPH    mode_denorm,&affine
        jne    g_unorder
        EXTFW   &0,&31,fdu(%r7),%r5
        CMPH    %r5,%r4
        jne    comp_C
        ORH2    &0x4,pswflgs            # cond = EQUAL, Z in PSW set
        jmp     raise_flag
g_unorder:
        ORH2    &0x2,pswflgs            # cond = UNORDERED, V in PSW set
        jmp     raise_flag
#
#       NaNs
#
comp_H:
        jsb    two_nans
        ORH2    &0x2,pswflgs            # cond = UNORDERED, V in PSW set
#
# Currently the only thing raise_flag does is return.  This is only a stub
# for a later version that sets err_invalid to TRUE if necessary.  Until
# the stub is added, all jmps to raise_flag can be replaced with RSBs.
#
raise_flag:
        RSB
########################################################################
#
# Set the result to the difference between op1 and op2
#
subtract:
        MOVH    &1,subflg
        EXTFW   &0,&31,fdu(%r7),%r6
        MCOMB   %r6,%r6
        INSFW   &0,&31,%r6,fdu(%r7)
        jsb    add
        RSB
########################################################################
#
# Set the result to the sum of op1 and op2
#
#                     Case Table
#
#                |  0    NUM   INF   NAN
#           ------------------------------
#             0  |  A     B     B     F
#            NUM |  C     D     B     F
#            INF |  C     C     E     F
#            NAN |  F     F     F     F
#
add:
        jsb    unpack          
        MOVW    %r0,%r2
        MOVW    %r1,%r3         # op2 in r2,r3
        MOVW    %fp,%r7
        jsb    unpack          # op1 in r0,r1
        ADDW3   &12,%fp,%r7
        jsb    nanchk
        CMPH    &1,isnan
        je     add_F
        CMPH    fdt(%fp),&zerotag
        jne    addnumchk
        CMPH    fdt(%r7),&zerotag
        je     add_A
        jmp     add_B
addnumchk:
        CMPH    fdt(%fp),&numtag
        jne    addinfchk
        CMPH    fdt(%r7),&zerotag
        je     add_C
        CMPH    fdt(%r7),&numtag
        je     add_D
        jmp     add_B
addinfchk:
        CMPH    fdt(%r7),&inftag
        je     add_E
        jmp     add_C
#
#       0 vs 0
#
add_A:                                      # result = op1
        ORH2    &0x4,pswflgs                # Z in PSW set
        EXTFW   &0,&31,fdu(%fp),%r4
        EXTFW   &0,&31,fdu(%r7),%r5
        CMPW    %r4,%r5
        je     add_ret
#       CMPH    mode_round,&toward_minus
#       jne    a_sign
#       INSFW   &0,&31,&1,fdu(%fp)
#       RSB
a_sign:
        INSFW   &0,&31,&0,fdu(%fp)
add_ret:
        RSB
#
#       Op1 is 0 or op2 is infinity
#
add_B:                                      # result = op2
        MOVW    fdt(%r7),fdt(%fp)            # fdt & fdc(%fp) moved
        MOVW    fdu(%r7),fdu(%fp)
        MOVW    fdl(%r7),fdl(%fp)
	jmp	add_C
#
#       Op2 is 0 or op1 is infinity
#
add_C:                                      # result = op1
	EXTFW	&0,&31,fdu(%fp),%r4	    # get the sign bit
	je	add_ret
	ORH2	&0x8,pswflgs
        RSB
#
#       Typical case of two finite, non-zero numbers
#
add_D:
        CLRB    %r8                               # 1st operand is op1
	MOVH	fdc(%fp),temp1;			     # save first op exp
	MOVH	fdc(%r7),fdc(%fp)		     # exp from 2nd op
        SUBH3   temp1,fdc(%r7),shiftcnt              # shiftcnt = %r7 - %fp
        jge    chk_shift
        jsb    swap
        MNEGH   shiftcnt,shiftcnt
        MOVH    temp1,fdc(%fp)                       # load exponent of result
chk_shift:
        CMPH    maxshft,shiftcnt
        jl     align
        ORW3    &1,%r3,%r1              # first (r0,r1) is effectively zero
				        # just set sticky and we're done
	MOVW	%r2,%r0
	MCOMB	%r8,%r8			# effectively a swap
        CMPH    mode_precision,&single
        jne    sign_compute
	ORW2	&1,%r0			# sticky set to 1 for single
        jmp     sign_compute
align:
        jsb    shift_right                          # align op1
        ORW2    rstky,%r1
        EXTFW   &0,&31,fdu(%fp),%r4
        EXTFW   &0,&31,fdu(%r7),%r5
        CMPH    %r4,%r5                              # compare operand signs
        jne    magn_subtr
add_magn:
        ADDW2   {uword}%r3,%r1                      # op1.l = op1.l + op2.l
        jgeu    add_upper
        INCW    %r0                                 # add in carry
add_upper:
        ADDW2   %r2,%r0                             # op1.u = op1.u + op2.u
	TSTW	%r0				    # see if high bit is one
        jge    sign_compute
        MOVH    &1,shiftcnt
        jsb    shift_right
        ORW2    rstky,%r1
        INCH    fdc(%fp)
sign_compute:
        CMPB    &0,%r8
        je     a_return
        MOVW    fdu(%r7),fdu(%fp)            # result sign
a_return:
        CMPH    mode_precision,&single
        jne    stky_ok
        CMPW    %r1,&0
        je     stky_ok
        ORW2    &0x1,%r0
stky_ok:
        jsb    trim_result
        jsb    pack
        RSB
magn_subtr:
        CMPW    %r0,%r2
        jl     sub_magn
        jg     op_swap
        CMPW    %r1,%r3
        jleu   sub_magn
op_swap:
        jsb    swap
sub_magn:
	CMPB	&0,%r8				# trickyness here.  this
	je	sub_op1big			# calculates the sign of the 
	MOVW	fdu(%r7),fdu(%fp)		# result.  r8 = 0 if r0,r1
						# corresponds to op1.  
						# and r8 != 0 if r2,r3
						# corresponds to op1. We know
						# that |(r0,r1)| > |(r1,r2)| so
						# the result has the same sign
						# as the op corressponding 
						# to (r0,r1).
						# Neat huh? Also we don't
						# care about any bits in
						# fdu(%fp) other than the
						# high bit which is result sign
sub_op1big:
        SUBW2   {uword}%r3,%r1                      # op1.l = op1.l - op2.l
        jgeu    sub_upper
        DECW    %r0                                 # subtract the borrow
sub_upper:
        SUBW2   %r2,%r0                             # op1.u = op1.u - op2.u
        ORW3    %r0,%r1,%r2                         # %r2 = 0 if result is 0
        jne    add_norm
        MOVH    &zerotag,fdt(%fp)
#       CMPH    mode_round,&toward_minus
#       je     a2_minus
        INSFW   &0,&31,&0,fdu(%fp)
#       jmp     add_norm
a2_minus:
#       INSFW   &0,&31,&1,fdu(%fp)
add_norm:
        MOVW    %fp,%r7
        jsb    norm
        jmp     a_return
#
#       Op1 and op2 are infinity
#
add_E:                                      # result = op1
        EXTFW   &0,&31,fdu(%fp),%r0
        EXTFW   &0,&31,fdu(%r7),%r1
        CMPW    %r0,%r1
        jne    e_error
#       CMPH    mode_inf,&affine
#       jne    e_error
        RSB
e_error:
        ORH2    &0x2,pswflgs                # V in PSW set
        jsb    nangen
        RSB
#
#       NaNs
#
add_F:
        jsb    two_nans
        RSB
#########################################################################
#
# The operands are swapped for accuracy in addition and subtraction.  The
# borrow problem, for example, is avoided.  The opposite operand is now op1.
#
swap:
        MOVW    %r0,%r6
        MOVW    %r2,%r0
        MOVW    %r6,%r2
        MOVW    %r1,%r6
        MOVW    %r3,%r1
        MOVW    %r6,%r3
        MCOMB   %r8,%r8
        RSB
###########################################################################
#
# This routine determines which operand to use as the nan result and 
#  fixes up the sign in the case of a subtraction.
#
two_nans:
        CMPH    fdt(%fp),&n_nantag
        je     nan_ret                # op1 is the nan
	MOVW	fdu(%r7),fdu(%fp)	# op2 is nan, get top half
	MOVW	fdl(%r7),fdl(%fp)	# get bottom half
        CMPH    subflg,&0
        je     nan_ret			# if not subtract all done
        EXTFW   &0,&31,fdu(%fp),%r4	# get the sign bit
        MCOMB   %r4,%r4                 # op2 = nan, sign fixed for subtraction
        INSFW   &0,&31,%r4,fdu(%fp)
nan_ret:
        MOVH    &n_nantag,fdt(%fp)
        RSB
############################################################################
#
# This procedure generates a NAN by inserting the program counter, pc,
#  in the fractional portion of the number and setting the exponent to
#  all ones.
#
nangen:
        CLRW    fdu(%fp)
        CLRW    fdl(%fp)
        MOVH    &n_nantag,fdt(%fp)
        INSFH   expw,exp,bias2,{uword}fdu(%fp)
	CMPH	&double,mode_precision
	BEB	dbl_nan
        INSFW   {uhalf}mant,&0,{uword}pc,fdu(%fp)
        RSB
dbl_nan:
	INSFW	&3,&28,{uword}pc,fdl(%fp)
	EXTFW	&18,&4,{uword}pc,%r8
	INSFW	&18,&0,%r8,fdu(%fp)
########################################################################
#
# This trim sequence applies to all operations that produce floating
# point results.
#
trim_result:
        jsb    under_result
        jsb    round_result
#       jsb    invalid_result
        jsb    over_result
        RSB
#########################################################################
#
# Check for an underflow first so that any denormalisation will occur
# before rounding.
#
under_result:
        CMPH    fdt(%fp),&numtag
        je     number
        RSB                                   # bypass special results
number:
        CMPH    uthresh,fdc(%fp)
        jl     underflow
        RSB
underflow:
        MOVH    fdc(%fp),exponent
        SUBH3   exponent,uthresh,shiftcnt
        CMPH    maxshft,shiftcnt
        jg     flag_uflow
        jsb    shift_right
        MOVH    exponent,fdc(%fp)
        ORW2    rstky,%r1
        CMPH    mode_precision,&single
        jne    uflow_ret
        ORW2    rstky,%r0
uflow_ret:
        RSB
flag_uflow:
        ORH2    &0x2,pswflgs                # set V in PSW
        MOVH    &zerotag,fdt(%fp)
        RSB
#########################################################################
#
# Round the result.
#
round_result:
        CMPH    fdt(%fp),&numtag
        je     round
        RSB                                          # bypass special results
round:
        CMPH    mode_precision,&single
        jne    d_round
        EXTFW   &0,&7,%r0,%r6                  # %r6 = least significant digit
        ANDW3   &0x3f,%r0,%r3                    # %r3 = bits beyond guard
        je     sguard_sticky                    # round/sticky = 0
        ORW2    &0x20,%r0                        # round/sticky = 1 (b5)
sguard_sticky:
        EXTFW   &1,&5,%r0,%r3                    # %r3 = guard & round/sticky
        jmp     rndchk
d_round:
        EXTFW   &0,&10,%r1,%r6                  # %r6 = least significant digit
        ANDW3   &0x1ff,%r1,%r3                   # %r3 = bits beyond guard
        je     dguard_sticky                   # round/sticky = 0
        ORW2    &0x100,%r1                      # round/sticky = 1
dguard_sticky:
        EXTFW   &1,&8,%r1,%r3                   # guard & round/sticky
rndchk:
        CMPH    %r3,&0
        jne    round_err
        RSB
round_err:
        ORH2    &0x1,pswflgs            # inexact result, C in PSW set
#       CMPH    mode_round,&toward_minus
#       je     to_minus
#       CMPH    mode_round,&toward_plus
#       je     to_plus
#       CMPH    mode_round,&to_nearest
#       jne    chop_result                         # round toward 0
        CMPH    %r3,&3
        je     inc_result
        CMPH    %r3,&1
        je     chop_result
        CMPH    %r6,&1               # check least significant digit
        je     inc_result
        jmp     chop_result
to_minus:
#       EXTFW   &0,&31,fdu(%fp),%r2
#       CMPH    %r2,&1
#       je     inc_result
#       jmp     chop_result
to_plus:
#       EXTFW   &0,&31,fdu(%fp),%r2
#       CMPH    %r2,&0
#       je     inc_result
#       jmp     chop_result
inc_result:
        MOVW    &0x80,%r6
        CMPH    mode_precision,&single
        jne    inc_dbl
        ADDW2   %r6,%r0
        BVSB    carry_found
        RSB
inc_dbl:
        MOVW    &0x400,%r6
        ADDW2   {uword}%r6,%r1
        jgeu    clean_up
        INCW    %r0
        BVCB    clean_up
carry_found:
        MOVH    &1,shiftcnt
        INCH    fdc(%fp)
        jsb    shift_right
clean_up:
        RSB
chop_result:
        ANDW2  &0xfffffc00,%r1
        CMPH   mode_precision,&single
        jne   clean_up
        ANDW2  &0xffffff80,%r0
        RSB
########################################################################
#
# Check for overflow.
#
over_result:
        CMPH    fdt(%fp),&numtag
        jne    ovf_return
        CMPH    othresh,fdc(%fp)
        jle    ovf_return
#       CMPH    trap_oflow,&0
#       jne    ovf_return
#       MOVH    &1,err_inexact
#       CMPH    mode_round,&to_nearest
#       je     ovfl
#       CMPH    mode_round,&toward_0
#       jne    ovf_return
ovfl:
        ORH2    &0x2,pswflgs            # V in PSW set
        MOVH    &inftag,fdt(%fp)        # exponent = all ones
ovf_return:
        RSB
##########################################################################
#
# This procedure is not needed for this release since only normalised
# numbers are permitted.
#
invalid_result:
#       CMPH    fdt(%fp),&numtag
#       jne    valid_result
#       EXTFW   &0,&30,%r0,%r2                  # r2 = msd
#       CMPH    %r2,&0
#       jne    valid_result
#       CMPH    unnorm_ok,&1
#       je     valid_result
#       CMPH    denorm_ok,&1
#       jne    set_invalid
#       CMPH    uthresh,fdc(%fp)
#       je     valid_result
set_invalid:
#       MOVH    &1,err_invalid
valid_result:
#       RSB
#########################################################################
#
# This procedure shifts double precision remainders left by 1.
#
shift_R:
        LLSW3   &1,%r0,%r0
        EXTFW   &0,&31,%r1,%r6
        ORW2    %r6,%r0
        LLSW3   &1,%r1,%r1                # shift remainder
        RSB
#########################################################################
#
# This procedure performs a left shift of single or double precision
# numbers.  shiftcnt specifies the number of places to be shifted.
# The number to be shifted is contained in %r0 and %r1.
#
shift_left:
        CMPH    shiftcnt,&0
        je     lshift_end
        EXTFW   &0,&31,%r1,%r6                    # r6 = carry out
        LLSW3   &1,%r1,%r1
        LLSW3   &1,%r0,%r0
        ORW2    %r6,%r0
        DECH    shiftcnt
        DECH    exponent                       # decrease exponent by 1
        jmp     shift_left
lshift_end:
        RSB
##########################################################################
#
# This procedure performs a right shift shiftcnt number of places on
# single or double precision numbers.  %r0 and %r1 contain the number
# to be shifted.
#
shift_right:
        CLRW    rstky
r_loop:
        CMPH    shiftcnt,&0
        je     rshift_end
        ANDW3   &0x1,%r1,%r6
        ORW2    %r6,rstky
        LRSW3   &1,%r1,%r1
        EXTFW   &0,&0,%r0,%r6                 # carry out
        LRSW3   &1,%r0,%r0
        INSFW   &0,&31,%r6,%r1
        DECH    shiftcnt
        INCH    exponent                      # increment exponent by 1
        jmp     r_loop
rshift_end:
        RSB
###########################################################################
#
# Floating point numbers are packed into the proper format here.
#
pack:
        EXTFW   &0,&31,fdu(%fp),%r4       # sign of the result
        je     tagchk
        ORH2    &0x8,pswflgs              # N in PSW set (negative result)
tagchk:
        CMPH    fdt(%fp),&zerotag
        je     p_zero
        CMPH    fdt(%fp),&inftag
        je     p_inf
        CMPH    fdt(%fp),&numtag
        je     p_num
p_nan:
        RSB
p_inf:
        INSFW   &30,&0,&0,fdu(%fp)
        INSFH   expw,exp,bias2,{uword}fdu(%fp)
        CLRW    fdl(%fp)
        RSB
p_zero:
        ORH2    &0x4,pswflgs
        CLRW    fdl(%fp)
        INSFW   &30,&0,&0,fdu(%fp)
        RSB
p_num:
        ADDH2   bias,fdc(%fp)
        EXTFW   &0,&30,%r0,%r2              # get msd
        jne    p_norm
#
# denormalised numbers have a bias of 1022 (double),%r2 or 126 (single)
#
p_denorm:                                   # msd = 0
        DECH    fdc(%fp)
p_norm:
        INSFH   expw,exp,fdc(%fp),{uword}fdu(%fp)
        EXTFW   &29,&0,%r0,%r0            # fraction only in r0
        MOVH    expw,shiftcnt
        jsb    shift_right
        INSFH   mant,&0,{uword}%r0,fdu(%fp)
        CMPH    mode_precision,&double
        jne    pack_ret
        MOVW    %r1,fdl(%fp)
pack_ret:
        RSB
fpabort:
        RSB
