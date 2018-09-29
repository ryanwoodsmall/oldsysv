/* @(#)main.s	1.1 */
/*
 * This is the KMC-11 portion of the UTS driver for a DZ-11
 * with KMC assist.
 *
 * Permanently assigned registers:
 *
 *	r15 - 100-microsecond timer/DZ-11 commutator
 *	r14 - dispatcher flag bits
 *	r13 - current state of the dialog with the PDP-11
 *	r12 - pointer to list of available queue entries
 *	r11 - unused
 *	r10 - address of current line-table entry
 *	r9 - unused
 *	r8 - DZ number currently being serviced
 *	r7 - entry number of current line-table entry
 *
 * The structure of a line-table entry is as follows:
 *
 *  Byte 	Contents
 *
 *   0 - current state of this line
 *
 *   1 - high-order byte of the current output word
 *
 *   2 - saved character to be sent after the current character
 *
 *   3 - flags
 *
 *   4 - More flags (overflow from flags)
 *
 *   5 - count for time delay, etc.
 *
 *  6 - column pointer for tab and delay calculations
 *
 *  7 - pointer to 4-byte queue entry which contains the
 *	 address and byte count of the output buffer
 *
 *  8 - address of temp storage area for input
 *
 *  9 - offset in above circular buffer of first character
 *
 *  10 - number of characters in circular buffer
 *
 *  11 - initial value for ITIME
 *
 *  12 - timer from last character input
 *
 *  13 - low byte of c_iflag
 *
 *  14 - high byte of c_iflag
 *
 *  15 - low byte of c_oflag
 *
 *  16 - high byte of c_oflag (delay bits)
 *
 *  17 - next (express) character to be output (used with SNDEXP)
 *
 *  18 - pointer to queue entry containing address and count of input buffer
 *
 *  19 - input completion code storage area
 *
 *
 * The flag bits in byte 7 are used as follows:
 *
 *  Bit		Meaning
 *
 * 1<<1 - The high-order data byte has not been sent
 * 1<<4 - There is a byte in SAVECHAR to be sent
 * 1<<0 - The current newline character has already been expanded
 *
 * Define useful constants
 */
#define	NIL	0377
#define	EOT	004
#define	NUL	0
#define	CEOT	004
#define	NL	012
#define	CR	015
#define	CSTART	021
#define	CSTOP	023
#define	SP	040
#define BS	010
#define HT	011
#define	ESCAPE	0134
#define	DEL	0177
#define	CQUIT	034
#define	CINTR	0177
/*
 * Define report types
 */
#define	XBUFOUT	0
#define	RBUFOUT	1
#define	XPRSOUT	2
#define	RBRK	3
#define	COCHG	4
#define	ERROUT	5
/*
 * Define the byte offset for the fields of a line-table entry
 *
 * Note:  Must have STATE addr = 0
 *
 */
#define	STATE	0
#define	HIBYTE	1
#define	SAVECHAR  2
#define	FLAGS	3
#define	FLAGS1  4
#define	COUNT	5
#define	ICOL	6
#define	OUTBUF	7
#define	ICIRC	8
#define	IFIRST	9
#define	NICIRC	10
#define	INITIM	11
#define	ITIME  12
#define IFLAG1	13
#define IFLAG2	14
#define OFLAG	15
#define DELAYS	16
#define EXPCHR 17
#define IBUFF 18
#define ICCODE 19

/*
 * STATE == 0	no output in progress
 * STATE == 1	sending normal data
 * STATE == 2	sending character that needs time fill
 * STATE == 3	expanding horizontal tabs
 * STATE == 4	sending fill characters
 * STATE == 5	twait state (Timer running)
 */
/*
 * Define bits in FLAGS 
 */
#define	NLEXP		(1<<0)
#define	SENDHIGH	(1<<1)
#define	IDRAIN		(1<<2)
#define	PREVESC		(1<<3)
#define	SENDSAVE	(1<<4)
#define	IPERR		(1<<5)
#define	CARR_ON		(1<<6)
/*
 * Define bits in FLAGS1 (overflow from FLAGS)
 */
#define	TTSTOP	(1<<0)
#define TIMACT	(1<<2)
#define PREVNB	(1<<5)
#define SNDEXP	(1<<6)
#define OUTBRK	(1<<7)
/*
 * Define the bits in IFLAG1
 */
#define	IGNBRK	(1<<0)
#define	BRKINT	(1<<1)
#define	IGNPAR	(1<<2)
#define	PARMRK	(1<<3)
#define	INPCK	(1<<4)
#define	ISTRIP	(1<<5)
#define	INLCR	(1<<6)
#define	IGNCR	(1<<7)
/*
 * Define bits in IFLAG2
 */
#define	ICRNL	(1<<0)
#define	IUCLC	(1<<1)
#define	IXON	(1<<2)
#define	IXANY	(1<<3)
/*
 * Define the bits in OFLAGS
 */
#define	OPOST	(1<<0)
#define	OLCUC	(1<<1)
#define	ONLCR	(1<<2)
#define	OCRNL	(1<<3)
#define	ONOCR	(1<<4)
#define	ONLRET	(1<<5)
#define	OFILL	(1<<6)
#define	OFDEL	(1<<7)
/*
 * Define the bits in DELAYS
 */
#define	NLDELAY	(1<<0)
#define	CRDELAY	(3<<1)
#define	HTDELAY	(3<<3)
#define	BSDELAY	(1<<5)
#define	VTDELAY	(1<<6)
#define	FFDELAY	(1<<7)
/*
 * Define the input completion code bits used in ICCODE
 */
#define	ICBRK	(1<<0)
#define	ICFLU	(1<<1)
#define	ICEIB	(1<<2)
#define	ICLOST	(1<<3)
/*
 * Define the output completion code bits
 */
#define OCFLU	(1<<0)
/*
 * Define the "BUSWAIT" macro
 */
#define BUSWAIT(X) 1:mov npr,brg;br0 1b;mov nprx,brg;br0 X;
/*
 * Macros to get and release queue entries
 *
 * r12 points to the list of available queue entries
 */
#define	FREEQ(X)\
	mov	03,brg;\
	and	brg,X,%mar;\
	mov	~03,brg;\
	and	brg,X,mar;\
	mov	r12,mem;\
	mov	X,brg;\
	mov	brg,r12

#define	GETQ(X,Y)\
	mov	r12,brg;\
	brz	Y;\
	mov	brg,X;\
	mov	03,brg;\
	and	brg,X,%mar;\
	mov	~03,brg;\
	and	brg,X,mar;\
	mov	mem,r12
/*
 * Subroutine CALL and RETURN macros
 *
 * Nested calls are not supported
 */
#define	CALL(X,R)\
	mov	%R,%mar;\
	mov	R,mar;\
	mov	%0f,mem|mar++;\
	mov	0f,mem;\
	mov	%X,brg;\
	mov	brg,pcr;\
	jmp	X;\
0:

#define	RETURN(R)	\
	mov	%R,%mar;\
	mov	R,mar;\
	mov	mem,pcr|mar++;\
	jmp	(mem)

#define	ERROR(X) \
	mov	X,brg; \
	mov	brg,r1; \
	CALL(erqueue,ret1);

#define HALT	br	.;

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
#define RQI (1<<7)
#define IEI (1<<0)
#define IEO (1<<4)
/*
 * Define bits in csr2
 */
#define RDYO (1<<7)
#define RDYI (1<<4)
/*
 * Define bits in r14
 */
#define	CHKOUTQ	(1<<0)
#define	CHKXPRQ	(1<<1)
#define	CHKBRKQ	(1<<2)
#define	CHKERRQ	(1<<3)
#define	CHKCSRQ	(1<<4)
#define	CHKCOCQ	(1<<5)
#define	CHKINRQ	(1<<6)
/*
 * Define error codes
 */
#define	BUSERR0	1
#define	BASEINERR  2
#define	ERRA	3
#define	ERRB	4
#define	ERRC	5
#define	BUSERR2	6
#define	OUTSTART  7
#define	FLUSH	8
#define	ERRE	9
#define	ERRF	10
#define	ERRG	11
#define	ERRH	12
#define	QUEUERR0	13
#define BUSERR1	14
#define BUSERR3	15
#define INVALCD	16
/*
 * Data definitions
 */
	.data
/*
 * The following data structures are replicated on a different
 * page for each DZ-11
 */
	.org	0
/*
 * Save room for the line-table entry address table
 * and the enable bits
 */
	.org	.+8
/*
 * Define the macro for generating line-table entries
 */
#define	LTENT	.org	.+24
/*
 * Now generate the actual line-table entries
 */
lte0:	LTENT
lte1:	LTENT
lte2:	LTENT
lte3:	LTENT
lte4:	LTENT
lte5:	LTENT
lte6:	LTENT
lte7:	LTENT
/*
 * Unibus base address for this DZ-11
 *
 * Bits 7-0 are in the first byte, bits 15-8 are in the second byte,
 * and bits 17-16 are in the third byte (right-adjusted)
 */
csrget:	.byte	0,0,0
/*
 * Cell used to save status info for each active DZ-11
 * (There is a different cell for each active DZ-11)
 *
 * The bits in the status cell are used as follows:
 *
 *	1<<0 - CSR check requested
 *	1<<1 - driver notification requested
 *	1<<2 - carrier-on change report requested
 *	1<<3 - dz overrun reportedier-on change report requested
 *	1<<5 - error has occurred
 *	1<<7 - base-in command received
 */
status:	.byte	0
/*
 * Carrier-on status bits from last carrier-on status check
 */
costatus:	.byte	0
/*
 * Error counter for DZ
 */
errcnt:	.byte	0
/*
 * kmc copy of the transmit-enable bits in the DZ-11
 */
actline:	.byte	0
/*
 * kmc copy of the break bits in the dz
 */
brkbits:	.byte	0
/*
 * 16.7 msec software timer
 */
clock1:	.byte	0
/*
 * 12.8 msec software timer--used to schedule modem status checks
 */
clock2:	.byte	0
/*
 * Link to next DZ in report-needed queue
 */
rptlink:	.byte	0
/*
 * Link to next DZ in csr-check-needed queue
 */
csrlink:	.byte	0
/*
 * Link to next DZ in carrier-on-change-report-needed queue
 */
coclink:	.byte	0
/*
 * Number used by the driver to identify this DZ
 * (The low-order three bits select one of the four possible
 * DZ's for this KMC)
 */
dznr:	.byte	0
/*
 * Set up the table of addresses of the line-table entries
 *
 * Each byte holds the address of one line-table entry
 *
 * This table must be at location zero of the page for each DZ
 */
	.org	12*256
dzst:
	.byte	lte0,lte1,lte2,lte3,lte4,lte5,lte6,lte7
/*
 * Ditto for DZ 1
 */
	.org	13*256
	.byte	lte0,lte1,lte2,lte3,lte4,lte5,lte6,lte7
/*
 * Ditto for DZ 2
 */
	.org	14*256
	.byte	lte0,lte1,lte2,lte3,lte4,lte5,lte6,lte7
/*
 * Ditto for DZ 3
 */
	.org	15*256
	.byte	lte0,lte1,lte2,lte3,lte4,lte5,lte6,lte7
/*
 * Global data structures
 */
	.org	0
/*
 * Last command received from the host for this DZ
 */
cmdsave:  .byte	0,0,0,0,0,0,0,0
rptsave:  .byte	0,0,0,0,0,0,0,0
/*
 * Masks to set or clear the transmit-enable bit for each line
 */
enblbits:	.byte	1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7
ret1:	.byte	0,0
retesc:	.byte	0,0
retdra:	.byte	0,0
retifl:	.byte	0,0
charsave:  .byte  0
/*
 * Input-ready queue
 */
headinrq:	.byte	NIL
tailinrq:	.byte	NIL
/*
 * Output-buffer-completion-report queue
 */
headoutq:	.byte	NIL
tailoutq:	.byte	NIL
/*
 * Express character output-buffer-completion-report queue
 */
headxprq:	.byte	NIL
tailxprq:	.byte	NIL
/*
 * Interrupt-request queue
 */
headbrkq:	.byte	NIL
tailbrkq:	.byte	NIL
/*
 * CSR-check-needed queue
 */
headcsrq:	.byte	NIL
tailcsrq:	.byte	NIL
/*
 * Error-report-needed queue
 */
headerrq:	.byte	NIL
tailerrq:	.byte	NIL
/*
 * Carrier-on-change-report-needed queue
 */
headcocq:	.byte	NIL
tailcocq:	.byte	NIL
/*
 * Empty-clist-block queue
 */
headecbq:	.byte	NIL
tailecbq:	.byte	NIL
/*
 * Queue entries
 */
inqueue:
/*
 * Instruction text starts here
 */
	.text
/*
 * Segment 0
 *
 * This is the main segment
 */
	.org	0
seg0:
/*
 * KMC initialization sequence--keep this at location zero
 */
	CALL(init,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	br	disp
/*
 * Tables of jump instructions
 *
 * The reason for putting the tables here is to avoid
 * splitting a table across a page boundary.
 */
dispsw:
	br	disp0
	br	disp1
	br	disp2
	br	disp3
	br	disp4
	ERROR(ERRC)
	br	disp
/*
 * Dispatcher loop--keep looking for something to do
 */
disp:
/*
 * If the 50-microsecond timer has expired then go to tick
 */
	mov	nprx,brg
	br4	1f
	br	2f
1:
	mov	%tick,brg
	mov	brg,pcr
	jmp	tick
2:
/*
 * Go to disp0, disp1, disp2, disp3, or disp4 depending on the
 * current state of the dialog with the PDP-11
 */
	mov	dispsw,brg
	br	(add,brg,r13),%dispsw
/*
 * Case 0 (interface inactive):
 *
 * If the driver has set RQI then go to rqiset
 */
disp0:
	mov	csr0,brg
	br7	rqiset
/*
 * If the input-ready queue is nonempty
 * then go to rbufout
 */
	mov	CHKINRQ,brg
	orn	brg,r14,-
	brz	rbufout
/*
 * If the carrier-on-change-report queue is nonempty
 * then go to cochange
 */
	mov	CHKCOCQ,brg
	orn	brg,r14,-
	brz	cochange
/*
 * If the break-request queue is nonempty
 * then go to rbrkrpt
 */
	mov	CHKBRKQ,brg
	orn	brg,r14,-
	brz	rbrkrpt
/*
 * If the output-buffer-completion-report queue is nonempty
 * then go to xbufout
 */
	mov	CHKOUTQ,brg
	orn	brg,r14,-
	brz	xbufout
/*
 * If the  express char oput-buffer-completion-report queue is nonempty
 * then go to xprout
 */
	mov	CHKXPRQ,brg
	orn	brg,r14,-
	brz	xprout
/*
 * If the error queue is nonempty then go to errout
 */
	mov	CHKERRQ,brg
	orn	brg,r14,-
	brz	errout

/*
 * Go to dispb
 */
	br	dispb
/*
 * The driver has requested the interface (by setting RQI)
 * in order to pass a command to the KMC.
 */
rqiset:
/*
 * Set RDYI
 */
	mov	csr2,r0
	mov	RDYI,brg
	or	brg,r0,csr2
/*
 * Set dialog state = 1
 */
	mov	1,brg
	mov	brg,r13
/*
 * Go to disp
 */
	br	disp
/*
 * Case 1 (RQI  received, RDYI sent):
 */
disp1:
/*
 * If the driver has cleared RDYI then go to command
 */
	mov	csr2,brg
	br4	1f
	br	command
1:
/*
 * If the driver has set IEI then go to ieiset
 */
	mov	csr0,brg
	br0	ieiset
/*
 * Go to dispb
 */
	br	dispb
/*
 * The driver has requested an interrupt (by setting IEI).
 * The driver has already set RQI and the KMC has responded
 * by setting RDYI.
 */
ieiset:
/*
 * Send interrupt 1 to the PDP-11
 */
	mov	BRQ,brg
	mov	brg,nprx
/*
 * Set dialog state = 2
 */
	mov	2,brg
	mov	brg,r13
/*
 * Go to disp
 */
	br	disp
/*
 * Case 2 (RQI and IEI received, RDYI and BRQ sent):
 */
disp2:
/*
 * If the driver has cleared RDYI and BRQ then go to command,
 * otherwise go to dispb
 */
	mov	csr2,brg
	br4	dispb
	mov	nprx,brg
	br7	dispb
	br	command
/*
 * Case 3 (RDYO has been set)
 */
disp3:
/*
 * If the driver has set IEO then go to report2, else if the
 * driver has cleared RDYO then set dialog state = 0 and
 * go to disp, otherwise go to dispb
 */
	mov	csr0,brg
	br4	report2
	mov	csr2,brg
	br7	dispb
	mov	0,brg
	mov	brg,r13
	br	disp
/*
 * Case 4 (RDYO, BRQ, and VEC4 have been set):
 */
disp4:
/*
 * If the driver has cleared RDYO and BRQ then set dialog state = 0
 * and go to disp, otherwise go to dispb
 */
	mov	csr2,brg
	br7	dispb
	mov	nprx,brg
	br7	dispb
	mov	0,brg
	mov	brg,r13
	br	disp
dispb:
/*
 * If a CSR check is needed then go to csrcheck
 */
	mov	CHKCSRQ,brg
	orn	brg,r14,-
	brz	1f
	br	disp
1:
	mov	%csrcheck,brg
	mov	brg,pcr
	jmp	csrcheck
/*
 * Report an error
 *
 * The format is:
 *
 * csr2 - report type
 * csr3 - dznr<<3|line nr
 * csr4 - error number
 * csr5-7 zero
 *
 */
errout:
/*
 * Get the address of the first queue entry on the  error queue
 */
	mov	headerrq,mar
	mov	%headerrq,%mar
	mov	mem,r4
	brz	errnon
/*
 * Save the link to the next queue entry
 */
	mov	3,brg
	and	brg,r4,%mar
	mov	~3,brg
	and	brg,r4,mar
/*
 * Get link to next entry
 */
	mov	mem,r5|mar++
/*
 * Set up csrs
 */
	mov	mem,csr3|mar++
	mov	mem,csr4|mar++
	mov	0,brg
	mov	brg,csr5
	mov	brg,csr6
	mov	brg,csr7
/*
 * Reset error bit in status for this dz
 */
	mov	mem,r8
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	status,mar
	mov	mem,r0
	mov	~(1<<5),brg
	and	brg,r0,mem
/*
 * Put this queue entry onto the front of the list of 
 * available queue entries
 */
	FREEQ(r4)
/*
 * Remove this queue entry from error queue
 */
	mov	headerrq,mar
	mov	%headerrq,%mar
	mov	r5,mem|mar++
/*
 * If the queue is empty then adjust the pointer to the end of
 * the queue and clear the CHKERRQ bit in r14
 */
	brz	1f
	br	2f
1:
	mov	r5,mem
	mov	~CHKERRQ,brg
	and	brg,r14
2:
/*
 * Put the report type in csr2
 */
	mov	ERROUT,brg
	mov	brg,csr2
/*
 * Go to reporte
 */
	br	reporte
errnon:
	mov	~CHKERRQ,brg
	and	brg,r14
	br	dispb
/*
 * Report an output buffer completion
 *
 * The XBUFOUT format is:
 *
 * csr2 - report type
 * csr3 - line number and dz identification number
 * csr4 - (bits 0-5)<<2 of the residual byte count
 * csr5 - bits 6-13 of the residual byte count
 * csr6 - completion code
 * csr7 - unused
 *
 * This code is entered from the dispatcher loop when the
 * report window is open and the CHKOUTQ
 * bit is set in r14.  At most one line is serviced on each pass
 * through the dispatcher loop.
 */
xbufout:
/*
 * Get the pointer to the queue entry at the head of the output-
 * ready queue.  If the queue is empty then go to xbufout
 */
	mov	headoutq,mar
	mov	%headoutq,%mar
	mov	mem,r2
	brz	xbnomo
/*
 * Get the line number and the DZ number
 * from the queue entry at the head of the output-ready queue
 */
	mov	~03,brg
	and	brg,r2,mar
	mov	03,brg
	and	brg,r2,%mar
	mov	mem,r3|mar++
	mov	mem,r7|mar++
	mov	mem,r8|mar++
	mov	mem,r4
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	r7,mar
	mov	mem,r10
/*
 * Remove from output list
 */
	mov	headoutq,mar
	mov	%headoutq,%mar
	mov	r3,mem|mar++
	brz	1f
	br	2f
1:
	mov	r3,mem
2:
/*
 * Set up the csrs
 */
	mov	XBUFOUT,brg
	mov	brg,csr2
/*
 * Copy the byte count, completion code to the csrs
 */
	mov	03,brg
	and	brg,r4,%mar
	mov	~03,brg
	and	brg,r4,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6
	mov	0,brg
	mov	brg,csr7
/*
 * Free the queue entries
 */
	FREEQ(r2)
	FREEQ(r4)
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get line number, dz number into csr3
 */
	mov	dznr,mar
	mov	mem,r0
	asl	r0
	asl	r0
	asl	r0
	mov	07,brg
	and	brg,r7,brg
	or	brg,r0
	mov	r0,csr3
	br	reporte
/*
 * No more data, turn off CHKOUTQ
 */
xbnomo:
	mov	~CHKOUTQ,brg
	and	brg,r14
	br	disp

/*
 * Report an express character output buffer completion
 *
 * The XPRSOUT format is:
 *
 * csr2 - report type
 * csr3 - line number and dz identification number
 * csr4 - unused
 * csr5 - unused
 * csr6 - unused
 * csr7 - unused
 *
 * This code is entered from the dispatcher loop when the
 * report window is open and the CHKXPRQ
 * bit is set in r14.  At most one line is serviced on each pass
 * through the dispatcher loop.
 */
xprout:
/*
 * Get the pointer to the queue entry at the head of the
 * express queue.  If the queue is empty then go to xpnomo
 */
	mov	headxprq,mar
	mov	%headxprq,%mar
	mov	mem,r2
	brz	xpnomo
/*
 * Get the line number and the DZ number
 * from the queue entry at the head of the express queue
 */
	mov	~03,brg
	and	brg,r2,mar
	mov	03,brg
	and	brg,r2,%mar
	mov	mem,r3|mar++
	mov	mem,r7|mar++
	mov	mem,r8
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	r7,mar
	mov	mem,r10
/*
 * Remove from list
 */
	mov	headxprq,mar
	mov	%headxprq,%mar
	mov	r3,mem|mar++
	brz	1f
	br	2f
1:
	mov	r3,mem
2:
/*
 * Set up the csrs
 */
	mov	XPRSOUT,brg
	mov	brg,csr2
/*
 * Free the queue entry
 */
	FREEQ(r2)
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get line number, dz number into csr3
 */
	mov	dznr,mar
	mov	mem,r0
	asl	r0
	asl	r0
	asl	r0
	mov	07,brg
	and	brg,r7,brg
	or	brg,r0
	mov	r0,csr3
	br	reporte
/*
 * No more data, turn off CHKXPRQ
 */
xpnomo:
	mov	~CHKXPRQ,brg
	and	brg,r14
	br	disp

/*
 * Report an input buffer completion
 *
 * The RBUFOUT format is:
 *
 * csr2 - report type
 * csr3 - line number and dz identification number
 * csr4 - (bits 0-5)<<2 of the residual byte count
 * csr5 - bits 6-13 of the residual byte count
 * csr6 - completion code
 * csr7 - unused
 *
 * This code is entered from the dispatcher loop when the
 * report window is open and the CHKINRQ
 * bit is set in r14.  At most one line is serviced on each pass
 * through the dispatcher loop.
 */
rbufout:
/*
 * Get the pointer to the queue entry at the head of the input-
 * ready queue.  If the queue is empty then go to rbufoutf
 */
	mov	headinrq,mar
	mov	%headinrq,%mar
	mov	mem,r2
	brz	rbnomo
/*
 * Get the line number and the DZ number
 * from the queue entry at the head of the input-ready queue
 */
	mov	~03,brg
	and	brg,r2,mar
	mov	03,brg
	and	brg,r2,%mar
	mov	mem,r3|mar++
	mov	mem,r7|mar++
	mov	mem,r8|mar++
	mov	mem,r4
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	r7,mar
	mov	mem,r10
/*
 * Remove from input list
 */
	mov	headinrq,mar
	mov	%headinrq,%mar
	mov	r3,mem|mar++
	brz	1f
	br	2f
1:
	mov	r3,mem
2:
/*
 * Set up the csrs
 */
	mov	RBUFOUT,brg
	mov	brg,csr2
/*
 * Copy the byte count, completion code to the csrs
 */
	mov	03,brg
	and	brg,r4,%mar
	mov	~03,brg
	and	brg,r4,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6
	mov	0,brg
	mov	brg,csr7
/*
 * Free the queue entries
 */
	FREEQ(r2)
	FREEQ(r4)
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get line number, dz number into csr3
 */
	mov	dznr,mar
	mov	mem,r0
	asl	r0
	asl	r0
	asl	r0
	mov	07,brg
	and	brg,r7,brg
	or	brg,r0
	mov	r0,csr3
	br	reporte
/*
 * No more data, turn off CHKINRQ
 */
rbnomo:
	mov	~CHKINRQ,brg
	and	brg,r14
	br	disp

/*
 * Fini
 */
/*
 * Report a break request (interrupt) (FS or DEL/break)
 *
 * The format is:
 *
 * csr2 - report type
 * csr3 - line number and DZ identification number
 */
rbrkrpt:
/*
 * Get the address of the first queue entry on the 
 * break-request-report-needed queue
 */
	mov	headbrkq,mar
	mov	%headbrkq,%mar
	mov	mem,r4
	brz	queuerr0
/*
 * Save the link to the next queue entry
 */
	mov	3,brg
	and	brg,r4,%mar
	mov	~3,brg
	and	brg,r4,mar
	mov	mem,r5|mar++
	mov	mem,brg|mar++
	mov	mem,csr4
	mov	7,r7
	and	brg,r7
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	3,r3
	and	brg,r3
	mov	%dzst,brg
	add	brg,r3,%mar
	mov	dznr,mar
	mov	mem,r0
	asl	r0
	asl	r0
	asl	r0,brg
	or	brg,r7,brg
	mov	brg,csr3
	mov	0,brg
	mov	brg,csr5
	mov	brg,csr6
	mov	brg,csr7
/*
 * Put this queue entry onto the front of the list of 
 * available queue entries
 */
	FREEQ(r4);
/*
 * Remove this queue entry from the break-report-
 * report-needed queue
 */
	mov	headbrkq,mar
	mov	%headbrkq,%mar
	mov	r5,mem|mar++
/*
 * If the queue is empty then adjust the pointer to the end of
 * the queue and clear the CHKBRKQ bit in r14
 */
	brz	1f
	br	2f
1:
	mov	r5,mem
	mov	~CHKBRKQ,brg
	and	brg,r14
2:
/*
 * Put the report type in csr2
 */
	mov	RBRK,brg
	mov	brg,csr2
/*
 * Go to reporte
 */
	br	reporte
/*
 * Report a carrier-on status change
 */
cochange:
/*
 * Get the address of the first queue entry on the 
 * carrier-on-change-report-needed queue
 */
	mov	headcocq,mar
	mov	%headcocq,%mar
	mov	mem,r8
	brz	1f
	br	2f
1:
	mov	~CHKCOCQ,brg
	and	brg,r14
	br	disp
/*
 * Remove the page for this DZ from the coc-report-needed queue
 */
2:
	mov	coclink,mar
	mov	mem,brg
	mov	%headcocq,%mar
	mov	headcocq,mar
	mov	brg,mem|mar++
	brz	1f
	br	2f
1:
	mov	brg,mem
	mov	~CHKCOCQ,brg
	and	brg,r14
2:
/*
 * Clear the bit which indicates that a carrier-on
 * status-change report has been scheduled for this DZ
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	status,mar
	mov	~(1<<0),brg
	mov	brg,r0
	and	mem,r0,mem
/*
 * Put the carrier-on status flags in csr4
 */
	mov	costatus,mar
	mov	mem,csr4
/*
 * Put the DZ identification in csr3
 */
	mov	dznr,mar
	mov	mem,csr3
/*
 * Put the report type in csr2
 */
	mov	COCHG,brg
	mov	brg,csr2
/*
 * Go to reporte
 */
	br	reporte
/*
 * Wrap-up of report processing
 */
reporte:
/*
 * Save command sent to host
 */
	mov	rptsave,mar
	mov	%rptsave,%mar
	mov	csr0,mem|mar++
	mov	csr1,mem|mar++
	mov	csr2,mem|mar++
	mov	csr3,mem|mar++
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
/*
 * Set RDYO
 */
	mov	csr2,r0
	mov	RDYO,brg
	or	brg,r0,csr2
/*
 * If IEO is set then go to report2
 */
	mov	csr0,brg
	br4	report2
/*
 * Set dialog state = 3
 */
	mov	3,brg
	mov	brg,r13
/*
 * Go to disp
 */
	br	disp
/*
 * Continuation of driver-report processing
 *
 * We come here when the driver sets IEO (in response to
 * our setting RDYO)
 */
report2:
/*
 * Send interrupt 2 to the PDP-11
 */
	mov	BRQ|VEC4,brg
	mov	brg,nprx
/*
 * Set dialog state = 4
 */
	mov	4,brg
	mov	brg,r13
/*
 * Go to disp
 */
	br	disp
/*
 * Process a command from the driver
 */
command:
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
 * Extract DZ number and set page register = DZ number.
 */
	mov	csr3,brg
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	3,r3
	and	brg,r3,brg
	mov	brg,r8
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get the command type from csr2
 */
	mov	csr2,r0
	mov	7,brg
	and	brg,r0
/*
 * If command type == 0 then go to xbufin
 * else if command type == 1 then go to sflag
 * else if command type == 2 then go to basein
 * else if command type == 3 then go to rbufin
 * else if command type == 4 then go to cmdin
 * else go to disp
 */
	dec	r0
	brz	xbufin
	dec	r0
	brz	sflag
	dec	r0
	brz	basein
	dec	r0
	brz	rbufin
	dec	r0
	brz	cmdin
	br	disp
/*
 * Process a BASEIN command
 *
 * The format of a BASEIN command is:
 *
 *	csr3 - DZ number<<3
 *	csr4 - low byte of DZ-11 unibus address
 *	csr5 - high byte of DZ-11 unibus address
 *	csr6 - bits 17-16 of DZ-11 unibus address
 */
basein:
/*
 * If this DZ is already active then go to basinerr
 * otherwise mark this DZ as active.
 */
	mov	status,mar
	mov	mem,brg
	br7	basinerr
	mov	1<<7,mem
/*
 * Record the DZ number
 */
	mov	csr3,brg
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	7,r7
	mov	dznr,mar
	and	brg,r7,mem
/*
 * Record the unibus address for this DZ-11
 */
	mov	csrget,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem
	mov	3,r3
	and	brg,r3,mem
	mov	3,mem	/* *** temp for testing *** */
/*
 * Go to disp
 */
	br	disp
/*
 * Process a buffer-send command
 *
 * The format of a buffer-send command is:
 *
 *	csr3 - ((DZ number)<<3)|(line number)
 *	csr4 - low byte of buffer address
 *	csr5 - high byte of buffer address
 *	csr6 - bitts 0-5 of byte count<<2|bits 16-17 of address
 *	csr7 - bits 6-13 of byte count
 */
xbufin:
/*
 * If a base-in command has not been received yet for this DZ
 * then report error
 */
	mov	status,mar
	mov	mem,brg
	br7	1f
	ERROR(ERRE)
	br	disp
1:
/*
 * Extract the line number and look up the address of the
 * line-table entry
 */
	mov	7,r7
	mov	csr3,brg
	and	brg,r7,r7|mar
	mov	mem,r10|mar
/*
 * Get an unused queue entry
 */
	GETQ(r4,queuerr0)
/*
 * Copy the buffer address into the queue entry
 */
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
/*
 * Restore the page register
 */
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Put a pointer to the queue entry into the LTE
 */
	mov	OUTBUF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	1f
	ERROR(ERRF)
	br	disp
1:
	mov	r4,mem
/*
 * If the line is already active (state != 0) then go to disp
 */
	mov	STATE,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brc	disp
/*
 * Set state = 1
 */
	mov	1,mem
/*
 * Turn on the enable bit for this line
 */
	CALL(enable,ret1)
/*
 * Go to disp
 */
	br	disp
/*
 * Process a receive-buffer-in command
 *
 * The format of a receive-buffer-in command is:
 *
 * 	csr3 - dznr<<3|line nr
 *	csr4 - low byte of buffer address
 *	csr5 - high byte of buffer address
 *	csr6 - bits 0-5 of bytecount<<2|bits 16-17 of address
 *	csr7 - bits 6-13 of bytecount
 */
rbufin:
/*
 * If a base-in command has not been received yet for this DZ
 * then report error
 */
	mov	status,mar
	mov	mem,brg
	br7	1f
	ERROR(ERRE)
	br	disp
1:
/*
 * Get an unused queue entry
 */
	GETQ(r4,queuerr0)
/*
 * Copy the data into the queue entry
 */
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
/*
 * Extract the line number and look up the address of the
 * line-table entry
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	7,r7
	mov	csr3,brg
	and	brg,r7,r7|mar
	mov	mem,r10|mar
/*
 * Attach the queue entries to the LTE
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	1f
	ERROR(ERRG)
	br	disp
1:
	mov	r4,mem
/*
 * Call drain to move in data buffered so far
 */
	CALL(drain,retdra)
/*
 * Go to disp
 */
	br	disp
/*
 * Process an sflag command
 *
 * The format of an sflag command is:
 *
 *	csr3 - ((DZ number)<<3)|(line number)
 *	csr4 - new value for IFLAG1
 *	csr5 - new values for IFLAG2
 *	csr6 - new values for OFLAG
 *	csr7 - new values of DELAYS
 *
 */
sflag:
/*
 * If a base-in command has not been received yet for this DZ
 * then report error
 */
	mov	status,mar
	mov	mem,brg
	br7	1f
	ERROR(ERRA)
	br	disp
1:
/*
 * Extract the line number and look up the address of the
 * line-table entry
 */
	mov	7,r7
	mov	csr3,brg
	and	brg,r7,r7|mar
	mov	mem,r10|mar
/*
 * Copy the stty flags to the lte (four bytes)
 */
	mov	IFLAG1,brg
	add	brg,r10,mar
	mov	csr4,mem|mar++
	mov	csr5,mem|mar++
	mov	csr6,mem|mar++
	mov	csr7,mem
/*
 * Go to disp
 */
	br	disp
/*
 * Process an cmdin command
 *
 * The format of an cmdin command is:
 *
 *	csr3 - ((DZ number)<<3)|(line number)
 *	csr4 - command byte as described below
 *	csr5 - unused
 *	csr6 - character to be transmitted
 *	csr7 - unused
 *
 */

/*
 * The following bits describe the commands
 */
#define IFLUSH	(1<<0)	/* Flush input */
#define OFLUSH	(1<<1)	/* Flush output */
#define OSPND	(1<<2)	/* Suspend output */
#define ORSME	(1<<3)	/* Resume output */
#define SCHAR	(1<<4)	/* Send character in csr6 */
#define SETTMR	(1<<5)	/* Set ITIME to value in csr7 */
#define SBRK	(1<<6)	/* Send break */

cmdin:
/*
 * If a base-in command has not been received yet for this DZ
 * then report error
 */
	mov	status,mar
	mov	mem,brg
	br7	1f
	ERROR(ERRA)
	br	disp
1:
/*
 * Extract the line number and look up the address of the
 * line-table entry
 */
	mov	7,r7
	mov	csr3,brg
	and	brg,r7,r7|mar
	mov	mem,r10|mar
	mov	csr4,r4
/*
 * Process commands
 */
	mov	IFLUSH,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
	CALL(iflush,retifl)
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	csr4,r4
2:
	mov	OFLUSH,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
	CALL(oflush,ret1)
	CALL(ubreak,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	csr4,r4
2:
/*
 * Setup for next two commands
 */
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r0
/*
 * See if suspend
 */
	mov	OSPND,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
	mov	TTSTOP,brg
	or	brg,r0,mem
2:
	mov	ORSME,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
/*
 * See if line was stopped
 */
	mov	~TTSTOP,brg
	or	brg,r0,-
	brz	1f
	br	2f
/*
 * Restart line
 */
1:
	and	brg,r0,mem
	CALL(enable,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	csr4,r4
2:
/*
 * See if character is to be sent
 */
	mov	SCHAR,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
/*
 * Put character into LTE and set SNDEXP
 */
	mov	EXPCHR,brg
	add	brg,r10,mar
	mov	csr6,mem
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	SNDEXP,brg
	or	brg,r0,mem
	CALL(enable,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	csr4,r4
2:
/*
 * See if ITIME is to be loaded
 */
	mov	SETTMR,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
	mov	INITIM,brg
	add	brg,r10,mar
	mov	csr7,mem
2:
/*
 * See if break is to be sent
 */
	mov	SBRK,brg
	orn	brg,r4,-
	brz	1f
	br	2f
1:
/*
 * Send a break
 */
	CALL(break,ret1)
2:
/*
 * Return to dispatcher
 */
	br	disp
/*
 * The unibus transfer failed to complete within a reasonable time
 */
buserr0:
/*
 * Clear the non-existent memory bit
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO|NEM),brg
	and	brg,r0,nprx
	ERROR(BUSERR0)
	br	disp
basinerr:
	ERROR(BASEINERR)
	br	disp
/*
 * No queue entries available
 */
queuerr0:
	ERROR(QUEUERR0)
	HALT
/*
 * End of segment zero
 */
endseg0:
/*
 * Pick up code for other segments
 */
#include	"output.s"
#include	"input.s"
#include	"subs.s"
