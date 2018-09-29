/*	disp.s 1.6 of 3/23/82
	@(#)disp.s	1.6
 */

/*
 * VPM interpreter
 *
 * This version of the interpreter supports both BISYNC and HDLC.
 *
 * The desired protocol class is selected by the #define at the beginning
 * of main.s
 *
 * Segment one
 *
 * This segment contains the main dispatcher loop
 *
 * Entered from segment zero via label disp
 */
	.org	1024
seg1:
/*
 * Jump table used for branch on dialog state
 */
dispsw:
	br	disp0
	br	disp1
	br	disp2
	br	disp3
	br	disp4
edispsw:
#define	LDISPSW	edispsw-dispsw
/*
 * Dispatcher loop--keep looking for something to do
 */
disp:
/*
 * If the 75-microsecond timer has expired then go to tick
 */
	mov	nprx,brg
	br4	tick
/*
 * Go to disp0, disp1, disp2, disp3, or disp4 depending on the
 * current state of the dialog with the host computer
 */
	mov	LDISPSW,brg
	sub	brg,r13,-
	brc	staterr
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
 * If there is something to report to the driver then go to report
 */
	mov	REPORTG,brg
	orn	brg,r14,-
	brz	report
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
 * Send interrupt 1 to the host computer
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
 * If the driver has set IEO then go to ieoset, else if the
 * driver has cleared RDYO then set dialog state = 0 and
 * go to disp; otherwise go to dispb
 */
	mov	csr0,brg
	br4	ieoset
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
 * and go to disp; otherwise go to dispb
 */
	mov	csr2,brg
	br7	dispb
	mov	nprx,brg
	br7	dispb
	mov	0,brg
	mov	brg,r13
	br	disp
/*
 * Continuation of driver-report processing
 *
 * We come here when the driver sets IEO (in response to
 * our setting RDYO)
 */
ieoset:
/*
 * Send interrupt 2 to the host computer
 */
	mov	BRQ|VEC4,brg
	mov	brg,nprx
/*
 * Set dialog state = 4
 */
	mov	4,brg
	mov	brg,r13
/*
 * Return to the dispatcher
 */
	br	disp
tick:
	SEGJMP(tickc);
/*
 * Continuation of dispatcher loop
 */
dispb:
/*
 * If a crash has occurred then go to disp
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	disp
/*
 * If any line is requesting an output character then go to dispxmt
 */
	mov	lur6,r0
	dec	r0,-
	brc	dispxmt
/*
 * If any line has input data available then go to disprcv
 */
	mov	lur7,r0
	dec	r0,-
	brc	disprcv
/*
 * Set the page register for the line which is currently active;
 * if there is not one then go to dispc
 */
	mov	actline,mar
	mov	%actline,%mar
	mov	mem,r12|%mar
	brz	dispc
/*
 * Go to the instruction fetch/decode/interpret
 * sequence in segment two
 */
	SEGJMP(fetch);
dispc:
/*
 * Get the page number for the line at the head of the ready-to-run queue.
 * If the queue is empty then go to dispe
 */
	mov	%readyq,%mar
	mov	readyq,mar
	mov	mem,r12|%mar
	brz	dispe
/*
 * Remove the line from the queue
 */
	mov	rdylink,mar
	mov	mem,r0
	mov	%readyq,%mar
	mov	readyq,mar
	mov	r0,mem|mar++
	brz	1f
	br	2f
1:
	mov	r0,mem
2:
/*
 * If r12 does not contain a valid page number then go to
 * acterr1
 */
	LINECHK(acterr1);
/*
 * Clear the bit that says that this line is in the ready-to-run
 * queue
 */
	mov	r12,%mar
	mov	flags,mar
	mov	mem,r0
	mov	~ISACT,brg
	and	brg,r0,mem
/*
 * If there is a report in progress for this line then go to disp
 * (The line will be reactivated when the line is removed
 * from the report queue.)
 */
	mov	REPORTL,brg
	orn	brg,r0,-
	brz	disp
/*
 * If the process state is not READY then return to the dispatcher
 * (It may have been halted because of an error condition.)
 */
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0,-
	brz	1f
	br	disp
1:
/*
 * Restore saved VPM registers (AC, PC, SP, etc.)
 */
	mov	psave,mar
	mov	mem,r6|mar++
	mov	mem,r7|mar++
	mov	mem,r8|mar++
	mov	mem,r9|mar++
	mov	mem,r10|mar++
	mov	mem,r11
/*
 * Record the page number for later use by the dispatcher
 */
	mov	actline,mar
	mov	%actline,%mar
	mov	r12,mem
/*
 * Return to the dispatcher
 */
	br	disp
dispe:
	mov	iticks,mar
	mov	%iticks,%mar
	mov	mem,r0
	inc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem
	br	disp
/*
 * Bridge to command-processing code in segment one
 */
command:
	SEGJMP(commandc);
#ifdef	HDLC
/*
 * Transmitter dispatcher: put a character into tbuf
 */
dispxmt:
/*
 * Find the line number of the lowest-numbered line that is
 * ready for an output character; if there isn't one then error
 */
	mov	7,r7
1:
	asl	r0
	brc	1f
	dec	r7
	brc	1b
	br	xmterr2
1:
	mov	7,brg
	xor	brg,r7
/*
 * Convert the line number to a page number and set the page register
 * to the page for the selected line
 */
	mov	NZERO,brg
	xor	brg,r7,brg
	mov	brg,r12|%mar
/*
 * Position the line number for use later
 */
	asl	r7
	asl	r7
	asl	r7
	asl	r7
/*
 * Branch on transmit state
 */
	mov	xmtstate,mar
	mov	mem,r0
	dec	r0
	brz	xmtst0
	dec	r0
	brz	xmtst1
	dec	r0
	brz	xmtst2
	dec	r0
	brz	xmtst3
	dec	r0
	brz	xmtst4
	br	xmtsterr
/*
 * Transmit state 0:	Sending continuous flags
 */
xmtst0:
/*
 * Put a dummy character into tbuf
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Go to disp
 */
	br	disp
/*
 * Transmit state 1:	Sending flags to begin a new frame
 */
xmtst1:
/*
 * Decrement the flag count.
 * If the result is non-negative then go to xmtst1a
 */
	mov	flagcnt,mar
	mov	mem,r0
	dec	r0,mem
	brc	xmtst1a
/*
 * Clear TSOM
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR3|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Set transmit state = 2
 */
	mov	xmtstate,mar
	mov	2,mem
/*
 * Go to xmtst2
 */
	br	xmtst2
xmtst1a:
/*
 * Put a dummy character into tbuf
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Go to disp
 */
	br	disp
/*
 * Transmit state 2:  Sending the control portion of an i-frame
 */
xmtst2:
/*
 * Prepare to send a byte of control info
 */
	mov	xctlptr,mar
	mov	mem,r0
	inc	r0,mem
	mov	xmtac,brg
	add	brg,r0,mar
	mov	mem,r4
/*
 * If xctlptr < nxctl then go to xmtst2a
 */
	mov	xctlptr,mar
	mov	mem,r0
	mov	nxctl,mar
	sub	mem,r0,-
	brc	1f
	br	xmtst2a
1:
/*
 * If there is no transmit buffer
 * then go to xmtst3e
 */
	mov	xmtbuf,mar
	mov	mem,-
	brz	xmtst3e
/*
 * Set transmit state = 3
 */
	mov	xmtstate,mar
	mov	3,mem
xmtst2a:
/*
 * Send a byte of control info
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	r4,brg
	mov	brg,lur0
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Transmit state 3:	Sending the data portion of an i-frame
 */
xmtst3:
/*
 * If there is no transmit buffer then error
 */
	mov	xmtbuf,mar
	mov	mem,-
	brz	xmterr3
/*
 * Increment the count of transmitted characters
 */
	mov	xmtnpc,mar
	mov	mem,r0
	inc	r0,mem|mar++
	mov	mem,r1
	adc	r1,mem
/*
 * If X25FLAG is set then go to xmtst3b
 */
	mov	flags,mar
	mov	mem,r3
	mov	X25FLAG,brg
	orn	brg,r3,-
	brz	xmtst3b
/*
 * If the number of characters transmitted is odd then go to xmtst3a
 */
	inc	r0,brg
	br0	xmtst3a
/*
 * Prepare to send the saved byte from the previous fetch
 */
	mov	xmtsave,mar
	mov	mem,r4
/*
 * Go to xmtst3c
 */
	br	xmtst3c
/*
 * There is no saved byte from a previous fetch.  Therefore
 * we need to fetch two more characters from the host computer.
 */
xmtst3a:
/*
 * Set up a unibus request for the next two bytes
 */
	mov	xmtcp,mar
	mov	mem,ial|mar++
	mov	mem,iah|mar++
	mov	3,r3
	and	mem,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3,brg
	mov	brg,npr
/*
 * Wait for the unibus transfer to complete
 */
	BUSWAIT(buserr1)
/*
 * Increment the buffer pointer
 */
	mov	xmtcp,mar
	mov	2,r2
	add	mem,r2,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem
/*
 * Save the high-order byte
 */
	mov	xmtsave,mar
	mov	idh,mem
/*
 * Prepare to send the low-order byte
 */
	mov	idl,r4
xmtst3c:
/*
 * If there are no characters remaining after this one then go to xmtst3e
 */
	mov	xmtnpc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	xmtsize,mar
	sub	mem,r0,mar++
	subc	mem,r1,-
	brc	xmtst3e
/*
 * Put the byte into tbuf
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	r4,brg
	mov	brg,lur0
/*
 * Go to disp
 */
	br	disp
/*
 * X25 mode is set
 */
xmtst3b:
/*
 * If the number of characters transmitted is geq three
 * then go to xmtst3b2
 */
	dec	r1,-
	brc	xmtst3b2
	mov	3,brg
	sub	brg,r0,-
	brc	xmtst3b2
/*
 * Get next byte from packet header
 */
	mov	xbparam,brg
	add	brg,r0,mar
	mov	mem,r4
/*
 * Go to xmtst3c
 */
	br	xmtst3c
/*
 * X25 mode--packet header has already been sent
 */
xmtst3b2:
/*
 * If the number of characters transmitted is even then go to xmtst3a
 */
	mov	r0,brg
	br0	xmtst3a
/*
 * Prepare to send the saved byte from the previous fetch
 */
	mov	xmtsave,mar
	mov	mem,r4
/*
 * Go to xmtst3c
 */
	br	xmtst3c
/*
 * No characters remaining after this one--send last byte + CRC
 * and set transmit state = 4
 */
xmtst3e:
/*
 * Set TEOM
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR3|WRITE,brg
	mov	brg,lur2
	NOP
	mov	TEOM,brg
	mov	brg,lur0
/*
 * Put last byte into tbuf
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	r4,brg
	mov	brg,lur0
/*
 * Set transmit state = 4
 */
	mov	xmtstate,mar
	mov	4,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Transmit state 4:	Sending last byte + CRC
 */
xmtst4:
/*
 * Set TSOM
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR3|WRITE,brg
	mov	brg,lur2
	NOP
	mov	TSOM,brg
	mov	brg,lur0
/*
 * Put a dummy character into tbuf
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR2|WRITE,brg
	mov	brg,lur2
	NOP
	mov	0,brg
	mov	brg,lur0
/*
 * Set transmit state = 0
 */
	mov	xmtstate,mar
	mov	0,mem
	SCALL(react);
/*
 * Go to disp
 */
	br	disp
/*
 * Receiver dispatcher:  process an input character
 */
disprcv:
/*
 * Find the line number of the lowest-numbered line that has an
 * input character available; if there isn't one then error
 */
	mov	7,r7
	mov	1,r1
1:
	asl	r0
	brc	1f
	asl	r1
	dec	r7
	brc	1b
	mov	0113,brg
	br	crash1
1:
	mov	7,brg
	xor	brg,r7
/*
 * If the line requesting service is not currently active
 * then go to rcvdisp2
 */
	mov	actbits,mar
	mov	%actbits,%mar
	mov	mem,r0
	mov	r1,brg
	orn	brg,r0,-
	brz	1f
	br	disprcv2
1:
/*
 * Convert the line number to a page number and set the page register
 * to the page for the selected line
 */
	mov	NZERO,brg
	xor	brg,r7,brg
	mov	brg,r12|%mar
/*
 * Position the line number for later use
 */
	asl	r7
	asl	r7
	asl	r7
	asl	r7
/*
 * Get status bits for this line
 */
	mov	r7,brg
	mov	brg,lur1
	mov	lur3,brg
	mov	brg,r1
/*
 * If neither RDA nor RSA is set then halt,
 * else if RDA is not set then go to rcverr6
 */
	br0	1f
	br1	rcverr6
	mov	0111,brg
	br	crash1
1:
/*
 * Branch on receive state for this line
 */
	mov	rcvstate,mar
	mov	mem,r0
	dec	r0
	brz	rcvst0
	dec	r0
	brz	rcvst1
	dec	r0
	brz	rcvst2
	dec	r0
	brz	rcvst3
	br	rcvsterr
disprcv2:
	mov	actbits+1,mar
	mov	mem,r0
	orn	brg,r0,-
	brz	rcverr9
	or	brg,r0,mem
	br	disp
/*
 * Receive state 0:  Between frames
 */
rcvst0:
/*
 * Get control bits and input character
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR1|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r1
	mov	r7,brg
	mov	brg,lur1
	mov	SR0|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r4
/*
 * If receiver-overrrun is set then go to rcverr4
 */
	asl	r1,brg
	br4	rcverr4
/*
 * If received-abort-character is set then go to rcverr5
 */
	mov	RAB,brg
	orn	brg,r1,-
	brz	rcverr5
/*
 * If RSOM is not set then error
 */
	mov	r1,brg
	br0	1f
	mov	0112,brg
	br	crash1
1:
/*
 * If REOM is set then go to rcverr0
 */
	mov	r1,brg
	br1	rcverr0
/*
 * If the staging area still contains valid data
 * (because a control frame was received previously when no
 * receive buffer was available)
 * then go to rcverr1
 */
	mov	flags,mar
	mov	mem,r3
	mov	HAVCTL,brg
	orn	brg,r3,-
	brz	rcverr1
/*
 * Set receive state = 1
 */
	mov	rcvstate,mar
	mov	1,mem
/*
 * Save input character as first byte of control info
 */
	mov	rcvac,mar
	mov	r4,mem
/*
 * If ADRSWTCH is  set then switch address bit
 */
	mov	vpmopts,mar
	mov	mem,r2
	mov	ADRSWTCH,brg
	orn	brg,r2,-
	brz	1f
	br	2f
1:
	mov	rcvac,mar
	mov	mem,r0
	mov	1<<1,brg
	xor	brg,r0,mem
2:
/*
 * If there is already a receive buffer then error
 */
	mov	rcvbuf1,mar
	mov	mem,-
	brz	1f
	br	rbuferr1
1:
/*
 * Get an rbt entry from the queue of empty receive buffers.
 * If none is available then go to rcvst0b
 */
	REMOVEQ(erfq,r5,rcvst0b)
/*
 * If buffer state != 1 then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r0
	mov	1,brg
	addn	brg,r0,-
	brz	1f
	br	rbuferr1
1:
/*
 * Set buffer state = 2
 */
	mov	2,mem
/*
 * Save address of new receive buffer
 */
	mov	rcvbuf1,mar
	mov	r5,mem
/*
 * Get the descriptor address
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
/*
 * Get buffer address, count, type, station, and device from buffer descriptor
 *
 * The following loop fetches the buffer descriptor from the host machine
 */
	mov	BDLENGTH/2-1,brg
	mov	brg,r4
	mov	rcvdesc,mar
1:
	mov	r0,ial
	mov	r1,brg
	mov	brg,iah
	mov	r2,brg
	mov	3,r3
	and	brg,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3,brg
	mov	brg,npr
	BUSWAIT(buserr1)
	mov	idl,mem|mar++
	mov	idh,mem|mar++
	mov	2,brg
	add	brg,r0
	adc	r1
	adc	r2
	dec	r4
	brc	1b
rcvst0b:
/*
 * Go to disp
 */
	br	disp
/*
 * Receive state 1:  Receiving second byte of a frame
 */
rcvst1:
/*
 * Get control bits and input character
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR1|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r1
	mov	r7,brg
	mov	brg,lur1
	mov	SR0|READ,brg
	mov	brg,lur2
	NOP
	mov	rcvac+1,mar
	mov	lur0,mem
/*
 * Set number of received characters = 0
 */
	mov	rnrc,mar
	mov	0,mem|mar++
	mov	0,mem
/*
 * Set number of control characters = 2
 */
	mov	nrctl,mar
	mov	2,mem
/*
 * If receiver-overrrun is set then go to rcverr4
 */
	asl	r1,brg
	br4	rcverr4
/*
 * If received-abort-character is set then go to rcverr5
 */
	mov	RAB,brg
	orn	brg,r1,-
	brz	rcverr5
/*
 * If REOM is set then go to rcveom
 */
	mov	r1,brg
	br1	rcveom
/*
 * Set receive state = 2
 */
	mov	rcvstate,mar
	mov	2,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Receive state 2:  Receiving data portion of a frame
 */
rcvst2:
/*
 * Get control bits and input character
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR1|READ,brg
	mov	brg,lur2
	NOP
	mov	flagsave,mar
	mov	lur0,mem
	mov	mem,r1
	mov	r7,brg
	mov	brg,lur1
	mov	SR0|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r4
/*
 * If receiver-overrrun is set then go to rcverr4
 */
	asl	r1,brg
	br4	rcverr4
/*
 * If received-abort-character is set then go to rcverr5
 */
	mov	RAB,brg
	orn	brg,r1,-
	brz	rcverr5
/*
 * If nrctl >= 5 then go to rcvst2x
 */
	mov	nrctl,mar
	mov	mem,r2
	mov	5,brg
	sub	brg,r2,-
	brc	rcvst2x
/*
 * Increment nrctl
 */
	inc	r2,mem
/*
 * Stash character in rcvac
 */
	mov	rcvac,brg
	add	brg,r2,mar
	mov	r4,mem
/*
 * If X25FLAG is set then go to rcvst2b
 */
	mov	flags,mar
	mov	mem,r1
	mov	X25FLAG,brg
	orn	brg,r1,-
	brz	rcvst2b
rcvst2x:
/*
 * Increment the number of characters received
 */
	mov	rnrc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	inc	r0
	adc	r1
	mov	rnrc,mar
	mov	r0,mem|mar++
	mov	r1,mem
/*
 * If no buffer allocated then go to rcvst2b
 */
	mov	rcvbuf1,mar
	mov	mem,r5
	brz	rcvst2b
/*
 * If buffer size exceeded then go to rcvst2b
 */
	mov	rcvsize,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	rnrc,mar
	sub	mem,r0,mar++
	subc	mem,r1,-
	brc	1f
	br	rcvst2b
1:
/*
 * If the number of characters received is odd then go to rcvst2a
 */
	mov	rnrc,mar
	mov	mem,brg
	br0	rcvst2a
/*
 * Set up a unibus request to move two bytes to the host computer
 */
	mov	rsave,mar
	mov	mem,odl
	mov	r4,brg
	mov	brg,odh
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
	mov	OUT|NRQ,brg
	mov	brg,npr
/*
 * Wait for the unibus transfer to complete
 */
	BUSWAIT(buserr1)
/*
 * Increment the buffer address
 */
	mov	rcvcp,mar
	mov	2,r2
	add	mem,r2,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem
/*
 * Go to rcvst2b
 */
	br	rcvst2b
/*
 * Character count is odd:  save input character for transfer later
 */
rcvst2a:
/*
 * Save the input character for transfer to the host computer later
 */
	mov	rsave,mar
	mov	r4,mem
/*
 * Test for end of block
 */
rcvst2b:
/*
 * If REOM is set then go to rcveom
 */
	mov	flagsave,mar
	mov	mem,brg
	br1	rcveom
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Receive state 3:  Skipping a frame
 */
rcvst3:
/*
 * Get control bits and input character
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR1|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r1
	mov	r7,brg
	mov	brg,lur1
	mov	SR0|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r4
/*
 * If REOM is set then go to rcverre
 */
	mov	r1,brg
	br1	rcverre
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * End of block detected
 */
rcveom:
/*
 * If the CRC is incorrect then go to rcverr2
 */
	br7	rcverr2
/*
 * Get address of rbt entry for current receive buffer;
 * if no buffer allocated then go to rcveom2
 */
	mov	rcvbuf1,mar
	mov	mem,r5
	brz	rcveom2
/*
 * Indicate no current receive buffer
 */
	mov	NIL,mem
/*
 * If buffer size exceeded then error
 */
	mov	rcvsize,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	rnrc,mar
	sub	mem,r0,mar++
	subc	mem,r1,-
	brc	1f
	br	rcverr3
1:
/*
 * If the number of received characters is odd
 * then transfer the saved byte to the host buffer
 */
	mov	rnrc,mar
	mov	mem,brg
	br0	1f
	br	2f
1:
/*
 * Character count is even:  Transfer two bytes to the receive buffer
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
	mov	OUT|NRQ|BYTE,brg
	mov	brg,npr
	BUSWAIT(buserr1)
2:
/*
 * If X25FLAG is set then set rnrc += nrctl-2
 */
	mov	rnrc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	flags,mar
	mov	mem,r2
	mov	X25FLAG,brg
	orn	brg,r2,-
	brz	1f
	br	2f
1:
	mov	nrctl,mar
	add	mem,r0
	adc	r1
	mov	2,brg
	sub	brg,r0
	mov	0,brg
	subc	brg,r1
	mov	rnrc,mar
	mov	r0,mem|mar++
	mov	r1,mem
2:
/*
 * Copy byte count to rbt entry
 */
	mov	RCOUNT,brg
	add	brg,r5,mar
	mov	r0,mem|mar++
	mov	r1,mem
/*
 * Copy control info to rbt entry
 */
	mov	rcvac,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2|mar++
	mov	mem,r3|mar++
	mov	mem,r4
	mov	RCTL,brg
	add	brg,r5,mar
	mov	r0,mem|mar++
	mov	r1,mem|mar++
	mov	r2,mem|mar++
	mov	r3,mem|mar++
	mov	r4,mem
/*
 * Attach the rbt entry to the end of the completed-receive-frame
 * queue
 */
	APPENDQ(crfq,r5)
/*
 * Set buffer state = 3
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	3,mem
/*
 * Go to rcveom4
 */
	br	rcveom4
/*
 * No receive buffer available--set flag to indicate that the staging area
 * contains valid data
 */
rcveom2:
	mov	flags,mar
	mov	mem,r0
	mov	HAVCTL,brg
	or	brg,r0,mem
rcveom4:
/*
 * Set receive state = 0
 */
	mov	rcvstate,mar
	mov	0,mem
	SCALL(react);
/*
 * Go to disp
 */
	br	disp
/*
 * Frame too short (less than four bytes including CRC)
 */
rcverr0:
/*
 * Increment error counter
 */
	COUNTUP(nerr0);
/*
 * Go to rcverre
 */
	br	rcverre
/*
 * No room in staging area
 */
rcverr1:
/*
 * Increment error counter
 */
	COUNTUP(nerr1);
/*
 * Go to rcverrg
 */
	br	rcverrg
/*
 * CRC error
 */
rcverr2:
/*
 * Increment error counter
 */
	COUNTUP(nerr2);
/*
 * Go to rcverre
 */
	br	rcverre
/*
 * Frame too long (buffer size exceeded)
 */
rcverr3:
/*
 * Increment error counter
 */
	COUNTUP(nerr3);
/*
 * Go to rcverre
 */
	br	rcverre
/*
 * Receiver overrun
 */
rcverr4:
/*
 * Increment error counter
 */
	COUNTUP(nerr4);
/*
 * If REOM is set then go to rcverre
 */
	mov	REOM,brg
	orn	brg,r1,-
	brz	rcverre
/*
 * Set receive state = 3
 */
	mov	rcvstate,mar
	mov	3,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Received an abort character
 */
rcverr5:
/*
 * Increment error counter
 */
	COUNTUP(nerr5);
/*
 * If REOM is set then go to rcverre
 */
	mov	REOM,brg
	orn	brg,r1,-
	brz	rcverre
/*
 * Set receive state = 3
 */
	mov	rcvstate,mar
	mov	3,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * RSA is set but RDA is not set
 */
rcverr6:
	mov	r1,brg
	mov	brg,r2
/*
 * Increment error counter
 */
	COUNTUP(nerr6);
/*
 * Get receiver status register for this line
 */
	mov	r7,brg
	mov	brg,lur1
	mov	SR1|READ,brg
	mov	brg,lur2
	NOP
	mov	lur0,brg
	mov	brg,r1
/*
 * If REOM is set then go to rcverre
 */
	mov	REOM,brg
	orn	brg,r1,-
	brz	rcverre
/*
 * Set receive state = 3
 */
	mov	rcvstate,mar
	mov	3,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * No room in staging area
 */
rcverr7:
/*
 * Increment error counter
 */
	COUNTUP(nerr7);
/*
 * Go to rcverre
 */
	br	rcverre
/*
 * Wrap-up of receive-error processing
 */
rcverre:
/*
 * Get the address of the current receive buffer, if any.
 * If there is no current receive buffer then go to rcverrf
 */
	mov	rcvbuf1,mar
	mov	mem,r5
	brz	rcverrf
	mov	NIL,mem
/*
 * Attach the rbt entry to the end of the empty-receive-frame queue
 */
	APPENDQ(erfq,r5)
/*
 * Set buffer state = 1
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	1,mem
/*
 * Finish of receive-error processing
 */
rcverrf:
/*
 * Set receive state = 0
 */
	mov	rcvstate,mar
	mov	0,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Finish of receive-error processing
 */
rcverrg:
/*
 * Set receive state = 3
 */
	mov	rcvstate,mar
	mov	3,mem
/*
 * Return to the dispatcher
 */
	br	disp
#endif
/*
 * Bridge to report-generation code in segment three
 */
report:
	SEGJMP(reportc);
/*
 * Dispatcher state error
 */
staterr:
	mov	0117,brg
	mov	brg,r15
	br	.
/*
 * Ready queue contains invalid page number
 */
acterr1:
	mov	0116,brg
	br	crash1
/*
 * Invalid transmit state detected
 */
xmtsterr:
	mov	0107,brg
	br	crash1
/*
 * Invalid receive state detected
 */
rcvsterr:
	mov	0106,brg
	br	crash1
/*
 * Receive-service request on a line that is not currently active
 */
rcverr9:
	mov	0104,brg
	br	crash1
/*
 * Transmitter hardware error
 */
xmterr1:
	mov	0105,brg
	br	crash1
/*
 * Transmitter hardware error
 */
xmterr2:
	mov	0114,brg
	br	crash1
/*
 * No transmit buffer in transmit state 3
 */
xmterr3:
	mov	0115,brg
	br	crash1
/*
 * UNIBUS error while in segment one
 */
buserr1:
	mov	BUSERR,brg
	br	errterm1
/*
 * Receive-buffer error while in segment one
 */
rbuferr1:
	mov	RBUFERR,brg
	br	errterm1
/*
 * Error stop for debugging
 */
hang1:
	mov	brg,r15
	mov	HANG,brg
	br	errterm1
/*
 * Bridge to error-termination code in segment one
 */
errterm1:
	mov	brg,r0
	SEGJMP(errterm);
/*
 * Bridge to global-error-termination code in segment one
 */
crash1:
	mov	brg,r15
	SEGJMP(crash);
/*
 * *** end of segment one ***
 */
endseg1:
