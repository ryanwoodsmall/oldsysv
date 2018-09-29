#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"doprnt.s"
.ident	"@(#)libc-m32:print/doprnt.s	1.9"

#-----------------------------------------------------------------------#
#									#
# doprnt:  Common code for printf, fprintf, sprintf for IS25 machines	#
#									#
#									#
# 									#
# Overall program flow:							#
#									#
#	After brief initialization, the functions "printf", "fprintf",	#
# and "sprintf" converge in this module at the entry point "__doprnt".	#
# The program falls through to ".scan", where it searches in the printf	#
# format string for a percent sign.  Characters in the format string	#
# other than '%' are output immediately.  If the format string ends	#
# with no '%', control goes to ".return", and the program exits.	#
#									#
#	If '%' is found, the code at ".percent" decodes the characters	#
# following the '%'.  When a conversion character (d, o, x, X, f, etc.)	#
# is encountered, formatting is carried out in internal doprnt buffers	#
# according to that conversion character; but in most cases no output	#
# is generated until the program branches to ".putall", the final	#
# section of doprnt where all of the conversion characters converge to	#
# calculate the padding needed and to generate output from the internal	#
# buffers.  When this is done, the program loops back to ".scan" and	#
# continues to scan the format string.					#
#									#
#	"Generating output" usually just means putting output into	#
# the output buffer and incrementing the output buffer pointer and	#
# decrementing the output buffer count, just as putc would do.  But	#
# doprnt keeps its own copy of iop->_ptr in a register called "ptr",	#
# and generally it is only this pointer which is affected as output is	#
# generated; then iop->_ptr and iop->_cnt are adjusted just before	#
# doprnt returns.  Occasionally there will not be enough room in the	#
# output buffer for doprnt's output.  When this happens, doprnt calls	#
# an internal subroutine ".dowrite", which adjusts iop->_ptr and _cnt	#
# for any output already generated, and then calls fwrite to put out	#
# the new output.							#
#									#
#	If doprnt is entered through "sprintf", then iop->_ptr and	#
# iop->_cnt do not exist.  But the register "ptr" keeps track of the	#
# position of the pseudo-output in the caller's destination buffer,	#
# and consequently pseudo-output is handled identically to real output	#
# until just before the program exits.  At this point "sprintf"		#
# diverges:  rather than updating iop->_ptr and iop->_cnt, it simply	#
# null-terminates the pseudo-output string.				#
#									#
#	The register variable "buff_end" keeps a pointer to the first	#
# position beyond the end of the output buffer.  For "sprintf" calls,	#
# "buff_end" is set to -1, i.e., 0xffffffff.  This ensures that the	#
# pseudo-output buffer will never fill up, and the fact that the value	#
# is -1 also serves as a flag to doprnt indicating that it was		#
# called through "sprintf".						#
#-----------------------------------------------------------------------#




_m4_include_(print.defs)#		Shared definitions for printf family.

	.set	SIGN_MASK,0x7fffffff	# to mask off sign form double

	.text
	.align	4
.swittab:
	.word	._,._,._,._,._,._,._,._			# control characters
	.word	._,._,._,._,._,._,._,._			# control characters
	.word	._,._,._,._,._,._,._,._			# control characters
	.word	._,._,._,._,._,._,._,._			# control characters
	.word	.blank,._,._,.sharp			# space  ! " #
	.word	._,.dpercnt,._,._			# $ % & '
	.word	._,._,.star,.plus			# ( ) * +
	.word	._,.minus,.dot,._			# , - . /
	.word	.zero,.digit,.digit,.digit		# 0 1 2 3
	.word	.digit,.digit,.digit,.digit		# 4 5 6 7
	.word	.digit,.digit,._,._,._,._,._,._		# 8 9 : ; < = > ?
	.word	._,._,._,._,._,.E,._,.G			# @ A B C D E F G
	.word	._,._,._,._,._,._,._,._			# H I J K L M N O
	.word	._,._,._,._,._,._,._,._			# P Q R S T U V W
	.word	.X,._,._,._,._,._,._,._			# X Y Z [ \ ] ^ _
	.word	._,._,._,.c,.d,.e,.f,.g			# ` a b c d e f g
	.word	.h,.i,._,._,.l,._,._,.o			# h i j k l m n o
	.word	._,._,._,.s,._,.u,._,._			# p q r s t u v w
	.word	.x,._,._,._,._,._,._,._			# x y z { | } ~ 
#-----------------------------------------------------------------------#
#									#
# Data.									#
#									#
# All data is constant.							#
#									#
#-----------------------------------------------------------------------#

#
# Floating point zero.
#
	.align	4
.flzero:
	.word	0x0,0x0


# The number of characters in each of the padding character arrays
# ".blanks" and ".zeroes" is given by the constant PADMAX.

	.align	4
.blanks:
	.byte	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.byte	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.byte	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.byte	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
	.byte	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
.zeroes:
	.byte	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
	.byte	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
	.byte	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
	.byte	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
	.byte	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30

#
# The following tables are used for decimal conversions.   If a number
# lies in the interval 0 to 99, it can be used to index these tables.
# The indexed entry in the "ones" table gives the ascii one's digit
# of the number, and the indexed entry in the "tens" table gives the
# ascii ten's digit of the number.
#
	.align	4
.ones:
	.byte	48,49,50,51,52,53,54,55,56,57,48,49,50,51,52,53,54,55,56,57
	.byte	48,49,50,51,52,53,54,55,56,57,48,49,50,51,52,53,54,55,56,57
	.byte	48,49,50,51,52,53,54,55,56,57,48,49,50,51,52,53,54,55,56,57
	.byte	48,49,50,51,52,53,54,55,56,57,48,49,50,51,52,53,54,55,56,57
	.byte	48,49,50,51,52,53,54,55,56,57,48,49,50,51,52,53,54,55,56,57
.tens:
	.byte	48,48,48,48,48,48,48,48,48,48,49,49,49,49,49,49,49,49,49,49
	.byte	50,50,50,50,50,50,50,50,50,50,51,51,51,51,51,51,51,51,51,51
	.byte	52,52,52,52,52,52,52,52,52,52,53,53,53,53,53,53,53,53,53,53
	.byte	54,54,54,54,54,54,54,54,54,54,55,55,55,55,55,55,55,55,55,55
	.byte	56,56,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,57,57
#
# Plus sign, minus sign
#
	.align	4
.min_sgn:
	.byte	0x2d,0x0,0x0,0x0
.plus_sg:
	.byte	0x2b,0x0,0x0,0x0

#
# The following character tables are translation tables for hex digits and
# hex prefixes.  Specifically:
#
# .uc_digs = "0123456789ABCDEF"
# .uc_pfx = "0X"
# .lc_digs = "0123456789abcdef"
# .lc_pfx = "0x"
#
# Note: the program assumes that lc_digs stands in the same relation to lc_pfx
#	as does uc_digs to uc_pfx.
#
	.align	4
.uc_digs:
	.byte	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37
	.byte	0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46
.uc_pfx:
	.byte	0x30,0x58,0,0
	.align	4
.lc_digs:
	.byte	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37
	.byte	0x38,0x39,0x61,0x62,0x63,0x64,0x65,0x66
.lc_pfx:
	.byte	0x30,0x78,0,0

#
# HEXPFX is the offset of the prefix "0x" or "0X" from the start
# of the character conversion table.
#
	.set	HEXPFX,.uc_pfx - .uc_digs

	.align	4
#
# .uc_nan is the string "NaN"
# .lc_nan is the string "nan
# .uc_inf is the string "inf"
# .lc_inf is the string "INF"
#
.lc_nan:
	.byte  	0x6e,0x61,0x6e,0x30,0x78,0x0,0x0
	.align	4
.uc_nan:
	.byte 	0x4e,0x41,0x4e,0x30,0x58,0x0,0x0
	.align 	4
.lc_inf:
	.byte 	0x69,0x6e,0x66,0x0
	.align	4
.uc_inf:
	.byte	0x49,0x4e,0x46,0x0

#-----------------------------------------------------------------------#
# .dowrite								#
#									#
# The subroutine .dowrite carries out buffer pointer bookkeeping 	#
# surrounding a call to fwrite.  It is called only when the end of the	#
# file output buffer is approached, or in other unusual situations.	#
#									#
# Ordinarily what happens before .dowrite is called is that the main	#
# program copies register "ptr" into r1, advances "ptr", and then	#
# discovers that "ptr" is advanced too far.  Therefore, .dowrite	#
# expects the correct value of "ptr" to be in r1 on entry.		#
#									#
# The calling parameters for this subroutine are:			#
#									#
#	r0 = p = ptr to data to be written				#
#	r1 = correct value for register "ptr"				#
#	r2 = n = length of data to be written				#
#									#
# Exit with "ptr" correctly updated.					#
#									#
# 	if (this is not a call to sprintf) {				#
#		wcount += n + (bufptr - iop->_ptr);			#
# 		iop->_cnt -= (bufptr - iop->_ptr);			#
# 		iop->_ptr = bufptr;					#
# 		_bufsync(iop);						#
# 		(void) fwrite(p, 1, n, iop);				#
# 		bufptr = iop->_ptr;					#
# 	} else								#
# 		bufptr = (unsigned char *) 				#
#				memcpy((char *) bufptr, p, n) + n;	#
#									#
#									#
# The entry point .dopad40 is used when very long strings of blank	#
# or zero padding are being created.  It assumes r0 points to an array	#
# of pad characters and r2 gives a pad count.  r2 should be greater	#
# than PADMAX.  PADMAX pad characters are written, and on exit r3	#
# contains the entry value of r2 less PADMAX, i.e. the remaining number	#
# of pad characters needed.						#
#-----------------------------------------------------------------------#

_resynch.label:
.dopad40:
	movw	%r2,%r3
	movw	&PADMAX,%r2
	subw2	%r2,%r3
	movw	ptr,%r1

.dowrite:
	cmpw	buff_end,&-1		# if (!sprintf)) {
	je	.dowrits
	movw	iop,hold+SA1(%sp)
	movw	iophold+SA1(%sp),iop

	pushw	%r0			# pushes for call to fwrite
	pushw	&1
	pushw	%r2
	pushw	iop

	subw3	_ptr(iop),%r1,%r0	#	r0 = (bufptr - iop->_ptr)
	subw2	%r0,_cnt(iop)		# 	iop->_cnt -= r0
	addw2	%r2,%r0			#	r0 = n + bufptr - iop->_ptr
	addw2	%r0,wcount+SA5(%sp)	#	wcount += r0
	movw	%r1,_ptr(iop)		#	iop->_ptr = bufptr

	addw2	_cnt(iop),%r1		# Call bufsync only if there is a
	cmpw	%r1,buff_end		# synchronization problem
	jle	.dowrit1
	pushw	iop			# push for call to bufsync
	call	&1,_bufsync		# 	_bufsync(iop)
.dowrit1:
	call	&4,fwrite		# 	fwrite(p, 1, n, iop)
	movw	_ptr(iop),ptr		#	bufptr = iop->_ptr
	movzbw	_file(iop),%r0		# Get file number
	llsw2	&2,%r0			# and make it into word index.
#
# The following macro call is used for shared libraries.
# If SHLIB is defined, substitute _bufendtab for a pointer to _bufendtab 
# (_libc__bufendtab )
#
_m4_ifdef_(`SHLIB',
`	addw2	_libc__bufendtab,%r0
	movw	0(%r0),buff_end
',
`	movw	_bufendtab(%r0),buff_end# Reset buffer-end ptr in case changed.
')
	movw	hold+SA1(%sp),iop	# restore r5
	rsb				# }

.dowrits:				# else
	movblb				#	memcpy(bufptr, p, n)
	movw	%r1,ptr			#	bufptr +=n
	rsb

#-----------------------------------------------------------------------#
#									#
# "doprnt" entry point (from printf, fprintf, sprintf).			#
#									#
#-----------------------------------------------------------------------#

	.globl	__doprnt
	.align	4
__doprnt:
	MCOUNT
	movw	&0,NaN(%sp)
	movw	&0,aflag(%sp)

#-----------------------------------------------------------------------#
#									#
# Main Loop.  Scan through the format string looking for a % or for	#
#	the end of string.  Print directly all characters up to the %	#
#	or terminating null.						#
#									#
#-----------------------------------------------------------------------#

.scan:
	movzbw	0(fmt),%r3	# Next character in format string.
	jz	.return		# Return if '\0'.
	cmpw	%r3,&PERCENT	# If '%', go to ".percent".
	je	.percent

	movw	fmt,%r0		# Set up to scan for '%' or '\0'.
	movw	&PERCENT,%r1
.scan0:
	addw2	&1,fmt
	movzbw	0(fmt),%r3	# Pick up character.
	jz	.scan0a		# If null, terminate scan.
	cmpw	%r3,%r1
	jne	.scan0		# If not '%', continue scan.
.scan0a:
	subw3	%r0,fmt,%r2	# r2 = no. of chars passed over.
	movw	ptr,%r1		# Set up to put these chars in output buffer.
	addw2	%r2,ptr
	cmpw	ptr,buff_end	# Room in buffer for r2 characters?
	jleu	.scan1
	jsb	.dowrite	# No--get help from subroutine.
	jmp	.scan2
	.align	4
.scan1:
	movblb			# Put scanned characters into output buffer.
.scan2:
	movzbw	0(fmt),%r0	# Did we hit end of string?
	jnz	.percent	# No--must have been '%'.

#
# The null at the end of the format string has been reached.  Update
# iop->_cnt and iop->_ptr and flush output buffer as necessary.  If called
# through sprintf, put terminating null at end of output.  Return count
# of characters printed.

.return:
	movw	apsave(%sp),%ap		# Restore %ap.
	subw3	&.locals,%sp,%fp	# Restore %fp.

	movw	ptr,%r0			# Ptr to next avail pos in buf.
	cmpw	buff_end,&-1		# Were we called from sprintf?
	je	.retspr			# Go return from sprintf.

	movw	iophold(%sp),iop
	subw2	_ptr(iop),%r0		# r0 = amt added to buffer since fwrite
	subw2	%r0,_cnt(iop)		# Reduce cnt of space left in buffer.
	movw	ptr,_ptr(iop)		# Update iop->_ptr.
	addw3	_cnt(iop),ptr,%r2	# Check ptr/cnt synchronization in case
	cmpw	%r2,buff_end		# an interrupt occurred during last
	jg	.retsync		# several instructions.
	bitb	&(_IONBF+_IOLBF+_IOERR),_flag(iop)
	jnz	.ret1			# Jump if error or special buffering.
	addw2	wcount(%sp),%r0		# Add any amount written by fwrite.
	ret	&.Rcnt			# Return total number of chars written.

.retsync:
	movw	%r0,%r3			# Save output count in safe reg.
	pushw	iop
	call	&1,_bufsync		# Resynchronize iop->_ptr and iop->_cnt.
	jmp	.ret1a
.ret1:
	movw	%r0,%r3			# Save output count in safe reg.
.ret1a:
	bitb	&(_IONBF+_IOLBF),_flag(iop) # Unbuffered or line-buffered?
	jz	.ret2
	bitb	&_IONBF,_flag(iop)
	jnz	.retflsh		# If unbuffered, flush unconditionally.
	movw	%r3,%r1			# Line-buffered: look for '\n'.
	jz	.ret2			# Skip if no characters to look at.
	subw3	%r3,ptr,%r0		# Get start of area to look in.
	movw	&NEWLINE,%r2		# r2 = '\n'
.ret1b:
	cmpb	0(%r0),%r2		# '\n' in buffer?
	je	.retflsh		# If so, flush.
	addw2	&1,%r0
	subw2	&1,%r1
	jpos	.ret1b			# Continue to look for '\n'.
	jmp	.ret2

.retflsh:
	pushw	iop
	call	&1,_xflsbuf		# Flush.
.ret2:
	bitb	&_IOERR,_flag(iop)	# I/O error?
	jnz	.reterr
	addw3	wcount(%sp),%r3,%r0	# If not, return count of chars written.
	ret	&.Rcnt


.reterr:
	mnegw	&EOFN,%r0		# Error--return EOF.
	ret	&.Rcnt

#
# Return from sprintf.
#

.retspr:
	movb	&0,0(ptr)		# Null-terminate the output.
	subw2	outstart(%sp),%r0	# Return count of characters written.
	ret	&.Rcnt

#-----------------------------------------------------------------------#
#									#
# Percent sign encountered in format string.  Initialize flags, then	#
# interpret the characters which follow %.				#
# The interpretation of these characters is determined by a jump table.	#
# The format character upon which the jump is based is retained in %r3.	#
#									#
#-----------------------------------------------------------------------#

	.align	4
.percent:
	movw	&0,flags		# Initialize ...
	movw	&0,prefixle(%sp)
	addw2	&1,fmt

#
# Come here to interpret subsequent format characters.
#

.form_sw:
.h:
.l:
	movzbw	0(fmt),%r3		# Get format character.
	addw2	&1,fmt
	movw	%r3,%r1
	llsw2	&2,%r1			# compute index based on character
	bitw	&128,%r3		# Is high-order bit set?
	jz	*.swittab(%r1)		# If not, jump to matching case label.
					# If so, fall through to "._", below.

	#
	# The label ._ applies to all characters Q for which the effect of %Q
	# is undefined.  The code for ._ causes doprnt to resume format
	# scan with the character Q.


#
# Undefined format character.  Back up and rescan it.
# This will cause it to be printed along with any non-% characters
# which follow it in the format string.

._:
	subw2	&1,fmt		# Back up.
	jmp	.scan

#
# Flags in format specification--plus sign, minus sign, blank, and sharp.
# Set the appropriate bits for these flags.

	.align	4
.plus:
	orw2	&FPLUS,flags
	jmp	.form_sw

	.align	4
.minus:
	orw2	&FMINUS,flags
	jmp	.form_sw

	.align	4
.blank:
	orw2	&FBLANK,flags
	jmp	.form_sw

	.align	4
.sharp:
	orw2	&FSHARP,flags
	jmp	.form_sw

#
# Dot encountered--set precision to zero until number encountered.

	.align	4
.dot:
	orw2	&DOTSEEN,flags
	movw	&0,prec(%sp)
	jmp	.form_sw

#
# Asterisk.  We are dealing with %* or %width.* ; the two cases are
#	distinguished on the basis of the flag bit DOTSEEN.

	.align	4
.star:
	bitw	&DOTSEEN,flags
	jnz	.starprc

	orw2	&WIDTH,flags		# '*' represents width.
	addw2	&4,%ap
	movw	-4(%ap),width(%sp)
	jnneg	.form_sw
	mnegw	width(%sp),width(%sp)	# Negative width--make it positive
	xorw2	&FMINUS,flags		# and set flag for '-'.
	jmp	.form_sw

.starprc:
	addw2	&4,%ap			# '*' represents precision.
	movw	-4(%ap),prec(%sp)
	jnneg	.form_sw

	movw	&0,prec(%sp)		# Negative precision--make it zero.
	jmp	.form_sw

# Number.  In a format of the form %width.prec a number is either a width
#	or a precision, depending on whether the DOTSEEN flag is set.
#	For consistency with older versions of doprnt, if the leading digit
#	of the width is zero then a flag is set to cause left-zero-padding
#	of the result (left-justification, indicated by FMINUS, overrides this).

.zero:
	bitw	&(DOTSEEN+FMINUS),flags	# Ignore leading zero on precision or
	jnz	.digit			# if '-' flag is set.
	orw2	&PADZERO,flags		# Otherwise specify left-zero-padding.

.digit:
	subw2	&ASCII_0,%r3
	movzbw	0(fmt),%r1
#
# The following macro call is used for shared libraries.
# If SHLIB is defined, substitue _ctype for pointer to _ctype (_libc__ctype)
#
_m4_ifdef_(`SHLIB',
`	addw3	_libc__ctype,%r1,%r0
	bitb	&ISDIGIT,1(%r0)
',
`	bitb	&ISDIGIT,_ctype+1(%r1)
')
	jz	.dig2
.dig1:
	subw2	&ASCII_0,%r1		# Convert to decimal:
	addw2	%r3,%r1			# Add twice accumulated value to digit;
	addw2	%r3,%r1
	llsw2	&3,%r3			# (get value times 8)
	addw2	%r1,%r3			# add 8 times more to multiply by ten.

	addw2	&1,fmt
	movzbw	0(fmt),%r1
#
# The following macro call is used for shared libraries.
# If SHLIB is define, substitute _ctype for pointer to _cytpe (_libc__ctype)
#
_m4_ifdef_(`SHLIB',
`	addw3	_libc__ctype,%r1,%r0
	bitb	&ISDIGIT,1(%r0)
',
`	bitb    &ISDIGIT,_ctype+1(%r1)
')
	jnz	.dig1

.dig2:
	bitw	&DOTSEEN,flags		# if (flagword & DOTSEEN)
	jz	.dig3
	movw	%r3,prec(%sp)		#	prec = num
	jmp	.form_sw
.dig3:
	orw2	&WIDTH,flags
	movw	%r3,width(%sp)		# else width = num
	jmp	.form_sw

#-----------------------------------------------------------------------#
#									#
# Conversion characters.						#
#									#
# The code for the individual conversion characters sets up for		#
# printing but usually does not print anything nor put anything in	#
# the output buffer.  The code here establishes what will later be	#
# printed:  padding blanks, a prefix, left padding zeroes, a value,	#
# right padding zeroes, a suffix, and then padding blanks on the	#
# right if there were none before.					#
#									#
# The prefix is a sign, a blank, "0x", "0X", or empty.  Its length is 	#
# given by "prefixle".							#
#									#
# The suffix is an exponent or empty.  The flag SUFFIX indicated 	#
# whether there is one, and if there is, "suffixle" gives its length.	#
#									#
# The value to be printed starts at the position pointed to by "bp" and	#
# continues up to but not including the position pointed to by "p".	#
#									#
# The flag LZERO indicates whether there are left padding zeroes; 	#
# if there are, "lzero" tells the number of them.  Similarly for RZERO 	#
# and "rzero".								#
#									#
# The number of padding blanks, and whether they go on the left or the 	#
# right, will be computed at ".putall", where the output is finally 	#
# generated.								#
#-----------------------------------------------------------------------#




#-----------------------------------------------------------------------#
#									#
# %d and %u format							#
#									#
# Convert decimal number, generating sign and leading zeroes as needed.	#
#-----------------------------------------------------------------------#

	.align	4
.i:
.d:
	movw	%sp,p			# Get pointer to end of cvt buffer.
	movw	p,bp
	addw2	&4,%ap
	movw	-4(%ap),%r3		# Pick up value to convert.
	jge	.d_nneg

	movw	&.min_sgn,prefix(%sp)	# Negative number--put prefix "-".
	movw	&1,prefixle(%sp)
	mnegw	%r3,%r3			# Negate, then convert pos. number.
	cmpw	%r3,&0			# Was the number 0x80000000?
	jpos	.d_cvtnz
	jmp	.d_huge			# If so, need special conversion.

	.align	4
.d_nneg:
	bitw	&(FPLUS+FBLANK),flags	# Plus sign or leading blank desired?
	jz	.d_cvt
	bitw	&FPLUS,flags		# If so, was it plus sign?
	jz	.d_blnk
	movw	&.plus_sg,prefix(%sp)	# Yes--put prefix "+".
	movw	&1,prefixle(%sp)
	jmp	.d_cvt

	.align	4
.d_blnk:
	movw	&.blanks,prefix(%sp)	# Put prefix " ".
	movw	&1,prefixle(%sp)
	jmp	.d_cvt

#
# Come here to convert number whose high-order bit is set.
#

	.align	4
.d_huge:
	movw	%r3,%r2			# Save value.
	lrsw2	&1,%r3			# Divide by two, then by five--this
	divw2	&5,%r3			# is much quicker than udivw by 10.
	subw2	%r3,%r2			# Subtract quotient twice...
	subw2	%r3,%r2
	movw	%r3,%r0
	llsw2	&3,%r0			# Get eight times quotient
	subw2	%r0,%r2			# and subtract it.
	addw2	&ASCII_0,%r2		# Make digit into ascii digit.
	subw2	&1,bp
	movtwb	%r2,0(bp)
	jmp	.d_cvtnz		# Finish conversion of non-zero amt.
	
	.align	4
.d_cvt:
	cmpw	%r3,&0
	jne	.d_cvtnz
.d_cvtz:
	bitw	&DOTSEEN,flags		# Zero.  Min. no. of digits specified?
	jnz	.d_lzer
	subw2	&1,bp			# No--create single digit 0.
	movb	&ASCII_0,0(bp)
	jmp	.putall

.d_cvtnz:
	cmpw	%r3,&99			# Is number bigger than 99?
	jleu	.d_cvt2
.d_cvt3:
	movw	%r3,%r2			# Save copy of number.
	divw2	&100,%r3		# Divide by 100.
	llsw3	&2,%r3,%r1		# Get quotient times 4.
	subw2	%r1,%r2			# Subtract from original.
	llsw2	&3,%r1			# Quotient times 32.
	subw2	%r1,%r2			# Subtract three times, to give effect
	subw2	%r1,%r2			# of subtracting 96+4 = 100 times
	subw2	%r1,%r2			# quotient, thus giving remainder.
	subw2	&2,bp			# Make room for two digit remainder.
	movb	.tens(%r2),0(bp)	# Store tens digit.
	movb	.ones(%r2),1(bp)	# Store ones digit.
	cmpw	%r3,&99
	jgu	.d_cvt3			# Go back for more.

.d_cvt2:
	cmpw	%r3,&9			# Number less than 10?
	jgu	.d_cvt2a
	subw2	&1,bp			# Yes--generate one digit.
	movb	.ones(%r3),0(bp)
	bitw	&DOTSEEN,flags		# Min. no. of digits specified?
	jz	.putall			# If not, go output result.
	jmp	.d_lzer

.d_cvt2a:
	subw2	&2,bp			# Look up the two digit number in
	movb	.tens(%r3),0(bp)	# the digit table.
	movb	.ones(%r3),1(bp)

	bitw	&DOTSEEN,flags		# Min. no. of digits specified?
	jz	.putall			# If not, go output result.

.d_lzer:
	subw2	bp,p			# Get p = length of number.
	subw3	p,prec(%sp),%r3		# Do we have min. number of digits?
	jnpos	.putall1
	movw	%r3,lzero(%sp)		# No--fill out with leading zeroes.
	subw2	%r3,width(%sp)
	orw2	&LZERO,flags
	jmp	.putall1


	.align	4
.u:
	movw	%sp,p			# Get pointer to end of cvt buffer.
	movw	p,bp
	addw2	&4,%ap
	movw	-4(%ap),%r3		# Pick up value to convert.
	jpos	.d_cvtnz
	jneg	.d_huge
	jmp	.d_cvtz

#-----------------------------------------------------------------------#
#									#
# Non-decimal fixed point representations for radix equal to a		#
# power of two.								#
#									#
# Register r2 is one less than the radix for the conversion.		#
# Register r1 is the base 2 log of the radix for the conversion.	#
# Conversion is unsigned.						#
#									#
# Register r5, which on completion of the conversion is set to the      #
#	end of the conversion buffer (under the name 'p'), is used	#
#	during the conversion to hold a translate table pointer.	#
#-----------------------------------------------------------------------#

	.align	4
.o:
	movw	&.lc_digs,%r5
	movw	&7,%r2
	movw	&3,%r1
	jmp	.fix

	.align	4
.X:
	movw	&.uc_digs,%r5
	jmp	.x1

	.align	4
.x:
	movw	&.lc_digs,%r5
.x1:
	movw	&15,%r2
	movw	&4,%r1

.fix:
	movw	%r5,tab(%sp)		# Save ptr to digit conversion table.
	movw	%sp,bp			# Get pointer to end of cvt buffer.
	addw2	&4,%ap
	movw	-4(%ap),%r3		# Pick up value to convert.
	jnz	.fixcvt			# Skip if not zero.
# Entry point when printing a
# NaN with fraction part == 0
.xzero:
	movw	%sp,p			# Set pointer to end of buffer.
	bitw	&DOTSEEN,flags		# Was min number of digits specified?
	jnz	.fixlzr1
	movw	&1,lzero(%sp)		# If not, generate a single zero.
	subw2	&1,width(%sp)
	orw2	&LZERO,flags
	jmp	.putall

	.align	4
.fixcvt:
	movw	%r3,%r0			# Copy number.
	andw2	%r2,%r0			# Mask to low-order digit.
	addw2	%r5,%r0			# Compute offset into digit-table.
	subw2	&1,bp
	movb	0(%r0),0(bp)		# Move digit into buffer.
	lrsw2	%r1,%r3			# Shift number down.
	jnz	.fixcvt			# Continue conversion if more left.

	movw	%sp,p			# Set pointer to end of buffer.
	bitw	&DOTSEEN,flags
	jz	.fixalt

.fixlzr1:
	subw3	p,bp,%r3		# r3 = negation of length of number.
	addw2	prec(%sp),%r3		# Add min digits desired to compute
	jnpos	.fixalt			# number of leading zeroes needed.
	movw	%r3,lzero(%sp)
	subw2	%r3,width(%sp)
	orw2	&LZERO,flags

.fixalt:
	bitw	&FSHARP,flags		# '#' flag--prepend '0', '0x', or '0X'.
	jz	.putall
	cmpw	bp,p
	je	.putall			# Don't bother if number was empty.
	cmpw	%r1,&4			# Four-bit digits?
	je	.fixhex			# If so, handle hex prefix.

	bitw	&LZERO,flags		# Octal--just make sure there is a
	jnz	.putall			# leading zero.
	subw2	&1,bp
	movb	&ASCII_0,0(bp)		# Insert leading zero.
	jmp	.putall

.fixhex:
	addw3	&HEXPFX,tab(%sp),prefix(%sp)	# Set up hex prefix.
	movw	&2,prefixle(%sp)
	jmp	.putall

#-----------------------------------------------------------------------#
#									#
# %E and %e formats.							#
#									#
# "ecvt" is called to perform the conversion.  Its output is then	#
# reformatted.								#
#-----------------------------------------------------------------------#

.E:
.e:
	movw	%r3,letter_e(%sp)	# Save format character-- e or E.

	bitw	&DOTSEEN,flags		# Precision given?
	jnz	.e_1
	movw	&6,prec(%sp)		# Default is six decimal places.
.e_1:
	addw2	&8,%ap
	movw	-8(%ap),dval(%sp)	# Get value to convert.
	movw	-4(%ap),dval+4(%sp)

.excep:	
#
# Check for NaN and infinity
# %r1 contains the number
# If exponent == 2047 then
# floating point number is either a
# Nan or an infinity
#
	movw	dval(%sp),%r1
	extzv	&20,&11,%r1,%r2	
	cmpw	%r2,&2047		# Is the exponent == 2047?
	jne	.ret_where
	cmpw	dval+4(%sp),&0		# Check if low 32 bits == 0
	jne 	.nan
	llsw3	&12,%r1,%r2		# Check that the 20 low bits of leftmost
					# are 0
	jne	.nan			# If not, number is a NaN
#
# Floating pt. number is an infinity
#
	lrsw3	&31,%r1,%r2
	je	.inf
	movw    &.min_sgn,prefix(%sp)	# Negative infinity, set sign
	movw    &1,prefixle(%sp)
.inf:
	cmpw	&1,aflag(%sp)		# If formatting option is `f' use lowercase
	je	.inf_lc
	cmpw	&UPPER_E,letter_e(%sp)
	jne	.inf_lc
	movw	&.uc_inf,bp		# bp points to beginning of string ``INF''
	jmp	.set_ptrs
.inf_lc:
	movw	&.lc_inf,bp		# bp points to beginning of string ``inf''
.set_ptrs:
	movw	bp,p			
	addw2	&3,p			# p points to end of string pointed to by bp
	movw 	&0,aflag(%sp)
	jmp     .putall
.nan:
#
# Floating pt. number is a NaN
#
	lrsw3	&31,%r1,%r2
	je	.nan1			 
	movw	&.min_sgn,prefix(%sp)	# If negative Nan, take care of sign
	movw	&1,prefixle(%sp)
.nan1:
	cmpw	&1,aflag(%sp)		#  Uppercase or lowercase ?
	je	.nan_lc			
	cmpw	&UPPER_E,letter_e(%sp)
	jne	.nan_lc
	movw	&.uc_digs,%r5		
	jmp	.nan2
.nan_lc:
	movw	&.lc_digs,%r5
#
# NaN is used as a flag for a NaN and as the length of the string 
# (NAN0x or nan0x) that will be ouputted with the hex number
#
.nan2:
	movw	&5,NaN(%sp)		
	movw	&0,aflag(%sp)
	movw	%sp,bp
	llsw3	&12,%r1,%r2		# Extract lefmost 32 bits of fraction part
	lrsw3	&20,dval+4(%sp),%r3
	orw2	%r2,%r3
	jz	.xzero
	movw	&15,%r2			# Set registers for hex convertion
	movw	%r5,tab(%sp)
	movw	&4,%r1
	jmp	.fixcvt		

#
# Number was not a NaN nor an infinity
#
.ret_where:
	cmpw	&1,aflag(%sp)		# If coming from "%f", return there
	je	.f_enter
	cmpw	&2,aflag(%sp)		# If coming from "%g", return there
	je	.g_enter

.e_1b:
	pushw	dval(%sp)
	pushw	dval+4+SA1(%sp)
	addw3	&1,prec+SA2(%sp),%r0
	cmpw	%r0,&MAXECVT		# Get min(prec + 1, MAXECVT)
	jle	.e_1a
	movw	&MAXECVT,%r0
.e_1a:
	pushw	%r0			# This is the min of the two.
	pushaw	dec_pt+SA3(%sp)
	pushaw	sign+SA4(%sp)
	call	&5,ecvt			# bp = ecvt(dval, min(), &dec_pt, &sign)
	movw	%r0,bp

.e_merge:

#
# Determine the prefix.
#
	cmpw	sign(%sp),&0
	je	.e_nneg

	movw	&.min_sgn,prefix(%sp)	# Prefix is "-".
	movw	&1,prefixle(%sp)
	jmp	.e_first

.e_nneg:
	bitw	&FPLUS,flags
	jz	.e_nneg1

	movw	&.plus_sg,prefix(%sp)	# Prefix is "+".
	movw	&1,prefixle(%sp)
	jmp	.e_first

.e_nneg1:
	bitw	&FBLANK,flags
	jz	.e_first

	movw	&.blanks,prefix(%sp)	# Prefix is " ".
	movw	&1,prefixle(%sp)

# Place the first digit in the buffer

.e_first:
	movw	%sp,p			# Get address of buffer.
	subw2	&-buf,p

	movb	0(bp),0(p)		# Copy first digit.
	jz	.e_firs1		# If empty string, go generate a zero.
	addw2	&1,bp
	jmp	.e_firs2

.e_firs1:
	movb	&ASCII_0,0(p)		# Create zero as first digit.

.e_firs2:
	addw2	&1,p

# Put in a decimal point if needed

	movw	prec(%sp),%r3		# Decimal pt is needed if digits follow,
	jnz	.e_dcpt
	bitw	&FSHARP,flags		# or if # was specified.
	jz	.e_mantd

.e_dcpt:
	movb	&DECPOINT,0(p)
	addw2	&1,p

# Create the rest of the mantissa

	cmpw	%r3,&0			# Any digits?
	jle	.e_mantd		# Skip if not.

e_mant1:
	movb	0(bp),0(p)		# Copy a digit.
	jz	e_mant2
	addw2	&1,bp
	addw2	&1,p
	subw2	&1,%r3			# Decrease count of digits needed.
	jpos	e_mant1			# Loop if more needed.
	jmp	.e_mantd

e_mant2:
	movw	%r3,rzero(%sp)		# Set up to put trailing zeroes to
	subw2	%r3,width(%sp)		# fill out specified precision.
	orw2	&RZERO,flags

# Reset bp to point to doprnt's buffer instead of ecvt's buffer.

.e_mantd:
	subw3	&-buf,%sp,bp

# Create exponent.
# The exponent is generated from right to left;
# r1 points to the byte beyond the exponent buffer.
# r2 generally points to the most recently generated character of the exponent.
# r3 contains dec_pt.

	subw3	&-(expbuf+MAXESIZ),%sp,%r1	# Get position beyond exp.
	movw	%r1,%r2			# Make a copy of this position.
	movw	dec_pt(%sp),%r3		# Pick up dec_pt.
#	fcmpd	dval(%sp),.flzero	# Zero value?
	andw3	&SIGN_MASK,dval(%sp),%r0
	orw2	dval+4(%sp),%r0
	jne	.e_ex0
	movw	&1,%r3			# Force exponent zero.

.e_ex0:
	subw3	&1,%r3,%r0		# Compute exponent.
	jnneg	.e_ex1
	mnegw	%r0,%r0
.e_ex1:
	cmpw	%r0,&99			# See if more than two digits.
	jgu	.e_ex3

	subw2	&2,%r2
	movb	.tens(%r0),0(%r2)	# Fetch ten's digit.
	movb	.ones(%r0),1(%r2)	# Fetch one's digit.
	jmp	.e_xsgn

.e_ex3:
	subw2	&3,%r2			# Make room for 3 digit exponent.
	movb	&ASCII_0,0(%r2)		# Initialize first digit to 0.
.e_ex4:
	addb2	&1,0(%r2)		# Increment first digit.
	subw2	&100,%r0		# Reduce exponent.
	cmpw	%r0,&99
	jgu	.e_ex4			# Repeat if still >=100.

	movb	.tens(%r0),1(%r2)	# Fill in ten's digit.
	movb	.ones(%r0),2(%r2)	# Fill in one's digit.

# Put in the exponent sign

.e_xsgn:
	subw2	&2,%r2			# Make room for sign and letter 'e'.
	cmpw	%r3,&0
	jg	.e_xplus
#	fcmpd	dval(%sp),.flzero
	andw3	&SIGN_MASK,dval(%sp),%r0# mask off sign
	orw2	dval+4(%sp),%r0		# OR of all bits other than sign
	jne	.e_xmin
.e_xplus:
	movb	&PLUSSGN,1(%r2)
	jmp	.e_E
.e_xmin:
	movb	&MINUSSGN,1(%r2)


# Put in an upper or lower case 'e'

.e_E:
	movtwb	letter_e(%sp),0(%r2)

# Compute size of suffix

	movw	%r2,suffix(%sp)		# Save pointer to suffix.
	subw2	%r2,%r1			# Get length of suffix.
	movw	%r1,suffixle(%sp)
	subw2	%r1,width(%sp)
	orw2	&SUFFIX,flags
	jmp	.putall


#-----------------------------------------------------------------------#
#									#
# %f format.								#
#									#
# "fcvt" is called; its result is reformatted and then the number of	#
# trailing zeroes needed will be calculated.				#
#									#
#-----------------------------------------------------------------------#

.f:
	bitw	&DOTSEEN,flags		# Precision specified?
	jnz	.f_1
	movw	&6,prec(%sp)		# Default is six digits.
.f_1:
	addw2	&8,%ap
	movw	-8(%ap),dval(%sp)	# Get value to be converted.
	movw	-4(%ap),dval+4(%sp)	# Get value to be converted.
	movw	&1,aflag(%sp)		
	jmp	.excep			# Check for NaNs

.f_enter:
	movw	&0,aflag(%sp)
	pushw	dval(%sp)
	pushw	dval+4+SA1(%sp)
	cmpw	prec+SA2(%sp),&MAXFCVT	# Find min(prec, MAXFCVT)
	jge	.f_2
	pushw	prec+SA2(%sp)
	jmp	.f_3
.f_2:
	pushw	&MAXFCVT
.f_3:
	pushaw	dec_pt+SA3(%sp)
	pushaw	sign+SA4(%sp)
	call	&5,fcvt			# bp = fcvt(dval, min(), &dec_pt, &sign)
	movw	%r0,bp

# Determine the prefix

.f_merge:
	cmpw	sign(%sp),&0
	je	.f_sgn1
	mnegw	prec(%sp),%r0		# Will non-zero digits be lost?
	cmpw	dec_pt(%sp),%r0
	jle	.f_sgn1			# If so, treat number as zero.
	cmpb	0(bp),&ASCII_0
	je	.f_sgn1			# Zero.

	movw	&.min_sgn,prefix(%sp)	# Sign is "-"
	movw	&1,prefixle(%sp)
	jmp	.f_int
.f_sgn1:
	bitw	&FPLUS,flags
	jz	.f_sgn2

	movw	&.plus_sg,prefix(%sp)	# Sign is "+"
	movw	&1,prefixle(%sp)
	jmp	.f_int

.f_sgn2:
	bitw	&FBLANK,flags
	jz	.f_int

	movw	&.blanks,prefix(%sp)	# Sign is " "
	movw	&1,prefixle(%sp)

# Emit the digits before the decimal point.

.f_int:
	subw3	&-buf,%sp,p		# Get address buf(%sp) in p.
	movw	dec_pt(%sp),%r2		# Get no. of digs before decimal point.
	jpos	.f_int1			# Jump if digits before decimal.

	movtwb	&ASCII_0,0(p)		# Fraction--put one dig before dec pt.
	addw2	&1,p
	jmp	.f_decpt

.f_int1:
	movb	0(bp),0(p)		# Copy a digit.
	jz	.f_intp			# If no more digits, go pad with zeroes.
	addw2	&1,bp
	addw2	&1,p
	subw2	&1,%r2			# Decrement count.
	jpos	.f_int1			# Loop if more.
	jmp	.f_decpt

.f_intp:
	movw	&ASCII_0,%r0		# Pad character.
.f_intp1:
	movtwb	%r0,0(p)		# Store a zero.
	addw2	&1,p
	subw2	&1,%r2			# Decrement count.
	jpos	.f_intp1		# Loop if more.


# Decide whether we need a decimal point

.f_decpt:
	movw	prec(%sp),%r2		# Must have it if digits come after.
	jpos	.f_decp1
	bitw	&FSHARP,flags		# If '#' flag given, always put dec pt.
	jz	.f_frac5

.f_decp1:
	movb	&DECPOINT,0(p)
	addw2	&1,p


# Digits (if any) after the decimal point
# Keep in %r2 the number of digits still to be generated.

.f_frac:
	cmpw	%r2,&0			# Any digits at all?
	jle	.f_frac5		# Skip if not.
	cmpw	%r2,&MAXFCVT		# Can we generate this much?
	jle	.f_frac1

	movw	%r2,%r1			# No--save prec in r1.
	movw	&MAXFCVT,%r2		# Set precision to maximum, then
	orw2	&RZERO,flags		# set up to fill request with zeroes.
	subw2	%r2,%r1			# Compute trailing zeroes needed.
	movw	%r1,rzero(%sp)
	subw2	%r1,width(%sp)

# Generate the leading zeroes after decimal for a no. of the form 0.003582 .

.f_frac1:
	mnegw	dec_pt(%sp),%r1		# Get number of zeroes between dec pt
	jnpos	.f_frac3		# and first significant digit.
	subw2	%r1,%r2			# Subtract zeroes from total precision.
	jnneg	.f_frac2
	addw2	%r2,%r1			# Reduce number of zeroes if too many.
.f_frac2:
	movtwb	&ASCII_0,0(p)		# Move a zero.
	addw2	&1,p
	subw2	&1,%r1			# Keep count of zeroes produced.
	jpos	.f_frac2

# Now create the significant part of the fraction.

.f_frac3:
	cmpw	%r2,&0			# Any digits needed?
	jnpos	.f_frac5		# Skip if not.

.f_frac4:
	movb	0(bp),0(p)		# Copy a digit.
	jz	.f_fracp		# If no more digits, go pad.
	addw2	&1,bp
	addw2	&1,p
	subw2	&1,%r2			# Decrement count.
	jpos	.f_frac4		# Go back if more.
	jmp	.f_frac5

.f_fracp:
	movw	&ASCII_0,%r0		# Pad character.
.f_frap1:
	movtwb	%r0,0(p)		# Put a zero at end of number.
	addw2	&1,p
	subw2	&1,%r2			# Decrement count.
	jpos	.f_frap1		# Add more zeroes if necessary.

.f_frac5:
	subw3	&-buf,%sp,bp		# Put ptr to buffer in bp.
	jmp	.putall

#-----------------------------------------------------------------------#
#									#
# formats %G and %g							#
#									#
# The conversion is performed using "ecvt", then a choice is made	#
# between the %e and %f formats and the program branches to .e_merge	#
# or to .f_merge.							#
#									#
#-----------------------------------------------------------------------#

.G:
	movw	&UPPER_E,letter_e(%sp)	# Introduce exponent with E
	jmp	.g_1
.g:
	movw	&LOWER_e,letter_e(%sp)	# Introduce exponent with e
.g_1:
	bitw	&DOTSEEN,flags		# Precision specified?
	jnz	.g_2
	movw	&6,prec(%sp)		# Default is six decimal places.
	jmp	.g_cvt

.g_2:
	cmpw	prec(%sp),&0		# Zero significant places not permitted.
	jne	.g_cvt
	movw	&1,prec(%sp)		# Change zero to one.
.g_cvt:
	addw2	&8,%ap
	movw	-8(%ap),dval(%sp)	# Pick up value to convert.
	movw	-4(%ap),dval+4(%sp)	# Pick up value to convert.
	movw	&2,aflag(%sp)
	jmp	.excep			# Check for NaNs

.g_enter:
	movw	&0,aflag(%sp)
	pushw	dval(%sp)
	pushw	dval+4+SA1(%sp)
	cmpw	prec+SA2(%sp),&MAXECVT	# Get mininum of prec and MAXECVT.
	jge	.g_cvt1
	pushw	prec+SA2(%sp)		# Minimum is prec--put it on stack.
	jmp	.g_cvt2
.g_cvt1:
	pushw	&MAXECVT		# Minimum is MAXECVT--put it on stack.
.g_cvt2:
	pushaw	dec_pt+SA3(%sp)
	pushaw	sign+SA4(%sp)
	call	&5,ecvt			# bp = ecvt(dval, min(), &dec_pt, &sign)
	movw	%r0,bp
#	fcmpd	dval(%sp),.flzero
	andw3	&SIGN_MASK,dval(%sp),%r0# mask off sign
	orw2	dval+4(%sp),%r0		# OR of all bits other than sign
	jne	.g_rmz
	movw	&1,dec_pt(%sp)		# If val=0, it has 1 dig before dec pt.


# Remove trailing zeroes (unless '#' flag specified)

.g_rmz:
	movw	prec(%sp),%r3
	bitw	&FSHARP,flags
	jnz	.g_decid

	cmpw	%r3,&MAXECVT		# Is prec bigger than MAXECVT?
	jle	.g_rmz1
	movw	&MAXECVT,%r3		# If so, we only got MAXECVT digits.

.g_rmz1:
	movw	%r3,%r0
	jnpos	.g_decid		# No digits--can't trim any off.

	addw2	bp,%r0			# Get ptr to one past last digit.

.g_rmz2:
	cmpb	-1(%r0),&ASCII_0	# Last digit zero?
	jne	.g_decid		# If not, done with trimming.

	subw2	&1,%r0			# Back up by one digit,
	subw2	&1,%r3			# and reduce precision
	jpos	.g_rmz2			# as long as there are digits left.
	


# Decide whether to use %e or %f format.

.g_decid:
	cmpw	dec_pt(%sp),&-3		# Is number less than 0.0001 ?
	jl	.g_e			# Use %e format if so.
	cmpw	dec_pt(%sp),prec(%sp)	# Will %f fit into specified precision?
	jle	.g_f			# Use it if it will.

.g_e:
	subw2	&1,%r3			# Number of significant digits less one
	movw	%r3,prec(%sp)		# equals no. of digits after decimal.
	jmp	.e_merge		# Print significant digits with exp.

.g_f:
	subw2	dec_pt(%sp),%r3		# No. of significant digits less digits
	movw	%r3,prec(%sp)		# before decimal = digits after decimal.
	jmp	.f_merge

#-----------------------------------------------------------------------#
#									#
# %c format and double-percent.						#
#									#
# For %c, pick up the single character argument, place it in the	#
# buffer, and proceed with formatted output.				#
#									#
# For %%, simple use the % symbol (contained in %r3) as the output	#
# character.								#
#									#
#-----------------------------------------------------------------------#

	.align	4
.c:
	addw2	&4,%ap
	movw	-4(%ap),%r3
.dpercnt:
	movtwb	%r3,sbuf(%sp)
	addw3	&sbuf,%sp,bp
	movw	&1,p		# length of output.
	jmp	.putall1

#-----------------------------------------------------------------------#
#									#
# %s format								#
#									#
# Throughout the following code the value 0, representing the null	#
# character, is kept in register r2.					#
#									#
#-----------------------------------------------------------------------#

	.align	4
.s:
	addw2	&4,%ap
	movw	-4(%ap),bp	# Get pointer to beginning of string.
	movw	&0,%r2		# char to search for is null
	bitw	&DOTSEEN,flags	# was a precision specified?
	jnz	.s_prec		# Jump if so.
	bitw	&WIDTH,flags	# Mininum field width?
	jz	.s_as_is	# Skip if not.

# Minimum field width specified.  If there not room for the entire field,
# or if the string is not to be left-justified, go to .s_end.
# Otherwise copy string while looking for end.

	bitw	&FMINUS,flags	# Left-justification?
	jz	.s_end		# No--go find string end, compute length, etc.
	movw	width(%sp),%r3
	jz	.s_cp1
	addw3	%r3,ptr,%r0	# Compute what will happen to buffer pointer.
	cmpw	%r0,buff_end	# Will it go off the end of buffer?
	jgeu	.s_end		# If so, take cautious approach.
.s_cp:
	movb	0(bp),0(ptr)	# Copy a byte.	
	jz	.s_pad		# If end of string, go pad with blanks.
	addw2	&1,bp
	addw2	&1,ptr
	subw2	&1,%r3
	jpos	.s_cp		# Keep copying if width not exhausted.

.s_cp1:
	xorw2	&WIDTH,flags	# Turn off padding and finish string.
	jmp	.s_as_is

.s_pad:
	movw	&BLANK,%r1	# This is pad character.
.s_pad1:
	movtwb	%r1,0(ptr)	# Move pad character to end of string.
	addw2	&1,ptr
	subw2	&1,%r3
	jpos	.s_pad1		# Repeat until desired width obtained.
	jmp	.scan

# Simplest case--just put string (if there is room) and quit.

.s_as_is:
	mnegw	ptr,%r1
	addw2	buff_end,%r1	# Compute room available in buffer.
	jz	.s_end		# Skip if buffer full.
.s_as1:
	movb	0(bp),0(ptr)	# Move a byte of string into buffer.
	jz	.scan		# Quit if end of string.
	addw2	&1,bp
	addw2	&1,ptr
	subw2	&1,%r1
	jpos	.s_as1		# Move more bytes if buffer has room.

# For one reason or another, the string cannot be put with a single move.
# Find the end of the string and go to common output formatting.

.s_end:
	movw	bp,p
.s_end1:
	movzbw	0(p),%r0	# find end of string.
	jz	.putall
	addw2	&1,p
	jmp	.s_end1


# A precision was specified for the string.  This means that the string
# will be truncated after the number of characters specified by the
# precision, if it is longer than that.

	.align	4
.s_prec:
	movw	bp,p		# Remember where start of string is.
	movw	prec(%sp),%r1	# Get precision.
	jz	.putall		# Skip if zero.
	bitw	&WIDTH,flags	# Minimum width specified as well?
	jz	.s_prec1	# Jump if not.
	bitw	&FMINUS,flags	# Left-justification?
	jz	.s_prcnd	# If not, we must find end of string.

# Output up to prec characters or to end of string.

.s_prec1:
	subw3	ptr,buff_end,%r0	# Compute room available in buffer.
	cmpw	%r1,%r0		# Is it enough?  (r1 is precision)
	jgeu	.s_prcnd	# Skip if not.

.s_prec2:
	movb	0(p),0(ptr)	# Output prec characters or until end of string.
	jz	.s_prec3
	addw2	&1,p
	addw2	&1,ptr
	subw2	&1,%r1
	jpos	.s_prec2

.s_prec3:
	bitw	&WIDTH,flags	# Was width specified?
	jz	.scan		# If not, we are done.
	subw2	bp,p		# Compute length we wrote.
	subw3	p,width(%sp),%r2	# Get number of padding blanks needed.
	jpos	.putrpa1	# Put padding blanks.
	jmp	.scan		# (unless none are required)
#
# For one reason or another, the string cannot be put with a single move.
# Find end of string up to prec chars and go to common output formatting.

	.align	4
.s_prcnd:
	movzbw	0(p),%r0	# Scan for null, up to prec characters.
	jz	.putall
	addw2	&1,p
	subw2	&1,%r1
	jpos	.s_prcnd



#-----------------------------------------------------------------------#
#									#
# .putall -- calculate padding requirements and output the converted	#
#	value which has been set up in internal buffers.		#
#									#
# See also comments at "Conversion characters".				#
#									#
# On entry to .putall, register "bp" contains a pointer to the start	#
# of the converted value proper.  Register "p" contains a pointer to	#
# one character beyond this value.  On entry to .putall1 and		#
# thereafter, "p" contains the length of the value proper.  (This	#
# does not include the length of the prefix, suffix, etc.)		#
#-----------------------------------------------------------------------#
.putall:
	subw2	bp,p			# Compute length of value itself.
.putall1:
	bitw	&WIDTH,flags		# Minimum width specified?
	jz	.putpfx			# Skip padding if not.
	movw	width(%sp),%r2		# Compute padding needed:  desired width
					# (suffix len, etc., already subtracted)
	subw2	p,%r2			# less length of value
	subw2	prefixle(%sp),%r2	# less length of prefix.
	subw2	NaN(%sp),%r2		# less length of string NAN0x or nan0x 
					# if a NaN will be printed
	jnpos	.putpfx			# If not positive, skip padding.

	bitw	&PADZERO+FMINUS,flags	# Should we pad with blanks on left?
	jz	.padlblk		# If so, go do that.
	bitw	&FMINUS,flags		# Left-justification?
	jnz	.padblkd		# Yes--pad with blanks, but defer it.

					# PADZERO: pad with leading zeroes.
	bitw	&LZERO,flags		# Are there leading zeroes already?
	jnz	.putpdz1		# (If so, go add more)
	orw2	&LZERO,flags		# Set leading-zeroes flag;
	movw	%r2,lzero(%sp)		# set num of zeroes = pad count
	jmp	.putpfx

	.align	4
.putpdz1:
	addw2	%r2,lzero(%sp)		# Add pad count to leading zero count
	jmp	.putpfx

	.align	4
.padblkd:				# Come here for right-blank padding.
	movw	%r2,rpad(%sp)		# Save pad count for later.
	orw2	&RPAD,flags		# Set flag for right-padding.
	jmp	.putpfx			# Proceed with prefix for now.

	.align	4
.padlblk:				# Come here for immediate padding.
	movw	&.blanks,%r0
	cmpw	%r2,&PADMAX		# More than 40 padding blanks?
	jleu	.padlbk2		# Skip if not.
	jsb	.dopad40		# If so, emit 40 blanks.
	movw	%r3,%r2
	jmp	.padlblk

	.align	4
.padlbk2:
	movw	ptr,%r1			# Set up to emit remaining blanks:
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.padlbk3
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.putpfx

	.align	4
.padlbk3:
	movblb				# Yes--move blanks into buffer.

.putpfx:
	movw	prefixle(%sp),%r2	# Get prefix length.
	jz	.putpfx0		# If zero--skip prefix,check for NaN
	movw	prefix(%sp),%r0
	movw	ptr,%r1
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putpfx1
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.putpfx0

	.align	4
.putpfx1:
	movblb				# Yes--move prefix into buffer.
.putpfx0:
	cmpw	&5,NaN(%sp)		# If flag is set, get string NaN for output
	jne	.putlzer
	movw	&0,NaN(%sp)		# Set NaN flag back to zero
	cmpw	&UPPER_E,letter_e(%sp)  # Uppercase or lowercase ?
	jne	.putpfx3		
	movw	&.uc_nan,%r0
	jmp	.putpfx4
.putpfx3:
	movw	&.lc_nan, %r0
.putpfx4:
	movw	ptr,%r1
	movw	&5,%r2
	addw2	&5,ptr
	cmpw	ptr,buff_end		# Room in buffer before end?
	jleu	.putpfx2
	jsb 	.dowrite		# No-- get help from subroutine
	jmp	.putlzer

	.align 	4
.putpfx2:
	movblb
.putlzer:
	bitw	&LZERO,flags
	jz	.putvalu
	movw	lzero(%sp),%r2		# Get number of zeroes in r2.
.putlzr1:
	movw	&.zeroes,%r0
	cmpw	%r2,&PADMAX		# More than 40 leading zeroes?
	jleu	.putlzr2		# Skip if not.
	jsb	.dopad40		# Emit 40 zeroes.
	movw	%r3,%r2
	jmp	.putlzr1

	.align	4
.putlzr2:
	movw	ptr,%r1			# Set up to emit remaining zeroes.
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putlzr3
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.putvalu

	.align	4
.putlzr3:
	movblb				# Yes--move zeroes into buffer.

.putvalu:
	movw	p,%r2			# Pick up length of value.
	jz	.puttail		# Skip if null string.
	movw	bp,%r0			# Set up to move value.
	movw	ptr,%r1
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putval1
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.puttail

	.align	4
.putval1:
	movblb				# Yes--move value into buffer.

.puttail:
	bitw	&(RZERO+SUFFIX+RPAD),flags
	jz	.scan

	bitw	&RZERO,flags
	jz	.putsufx
	movw	rzero(%sp),%r2		# Get number of trailing zeroes in r2.

.putrzr1:
	movw	&.zeroes,%r0
	cmpw	%r2,&PADMAX
	jleu	.putrzr2
	jsb	.dopad40
	movw	%r3,%r2
	jmp	.putrzr1

	.align	4
.putrzr2:
	movw	ptr,%r1
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putrzr3
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.putsufx

	.align	4
.putrzr3:
	movblb				# Yes--move value into buffer.

.putsufx:
	bitw	&SUFFIX,flags
	jz	.putrpad
	movw	suffix(%sp),%r0
	movw	ptr,%r1
	movw	suffixle(%sp),%r2
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putsuf1
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.putrpad

	.align	4
.putsuf1:
	movblb				# Yes--move suffix into buffer.

.putrpad:
	bitw	&RPAD,flags
	jz	.scan
	movw	rpad(%sp),%r2
.putrpa1:
	movw	&.blanks,%r0
	cmpw	%r2,&PADMAX
	jleu	.putrpa2
	jsb	.dopad40
	movw	%r3,%r2
	jmp	.putrpa1

	.align	4
.putrpa2:
	movw	ptr,%r1			# Set up to move remaining blanks.
	addw2	%r2,ptr
	cmpw	ptr,buff_end		# Room in buffer before buffer end?
	jleu	.putrpa3
	jsb	.dowrite		# No--get help from subroutine.
	jmp	.scan

	.align	4
.putrpa3:
	movblb				# Yes--move blanks into buffer.
	jmp	.scan

