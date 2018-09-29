		/ UNIX/TS - Bell Laboratories /

/ Low core

br4 = 200
br5 = 240
br6 = 300
br7 = 340
SSR0 = 177572

.globl	powmm, powback, tstart, restart, dstart, start, dump, call, _utsname
.globl	trap, trap4, trap10, trap14, trap24, trap240, trap250


.data

DZERO:

. = DZERO+0
	jmp	dstart		/ Initial startup
. = DZERO+4
	trap4;	br7+0.		/ Bus error
. = DZERO+10
	trap10;	br7+1.		/ Illegal instruction
. = DZERO+14
	trap14;	br7+2.		/ Bpt-trace trap
. = DZERO+20
	trap;	br7+3.		/ Iot trap
. = DZERO+24
	trap24;	br7+4.		/ Power failure
. = DZERO+30
	trap;	br7+5.		/ Emulator trap
. = DZERO+34
	trap;	br7+6.		/ System entry
. = DZERO+40
	_utsname;		/ Name of system
	br .
. = DZERO+44
dump:				/ Memory dump
.globl	dump_tm
	jmp	*$dump_tm
. = DZERO+50
	.+2;	br .		/ Trap catcher
. = DZERO+54
	.+2;	br .
. = DZERO+60
	klin;	br4+0.
	klou;	br4+0.
	.+2;	br .
	.+2;	br .
. = DZERO+100
	clio;	br6+0.
. = DZERO+104
	clio;	br6+1.
	.+2;	br .
. = DZERO+114
	trap;	br7+10.		/ Memory system error
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
. = DZERO+224
	tmio;	br5+0.
	.+2;	br .
	.+2;	br .
. = DZERO+240
	trap240;	br7+7.		/ Programmed interrupt
. = DZERO+244
	trap;	br7+8.		/ Floating point
. = DZERO+250
	trap250;		/ Segmentation violation
	br7+9.
. = DZERO+254
	rpio;	br5+0.
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
. = DZERO+1000

.text

TZERO:

. = TZERO+0
	42;	br .
. = TZERO+4
	trap4;	br7+0.		/ Bus error
. = TZERO+10
	trap10;	br7+1.		/ Illegal instruction
. = TZERO+14
	trap14;	br7+2.		/ Bpt-trace trap
. = TZERO+20
	trap;	br7+3.		/ Iot trap
. = TZERO+24
	trap24;	br7+4.		/ Power failure
. = TZERO+30
	trap;	br7+5.		/ Emulator trap
. = TZERO+34
	trap;	br7+6.		/ System entry
. = TZERO+40
	_utsname;		/ Name of system
	br .
. = TZERO+44
Dump:				/ Memory dump
.globl	dump_tm
	jmp	*$dump_tm
. = TZERO+50
	.+2;	br .		/ Trap catcher
. = TZERO+54
	.+2;	br .
. = TZERO+60
	klin;	br4+0.
	klou;	br4+0.
	.+2;	br .
	.+2;	br .
. = TZERO+100
	clio;	br6+0.
. = TZERO+104
	clio;	br6+1.
	.+2;	br .
. = TZERO+114
	trap;	br7+10.		/ Memory system error
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
. = TZERO+224
	tmio;	br5+0.
	.+2;	br .
	.+2;	br .
. = TZERO+240
	trap240;	br7+7.		/ Programmed interrupt
. = TZERO+244
	trap;	br7+8.		/ Floating point
. = TZERO+250
	trap250;		/ Segmentation violation
	br7+9.
. = TZERO+254
	rpio;	br5+0.
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
	.+2;	br .
. = TZERO+1000

.data
powmm:	inc	SSR0
	jmp	powmm

tstart:	inc	SSR0
	jmp	dstart

.text
	dec	SSR0
	jmp	powback

restart:	dec	SSR0
	jmp	start


/ Interface code to C

.globl	_clock
clio:
	jsr	r0,call; jmp  _clock

.globl	_klrint, _klxint
klin:
	jsr	r0,call; jmp  _klrint
klou:
	jsr	r0,call; jmp  _klxint

.globl	_rpintr
rpio:
	jsr	r0,call; jmp  _rpintr

.globl	_tmintr
tmio:
	jsr	r0,call; jmp  _tmintr

