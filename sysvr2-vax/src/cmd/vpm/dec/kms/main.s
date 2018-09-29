/*	main.s 1.11 of 4/14/83
	@(#)main.s	1.11
 */


/*
 * KMS VPM interpreter
 *
 * This version of the interpreter supports both BISYNC and HDLC.
 *
 * The desired protocol class is selected by defining BISYNC or
 *	HDLC on the "cc -E -I" line of vpmc.
 * If BISYNC is selected, then ascii-to-ebcdic and ebcdic-to-ascii
 *	tables may be included for use by etoa and atoe primitives
 *	by defining ATOETBL and/or ETOATBL, respectively, on
 *	the "cc -E -I" line of vpmc.
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
 *
 *	NOTE: when the BUSWAIT macro is used in HDLC primitives, the
 *		macro is placed immediately following the assembler
 *		instruction that sets the NRQ bit in the npr register.
 *		This results in readable, "safe" code.
 *	      when the BUSWAIT macro is used in BISYNC primitives, the
 *		macro is placed wherever the smallest time of waiting
 *		for completion of unibus transfer can be achieved.
 *		This results in "safe" code that is somewhat less read-
 *		able but which permits faster execution of primitives.
 */
#define BUSWAIT(X)\
0:\
	mov	npr,brg;\
	br0	0b;\
	mov	nprx,brg;\
	br0	X

/*
 * Macro to transfer between segments using jmp instruction
 */
#define SEGJMP(X)\
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
 * Macro to perform an npr transaction to write to either the modem
 * control status register or the modem line status register.
 * 
 * Usage: 	MODEMWRITE(internalregister,modemregister)
 *
 * where internalregister is the kmc internal register that
 * contains the data to be put in idl, and modemregister is
 * the modem control unit register to be written.  The
 * correct data page (the data page that contains the
 * unibus address of the modem control unit register, i.e.
 * %modemregister) must be loaded into %mar before this macro
 * is executed.
 */
#define MODEMWRITE(INTERNREG,_MODEMREG)\
	mov	INTERNREG,brg;\
	mov	brg,odl;\
	mov	0,brg;\
	mov	brg,odh;\
	mov	_MODEMREG,mar;\
	mov	mem,oal|mar++;\
	mov	mem,oah|mar++;\
	mov	VEC4,brg;\
	mov	nprx,r0;\
	and	brg,r0;\
	or	mem,r0;\
	mov	VEC4|(3<<2),brg;\
	and	brg,r0,nprx;\
	mov	OUT|NRQ,brg;\
	mov	brg,npr

/*
 * Macro to perform an npr transaction to read either the modem
 * control status register or the modem line status register.
 *
 * Usage:	MODEMREAD(modemregister)
 *
 * where modemregister is the modem control unit register to be
 * read. As in MODEMWRITE, %modemregister must be loaded into
 * %mar before this macro is executed.
 */
#define MODEMREAD(MODMREG)\
	mov	MODMREG,mar;\
	mov	mem,ial|mar++;\
	mov	mem,iah|mar++;\
	mov	mem,r0;\
	mov	NRQ,brg;\
	or	brg,r0,npr

/*
 * Macro to delete a process's element from the csneedq or dsrneedq.
 *
 * Usage:	REM_NEEDQ(nameofneedqueue,nameofneedlink)
 *
 * First put the contents of this process's NEEDLINK into r2.
 * Then determine if this queue element is pointed to by
 * another element in the queue or by the first byte of the
 * NEEDQ header.
 *
 * Note:  This macro assumes that r1 contains the page number of the
 *	  previous process on the designated queue.  Also, this macro 
 *	  usages r2 as a scratch register.
 */
#define REM_NEEDQ(NEEDQ,NEEDLINK)\
	mov	NEEDLINK,mar;\
	mov	mem,r2;\
	mov	NIL,mem;\
	mov 	%NEEDQ,brg|%mar;\
	addn	brg,r1,-;\
	brz	2f;\
	mov	r1,%mar;\
	mov	NEEDLINK,mar;\
	mov	r2,mem;\
	mov	%NEEDQ,%mar;\
	mov	NEEDQ+1,mar;\
	mov	mem,brg;\
	addn	brg,r12,-;\
	brz	1f;\
	br	4f;\
1:\
	mov	r1,mem;\
	br	4f;\
2:\
	mov	NEEDQ,mar;\
	mov	r2,mem|mar++;\
	addn	mem,r12,-;\
	brz	3f;\
	br	4f;\
3:\
	mov	NIL,mem;\
4:
/*
 * Macro to activate a virtual process -
 *	check to see if the process is already active
 *		if active, do nothing
 *		else set ISACT bit in flags register
 *		     put process in ready queue
 *		     set process state to READY
 */
#define ACT\
	mov	r12,%mar;\
	mov	flags,mar;\
	mov	mem,r0;\
	mov	ISACT,brg;\
	orn	brg,r0,-;\
	brz	2f;\
	or	brg,r0,mem;\
	mov	rdylink,mar;\
	mov	NIL,mem;\
	mov	%readyq,%mar;\
	mov	readyq,mar;\
	mov	mem,r0|mar++;\
	brz	1f;\
	mov	mem,r0;\
	mov	r12,mem;\
	mov	r0,%mar;\
	mov	rdylink,mar;\
	mov	r12,mem;\
	br	2f;\
1:\
	mov	r12,mem;\
	mov	readyq,mar;\
	mov	r12,mem;\
2:
/*
 * Define the number of transmit and receive buffers
 */
#define	NXB	2
#define	NRB	2
/*
 * Define command codes and report codes for communication with driver
 */
#include <sys/csikmc.h>
#include <sys/csierrs.h>
/*
 * Define an additional error code
 */
#define	MODMADDRERR	25
/*
 * Define bits in r14
 */
#define DSRNEED (1<<0)
#define	REPORTG	(1<<1)
#define	DRVOK	(1<<2)
#define	CRASHED	(1<<3)
#define CSNEED  (1<<4)
#define	OKCHK	(1<<5)
#define	BASEOUT	(1<<6)
#define	SIZE_ERR	(1<<7)
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
#define XSOMWAITING	7
#define RSOMWAITING	8
#define RAC (1<<2)
#define TAC (1<<6)
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
 * Define bits of mode control register (sr5)
 */
#define IDLEBIT (1<<3)
#define STRPSNC (1<<5)
#define CCPMODE (1<<6)
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
 * Define bits in modem line status register
 */
#define LIN_ENA	(1<<0)
#define DTR	(1<<1)
#define RS	(1<<2)
#define DSR	(1<<4)
#define CS	(1<<5)
/*
 * Define useful constants
 */
#define	NIL		0377
#define	NZERO		017
#define NUMSYNC		6
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
#define BDLENGTH	12
/*
 * Define the offsets for the relevant fields of a buffer descriptor
 */
#define BDBUFADR	0
#define BDSIZE	4
#define	BDPARAM	8
/*
 * Data definitions
 */
	.data
/*
 * Character translation tables (if needed) and global data
 *   are located in the low address pages of the data memory.
 */
	.org	0*256
#ifdef ATOETBL
/*
 * Table to convert 8-bit ASCII to EBCDIC
 */
atoetbl:
#include	ATOETBL
	.org	atoetbl + 256
#else
atoetbl:
#endif
#ifdef ETOATBL
/*
 * Table to convert EBCDIC to 8-bit ASCII
 */
etoatbl:
#include	ETOATBL
	.org	etoatbl + 256
#else
etoatbl:
#endif
/*
 * Register save area (for debugging)
 */
#define	regsave	0
reportq:  .byte	NIL,NIL
timeq:	.byte	NIL,NIL
readyq:	.byte	NIL,NIL
dsrneedq: .byte  NIL,NIL
csneedq: .byte   NIL,NIL
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
needclock:	.byte	0
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
modemcsr:	.byte	0,0,0
modemlsr:	.byte	0,0,0
xmt_cnt:	.byte	0,0
rcv_cnt:	.byte	0,0
size_err:	.byte	0,0
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
/*
 *  The following data items are replicated on the data page for each line
 */
	.data
	.org	15*256
/*
 * Last command received from the host computer
 */
#ifdef DEBUG
cmdsave: .byte	0,0,0,0,0,0,0,0
#endif
state:	.byte	0
flags:	.byte	0
flags2: .byte	0
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
 * Define bits in flags2 cell
 */
#define XSILOFULL	(1<<0)
#define RSILOFULL	(1<<1)
#define CSNEEDL		(1<<2)
#define DSRNEEDL	(1<<3)
/*
 * Links for global queues (readyq, reportq, timerq, etc.)
 */
rptlink:  .byte	0
timelink:  .byte	0
rdylink:  .byte	0
dsrneedlink:  .byte	0
csneedlink:  .byte	0
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
#define	STOPOUT	 (1<<2)
#define ERROUT  (1<<3)
#define PCDOUT	(1<<4)
#define	TRACEOUT  (1<<5)
#define	SNAPOUT	(1<<6)
#define	RPTOUT	(1<<7)
/*
 * The following flags are in the second byte
 */
#define CMDACK	(1<<0)
#define ERRCNT1	(1<<1)
#define ERRCNT2	(1<<2)
/*
 * Temp save for report parameter
 */
errcode: .byte	0
/*
 * Reason for script termination
 */
termcode: .byte	0
/*
 * Reason for other terminations
 */
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
rcvbuf:	.byte	0
/*
 * Current receive buffer used to record incoming frame
 */
rcvbuf1:	.byte	0
/*
 * Current transmit buffer
 */
xmtbuf:	.byte	0
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
urbq:	.byte	0,0
/*
 * Empty-receive-frame queue
 */
erfq:	.byte	0,0
/*
 * Receive frame ready queue (used by getrfrm)
 */
rbrq:	.byte	0,0
/*
 * Completed receive frame queue
 */
crfq:	.byte	0,0
/*
 * Unused transmit buffer table entry queue
 */
uxbq:	.byte	0,0
/*
 * Transmit-buffer-get queue (used by getxfrm)
 */
xbgq:	.byte	0,0
/*
 * Transmit-buffer-open queue
 */
xboq:	.byte	0,0
/*
 * Transmit-buffer-return queue (xbufout queue)
 */
xbrq:	.byte	0,0
#ifdef	BISYNC
/*
 * Address of source/destination for i/o instruction
 */
ioadr:	.byte	0
/*
 * Address of array to receive CRC accumulation
 */
crcsav:	.byte	0
/*
 * Single byte transmit silo
 */
xsilo:	.byte	0
/*
 * Single byte receive silo
 */
rsilo:	.byte	0
/*
 * Number of remaining bytes (after xeomi invoked) before shutting down
 * the transmitter
 */
lastbytes:	.byte	NIL
/* 
 * Number of remaining sync characters to be transmitted before data
 * bytes may be sent out
 */
synccnt:	.byte	0
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
	.byte	0,0,0,0,0,0,0,0
#endif
/*
 * Interpreter options
 */
vpmopts:  .byte  0,0
edata:
/*
 * Instruction text starts here
 */
	.text
/*
 * KMC initialization sequence--keep this at location zero
 */
	.org	0
seg0:
	mov	7,r7
main:
/*
 * Put line number in lur1
 */
	mov	NZERO,brg
	xor	brg,r7,brg
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
 * Put a dummy character into tbuf
 */
	mov	r2,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Repeat until all lines have been initialized
 */
	dec	r7
	brc	main
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
	mov	15,brg
	and	brg,r0
/*
 * If command type == 7 then go to basein;
 * else if command type == 15 then go to okcmd
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
	br	sizerr	
1:
	LINECHK(cmderr3)
#ifdef DEBUG
/*
 * Save the command info for debugging
 */
	mov	cmdsave,mar
	mov	csr0,mem|mar++
	mov	csr1,mem|mar++
	mov	csr2,mem|mar++
	mov	csr3,mem|mar++
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem|mar++
#endif
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
 * else if command type == 4 then go to cmdin
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
	brz	cmdin
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
	mov	r12,%mar
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
 * Initialize rcvbuf, rcvbuf1, xmtbuf, tcount, and clock1
 */
	mov	rcvbuf,mar
	mov	NIL,mem
	mov	rcvbuf1,mar
	mov	NIL,mem
	mov	xmtbuf,mar
	mov	NIL,mem
	mov	tcount,mar
	mov	NIL,mem
	mov	clock1,mar
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
#ifdef BISYNC
/*
 * Set the value of lastbytes to NIL.
 * lastbytes is a variable that insures that all data bytes residing in
 * the software transmit silos (xsilo and ioadr) and within the line unit
 * transmit data buffer at the time an xeom primitive is executed are
 * properly transmitted before the line unit transmitter is shut down
 */
	mov	r12,%mar
	mov	lastbytes,mar
	mov	NIL,mem
#endif
/* 
 * Get interpreter options - and store them for later use in r3
 *	if DSRMODE is set
 *	  then
 *		set process state = WAITDSR
 *		add an element for this process in dsrneedq
 *		set DSRNEEDL bit in flags2
 *		if modemcsr addr passed in basein command
 *		  then set DTR in modem control unit
 *		  else ERROR
 *	  else
 *		set process state = READY
 *		if modemcsr addr passed in basein command
 *		  then set DTR in modem control unit
 *		    #ifdef BISYNC
 *			set DTR in modem control unit
 *		    #else
 *			set DTR|RS in modem control unit
 *		    #endif
 *		  else
 *		    #ifdef HDLC
 *			branch ahead to lineunit code (the modemcsr addr need
 *			  not be supplied to hdlc interp. if DSRMODE not set)
 *		    #else
 *			ERROR (modemcsr addr must always be supplied to bisync
 *			  interp.)
 *		    #endif
 */
	mov	r12,%mar
	mov	vpmopts,mar
	mov	mem,r3
	mov	state,mar
	mov	DSRMODE,brg
	orn	brg,r3,-
	brz	1f
	mov	READY,mem
	br	setdtr
1:
	mov	WAITDSR,mem
/*
 * Put this virtual process in the dsrneedq
 */
	mov	dsrneedlink,mar
	mov	NIL,mem
	mov	%dsrneedq,%mar
	mov	dsrneedq+1,mar
	mov	mem,r0
	brz	1f
	mov	r12,mem
	mov	r0,%mar
	mov	dsrneedlink,mar
	mov	r12,mem
	br	2f
1:
	mov	dsrneedq,mar
	mov	r12,mem|mar++
	mov	r12,mem
2:
/*
 * Set DSRNEEDL bit in flags2
 */
	mov	r12,%mar
	mov	flags2,mar
	mov	mem,r0
	mov	DSRNEEDL,brg
	or	brg,r0,mem
setdtr:
/*
 * Check to see if modem control unit address was sent in the basein
 * command and
 *	branch ahead to line unit code if this interpreter is for
 *	  hdlc and the current line doesn't have DSRMODE option set
 *	go to error termination if this interpreter is for bisync OR
 *	  if this interpreter is for hdlc and the current line has
 *	  DSRMODE interpreter option set
 */
	mov	%modemcsr,%mar
	mov	modemcsr,mar
	mov	mem,r1|mar++
	mov	mem,r2|mar++
	dec	r1,-
	brc	1f
	dec	r2,-
	brc	1f
	mov	r12,%mar
#ifdef HDLC
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0
	brz	lineunit
#endif
	mov	startcode,mar
	mov	NODMKDEV,brg
	mov	brg,mem
	br	errterm0
1:
/*
 * Turn on data-terminal-ready:
 *	get the process's line number from the value in r12
 *	write the line number to modemcsr
 *	write DTR (and RS for HDLC) to modemlsr
 */
	BUSWAIT(buserr0)
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	%modemcsr,%mar
	MODEMWRITE(brg,modemcsr)
	BUSWAIT(buserr0)
#ifdef BISYNC
	mov	DTR|LIN_ENA,brg
#else
	mov	DTR|RS|LIN_ENA,brg
#endif
	MODEMWRITE(brg,modemlsr)
	mov	r12,%mar
lineunit:
/*
 * Now make appropriate line unit settings -
 * First, convert virtual process page number to "left-
 * 	shifted line number", which will be temporarily
 *	stored in r2.  This value, when loaded into lur1,
 *	associates the appropriate line with the current
 *	virtual process.
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
 * Set line parameters -- set protocol mode:
 *   Load 0 into the mode control register (SR5) for bit oriented protocol
 *   Load CCPMODE into SR5 for byte oriented protocol
 */
	mov	r2,brg
	mov	brg,lur1
	mov	SR5|WRITE,brg
	mov	brg,lur2
	NOP
#ifdef BISYNC
	mov	CCPMODE,brg
#else
	mov	0,brg
#endif
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
#ifdef BISYNC
	mov	MDMCLK,brg
	mov	brg,r0
#else
/*
 * If hardware loopback option is set (check options stored in r3)
 * then set internal loopback mode.
 */
	mov	TENA|RENA|MDMCLK,brg	/* MODEM clock */
	mov	brg,r0
	mov	HWLOOP,brg
	orn	brg,r3,-
	brz	1f
	br	2f
1:
	mov	TENA|RENA|INTCLK,brg	/* internal clock */
	mov	brg,r0
2:
#endif
/*
 * Set line parameters
 */
	mov	r2,brg
	mov	brg,lur1
	mov	r0,lur4
	NOP
/*
 * Clear flags
 */
	mov	ISACT|REPORTL,brg
	mov	brg,r0
	mov	flags,mar
	and	mem,r0,mem
/*
 * Clear flags2
 */
	mov	CSNEEDL|DSRNEEDL,brg
	mov	brg,r0
	mov	flags2,mar
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
	br	dispret0
/*
 * Process a BASEIN command from the driver
 */
basein:
	mov	OKCHK|DRVOK,brg
	or	brg,r14
	mov	REPORTG|BASEOUT,brg
	or	brg,r14
/*
 * For a KMS, the low order 16 bits of a modem control
 * status register may be passed to the KMS in csr4 and csr5.
 * If so, then the high order 2 bits of this unibus address are 
 * assumed to always be ones.
 * The address of the modem control line status register can be
 * calculated from that of the control status register.
 */
	mov	%modemcsr,%mar
	mov	modemcsr,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	3<<2,mem
	mov	modemlsr,mar
	mov	csr4,r0
	mov	2,brg
	add	brg,r0,mem|mar++
	mov	csr5,r1
	adc	r1,mem|mar++
	mov	3<<2,mem
/*
 * If the unibus address of a modem control unit has been passed in
 * csr4 and csr5 (i.e. if csr4 and cs5 do not contain zero values),
 * then clear the modem line status register
 */
	dec	r0,-
	brc	1f
	dec	r1,-
	brc	1f
	br	2f
1:
/*
 * First, wait for the previous unibus transfer (if any) to complete
 */
	BUSWAIT(buserr0)
	mov	0,brg
	mov	brg,odl
	mov	brg,odh
	mov	%modemlsr,%mar
	mov	modemlsr,mar
	mov	mem,oal|mar++
	mov	mem,oah|mar++
	mov	mem,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr
/*
 * Wait for the current unibus transfer to complete
 */
	BUSWAIT(buserr0)
/*
 * Clear modem control status register
 */
	mov	modemcsr,mar
	mov	mem,oal|mar++
	mov	mem,oah|mar++
	mov	mem,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr
/*
 * Go to dispret0
 */
2:
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
 * If the virtual process state == PAUSING then call react.
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
	mov	state,mar
	mov	mem,r0
	mov	PAUSING,brg
	addn	brg,r0,-
	brz	3f
	br	4f
3:
	SCALL(react);
/*
 * Go to dispret0
 */
4:
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
 * If the state of the virtual process == PAUSING then call react.
 */
	APPENDQ(erfq,r5)
	mov	state,mar
	mov	mem,r0
	mov	PAUSING,brg
	addn	brg,r0,-
	brz	3f
	br	4f
3:
	SCALL(react);
/*
 * Go to dispret0
 */
4:
	br	dispret0
/*
 * I_AM_OK signal (command) from driver
 */
okcmd:
/*
	mov	okbuf,mar
	mov	%okbuf,%mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
*/
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
/*
 * If the state of the virtual process == PAUSING, then reactivate it
 */
	mov	state,mar
	mov	mem,r0
	mov	PAUSING,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	SCALL(react);
2:
	br	dispret0
/*
 * The 75-microsecond hardware timer has expired
 */
tickc:
/*
 * Reset the 75-microsecond timer
 */
#ifdef BISYNC
	BUSWAIT(buserr0)
#endif
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
 * Reset rcv and xmt counters (used for detecting excessive line speed 
 * or to detect that line not connected to modem)
 */
	mov	%rcv_cnt,%mar
	mov	rcv_cnt,mar
	mov	0,brg
	mov	brg,mem|mar++
	mov	brg,mem
	mov	xmt_cnt,mar
	mov	brg,mem|mar++	
	mov	brg,mem
/*
 * Increment the counter for the line which is currently active;
 * if no line is currently active then increment the idle-time counter.
 */
	mov	actline,mar
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
 * Decrement the contents of the needclock register
 * If it has expired, then reset its value (time = 10ms)
 *     and set CSNEED (for bisync) and DSRNEED bits in r14 as
 *     necessary.
 */
	mov	%needclock,%mar
	mov	needclock,mar
	mov	mem,r0
	dec	r0,mem
	brc	clockreg
/*
 * Reset value in needclock
 */
	mov	133,mem
#ifdef BISYNC
/*
 * If the csneed queue is not empty then set CSNEED in r14
 */
	mov	csneedq,mar
	mov	mem,brg
	brz	1f
	mov	CSNEED,brg
	or	brg,r14
1:
#endif
clockreg:
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
 * If the dsrneed queue is not empty then set DSRNEED in r14
 */
	mov	dsrneedq,mar
	mov	mem,brg
	brz	1f
	mov	DSRNEED,brg
	or	brg,r14
1:
/*
 * If OK checking isn't enabled then go to tickd
 */
	mov	OKCHK,brg
	orn	brg,r14,-
	brz	1f
	br	tickd
1:
/*
 * If the 7-second timer hasn't expired yet then go to tickd
 */
	mov	clock2,mar
	mov	mem,r0
	dec	r0,mem
	brc	tickd
/*
 * Reload the 8-second timer
 */
	mov	80,mem
/*
 * If DRVOK isn't set then error
 */
	mov	~DRVOK,brg
	or	brg,r14,-
	brz	1f
	br	drverr
1:
/*
 * Clear DRVOK
 */
	and	brg,r14
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
 * If the state of the virtual process == PAUSING then reactivate it
 */
	mov	state,mar
	mov	mem,r0
	mov	PAUSING,brg
	addn	brg,r0,-
	brz	1f
	br 	ticklb
1:
	SCALL(react);
	mov	r12,%mar
ticklb:
/*
 * This code reactivates a virtual process if it has a timeout
 * that expires now.
 * If there is not a timeout in progress then go to ticklc
 */
	mov	clock1,mar
	mov	mem,r0
	brz	ticklc
/*
 * Decrement timeout timer; if result is greater than or equal to zero
 * then go to ticklc
 */
	dec	r0,mem
	brc	ticklc
/*
 * Restore saved stack pointer and program counter:
 * if the virtual process is the currently  active process, then restore
 * stack pointer and program counter into r8 - r11, otherwise store
 * them into the appropriate psave registers.
 */
	mov	actline,mar
	mov	%actline,%mar
	addn	mem,r12,-
	brz	1f
/*
 * This code executed if the virtual process is not the currently active
 * process.
 */
	mov	r12,%mar
	mov	toutsave,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2|mar++
	mov	mem,r3
	mov	psave,mar
	mov	1,mem|mar++
	mov	0,mem|mar++
	mov	r2,mem|mar++
	mov	r3,mem|mar++
	mov	r0,mem|mar++
	mov	r1,mem
/*
 * Reactivate the line
 */
	SCALL(react);
	mov	r12,%mar
	br	ticklc
1:
/* This code executed if the virtual process is the currently active
 * process
 */
	mov	r12,%mar
	mov	toutsave,mar
	mov	mem,r10|mar++
	mov	mem,r11|mar++
	mov	mem,r8|mar++
	mov	mem,r9|mar++
	mov	1,brg
	mov	brg,r6
ticklc:
#ifdef HDLC
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
#endif
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
 * Script too large to allow this particular process to run
 */
sizerr:
/*
 * Set the SIZE_ERR bit in r14, then store the device number, and the
 * command type in size_err and size_err+1, respectively.
 */
	mov	SIZE_ERR,brg
	or	brg,r14
	mov	%size_err,%mar
	mov	size_err,mar
	mov	csr6,mem|mar++
	mov	csr2,r0
	mov	15,brg
	and	brg,r0,mem
	br	dispret0
/*
 * UNIBUS error while in segment zero
 */
buserr0:
#ifdef BISYNC
	mov	~NEM,brg
	mov	nprx,r0
	and	brg,r0,nprx
#endif
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
	mov	0162,brg
	br	crash0
drverr:
	mov	0161,brg
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
 * The following two pieces of code are commented out because
 * they are no longer accessed.
 *
 *
 * Disconnect (loss of modem-ready)
 *
hangup:
	mov	HANGUP,brg
	br	errterm0
 *
 * Error stop for debugging
 *
hang0:
	mov	brg,r15
	mov	HANG,brg
	br	crash0
 */
/*
 * Bridge to error-termination code in segment one
 */
errterm0:
	mov	brg,r0
	br	errterm
/*
 * Terminate the virtual process
 */
errterm:
/*
 * If r12 does not contain a valid page number then go to
 * errerr
 */
	LINECHK(errerr);
/*
 * Save the reason for termination
 */
	mov	r12,%mar
	mov	termcode,mar
	mov	r0,mem
/*
 * If the termination code is non-zero then
 * schedule an error termination report
 */
	dec	r0,-
	brz	1f
	mov	ERROUT,brg
	mov	brg,r0
	SCALL(rptschd);
	mov	r12,%mar
1:
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
 * If the address of a modem control unit was passed to
 * the interpreter with the basein command, then
 * turn off data-terminal-ready on the modem control unit
 */
	mov	%modemcsr,%mar
	mov	modemcsr,mar
	mov	mem,r1|mar++
	mov	mem,r2
	dec	r1,-
	brc	1f
	dec	r2,-
	brc	1f
	br	2f
1:
	mov	r12,%mar
	BUSWAIT(buserr0)
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	%modemcsr,%mar
	MODEMWRITE(brg,modemcsr)
	BUSWAIT(buserr0)
	mov	0,brg
	MODEMWRITE(brg,modemlsr)
2:
#ifdef BISYNC
/*
 * If the virtual process has an element on the
 * csneed queue, then
 * 	reset the CSNEEDL bit in flags2
 *	remove the element from the csneed queue
 */
	mov	r12,%mar
	mov	flags2,mar
	mov	~CSNEEDL,brg
	mov	mem,r0
	or	brg,r0,-
	brz	csclean1
	br	dsrcln0
csclean1:
	and	brg,r0,mem
	mov	%csneedq,brg|%mar
	mov	brg,r1
	mov	csneedq,mar
csclean2:
	mov	mem,brg|%mar
	addn	brg,r12,-
	brz	csclean3
	mov	brg,r1
	mov	csneedlink,mar
	br	csclean2
csclean3:
	REM_NEEDQ(csneedq,csneedlink);
#endif
/*
 * If the virtual process has an element on the
 * dsrneed queue, then
 *	reset the DSRNEEDL bit in flags2
 *	remove the element from the dsrneed queue
 */
dsrcln0:
	mov	r12,%mar
	mov	flags2,mar
	mov	~DSRNEEDL,brg
	mov	mem,r0
	or	brg,r0,-
	brz	dsrcln1
	br	cleaned
dsrcln1:
	and	brg,r0,mem
	mov	%dsrneedq,brg|%mar
	mov	brg,r1
	mov	dsrneedq,mar
dsrcln2:
	mov	mem,brg|%mar
	addn	brg,r12,-
	brz	dsrcln3
	mov	brg,r1
	mov	dsrneedlink,mar
	br	dsrcln2
dsrcln3:
	REM_NEEDQ(dsrneedq,dsrneedlink);
cleaned:
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
/*
	mov	(6<<4),brg
	mov	brg,lur1
	mov	(SR0|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR1|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR2|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR3|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR4|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR5|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR6|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
	mov	(SR7|READ),brg
	mov	brg,lur2
	NOP
	mov	lur0,mem|mar++
*/
	br	dispret0
/*
 * Go to top of dispatcher loop in main segment
 */
dispret0:
	SEGJMP(disp);
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
