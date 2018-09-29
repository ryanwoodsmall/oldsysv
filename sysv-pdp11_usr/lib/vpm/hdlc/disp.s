/*	disp.s 1.5 of 3/23/82
	@(#)disp.s	1.5
 */

/*
 * VPM interpreter
 *
 * This version of the interpreter supports both BISYNC and HDLC.
 *
 * The desired protocol class is selected by the #define at line 1
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
edispsw:
#define	LDISPSW	edispsw-dispsw
/*
 * Switch on process state
 */
statesw:
	br	disp		/* 0 - idle */
	br	ready		/* 1 - ready */
	br	waitdsr		/* 2 - dsr wait */
	br	disp		/* 3 - halted */
	br	inwait		/* 4 - input wait */
	br	outwait		/* 5 - output wait */
	br	xendwait	/* 6 - wait for transmitter to go idle */
estatesw:
#define	LSTATESW	estatesw-statesw
/*
 * Dispatcher loop--keep looking for something to do
 */
disp:
/*
 * If the 75-microsecond timer has expired then go to tick
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
	mov	REPORT,brg
	orn	brg,r14,-
	brz	report
/*
 * Go to dispb
 */
	br	dispb
report:
	mov	%reportc,brg
	mov	brg,pcr
	jmp	reportc
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
 * Branch to disp, ready, inwait, outwait, or waitdsr
 * depending on the current state of the virtual process
 */
dispb:
	mov	LSTATESW,brg
	sub	brg,r12,-
	brc	staterr
	mov	statesw,brg
	br	(add,brg,r12),%statesw
/*
 * Bridge to command-processing code in segment one
 */
command:
	mov	%commandc,brg
	mov	brg,pcr
	jmp	commandc
/*
 * Process state == ready to run
 */
ready:
#ifdef	HDLC
/*
 * Set page bits
 */
	mov	%cmdsave,%mar
/*
 * If there is room in the output silo for another character
 * then go to dispxmt
 */
	mov	lur1,brg
	br4	dispxmt
/*
 * If there is data in the input silo then go to disprcv
 */
	mov	lur2,brg
	br4	disprcv
#endif
/*
 * If there is a report in progress then return to the dispatcher
 */
	mov	REPORT,brg
	orn	brg,r14,-
	brz	disp
/*
 * Go to the instruction fetch/decode/interpret
 * sequence in segment two
 */
	mov	%fetch,brg
	mov	brg,pcr
	jmp	fetch
/*
 * Process state == WAIT DSR:  Check for modem ready
 */
waitdsr:
/*
 * If modem ready is not on then return to the dispatcher
 */
	mov	lur3,r0
	asl	r0,brg
	br4	1f
	br	disp
1:
/*
 * Set process state = READY
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Set flag to terminate script execution if modem ready drops
 */
	mov	HUPCLOSE,brg
	or	brg,r14
/*
 * Go to disp
 */
	br	disp
#ifdef	BISYNC
/*
 * Process state == INPUT WAIT:  Check for input ready
 */
inwait:
/*
 * If the input silo is empty then return to the dispatcher
 */
	mov	lur2,brg|r0
	br4	1f
	br	disp
1:
/*
 * Get the saved address of the vpm memory location
 * to receive this byte
 */
	mov	ioadr,mar
	mov	%ioadr,%mar
	mov	mem,mar
	mov	%regs,%mar
/*
 * Move the received data byte to the specified location
 */
	mov	lur0,brg
	mov	brg,mem
/*
 * Set process state = READY
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Process state == OUTPUT WAIT:  Check for output ready
 */
outwait:
/*
 * If the output silo is full then return to the dispatcher
 */
	mov	lur1,brg
	br4	1f
	br	disp
1:
/*
 * If XEOM is set then go to xeomset
 */
	mov	XEOM,brg
	orn	brg,r14,-
	brz	xeomset
/*
 * Clear TSOM
 */
	mov	0,brg
	mov	brg,lur1
/*
 * Load the character to be transmitted into the output
 * data silo
 */
	mov	ioadr,mar
	mov	%ioadr,%mar
	mov	mem,lur0
/*
 * If XLAST is set then go to xlastset
 */
	mov	XLAST,brg
	orn	brg,r14,-
	brz	xlastset
/*
 * Set process state = READY.
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Prepare to shut down the transmitter after this character.
 */
xlastset:
/*
 * Set XEOM
 */
	mov	XEOM,brg
	or	brg,r14
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Shut down the transmitter.
 */
xeomset:
/*
 * Set the TEOM bit in the out control register
 */
	mov	TEOM,brg
	mov	brg,lur1
/*
 * Put a dummy character into lur0
 */
	mov	07,brg
	mov	brg,lur0
/*
 * Clear XEOM and XLAST
 */
	mov	~(XEOM|XLAST),brg
	and	brg,r14
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Process state == XEND WAIT:  Wait for transmitter to become idle
 */
xendwait:
/*
 * If the transmitter is still active then return to the dispatcher
 */
	mov	lur1,r0
	asl	r0,brg
	brc	disp
/*
 * Clear the transmitter
 */
	mov	OCLRP,brg
	mov	brg,lur1
/* Redundant instructions inserted for time fill */
	mov	0,brg
	mov	0,brg
/*
 * Get saved value for sync character
 */
	mov	ioadr,mar
	mov	%ioadr,%mar
	mov	mem,r5
/*
 * Set TSOM
 */
	mov	TSOM,brg
	mov	brg,lur1
/* Redundant instructions inserted for time fill */
	mov	0,brg
	mov	0,brg
/*
 * Send first sync character
 */
	mov	r5,brg
	mov	brg,lur0
/*
 * Clear TSOM
 */
	mov	0,r0
	mov	0,r0
	mov	r0,lur1
/*
 * Send six more sync characters
 */
	mov	r5,brg
	mov	brg,lur0
	mov	r5,brg
	mov	brg,lur0
	mov	r5,brg
	mov	brg,lur0
	mov	r5,brg
	mov	brg,lur0
	mov	r5,brg
	mov	brg,lur0
	mov	r5,brg
	mov	brg,lur0
/*
 * Set process state = READY.
 */
	mov	READY,brg
	mov	brg,r12
/*
 * Return to the dispatcher
 */
	br	disp
#else
inwait:
outwait:
xendwait:
	br	staterr
#endif
#ifdef	HDLC
/*
 * Receiver dispatcher:  process an input character
 */
disprcv:
/*
 * Branch on receive state
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
/*
 * Transmitter dispatcher: put a character into the output silo
 */
dispxmt:
/*
 * Branch on transmit state
 */
	mov	xmtstate,mar
	mov	%xmtstate,%mar
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
	dec	r0
	brz	xmtst5
	br	xmtsterr
/*
 * Transmit state 0:	Sending continuous flags
 */
xmtst0:
/*
 * Set TSOM
 */
	mov	TSOM,brg
	mov	brg,lur1
/*
 * Put a dummy character into the output silo
 */
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
 * Set TSOM
 */
	mov	TSOM,brg
	mov	brg,lur1
/*
 * Put a dummy character into the output silo
 */
	mov	0,brg
	mov	brg,lur0
/*
 * Decrement the flag count
 */
	mov	flagcnt,mar
	mov	mem,r0
	dec	r0,mem
/*
 * If the result is greater than zero then go to disp
 */
	mov	mem,r0
	dec	r0,-
	brc	disp
/*
 * Initialize transmit CRC
 */
	mov	xmtcrc,mar
	mov	NIL,mem|mar++
	mov	NIL,mem
/*
 * Set transmit state = 2
 */
	mov	xmtstate,mar
	mov	2,mem
/*
 * Go to disp
 */
	br	disp
/*
 * Transmit state 2:  Sending the control portion of an i-frame
 */
xmtst2:
/*
 * Clear TSOM
 */
	mov	0,brg
	mov	brg,lur1
/*
 * Send a byte of control info
 */
	mov	xctlptr,mar
	mov	mem,r0
	mov	xmtac,brg
	add	brg,r0,mar
	mov	mem,lur0
/*
 * Get transmitted byte in r3
 */
	mov	mem,r3
/*
 * Update transmit CRC accumulation
 */
	SCALL(axcrc)
/*
 * Increment xctlptr.  If the result is >= nxctl then
 * set transmit state = 3
 */
	mov	xctlptr,mar
	mov	mem,r0
	inc	r0
	mov	r0,mem
	mov	nxctl,mar
	sub	mem,r0,-
	brc	1f
	br	2f
1:
	mov	xmtstate,mar
	mov	3,mem
2:
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Transmit state 3:	Sending the data portion of an i-frame
 */
xmtst3:
/*
 * If there is no transmit buffer then go to xmtst3e
 */
	mov	xmtbuf,mar
	mov	mem,-
	brz	xmtst3e
/*
 * If there are no characters remaining then go to xmtst3e
 */
	mov	xmtnpc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	xmtsize,mar
	sub	mem,r0,mar++
	subc	mem,r1,-
	brc	xmtst3e
/*
 * Increment the count of transmitted characters
 */
	mov	xmtnpc,mar
	inc	r0,mem|mar++
	adc	r1,mem
/*
 * If X25FLAG is set then go to xmtst3a0
 */
	mov	X25FLAG,brg
	orn	brg,r14,-
	brz	xmtst3a0
/*
 * If the character count is odd then go to xmtst3a
 */
	inc	r0,brg
	br0	xmtst3a
/*
 * Go to xmtst3a2
 */
	br	xmtst3a2
/*
 * X25 mode is set
 */
xmtst3a0:
/*
 * If the character count is less than three then go to xmtst3b
 */
	dec	r1,-
	brc	1f
	mov	3,brg
	sub	brg,r0,-
	brc	1f
	br	xmtst3b
1:
/*
 * If the character count is even then go to xmtst3a
 */
	mov	r0,brg
	br0	xmtst3a
/*
 * The character count is even (odd if in X25 mode) so we need to
 * send the high-order byte from the current output word
 */
xmtst3a2:
/*
 * Transmit the saved byte from the previous fetch
 */
	mov	0,brg
	mov	brg,lur1
	mov	xmtsave,mar
	mov	mem,lur0
/*
 * Get transmitted byte in r3
 */
	mov	mem,r3
/*
 * Update transmit CRC accumulation
 */
	SCALL(axcrc)
/*
 * Return to the dispatcher
 */
	br	disp
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
 * Transmit the low-order byte
 */
	mov	0,brg
	mov	brg,lur1
	mov	idl,lur0
/*
 * Get the transmitted byte in r3
 */
	mov	idl,r3
/*
 * Update transmit CRC accumulation
 */
	SCALL(axcrc)
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Sending three-byte packet header
 */
xmtst3b:
/*
 * Send next byte from packet header
 */
	mov	xbparam,brg
	add	brg,r0,mar
	mov	0,brg
	mov	brg,lur1
	mov	mem,lur0
/*
 * Get the transmitted byte in r3
 */
	mov	mem,r3
/*
 * Update transmit CRC accumulation
 */
	SCALL(axcrc)
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * No characters remaining--send low byte of CRC
 */
xmtst3e:
/*
 * Set control bits = 0
 */
	mov	0,brg
	mov	brg,lur1
/*
 * Put low byte of CRC into lur0
 */
	mov	xmtcrc,mar
	mov	0,r0
	orn	mem,r0,lur0
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
 * Transmit state 4:  Sending low byte of CRC
 */
xmtst4:
/*
 * Clear control bits
 */
	mov	0,brg
	mov	brg,lur1
/*
 * Put high byte of CRC into lur0
 */
	mov	xmtcrc+1,mar
	mov	0,r0
	orn	mem,r0,lur0
/*
 * Set transmit state = 5
 */
	mov	xmtstate,mar
	mov	5,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Transmit state 5:  Sending high byte of CRC
 */
xmtst5:
/*
 * Set TEOM
 */
	mov	TEOM,brg
	mov	brg,lur1
/*
 * Put a dummy character into lur0
 */
	mov	brg,lur0
/*
 * Set transmit state = 0
 */
	mov	xmtstate,mar
	mov	0,mem
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Receive state 0:  Between frames
 */
rcvst0:
/*
 * Get control bits and input character
 */
	mov	lur2,brg
	mov	brg,r1
	mov	lur0,r0
/*
 * If BLOCKEND is set then go to rcverr0
 */
	br1	rcverr0
/*
 * If the staging area still contains valid data
 * (because a control frame was received previously when no
 * receive buffer was available)
 * then go to rcverr4
 */
	mov	HAVCTL,brg
	orn	brg,r14,-
	brz	rcverr4
/*
 * Set receive state = 1
 */
	mov	rcvstate,mar
	mov	1,mem
/*
 * Initialize receive CRC
 */
	mov	rcvcrc,mar
	mov	NIL,mem|mar++
	mov	NIL,mem
/*
 * Save input character as first byte of control info
 */
	mov	rcvac,mar
	mov	r0,mem
/*
 * Get received byte in r3
 */
	mov	mem,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
/*
 * If ADRSWTCH is  set then switch address bit
 */
	mov	vpmopts,mar
	mov	%vpmopts,%mar
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
	mov	lur2,brg
	mov	brg,r1
	mov	lur0,r0
/*
 * If BLOCKEND is set then go to rcverr0
 */
	br1	rcverr0
/*
 * Save input character as second byte of control info
 */
	mov	rcvac+1,mar
	mov	r0,mem
/*
 * Get received byte in r3
 */
	mov	mem,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
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
 * If there is already a receive buffer then error
 */
	mov	rcvbuf1,mar
	mov	mem,-
	brz	1f
	br	rbuferr1
1:
/*
 * Get an rbt entry from the queue of empty receive buffers.
 * If none is available then go to rcvst1b
 */
	REMOVEQ(erfq,r5,rcvst1b)
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
rcvst1b:
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
	mov	lur2,brg
	mov	brg,r1
	mov	lur0,r0
/*
 * If BLOCKEND is set then go to rcveom
 */
	br1	rcveom
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
	mov	r0,mem
/*
 * If X25FLAG is set then go to rcvst2d
 */
	mov	X25FLAG,brg
	orn	brg,r14,-
	brz	rcvst2d
rcvst2x:
/*
 * Increment the number of characters received
 */
	mov	rnrc,mar
	mov	mem,r2|mar++
	mov	mem,r3
	inc	r2
	adc	r3
	brc	1f
	mov	rnrc,mar
	mov	r2,mem|mar++
	mov	r3,mem
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
	mov	mem,r2|mar++
	mov	mem,r3
	mov	rnrc,mar
	sub	mem,r2,mar++
	subc	mem,r3,-
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
 * Character count is even:  transfer two bytes to the receive buffer
 */
/*
 * Set up a unibus request to move two bytes to the host computer
 */
	mov	rsave,mar
	mov	mem,odl
	mov	r0,odh
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
 * Get received byte in r3
 */
	mov	odh,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Character count is odd:  save input character for transfer later
 */
rcvst2a:
/*
 * Save the input character for transfer to the host computer later
 */
	mov	rsave,mar
	mov	r0,mem
/*
 * Get received byte in r3
 */
	mov	mem,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * Buffer size exceeded or no buffer allocated
 */
rcvst2b:
/*
 * Get received byte in r3
 */
	mov	r0,brg
	mov	brg,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
/*
 * Return to the dispatcher
 */
	br	ready
/*
 * Receiving three-byte packet header
 */
rcvst2d:
/*
 * If no buffer allocated then go to rcvst2b
 */
	mov	rcvbuf1,mar
	mov	mem,r5
	brz	rcvst2b
/*
 * Stash char in rbparam
 */
	mov	rbparam-2,brg
	add	brg,r2,mar
	mov	r0,mem
/*
 * Get received byte in r3
 */
	mov	mem,r3
/*
 * Update receive CRC accumulation
 */
	SCALL(arcrc);
/*
 * Return to the dispatcher
 */
	br	ready
/*
 * Receive state 3:  Skipping a frame
 */
rcvst3:
/*
 * Get control bits and input character
 */
	mov	lur2,brg
	mov	brg,r1
	mov	lur0,r0
/*
 * If BLOCKEND is set then go to rcverr1
 */
	br1	rcverr1
/*
 * Return to the dispatcher
 */
	br	disp
/*
 * End of block detected
 */
rcveom:
/*
 * Update receive CRC accumulation
 */
	mov	r0,brg
	mov	brg,r3
	SCALL(arcrc);
/*
 * If less than four bytes received (including CRC) then go to rcverr0
 */
	mov	nrctl,mar
	mov	mem,r2
	mov	3,brg
	sub	brg,r2,-
	brc	1f
	br	rcverr0
1:
/*
 * If the CRC is incorrect then go to rcverr2
 */
	mov	rcvcrc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	0270,brg
	addn	brg,r0,-
	brz	1f
	br	rcverr2
1:
	mov	0360,brg
	addn	brg,r1,-
	brz	1f
	br	rcverr2
1:
/*
 * Get address of current receive buffer;
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
	mov	mem,r2|mar++
	mov	mem,r3
	mov	2,brg
	add	brg,r2
	adc	r3
	mov	rnrc,mar
	sub	mem,r2,mar++
	subc	mem,r3,-
	brc	1f
	br	rcverr3
1:
/*
 * If the buffer size is odd and the number of data bytes received
 * is exactly equal to the buffer size then copy the last data byte
 * to the input buffer
 */
	mov	rcvsize,mar
	mov	mem,brg
	br0	1f
	br	2f
1:
	mov	rnrc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	dec	r0
	mov	0,brg
	subc	brg,r1
	mov	rcvsize,mar
	addn	mem,r0,mar++
	brz	1f
	br	2f
1:
	addn	mem,r1,-
	brz	1f
	br	2f
1:
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
	BUSWAIT(buserr1)
2:
/*
 * If X25FLAG is set then add length of packet header to byte count
 */
	mov	rnrc,mar
	mov	mem,r0|mar++
	mov	mem,r1
	mov	X25FLAG,brg
	orn	brg,r14,-
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
/* off by one */
	dec	r0
	mov	0,brg
	subc	brg,r1
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
	mov	HAVCTL,brg
	or	brg,r14
rcveom4:
/*
 * Set receive state = 0
 */
	mov	rcvstate,mar
	mov	0,mem
/*
 * Go to disp
 */
	br	disp
/*
 * Frame too short (less than four bytes including CRC)
 */
rcverr0:
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
 * Get address of error counter
 */
	mov	nerr0,brg|mar
/*
 * Go to countup
 */
	br	countup
/*
 * No receive buffer available
 */
rcverr1:
/*
 * Get address of error counter
 */
	mov	nerr1,brg|mar
/*
 * Go to countup
 */
	br	countup
/*
 * CRC error
 */
rcverr2:
/*
 * Get address of error counter
 */
	mov	nerr2,brg|mar
/*
 * Go to countup
 */
	br	countup
/*
 * Frame too long (buffer size exceeded)
 */
rcverr3:
/*
 * Get address of error counter
 */
	mov	nerr3,brg|mar
/*
 * Go to countup
 */
	br	countup
/*
 * No room in staging area
 */
rcverr4:
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
 * Increment error counter (address in brg) but don't wrap around
 */
countup:
	mov	brg,mar
	mov	mem,r0
	inc	r0
	brc	1f
	mov	mem,r0
	inc	r0,mem
1:
	mov	errset,mar
	mov	1,mem
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
 * Attach the rbt entry to the end of the empty-receive frame queue
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
 * Update transmit CRC accumulation
 */
axcrc:
	mov	xmtcrc,brg
	br	arcrc2
/*
 * Update receive CRC accumulation
 */
arcrc:
	mov	rcvcrc,brg
	br	arcrc2
/*
 * Note:  The following code calculates CRC-CCITT
 *	 (x**0 + x**5 + x**12 + x**16).
 *
 *	 The algorithm processes one data byte in parallel.
 */
arcrc2:
	mov	brg,r5|mar
	xor	mem,r3,r3|brg|mar++
	asl	r3
	asl	r3
	asl	r3
	asl	r3
	xor	brg,r3,r3|brg	/* r3 ^= (r3<<4) */
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	brg,r0		/* r0 = r3>>4 */
	xor	mem,r0		/* r0 ^= xupper */
	mov	0,brg>>
	xor	brg,r3,mem	/* xupper = r3 ^ (r3>>5) */
	asl	r3
	asl	r3
	asl	r3,brg
	mov	r5,mar
	xor	brg,r0,mem	/* xlower = r0 ^ (r3<<3) */
	SRETURN
#endif
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
 * Go to disp
 */
	br	disp
/*
 * Dispatcher state error
 */
staterr:
	mov	STATERR,brg
	mov	brg,r15
	br	.
/*
 * Invalid transmit state detected
 */
xmtsterr:
	mov	XMTSTERR,brg
	br	errterm1
/*
 * Invalid receive state detected
 */
rcvsterr:
	mov	RCVSTERR,brg
	br	errterm1
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
 * Transmit-buffer error while in segment 0
 */
xbuferr1:
	mov	XBUFERR,brg
	br	errterm1
/*
 * Error stop for debugging
 */
hang1:
	mov	brg,r15
	mov	HANG,brg
	br	errterm1
errterm1:
	/*save reason for termination */
	mov	termcode,mar
	mov	%termcode,%mar
	mov	brg,mem
	/*mark type of report to be scheduled */
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	mov	ERROUT,brg
	or	brg,r0,mem
	/* branch errterm */
	br	errterm
/*
 * Terminate the virtual process
 */
errterm:
/*
 * Save scratch regs
 */
	mov	regsave,mar
	mov	%regsave,%mar
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
 * Set process state = HALTED
 */
	mov	HALTED,brg
	mov	brg,r12
/*
 * Clear the timeout-in-progress flag
 */
	mov	~TOUT,brg
	and	brg,r14
/*
 * Clear data-terminal-ready
 */
	mov	0,brg
	mov	brg,lur3
/*
 * If termcode is non-zero then set the report-needed flag
 */
	mov	%termcode,%mar
	mov	termcode,mar
	mov	mem,r0
	dec	r0
	brz	1f
	mov	REPORT,brg
	or	brg,r14
1:
	br	disp
/*
 * *** end of segment one ***
 */
endseg1:
