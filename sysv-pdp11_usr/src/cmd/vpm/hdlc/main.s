/*	main.s 1.5 of 3/23/82
	@(#)main.s	1.5
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
 * Usage:	REMOVEQ(queue,reg,Z);
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
	mov	%X,%mar;\
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
	mov	%xbt,%mar;\
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
 * Define the number of transmit and receive buffers
 */
#define	NXB	4
#define	NRB	4
/*
 * Define command codes and report codes for communication with driver
 */
#include "sys/csikmc.h"
#include "sys/csierrs.h"
/*
 * Define flags in r14
 */
#define	TOUT	(1<<0)
#define	REPORT	(1<<1)
#define	DRVOK	(1<<2)
#define	HAVCMD	(1<<3)
#define	HUPCLOSE	(1<<4)
#define	OKCHK	(1<<5)
#define	X25FLAG	(1<<6)
#define	XEOM	(1<<6)
#define	XLAST	(1<<7)
#define	HAVCTL	(1<<7)
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
 * Define the states of a virtual process (values in r12)
 */
#define	IDLE	0
#define	READY	1
#define	WAITDSR	2
#define	HALTED	3
#define	INWAIT	4
#define	OUTWAIT	5
#define	XENDWAIT	6
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
/*
 * Define bits in out control register (lur1)
 */
#define OCLRP (1<<7)
#define OUTRDY (1<<4)
#define TEOM (1<<1)
#define TSOM (1<<0)
/*
 * Define bits in In control register (lur2)
 */
#define ICLRP (1<<7)
#define ALTLOOP (1<<5)
#define INRDY (1<<4)
/*
 * Define bits in modem control register (lur3)
 */
#define RING (1<<7)
#define DTR (1<<6)
#define HDX (1<<4)
#define MDMRDY (1<<3)
/*
 * Define useful constants
 */
#define	NIL		0377
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
 * Virtual-process instruction space starts here
 */
	.data
	.org	0
#include	"opdef.h"
#include	CFILE
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
 * Interpreter data starts here
 */
	.data
	.org	14*256
/*
 * Last command received from the host computer
 */
cmdsave: .byte	0,0,0,0,0,0,0,0
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
#define OKOUT 	(1<<0)

#define	STARTOUT  (1<<1)
#define STOPOUT		(1<<2)
#define ERROUT		(1<<3)
#define TRACEOUT	(1<<4)
#define SNAPOUT		(1<<5)
#define	RPTOUT	(1<<6)
#define CMDACK		(1<<7)
/*
 * The following flags are in the second byte
 */
#define ERRCNT1		(1<<0)
#define BASEOUT		(1<<1)
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
device: .byte	0
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
 * Number of 75-microsecond ticks until the current 100-millisecond
 * timer interval expires
 */
clock:	.byte	0,0
/*
 * Number of 100-millisecond intervals until next timeout occurs
 */
clock1:	.byte	0
/*
 * Number of 100-millisecond intervals until next driver-sanity check
 */
clock2:	.byte	0
/*
 * Saved info from last time-out request
 */
toutsave: .byte	0,0,0,0
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
#define xbparam (xmtdesc+BDPARAM)
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
#define rbparam (rcvdesc+BDPARAM)
/*
 * Saved byte from last two-byte fetch
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
 * Number of received characters in current buffer
 */
rnrc:	.byte	0,0
/*
 * Saved byte for next two-byte store into receive buffer
 */
rsave:	.byte	0
/*
 * Error counters
 */
#define	NERCNT	4
errcnt:	.org	.+NERCNT
#define	nerr0	(errcnt+0)
#define	nerr1	(errcnt+1)
#define	nerr2	(errcnt+2)
#define	nerr3	(errcnt+3)
errset: .byte 0
/*
 * Save area for subroutine CALL and RETURN macros
 */
return: .byte	0,0
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
/*
 * Interpreter options
 */
vpmopts:  .byte  0,0
#ifdef	BISYNC
/*
 * Address of source/destination for i/o instruction
 */
ioadr:	.byte	0
/*
 * Address of array to receive crc accumulation
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
#define	xbpptr	xctlptr
/*
 * Pointer to next byte in receive-buffer parameter area
 */
rbpptr:	.byte	0
/*
 * Number of transmit-control bytes to be sent
 */
nxctl:	.byte	0
/*
 * Number of bytes of receive-control info received
 */
nrctl:	.byte	0
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
 * Transmit CRC accumulation
 */
xmtcrc:	.byte	0,0
/*
 * Receive CRC accumulation
 */
rcvcrc:	.byte	0,0
#endif
/*
 * Virtual process d-space starts here
 */
	.data
	.org	15*256
/*
 * Register save area (for debugging)
 */
regsave:	.org	.+16
regs:
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
	mov	brg,r12
	mov	brg,lur3
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
 * Save command info for debugging
 */
	mov	cmdsave,mar
	mov	%cmdsave,%mar
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
 * else if command type == 4 then go to okcmd
 * else if command type == 5 then go to cmdin
 * else error
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
	brz	okcmd
	dec	r0
	brz	cmdin
	dec	r0
	brz	cmderr
	dec	r0
	brz	basein
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
 *	csr4-csr7 unused
 */
run:
	mov	startcode,mar
	mov	%startcode,%mar
	mov	0,mem
/*
 * If the state of the virtual process is anything except
 * halted or idle then error
 */
	mov	HALTED,brg
	addn	brg,r12,-
	brz	1f
	mov	IDLE,brg
	addn	brg,r12,-
	brz	1f
	mov	CMDERR,mem
	mov	STARTOUT,brg
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
	br	cmderr
1:
/* make sure line # is 0 */
	mov	csr3,brg
	mov	brg,r0
	dec	r0
	brz	1f
	mov	NOLINE,mem
	mov	STARTOUT,brg
	mov 	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
	br	linerr
1:
/*
 * If the script is too large then error
 */
	mov	%end,brg
	mov	brg,r0
	mov	%cmdsave,brg
	sub	brg,r0,-
	brc	1f
	br	2f
1:	mov	SIZERR,mem
	mov	STARTOUT,brg
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
	br	sizerr
2:
/*
 * Save device number
 */
	mov	device,mar
	mov	%device,%mar
	mov	csr6,mem
/*
 * Save interpreter options
 */
	mov	vpmopts,mar
	mov	%vpmopts,%mar
	mov	csr4,mem|mar++
	mov	csr5,mem
/*
 * Initialize queue headers
 */
	mov	urbq,mar
	mov	%urbq,%mar
	mov	xbrq-urbq+2-1,brg
	mov	brg,r0
1:
	mov	NIL,mem|mar++
	dec	r0
	brc	1b
/*
 * Initialize the receive-buffer-table:
 *
 *	%mar=%rbt;
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
	mov	%rbt,%mar
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
 *	%mar=%xbt;
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
	mov	%xbt,%mar
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
	mov	0,mem
	mov	rcvstate,mar
	mov	0,mem
#endif
/*
 * Initialize error counters
 */
	mov	errcnt,mar
	mov	NERCNT,brg
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
	mov	%regs,%mar
1:
	mov	0,mem
	inc	r0,r0|mar
	brc	1f
	br	1b
1:
/*
 * Initialize the program counter
 */
	mov	0,brg
	mov	brg,r8
	mov	brg,r9
/*
 * Initialize the stack pointer
 */
	mov	SSTACK,brg
	mov	brg,r10
#ifdef	BISYNC
/*
 * Turn on data-terminal-ready and set receive mode = half-duplex.
 */
	mov	DTR|HDX,brg
	mov	brg,lur3
#endif
#ifdef	HDLC
/*
 * Clear the output silo
 */
	mov	OCLRP,brg
	mov	brg,lur1
/*
 * Clear the input silo
 */
	mov	ICLRP,brg
	mov	brg,lur2
/*
 * Wait for the dust to settle
 */
	mov	0,brg
	mov	0,brg
	mov	0,brg
	mov	0,brg
	mov	0,brg
	mov	0,brg
	mov	0,brg
/*
 * Get interpreter options
 */
	mov	vpmopts,mar
	mov	%vpmopts,%mar
	mov	mem,r1
/*
 * If hardware loopback option is set then set internal loopback mode
 */
	mov	0,r0
	mov	HWLOOP,brg
	orn	brg,r1,-
	brz	1f
	br	2f
1:
	mov	ALTLOOP,brg
	or	brg,r0
2:
	mov	r0,lur2
/*
 * Select HDLC mode (clear DDCMP mode)
 */
	mov	0,brg
	mov	brg,lur7
/*
 * Turn on data-terminal-ready
 */
	mov	DTR,brg
	mov	brg,lur3
#endif
/*
 * Set process state = READY
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Clear the AC
 */
	mov	0,brg
	mov	brg,r6
/*
 * Clear flags in r14
 */
	mov	REPORT|OKCHK|DRVOK,brg
	and	brg,r14
/*
 * If X25 packet-header-separation mode was requested then set the
 * corresponding bit in flags
 */
	mov	csr4,brg
	br0	1f
	br	2f
1:
	mov	X25FLAG,brg
	or	brg,r14
2:
/*
 * Schedule a STARTUP report
 */
	mov	STARTOUT,brg
	br	rschd0a
/*
 * Process a BASEIN command from the driver
 */
basein:
	mov	OKCHK|DRVOK,brg
	or	brg,r14
	mov	BASEOUT,brg
	mov	reports+1,mar
	mov	%reports,%mar
	mov 	mem,r0
	or 	brg,r0,mem
	mov 	REPORT,brg
	or	brg,r14
	br	dispret0
/*
 * Process a HALT command from the driver
 */
halt:
	mov	stopcode,mar
	mov	%stopcode,%mar
	mov	HALTRCVD,mem		
/*
 * If the virtual process is already halted then go to dispret0
 */
	mov	HALTED,brg
	addn	brg,r12,-
	brz	1f
	br	2f
1:
	mov	CMDERR,mem
	mov	STOPOUT,brg
	br	rschd0a
2:
/*
 * If the virtual process has never been started then go to dispret0
 */
	dec	r12,-
	brz	1f
	br	2f
1:
	mov	CMDERR,mem
	mov	STOPOUT,brg
	br	rschd0a
2:
/*
 * Turn off data-terminal-ready
 */
	mov	0,brg
	mov	brg,lur3
/*
 * schedule a stopout report 
 */
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	mov	STOPOUT,brg
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
/*
 * Get the reason for termination
 */
	mov	HALTRCVD,brg
/*
 * Go to errterm0
 */
	br	errterm0
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
 * If the virtual process is idle or halted then error
 */
	mov	HALTED,brg
	addn	brg,r12,-
	brz	cmderr
	mov	IDLE,brg
	addn	brg,r12,-
	brz	cmderr
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
	mov	HALTED,brg
	addn	brg,r12,-
	brz	cmderr
	mov	IDLE,brg
	addn	brg,r12,-
	brz	cmderr
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
/*
 * Go to dispret0
 */
	br	dispret0
/*
 * I_AM_OK signal (command) from driver
 */
okcmd:
	mov	DRVOK,brg
	or	brg,r14
	br	dispret0
/*
 * Driver request to pass a four-byte command from the top half of the
 * driver to the script
 */
cmdin:
	mov	HAVCMD,brg
	orn	brg,r14,-
	brz	cmderr
	or	brg,r14
	mov	scrcmd,mar
	mov	%scrcmd,%mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
	br	dispret0
/*
 * The 75-microsecond hardware timer has expired
 */
tick:
/*
 * Reset the 75-microsecond timer
 */
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	mov	PCLK,brg
	or	brg,r0,nprx
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
 * If OK checking isn't enabled then go to tick2
 */
	mov	OKCHK,brg
	orn	brg,r14,-
	brz	1f
	br	tick2
1:
/*
 * If the 3-second timer hasn't expired yet then go to tick2
 */
	mov	clock2,mar
	mov	mem,r0
	dec	r0,mem
	brc	tick2
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
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	mov	OKOUT,brg
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
tick2:
/*
 * If the process is halted or idle then go to dispret0
 */
	dec	r12,-
	brz	dispret0
	mov	HALTED,brg
	addn	brg,r12,-
	brz	dispret0
/* generate a error counter report if needed */
	mov	errset,mar
	mov	%errset,%mar
	mov	mem,r0
	dec	r0
	brz	1f
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0|mar++
	mov	mem,r0
	mov	ERRCNT1,brg
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
1:
/*
 * If check-modem-ready is set then check to see if we still have modem-ready
 *
 *	if(r14&HUPCLOSE)
 *		if((lur3&MDMRDY)==0)
 *			goto hangup;
 */
	mov	HUPCLOSE,brg
	orn	brg,r14,-
	brz	1f
	br	2f
1:
	mov	lur3,r0
	mov	MDMRDY,brg
	orn	brg,r0,-
	brz	2f
	br	hangup
2:
/*
 * If the timer is running then decrement the counter
 */
	mov	%tcount,%mar
	mov	tcount,mar
	mov	mem,r0
	brz	1f
	dec	r0,mem
1:
/*
 * If no timeout in progress then go to disp
 */
	mov	TOUT,brg
	orn	brg,r14,-
	brz	1f
	br	dispret0
1:
/*
 * Decrement the timeout counter.
 * If the result is non-negative then go to disp.
 */
	mov	clock1,mar
	mov	mem,r0
	dec	r0,mem
	brc	dispret0
/*
 * Clear the timeout-in-progress flag
 */
	mov	~TOUT,brg
	and	brg,r14
/*
 * Set process state = READY.  (It may have been waiting on i/o.)
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Set AC = 1
 */
	mov	1,brg
	mov	brg,r6
/*
 * Point to save area
 */
	mov	toutsave,mar
/*
 * Restore the stack pointer and frame pointer
 */
	mov	mem,r10|mar++
	mov	mem,r11|mar++
/*
 * Get the saved jump address
 */
	mov	mem,r0|mar++
	mov	mem,r1|mar++
/*
 * Go to jump
 */
	SEGJMP(jump);
/*
 * Continuation of rtnxbuf primitive--entered from fetch.s
 */
rtnxbufc:
/* set the unibus request to update the octets in host  */
	mov	xbparam,mar
	mov	mem,odl|mar++
	mov	mem,odh
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	BDPARAM,brg
	add	brg,r0,brg
	mov	brg,oal
	adc	r1,brg
	mov	brg,oah
	adc	r2,brg
	mov	3,r3
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr

	BUSWAIT(buserr0)
	mov	xbparam+2,mar
	mov	mem,odh
	mov	mem,odl
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	BDPARAM+2,brg
	add	brg,r0,brg
	mov	brg,oal
	adc	r1,brg
	mov	brg,oah
	adc	r2,brg
	mov	3,r3
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	BYTE|OUT|NRQ,brg
	mov	brg,npr

	BUSWAIT(buserr0)
/*
 * Set the report-needed flag in r14
 */
	mov	REPORT,brg
	or	brg,r14
/*
 * Return to the dispatcher
 */
	br	dispret0
/*
 * Continuation of rtnrbuf primitive--entered from fetch.s
 */
rtnrbufc:
/*
 * Get the count of stored characters
 * If the count is even then go to rtnrbuf2
 */
	mov	rnrc,mar
	mov	mem,brg
	br0	1f
	br	rtnrbuf2
1:
/*
 * Set up a unibus request to move one byte to the host computer
 */
	mov	rsave,mar
	mov	mem,odl
	mov	rcvcp,mar
	mov	mem,oal|mar++
	mov	mem,oah|mar++
	mov	3,r3
	and	mem,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	BYTE|OUT|NRQ,brg
	mov	brg,npr
/*
 * Wait for the unibus transfer to complete
 */
	BUSWAIT(buserr0)
/*
 * Branch to here from above if count is even
 */
rtnrbuf2:
/*
 * Copy count to buffer descriptor
 *
 * Set up a unibus request to update the COUNT parameter
 * in the host computer copy of the buffer descriptor
 */
	mov	rnrc,mar
	mov	mem,odl|mar++
	mov	mem,odh
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov 	BDSIZE,brg
	add 	brg,r0,oal
	adc	r1,brg
	mov 	brg,oah
	adc	r2,brg
	mov	3,r3
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr
/*
 * Wait for the unibus transfer to complete
 */
	BUSWAIT(buserr0)
/*
 * Update the octet field (three bytes) in the host copy
 * of the buffer descriptor
 */
	mov	rbparam,mar
	mov	mem,odl|mar++
	mov	mem,odh
	mov	DESC,brg
	add	brg,r5,mar
	
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	BDPARAM,brg
	add	brg,r0,brg
	mov	brg,oal
	adc	r1,brg
	mov	brg,oah
	adc	r2,brg
	mov	3,r3
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr

	BUSWAIT(buserr0)
	mov	rbparam+2,mar
	mov	mem,odh
	mov	mem,odl
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	BDPARAM+2,brg
	add	brg,r0,brg
	mov	brg,oal
	adc	r1,brg
	mov	brg,oah
	adc	r2,brg
	mov	3,r3
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	asl	r3
	asl	r3,brg
	or	brg,r0,nprx
	mov	BYTE|OUT|NRQ,brg
	mov	brg,npr

	BUSWAIT(buserr0)
/*
 * Set the report-needed flag in r14
 */
	mov	REPORT,brg
	or	brg,r14
/*
 * Return to the dispatcher
 */
	br	dispret0
/*
 * Invalid command received from the driver
 */
cmderr:
	mov	CMDERR,brg
	br	errterm0
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
drverr:
	mov	DRVERR,brg
	br	errterm0
/*
 * Disconnect (loss of modem-ready)
 */
hangup:
	mov	HANGUP,brg
	br	errterm0
/*
 * Script too large
 */
sizerr:
	mov	SIZERR,brg
	br	errterm0
/*
 * Error stop for debugging
 */
hang0:
	mov	brg,r15
	mov	HANG,brg
	br	errterm0
/* line has incorrect #  */
linerr:
	mov	NOLINE,brg
	br	errterm0
/*
 * Bridge to termination code in segment one
 */
errterm0:
	/* save reason for termination */
	mov	termcode,mar
	mov	%termcode,%mar
	mov	brg,mem
/*
 * If termination code is non-zero then schedule an error-termination
 * report
 */
	mov	mem,r0
	dec	r0
	brz	1f
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	mov	ERROUT,brg
	or	brg,r0,mem
1:
	/* jump to errterm in seg #1 */
	SEGJMP(errterm);
/*
 * Schedule a report
 */
rschd0a:
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	or	brg,r0,mem
	mov	REPORT,brg
	or	brg,r14
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
