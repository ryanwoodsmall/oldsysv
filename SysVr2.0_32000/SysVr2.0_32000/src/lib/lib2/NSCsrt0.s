#	NSCsrt0.s -- start-up code for standalone programs
#			(installed as /lib/crt2.o)

	.file	"NSCsrt0.s"
	.globl	start
	.globl	_edata
	.globl	_end
	.globl	__init
	.globl	_main
	.globl	_exit
	.globl	__exit
	.globl	_close
	.globl	__cleanup
	.set	stackbase,0x70000
	.set	NFILES,9

	.text

	.double	0,0,0,0,0,0,0,0 # space for mod table
start:
	cmpd	$0x12345678,4(sp)	# called by s.a. shell or boot?
	beq	L0		# if called by s.a. shell, all is OK
	movd	tos,r0		#   else get boot ret. address off stack
	lprd	sp,$stackbase	# set sp,fp to known value
	lprd	fp,$stackbase
	movd	r0,tos		# restore ret. addr. to new stack
L0:
	addr	_end,r0		# set up to clear bss
	addr	_edata,r1
	cmpd	r0,r1		# make sure _end > _edata
	bls	L1
	subd	r1,r0		# find bss size
	addqd	-1,r0		# no overshoot (last 0 at _end-1)
	movqb	0,0(r1)		# clear first byte of bss
	addr	1(r1),r2	# dest. range 1 more than src. range
	movsb			# r0 = count, r1 = src, r2 = dst
L1:

# save sp, fp, and other register for calls to "exit()"

	addr	savearea,tos
	jsr	_setjmp
	adjspb	$-4
	cmpqd	0,r0	# Non-zero when returning via longjmp
	bne	L4

	jsr	__init		# initialize device and mount tables
	addr	_environ,tos
	cmpd	$0x12345678,8(sp)	# called by 'sa' or 'fboot'?
	bne	L2
	movd	16(sp),tos	# called by standalone shell; argv ptr
	movd	16(sp),tos	# argc (16 offset with argv pushed on stk)
	br	L3
L2:
	addr	argv0,tos	# argv ptr to null string
	movqd	1,tos		# argc = 1 means no arguments (boot)
L3:
	movmb	0xe00010,16,12	# restore interrupt vectors for monitor
	jsr	_main		# normally s.a. programs call _exit, which
				#   calls __cleanup and then __exit...
	jsr	_exit		# If not, clean up (flush file buffers)
L4:
	cmpd	0(sp),$stackbase # is return address above program stack?
	bge	L4ret
	bpt			# stop to avoid returning to an invalid address
L4ret:
	ret	0

__exit:
	movd	$NFILES,r7
	movd	4(sp),r6		# return value
L5:
	movd	r7,tos	# close any open files
	jsr	_close
	adjspb	$-4	# drop ptr
	addqd	-1,r7
	cmpd	r7,$0
	bge	L5

	movd	r6,tos
	addr	savearea,tos
	jsr	_longjmp		# return to call of setjmp

	.data
argv0:	.double	argv00
argv00:	.double	0
savearea: .space 10*4		# setjmp/longjmp register save area
	.comm	_environ,4
