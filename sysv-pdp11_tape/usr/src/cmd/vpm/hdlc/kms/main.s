/*	main.s 1.9 of 3/23/82
	@(#)main.s	1.9
 */


#define	HDLC
/*
 * VPM interpreter
 *
 * This version of the interpreter supports both BISYNC and HDLC.
 *
 * The desired protocol class is selected by the #define at line 1.
 *
 * Allocation of scratch-pad registers:
 *
 *	r15 - unused
 *	r14 - interpreter flags
 *	r13 - current state of the dialog with the PDP-11
 *	r12 - current state of the virtual process
 *	r11 - frame pointer (not used in phase 1)
 *	r10 - stack pointer
 *	 r9 - high-order bits of program counter (PC)
 *	 r8 - low-order bits of program counter (PC)
 *	 r7 - unused
 *	 r6 - accumulator (AC)
 *	 r5 - scratch
 *	 r4 - scratch
 *	 r3 - scratch
 *	 r2 - scratch
 *	 r1 - scratch
 *	 r0 - scratch
 *
 * Macro to delay until the previously requested unibus transfer,
 * if any, completes
 *
 * Usage:	BUSWAIT(label)
 *
 * where <label> is the address to receive control if a unibus
 * error occurs
 */
#define BUSWAIT(X)\
0:\
	mov	npr,brg;\
	br0	0b;\
	mov	nprx,brg;\
	br0	X;

/*
 * Macro to transfer between segments using jmp instruction
 */
#define	SEGJMP(X)\
	mov	%X,brg;\
	mov	brg,pcr;\
	jmp	X;

/*
 * Macro to remove the xbt entry or rbt entry from the head
 * of a specified queue
 *
 * Usage:	REMOVEQ(queue,reg,error);
 *
 * where <queue> is the name of the queue (the label for
 * the pointer to the head of the queue),
 * <reg> is the scratch-pad register to receive
 * the entry address, and <error> is the label to
 * receive control if the queue is empty
 *
 * Note:  This macro uses r0 as a scratch register.
 */
#define	REMOVEQ(X,Y,Z)\
	mov	X,mar;\
	mov	mem,Y;\
	brz	Z;\
	mov	LINK,brg;\
	add	brg,Y,mar;\
	mov	mem,r0;\
	mov	X,mar;\
	mov	r0,mem|mar++;\
	brz	0f;\
	br	9f;\
0:\
	mov	r0,mem;\
9:

/*
 * Macro to append an xbt entry or an rbt entry to the end
 * of a specified queue
 *
 * Note:  This macro uses r0 as a scratch register and uses numeric label 9
 *	   as a forward-branch label
 */
#define	APPENDQ(X,Y)\
	mov	LINK,brg;\
	add	brg,Y,mar;\
	mov	NIL,mem;\
	mov	X+1,mar;\
	mov	mem,r0;\
	brz	0f;\
	mov	Y,mem;\
	add	brg,r0,mar;\
	br	9f;\
0:\
	mov	X,mar;\
	mov	Y,mem|mar++;\
9:\
	mov	Y,mem

/*
 * Subroutine CALL and RETURN macros
 *
 * Nested calls are not supported
 */
#define	SCALL(X)\
	mov	%return,%mar;\
	mov	return,mar;\
	mov	%0f,mem|mar++;\
	mov	0f,mem;\
	mov	%X,brg;\
	mov	brg,pcr;\
	jmp	X;\
0:

#define	SRETURN	\
	mov	%return,%mar;\
	mov	return,mar;\
	mov	mem,pcr|mar++;\
	jmp	(mem)

/*
 * Macro to test for a valid page number in r12
 *
 * Usage:	LINECHK(error)
 *
 * where <error> is the label to branch to if the page
 * number is invalid
 */
#define	LINECHK(X)\
	mov	15,brg;\
	addn	brg,r12,-;\
	brc	X;\
	mov	7,brg;\
	addn	brg,r12,-;\
	brc	0f;\
	br	X;\
0:

/*
 * Macro to increment a one-byte error counter.
 * If the counter value is 0377, the value is not incremented.
 *
 * Usage:	COUNTUP(counter)
 *
 * where <counter> is the name of the counter to be incremented.
 * number is invalid
 */
#define	COUNTUP(X)\
	mov	X,mar;\
	mov	mem,r0;\
	brz	0f;\
	inc	r0,mem;\
0:

/*
 * Define the number of transmit and receive buffers
 */
#define	NXB	2
#define	NRB	2
/*
 * Define command codes and report codes for communication with driver
 */
#include "sys/csikmc.h"
#include "sys/csierrs.h"
/*
 * Define bits in r14
 */
#define	REPORTG	(1<<1)
#define	DRVOK	(1<<2)
#define	CRASHED	(1<<3)
#define	OKCHK	(1<<5)
#define	BASEOUT	(1<<6)
#define	OKOUT	(1<<7)
/*
 * Define flags in r6 for return code from <rcvfrm>
 */
#define	NOBUF	1<<4
/*
 * Define error flags for GETBYTE and PUTBYTE
 */
#define	NOMORE	(1<<0)
#define	NOROOM	(1<<0)
/*
 * Define the states of a virtual process (values in <state>)
 */
#define	IDLE	0
#define	READY	1
#define	WAITDSR	2
#define	HALTED	3
#define	INWAIT	4
#define	OUTWAIT	5
#define	PAUSING	6
/*
 * Define bits in npr
 */
#define NRQ (1<<0)
#define OUT (1<<4)
#define BYTE (1<<7)
/*
 * Define bits in nprx
 */
#define NEM (1<<0)
#define ACLO (1<<1)
#define PCLK (1<<4)
#define VEC4 (1<<6)
#define BRQ (1<<7)
/*
 * Define bits in csr0
 */
#define IEI (1<<0)
#define IEO (1<<4)
#define RQI (1<<7)
/*
 * Define bits in csr2
 */
#define RDYO (1<<7)
#define RDYI (1<<4)
/*--------------------------------------------------*/
/*
 * Define bits in line select register (lur1)
 */
#define LINE0 (0<<4)
#define LINE1 (1<<4)
#define LINE2 (2<<4)
#define LINE3 (3<<4)
#define LINE4 (4<<4)
#define LINE5 (5<<4)
#define LINE6 (6<<4)
#define LINE7 (7<<4)
/*
 * Define bits in line status register (lur3)
 */
#define RDA (1<<0)
#define RSA (1<<1)
#define RAC (1<<2)
#define TBMT (1<<4)
#define TAC (1<<6)
#define TSA (1<<7)
/*
 * Define bits in line control register (lur4)
*/
#define RENA (1<<0)
#define TENA (1<<4)
/* Define maintenance select bits */
#define MDMCLK (0<<5)
#define STEPON (2<<5)
#define STEPOFF (3<<5)
#define INTCLK (4<<5)
#define LOOPON (6<<5)
#define LOOPOFF (7<<5)
/*
 * Define bits in receive status register (sr1)
*/
#define RSOM (1<<0)
#define REOM (1<<1)
#define RAB  (1<<2)
#define	ROR	(1<<3)
/*
 * Define bits of transmit status/control reg (sr3)
*/
#define TXAB (1<<2)
#define TXGA (1<<3)
/*--------------------------------------------------*/
#define	TSOM	(1<<0)
#define	TEOM	(1<<1)
#define	RSOM	(1<<0)
#define	NOP	mov 0,brg
/*
 * Define bits in lur2
 */
#define RESET (1<<0)
#define	WRITE	(1<<4)
#define	READ	0
#define	SR0	(0<<5)
#define	SR1	(1<<5)
#define	SR2	(2<<5)
#define	SR3	(3<<5)
#define	SR4	(4<<5)
#define	SR5	(5<<5)
#define	SR6	(6<<5)
#define	SR7	(7<<5)
/*
 * Define useful constants
 */
#define	NIL		0377
#define	NZERO		017
/*
 * Define the size of a transmit-buffer-table entry
 */
#define	LXBTE	8
/*
 * Define the size of a receive-buffer-table entry
 */
#define	LRBTE	12
/*
 * Define the elements of a receive-buffer-table entry
 * (The elements of a transmit-buffer-table entry are
 * the same except that the RCOUNT and RCTL fields are missing.)
 */
#define	LINK	0
#define	STATE	1
#define	DESC	2
#define	RCOUNT	5
#define	RCTL	7
/*
 * Define the length of the KMC portion of a buffer descriptor
 */
#define	BDLENGTH	12
/*
 * Define the offsets for the relevant fields of a buffer descriptor
 */
#define	BDBUFADR	0
#define	BDSIZE	4
#define	BDPARAM	8
/*
 * Data definitions
 */
	.data
/*
 * Interpreter global data goes here
 */
	.org	0*256
/*
 * Register save area (for debugging)
 */
#define	regsave	0
reportq:  .byte	NIL,NIL
timeq:	.byte	NIL,NIL
readyq:	.byte	NIL,NIL
actline:	.byte	NIL
/*
 * Number of 75-microsecond ticks until the current 100-millisecond
 * timer interval expires
 */
clock:	.byte	0,0
/*
 * Number of 100-millisecond intervals until next driver-sanity check
 */
clock2:	.byte	0
/*
 * Save area for subroutine CALL and RETURN macros
 */
return: .byte	0,0
#define	LACTBUF	24
actbuf:	.org	.+LACTBUF
#define	iticks	(actbuf+2)
#define	lcnt	(iticks+4)
#define	icnt	(lcnt+2*8)
okbuf:	.byte	0,0,0
actbits:  .byte  0,0
/*
 * Virtual-process instruction space starts here
 */
start:
#include	"opdef.h"
#include	CFILE
/*
 * Virtual-process instruction space ends here
 */
end:
#ifdef	BISYNC
/*
 * Table to convert 8-bit ASCII to EBCDIC
 */
	.data
	.org	12*256
atoetbl:
#include	"atoetbl.h"
/*
 * Table to convert EBCDIC to 8-bit ASCII
 */
	.data
	.org	13*256
etoatbl:
#include	"etoatbl.h"
#endif
/*
 *  The following data items are replicated on the data page for each line
 */
	.data
	.org	15*256
/*
 * Last command received from the host computer
 */
cmdsave: .byte	0,0,0,0,0,0,0,0
state:	.byte	0
flags:	.byte	0
/*
 * Define bits in flags cell
 */
#define	ISACT	(1<<0)
#define	REPORTL	(1<<1)
#define	SAM	(1<<2)
#define	HAVCMD	(1<<3)
#define	HUPCLOSE	(1<<4)
#define	X25FLAG	(1<<6)
#define	XEOM	(1<<6)
#define	XLAST	(1<<7)
#define	HAVCTL	(1<<7)
/*
 * Links for global queues (readyq, reportq, timerq, etc.)
 */
rptlink:  .byte	0
timelink:  .byte	0
rdylink:  .byte	0
/*
 * Report flags
 *
 * Each bit is used to schedule a different report
 */
reports:	.byte	0,0
/*
 * Define flags for report scheduling
 * (bits in <reports> cell)
 */

#define	STARTOUT  (1<<1)
#define	STOPOUT  (1<<2)
#define	ERROUT	(1<<3)
#define	PCDOUT	(1<<4)
#define	TRACEOUT  (1<<5)
#define	SNAPOUT	(1<<6)
#define	RPTOUT	(1<<7)
/*
 * The following flags are in the second byte
 */
#define	CMDACK	(1<<0)
#define	ERRCNT1	(1<<1)
#define	ERRCNT2	(1<<2)
/*
 * Temp save for report parameter
 */
errcode: .byte	0
/*
 * Reason for script termination
 */
termcode: .byte	0
/* reason for other terminations */
startcode: .byte 0
stopcode: .byte 0
/*
 * Device number for reports to driver
 */
device:  .byte	0
/*
 * Temp save for TRACE parameters
 */
traceval: .byte	0,0
/*
 * Temp save for script command from driver
 */
scrcmd: .byte	0,0,0,0
/*
 * Timer count:  number of 100-millisecond intervals
 * until next timer expiration
 */
tcount:	.byte	0
/*
 * Number of 100-millisecond intervals until next timeout occurs
 */
clock1:	.byte	0
/*
 * Saved info from last time-out request
 */
toutsave: .byte	0,0,0,0
/*
 * Per-process register-save area
 */
psave:	.byte	0,0,0,0,0,0
/*
 * Number of characters transmitted
 */
xmtnpc:		.byte	0,0
/*
 * KMC copy of the buffer descriptor for the current transmit buffer
 */
xmtdesc:	.org	.+BDLENGTH
/*
 * Size of current transmit buffer
 */
#define	xmtsize	(xmtdesc+BDSIZE)
/*
 * Pointer to next byte in transmit buffer
 */
#define	xmtcp	(xmtdesc+BDBUFADR)
/*
 * Transmit-buffer parameters (type, station, and device)
 */
#define	xbparam	(xmtdesc+BDPARAM)
/*
 * KMC copy of the buffer descriptor for the current receive buffer
 */
rcvdesc:	.org	.+BDLENGTH
/*
 * Size of current receive buffer
 */
#define	rcvsize	(rcvdesc+BDSIZE)
/*
 * Pointer to next byte in receive buffer
 */
#define	rcvcp	(rcvdesc+BDBUFADR)
/*
 * Receive-buffer parameters (type, station, and device)
 */
#define	rbparam	(rcvdesc+BDPARAM)
/*
 * Saved byte from last two-byte fetch from current transmit buffer
 */
xmtsave:	.byte	0
/*
 * Current receive buffer obtained by rcvfrm
 */
rcvbuf:	.byte	NIL
/*
 * Current receive buffer used to record incoming frame
 */
rcvbuf1:	.byte	NIL
/*
 * Current transmit buffer
 */
xmtbuf:	.byte	NIL
/*
 * Number of received characters in current receive buffer
 */
rnrc:	.byte	0,0
/*
 * Saved byte for next two-byte store into receive buffer
 */
rsave:	.byte	0
/*
 * Error counters
 */
#define	NERCNT	8
errcnt:	.org	.+NERCNT
#define	nerr0	(errcnt+0)
#define	nerr1	(errcnt+1)
#define	nerr2	(errcnt+2)
#define	nerr3	(errcnt+3)
#define	nerr4	(errcnt+4)
#define	nerr5	(errcnt+5)
#define	nerr6	(errcnt+6)
#define	nerr7	(errcnt+7)
/*
 * Transmit-buffer table
 */
xbt:
	.org	.+NXB*LXBTE
/*
 * Receive-buffer table
 */
rbt:
	.org	.+NRB*LRBTE
/*
 * Unused-receive-buffer-table-entry queue
 */
urbq:	.byte	NIL,NIL
/*
 * Empty-receive-frame queue
 */
erfq:	.byte	NIL,NIL
/*
 * Receive frame ready queue (used by getrfrm)
 */
rbrq:	.byte	NIL,NIL
/*
 * Completed receive frame queue
 */
crfq:	.byte	NIL,NIL
/*
 * Unused transmit buffer table entry queue
 */
uxbq:	.byte	NIL,NIL
/*
 * Transmit-buffer-get queue (used by getxfrm)
 */
xbgq:	.byte	NIL,NIL
/*
 * Transmit-buffer-open queue
 */
xboq:	.byte	NIL,NIL
/*
 * Transmit-buffer-return queue (xbufout queue)
 */
xbrq:	.byte	NIL,NIL
#ifdef	BISYNC
/*
 * Address of source/destination for i/o instruction
 */
ioadr:	.byte	0
/*
 * Address of array to receive CRC accumulation
 */
crcsav:	.byte	0
#endif
#ifdef	HDLC
/*
 * Transmitter state
 */
xmtstate:	.byte	0
/*
 * receiver state
 */
rcvstate:	.byte	0
/*
 * Pointer to next byte of transmit control info
 */
xctlptr:	.byte	0
/*
 * Number of transmit-control bytes to be sent
 */
nxctl:	.byte	0
/*
 * Number of bytes of receive-control info received
 */
nrctl:	.byte	0
/*
 * Saved input character
 */
charsave:	.byte	0
/*
 * Saved input flags
 */
flagsave:	.byte	0
/*
 * Number of flags remaining to be sent before the start of the next frame
 */
flagcnt:	.byte	0
/*
 * Transmit-control info
 */
xmtac:		.byte	0,0,0,0,0
/*
 * Receive-control info
 */
rcvac:		.byte	0,0,0,0,0
/*
 * Open-transmit-buffer table
 *
 * Used to translate sequence numbers into xbt entry numbers
 */
oxbt:
	.byte	NIL,NIL,NIL,NIL,NIL,NIL,NIL,NIL
/*
 * Interpreter options
 */
vpmopts:  .byte  0,0
edata:
#endif
/*
 * Instruction text starts here
 */
	.text
/*
 * KMC initialization sequence--keep this at location zero
 */
	.org	0
seg0:
/*
 * Initialize the KMC
 */
	mov	PCLK,brg
	mov	brg,nprx
	mov	IEO,brg
	mov	brg,csr0
	mov	0,brg
	mov	brg,csr2
	mov	brg,r15
	mov	brg,r14
	mov	brg,r13
	mov	brg,lur3
	mov	NIL,brg
	mov	brg,r12
	br	dispret0
/*
 * Process a command from the driver
 */
commandc:
/*
 * Set dialog state = 0
 */
	mov	0,brg
	mov	brg,r13
/*
 * If a crash has occurred then go to dispret0
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	dispret0
/*
 * Get the command type from csr2
 */
	mov	csr2,r0
	mov	7,brg
	and	brg,r0
/*
 * If command type == 7 then go to basein;
 * else if command type == 4 then go to okcmd
 */
	mov	BASEIN,brg
	addn	brg,r0,-
	brz	basein
	mov	OKCMD,brg
	addn	brg,r0,-
	brz	okcmd
/*
 * Get the line number from csr3
 */
	mov	csr3,r0
	mov	NZERO,brg
	and	brg,r0
	mov	7,brg
	addn	brg,r0,-
	brc	drverr1
/*
 * Convert the line number to a page number
 * and set page register for this line
 */
	mov	NZERO,brg
	xor	brg,r0,brg
	mov	brg,r12|%mar
/*
 * If the script is too large then error
 */
	mov	%end,brg
	addn	brg,r12,-
	brc	1f
	mov	0151,brg
	br	crash0
1:
/*
 * Save the command info for debugging
 */
	LINECHK(cmderr3)
	mov	cmdsave,mar
	mov	csr0,mem|mar++
	mov	csr1,mem|mar++
	mov	csr2,mem|mar++
	mov	csr3,mem|mar++
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem|mar++
/*
 * Get the command type from csr2
 */
	mov	csr2,r0
	mov	7,brg
	and	brg,r0
/*
 * If command type == 0 then go to xbufin
 * else if command type == 1 then go to rbufin
 * else if command type == 2 then go to run
 * else if command type == 3 then go to halt
 * else if command type == 4 then go to cmderr1
 * else if command type == 5 then go to cmdin
 * else if command type == 7 then go to cmderr2
 * else cmderr
 */
	dec	r0
	brz	xbufin
	dec	r0
	brz	rbufin
	dec	r0
	brz	run
	dec	r0
	brz	halt
	dec	r0
	brz	cmderr1
	dec	r0
	brz	cmdin
	dec	r0
	dec	r0
	brz	cmderr2
	br	cmderr
/*
 * Process a RUN command
 *
 * The format of a RUN command is as follows:
 *
 *	Byte		Contents
 *
 *	csr2	Command type
 *	csr3	Line number
 *	csr4	Driver version number
 *	csr5	unused
 *	csr6	Device number for reports to driver
 *	csr7	unused
 */
run:
/*
 * Schedule a startup report
 */
	mov	STARTOUT,brg
	mov	brg,r0
	SCALL(rptschd);
/*
 * Restore the page register
 */
	mov	r12,%mar
/*
 * Set normal return code
 */
	mov	startcode,mar
	mov	0,mem
/*
 * If the state of the virtual process is anything except
 * halted or idle then error
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	1f
	mov	IDLE,brg
	addn	brg,r0,-
	brz	1f
	mov	startcode,mar
	mov	CMDERR,mem
	br	cmderr
1:
/*
 * Save the device number for use later
 */
	mov	device,mar
	mov	csr6,mem
/*
 * Save interpreter options
 */
	mov	vpmopts,mar
	mov	csr4,mem|mar++
	mov	csr5,mem
/*
 * Initialize queue headers
 */
	mov	urbq,mar
	mov	xbrq-urbq+2-1,brg
	mov	brg,r0
1:
	mov	NIL,mem|mar++
	dec	r0
	brc	1b
/*
 * Initialize the receive-buffer-table:
 *
 *	r5=rbt;
 *	r1=NRB-1;
 *	do{
 *		mar=r5+STATE;
 *		mem=0;
 *		APPENDQ(urbq,r5);
 *		r5+=LRBTE;
 *		--r1;
 *	}while(carry);
 */
	mov	rbt,brg
	mov	brg,r5
	mov	NRB-1,brg
	mov	brg,r1
1:
	mov	STATE,brg
	add	brg,r5,mar
	mov	0,mem
	APPENDQ(urbq,r5)
	mov	LRBTE,brg
	add	brg,r5
	dec	r1
	brc	1b
/*
 * Initialize the transmit-buffer-table:
 *
 *	rb=xbt;
 *	r1=NXB-1;
 *	do{
 *		mar=r5+STATE;
 *		mem=0;
 *		APPENDQ(uxbq,r5);
 *		r5-=LXBTE;
 *		--r1;
 *	}while(carry);
 */
	mov	xbt,brg
	mov	brg,r5
	mov	NXB-1,brg
	mov	brg,r1
1:
	mov	STATE,brg
	add	brg,r5,mar
	mov	0,mem
	APPENDQ(uxbq,r5)
	mov	LXBTE,brg
	add	brg,r5
	dec	r1
	brc	1b
/*
 * Initialize rcvbuf, rcvbuf1, and xmtbuf
 */
	mov	rcvbuf,mar
	mov	NIL,mem
	mov	rcvbuf1,mar
	mov	NIL,mem
	mov	xmtbuf,mar
	mov	NIL,mem
#ifdef	HDLC
/*
 * Initialize oxbt
 */
	mov	7,brg
	mov	brg,r0
	mov	oxbt,mar
1:
	mov	NIL,mem|mar++
	dec	r0
	brc	1b
/*
 * Clear xmtstate and rcvstate
 */
	mov	xmtstate,mar
	mov	4,mem
	mov	rcvstate,mar
	mov	0,mem
#endif
/*
 * Initialize error counters
 */
	mov	errcnt,mar
	mov	NERCNT-1,brg
	mov	brg,r0
1:
	mov	0,mem|mar++
	dec	r0
	brc	1b
/*
 * Clear the user data area
 */
	mov	SSTACK,brg
	mov	brg,r0|mar
1:
	mov	0,mem
	inc	r0,r0|mar
	brc	1f
	br	1b
1:
/*
 * Initialize the register save area
 */
	mov	psave,mar
	mov	0,mem|mar++		/* r6 */
	mov	0,mem|mar++		/* r7 */
	mov	0,mem|mar++		/* r8 */
	mov	0,mem|mar++		/* r9 */
	mov	SSTACK,mem|mar++	/* r10 */
	mov	0,mem			/* r11 */
/*
 * If the line has never been started then put it into the time-check queue
 */
	mov	state,mar
	mov	mem,r0
	mov	IDLE,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	mov	timelink,mar
	mov	NIL,mem
	mov	%timeq,%mar
	mov	timeq,mar
	mov	mem,r0|mar++
	brz	1f
	mov	mem,r0
	mov	r12,mem
	mov	r0,%mar
	mov	timelink,mar
	mov	r12,mem
	br	2f
1:
	mov	timeq,mar
	mov	r12,mem|mar++
	mov	r12,mem
2:
/*
 * Mark the line as active
 */
	mov	actbits,mar
	mov	%actbits,%mar
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	brg,r2
	mov	1,r1
1:
	dec	r2
	brz	1f
	asl	r1
	br	1b
1:
	or	mem,r1,mem
/*
 * Restore the page register
 */
	mov	r12,%mar
#ifdef	BISYNC
/*
 * Turn on data-terminal-ready and set receive mode = half-duplex.
 */
	mov	DTR|HDX,brg
	mov	brg,lur3
#endif
#ifdef	HDLC
/*
 * Put line number in lur1
 */
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	brg,r2
	asl	r2
	asl	r2
	asl	r2
	asl	r2
	mov	r2,brg
	mov	brg,lur1
/*
 * Reset USYRT
 */
	mov	RESET,brg
	mov	brg,lur2
/*
 * Wait for the dust to settle
 */
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
/*
 * Set line parameters
 */
	mov	r2,brg
	mov	brg,lur1
	mov	SR5|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Set line parameters
 */
	mov	r2,brg
	mov	brg,lur1
	mov	0,brg
	mov	brg,lur3
/*
 * Set line parameters
 */
	mov	r2,brg
	mov	brg,lur1
	mov	0,brg
	mov	brg,lur4
/*
 * Get interpreter options
 */
	mov	vpmopts,mar
	mov	mem,r1
/*
 * If hardware loopback option is set then set internal loopback mode
 */
	mov	TENA|RENA|MDMCLK,brg	/* MODEM clock */
	mov	brg,r0
	mov	HWLOOP,brg
	orn	brg,r1,-
	brz	1f
	br	2f
1:
	mov	TENA|RENA|INTCLK,brg	/* internal clock */
	mov	brg,r0
2:
/*
 * Set line parameters
 */
	mov	r2,brg
	mov	brg,lur1
	mov	r0,lur4
	NOP
#endif
/*
 * Set process state = READY
 */
	mov	state,mar
	mov	READY,mem
/*
 * Clear flags
 */
	mov	ISACT|REPORTL,brg
	mov	brg,r0
	mov	flags,mar
	and	mem,r0,mem
/*
 * If X25 packet-header-separation mode was requested then set the
 * corresponding bit in flags
 */
	mov	csr4,brg
	br0	1f
	br	2f
1:
	mov	X25FLAG,brg
	mov	brg,r0
	mov	flags,mar
	or	mem,r0,mem
2:
/*
 * Schedule a STARTUP report
 * (The process will be activated when the start-up report completes
 */
	mov	STARTOUT,brg
	br	rptschd0
/*
 * Process a BASEIN command from the driver
 */
basein:
	mov	OKCHK|DRVOK,brg
	or	brg,r14
	mov	REPORTG|BASEOUT,brg
	or	brg,r14
	br	dispret0
/*
 * Process a HALT command from the driver
 */
halt:
/*
 * Set normal return code
 */
	mov	stopcode,mar
	mov	HALTRCVD,mem
/*
 * Schedule a STOPOUT report
 */
	mov	STOPOUT,brg
	mov	brg,r0
	SCALL(rptschd);
/*
 * Restore the page register
 */
	mov	r12,%mar
/*
 * If the virtual process is already halted then go to halte
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	halte
/*
 * If the virtual process has never been started then go to halte
 */
	dec	r0,-
	brz	halte
/*
 * Get the reason for termination
 */
	mov	HALTRCVD,brg
/*
 * Go to errterm0
 */
	br	errterm0
halte:
	mov	stopcode,mar
	mov	CMDERR,mem
	br	dispret0
/*
 * Process an XBUFIN command
 *
 * The format of an XBUFIN command is:
 *
 *	Byte		Contents
 *
 *	csr2	Command type
 *	csr3	Line number
 *	csr4	Bits 7-0 of the descriptor address
 *	csr5	Bits 15-8 of the descriptor address
 *	csr6	Bits 17-16 of the descriptor address
 *	csr7	Unused
 */
xbufin:
/*
 * If the virtual process is halted or idle then error
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	1f
	mov	IDLE,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	br	cmderr
2:
/*
 * If the descriptor address is odd then go to xbuferr0
 */
	mov	csr4,brg
	br0	xbuferr0
/*
 * If the unused-xbt-entry queue is empty then go to xbuferr0;
 * otherwise get the xbt entry at the head of the unused-xbt-entry
 * queue and append it to the transmit-buffer-ready queue
 */
	REMOVEQ(uxbq,r5,xbuferr0)
	APPENDQ(xbgq,r5)
/*
 * If the xbt entry is marked as in-use then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r1
	dec	r1,-
	brc	xbuferr0
/*
 * Mark the transmit-buffer-table entry in-use
 */
	mov	1,mem
/*
 * Copy the descriptor address to the transmit-buffer-table entry
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
	SCALL(react);
/*
 * Go to dispret0
 */
	br	dispret0
/*
 * Process a receive-buffer-in command
 *
 * The format of an RBUFIN command is:
 *
 *	Byte		Contents
 *
 *	csr2	Command type
 *	csr3	Line number
 *	csr4	Bits 7-0 of the descriptor address
 *	csr5	Bits 15-8 of the descriptor address
 *	csr6	Bits 17-16 of the descriptor address
 *	csr7	Unused
 */
rbufin:
/*
 * If the virtual process is idle or halted then error
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	1f
	mov	IDLE,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	br	cmderr
2:
/*
 * If the descriptor address is odd then go to rbuferr0
 */
	mov	csr4,brg
	br0	rbuferr0
/*
 * Get an unused rbt entry from the list of unused rbt entries.
 * If the list is empty then go to rbuferr0
 */
	REMOVEQ(urbq,r5,rbuferr0)
	mov	r5,-
	brz	rbuferr0
/*
 * If the rbt entry is marked as in-use then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r1
	dec	r1,-
	brc	rbuferr0
/*
 * Mark the receive-buffer-table entry in-use
 */
	mov	1,mem
/*
 * Copy the descriptor address to the receive-buffer-table entry
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
/*
 * Attach the rbt entry to the end of the empty-receive-buffer queue
 */
	APPENDQ(erfq,r5)
	SCALL(react);
/*
 * Go to dispret0
 */
	br	dispret0
/*
 * I_AM_OK signal (command) from driver
 */
okcmd:
	mov	okbuf,mar
	mov	%okbuf,%mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
	mov	DRVOK,brg
	or	brg,r14
	br	dispret0
/*
 * Driver request to pass a four-byte command from the top half of the
 * driver to the script
 */
cmdin:
	mov	flags,mar
	mov	mem,r0
	mov	HAVCMD,brg
	orn	brg,r0,-
	brz	cmderr
	or	brg,r0,mem
	mov	scrcmd,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
	SCALL(react);
	br	dispret0
/*
 * The 75-microsecond hardware timer has expired
 */
tickc:
/*
 * Reset the 75-microsecond timer
 */
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	mov	PCLK,brg
	or	brg,r0,nprx
/*
 * If a crash has occurred then return to the dispatcher
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	dispret0
/*
 * Increment the counter for the line which is currently active;
 * if no line is currently active then increment the idle-time counter.
 */
	mov	actline,mar
	mov	%actline,%mar
	mov	mem,r0
	brz	1f
	mov	NZERO,brg
	xor	brg,r0
	asl	r0
	mov	lcnt,brg
	add	brg,r0,mar
	br	2f
1:
	mov	icnt,mar
2:
	mov	mem,r0
	inc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem
/*
 * Decrement the low-order byte of the clock register.
 * If the result is non-negative then return to the dispatcher.
 */
	mov	%clock,%mar
	mov	clock,mar
	mov	mem,r0
	dec	r0,mem|mar++
	brc	dispret0
/*
 * Decrement the high-order byte of the clock register.
 * If the result is non-negative then return to the dispatcher.
 */
	mov	mem,r0
	dec	r0,mem
	brc	dispret0
/*
 * Reload the 100-millisecond timer
 */
	mov	clock,mar
	mov	75-1,mem|mar++
	mov	5-1,mem
/*
 * If OK checking isn't enabled then go to tickd
 */
	mov	OKCHK,brg
	orn	brg,r14,-
	brz	1f
	br	tickd
1:
/*
 * If the 3-second timer hasn't expired yet then go to tickd
 */
	mov	clock2,mar
	mov	mem,r0
	dec	r0,mem
	brc	tickd
/*
 * Reload the 3-second timer
 */
	mov	30,mem
/*
 * If DRVOK isn't set then error
 */
	mov	~DRVOK,brg
	or	brg,r14,-
	brz	1f
	br	drverr
1:
/*
 * Clear DRVOK and schedule an I_AM_OK report
 */
	and	brg,r14
	mov	OKOUT,brg
	or	brg,r14
	mov	REPORTG,brg
	or	brg,r14
tickd:
	mov	timeq,mar
	mov	%timeq,%mar
tickl:
	mov	mem,r12|%mar
	brz	dispret0
	LINECHK(tickerr)
/*
 * If the virtual process is halted or idle then go to ticklb
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	ticklb
	mov	IDLE,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	br	ticklb
2:
/*
 * If the timer is not running then go to ticklb
 */
	mov	tcount,mar
	mov	mem,r0
	brz	ticklb
/*
 * Decrement the timer.  If the result is non-negative then go to
 * ticklb
 */
	dec	r0,mem
	brc	ticklb
/*
 * Reactivate the line
 */
	SCALL(react);
ticklb:
/*
 * If an ERRCNT1 report is needed then schedule it
 */
	mov	r12,%mar
	mov	errcnt,mar
	mov	3,r3
1:
	mov	mem,r0|mar++
	dec	r0,-
	brc	1f
	dec	r3
	brc	1b
	br	2f
1:
	mov	ERRCNT1,brg
	mov	brg,r0
	SCALL(rptschdb);
2:
/*
 * If an ERRCNT2 report is needed then schedule it
 */
	mov	r12,%mar
	mov	errcnt+4,mar
	mov	3,r3
1:
	mov	mem,r0|mar++
	dec	r0,-
	brc	1f
	dec	r3
	brc	1b
	br	2f
1:
	mov	ERRCNT2,brg
	mov	brg,r0
	SCALL(rptschdb);
2:
tickle:
/*
 * Point to next queue entry
 */
	mov	timelink,mar
	mov	r12,%mar
/*
 * Go to tickl
 */
	br	tickl
/*
 * Invalid command received from the driver
 */
cmderr:
	mov	CMDERR,brg
	br	errterm0
cmderr3:
	mov	0152,brg
	br	crash0
cmderr2:
	mov	0154,brg
	br	crash0
cmderr1:
	mov	0155,brg
	br	crash0
/*
 * UNIBUS error while in segment zero
 */
buserr0:
	mov	BUSERR,brg
	br	errterm0
/*
 * Transmit-buffer error while in segment zero
 */
xbuferr0:
	mov	XBUFERR,brg
	br	errterm0
/*
 * Receive-buffer error while in segment zero
 */
rbuferr0:
	mov	RBUFERR,brg
	br	errterm0
/*
 * Driver sanity-check failure
 */
drverr1:
drverr:
	mov	0161,brg
	br	crash0
/*
 * Error scheduling a report
 */
rpterr:
	mov	0162,brg
	br	crash0
/*
 * Act called with invalid page number in r12
 */
acterr:
	mov	0166,brg
	br	crash0
/*
 * react called with invalid page number in r12
 */
reacterr:
	mov	0165,brg
	br	crash0
/*
 * Act called with state = IDLE
 */
acterr2:
	mov	0163,brg
	br	crash0
/*
 * errterm called with invalid page number in r12
 */
errerr:
	mov	0160,brg
	br	crash0
tickerr:
	mov	0157,brg
	br	crash0
/*
 * Disconnect (loss of modem-ready)
 */
hangup:
	mov	HANGUP,brg
	br	errterm0
/*
 * Error stop for debugging
 */
hang0:
	mov	brg,r15
	mov	HANG,brg
	br	crash0
/*
 * Bridge to error-termination code in segment one
 */
errterm0:
	mov	brg,r0
	br	errterm
errterm:
/*
 * If r12 does not contain a valid page number then go to
 * acterr
 */
	LINECHK(errerr);
/*
 * Save the reason for termination
 */
	mov	termcode,mar
	mov	r0,mem
/*
 * If the termination code is non-zero then
 * schedule an error termination report
 */
	dec	r0,-
	brz	1f
	mov ERROUT,brg
	mov	brg,r0
	SCALL(rptschd);
	mov	r12,%mar
1:
/*
 * If r12 does not contain a valid page number then go to
 * acterr
 */
	LINECHK(errerr);
/*
 * Set process state = HALTED
 */
	mov	state,mar
	mov	HALTED,mem
/*
 * Cancel any outstanding time-out
 */
	mov	clock1,mar
	mov	NIL,mem
/*
 * Disable the receiver and transmitter
 */
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	brg,brg>>
	mov	brg,brg>>
	mov	brg,brg>>
	mov	brg,brg>>
	mov	brg,lur1
	mov	0,brg
	mov	brg,lur4
/*
 * Return to dispatcher
 */
	br	dispret0
/*
 * Schedule a report
 */
rptschd0:
	mov	brg,r0
	SCALL(rptschd);
	br	dispret0
/*
 * Subroutine to put a given line in the report queue
 *
 * Entered with page number in r12 and report flag in r0
 */
rptschd:
/*
 * If r12 does not contain a valid page number then go to
 * rpterr
 */
	LINECHK(rpterr);
/*
 * Set the page register to the page for this line
 */
	mov	r12,%mar
/*
 * Or-in the flag for this report
 */
	mov	reports,mar
	or	mem,r0,mem
/*
 * Go to rptschdc
 */
	br	rptschdc
/*
 * Subroutine to put a given line in the report queue
 *
 * Entered with page number in r12 and report flag in r0
 *
 * This entry is used when the report flag is in the second byte of the
 * reports cell
 */
rptschdb:
/*
 * If r12 does not contain a valid page number then go to
 * rpterr
 */
	LINECHK(rpterr);
/*
 * Set the page register to the page for this line
 */
	mov	r12,%mar
/*
 * Or-in the flag for this report
 */
	mov	reports+1,mar
	or	mem,r0,mem
/*
 * Enter here from above (rptschd)
 */
rptschdc:
/*
 * If this line is already in the report-needed queue then go to
 * rptschde; otherwise set the flag which indicates that this line
 * is in the queue and put it there
 */
	mov	flags,mar
	mov	mem,r0
	mov	REPORTL,brg
	orn	brg,r0,-
	brz	rptschde
	or	brg,r0,mem
	mov	rptlink,mar
	mov	NIL,mem
	mov	%reportq,%mar
	mov	reportq,mar
	mov	mem,r0|mar++
	brz	1f
	mov	mem,r0
	mov	r12,mem
	mov	r0,%mar
	mov	rptlink,mar
	mov	r12,mem
	br	2f
1:
	mov	reportq,mar
	mov	r12,mem|mar++
	mov	r12,mem
2:
/*
 * Make sure the global report-needed flag is set
 */
	mov	REPORTG,brg
	or	brg,r14
/*
 * Branch to here if the line is already in the report-needed queue
 */
rptschde:
/*
 * If the line is currently active then set actline = NIL and save
 * the virtual-machine registers
 */
	mov	actline,mar
	mov	%actline,%mar
	addn	mem,r12,-
	brz	1f
	br	2f
1:
	mov	NIL,mem
	mov	r12,%mar
	mov	psave,mar
	mov	r6,mem|mar++
	mov	r7,mem|mar++
	mov	r8,mem|mar++
	mov	r9,mem|mar++
	mov	r10,mem|mar++
	mov	r11,mem|mar++
2:
/*
 * Return to caller
 */
	SRETURN;
/*
 * Global error termination
 */
crash0:
	mov	brg,r15
	br	crash
/*
 * Global error termination
 */
crash:
/*
 * Set the flag which indicate that a global error termination
 * has occurred
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	dispret0
	or	brg,r14
/*
 * Save scratch regs in global save area
 */
	mov	%regsave,%mar
	mov	regsave,mar
	mov	csr0,mem|mar++
	mov	csr1,mem|mar++
	mov	csr2,mem|mar++
	mov	csr3,mem|mar++
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem|mar++
	mov	lur0,mem|mar++
	mov	lur1,mem|mar++
	mov	lur2,mem|mar++
	mov	lur3,mem|mar++
	mov	lur4,mem|mar++
	mov	lur5,mem|mar++
	mov	lur6,mem|mar++
	mov	lur7,mem|mar++
	mov	r0,mem|mar++
	mov	r1,mem|mar++
	mov	r2,mem|mar++
	mov	r3,mem|mar++
	mov	r4,mem|mar++
	mov	r5,mem|mar++
	mov	r6,mem|mar++
	mov	r7,mem|mar++
	mov	r8,mem|mar++
	mov	r9,mem|mar++
	mov	r10,mem|mar++
	mov	r11,mem|mar++
	mov	r12,mem|mar++
	mov	r13,mem|mar++
	mov	r14,mem|mar++
	mov	r15,mem|mar++
	br	dispret0
/*
 * Go to top of dispatcher loop in main segment
 */
dispret0:
	SEGJMP(disp);
/*
 * Subroutine to reactivate a given line
 *
 * Entered with page number in r12
 */
react:
/*
 * If r12 does not contain a valid page number then go to
 * reacterr
 */
	LINECHK(reacterr);
/*
 * Set the page register to the page for this line
 */
	mov	r12,%mar
/*
 * If process state == READY then go to react2
 */
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0,-
	brz	react2
/*
 * Set process state = READY
 */
	mov	READY,mem
/*
 * Go to act
 */
	br	act
/*
 * The process is currently ready to run
 */
react2:
/*
 * Set the somebody-activated-me (SAM) bit in the page for this line
 */
	mov	flags,mar
	mov	mem,r0
	mov	SAM,brg
	or	brg,r0,mem
/*
 * Go to act2
 */
	br	act2
/*
 * Subroutine to activate a given line
 *
 * Entered with page number in r12
 */
act:
/*
 * If r12 does not contain a valid page number then go to
 * acterr
 */
	LINECHK(acterr);
/*
 * If the line is already active then go to act2
 */
	mov	actline,mar
	mov	%actline,%mar
	addn	mem,r12,-
	brz	act2
/*
 * Set the page register to the page for this line
 */
	mov	r12,%mar
/*
 * If process state != READY then error
 */
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0,-
	brz	1f
	mov	0167,brg
	br	crash0
1:
/*
 * If the line is already in the ready-to-run queue
 * then go to act2
 */
	mov	flags,mar
	mov	mem,r0
	mov	ISACT,brg
	orn	brg,r0,-
	brz	act2
/*
 * Mark the line as being in the ready-to-run queue
 * and put it there
 */
	or	brg,r0,mem
	mov	rdylink,mar
	mov	NIL,mem
	mov	%readyq,%mar
	mov	readyq,mar
	mov	mem,r0|mar++
	brz	1f
	mov	mem,r0
	mov	r12,mem
	mov	r0,%mar
	mov	rdylink,mar
	mov	r12,mem
	br	2f
1:
	mov	r12,mem
	mov	readyq,mar
	mov	r12,mem
2:
act2:
/*
 * Return to caller
 */
	SRETURN
/*
 * *** end of segment zero ***
 */
endseg0:
/*
 * Include the code for the remaining segments
 */
#include	"disp.s"
#include	"fetch.s"
#include	"report.s"
