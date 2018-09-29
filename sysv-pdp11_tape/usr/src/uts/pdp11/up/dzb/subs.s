/* @(#)subs.s	1.1 */
/*
 * Subroutines 
 */

	.org	3072

seg3:

/*
 * KMC initialization
 */
init:
	mov	0,brg
	mov	brg,r13
	mov	brg,r14
	mov	brg,csr2
	mov	IEO,brg
	mov	brg,csr0
/*
 * Queue entries are pointed to by a pointer with a
 * format of OOOOOOPP where PP is the absolute page number
 * of the page containing the queue entry and OOOOOO is the 
 * offset of the beginning of the queue entry in the page.
 * Note that when the page number is masked out the offset
 * is on a four byte boundry.  Hence queue entries are
 * four bytes long and occupy the first four pages.  A small
 * amount of data shares page zero with queue entries which
 * explains the following hocus-pocus used to calculate
 * the number of queue entries (NPQ).
 */
#define	NPQ	4
/*
 * The -1 below is to prevent a queue entry with the address
 * of 0377 which will occur is there are four pages of queue entries
 */
#define	NQE	((NPQ*256-((inqueue+3)&~3))/4)-1
/*
 * Initialize the list of available queue entries
 */
	mov	NIL,brg
	mov	brg,r12
	mov	(inqueue+3)&~3,brg
	mov	brg,r0
	mov	NQE-1,brg
	mov	brg,r1
init4:
	FREEQ(r0)
	mov	4,brg
	add	brg,r0
	adc	r0
	dec	r1
	brc	init4
/*
 * Initialize LTE queue pointers
 */
	mov	3,brg
	mov	brg,r8
/*
 * Following the queue entries are eight pages of circular buffers
 * of 64 bytes used to buffer input data when the host  has not
 * presented a new buffer.  These circular buffers are pointed to
 * by a pointer of the format OOOOPPPP where PPPP is the absolute
 * page number of the page containing the circular buffer and
 * OOOO is the offset into the page.  Nothe that this addressing
 * scheme can be used to address any byte of data on a sixteen
 * byte boundry anywhere in a KMC11-B.  The sixty-four byte
 * circular input buffers satisfy this requriement.  Since the
 * first buffer begins at page four its addres is 04.
 * Also note to point to the next buffer 0100 must be
 * added to the current pointer.
 */
	mov	NPQ,brg
	mov	brg,r1
init6:
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	7,r7|mar
init8:
	mov	mem,r10
	mov	OUTBUF,brg
	add	brg,r10,mar
	mov	NIL,mem
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	NIL,mem
/*
 * Initialize to pointer to circular buffer
 */
	mov	ICIRC,brg
	add	brg,r10,mar
	mov	r1,mem
/*
 * Point to next circular buffer
 */
	mov	0100,brg
	add	brg,r1
	adc	r1
/*
 * Loop through all LTEs in a page
 */
	dec	r7,r7|mar
	brc	init8
/*
 * Loop through all pages of LTEs
 */
	dec	r8
	brc	init6
/*
 * Return to caller
 */
	RETURN(ret1)

/*
 * The 50-microsecond hardware timer has expired
 */
tick:
/*
 * Reset the 50-microsecond timer
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO|(3<<2)),brg
	and	brg,r0
	mov	PCLK,brg
	or	brg,r0,nprx
/*
 * The effect of the following code is to schedule a
 * csr check for a different DZ every 50(?) microseconds.
 * Since there are four possible DZ's, each DZ gets scheduled
 * for a csr check every 200 microseconds.  This is approximately
 * a factor of five better than we need for 9600 baud (one character
 * every 1042 microseconds).
 */
/*
 * Increment r15.
 */
	inc	r15
/*
 * Set the DZ number from bits 1-0 of r15.
 */
	mov	3,brg
	and	brg,r15,brg
	mov	brg,r8
/*
 * Set page register to page for this DZ
 */
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * If a base-in command has not been received yet for this DZ
 * then go to disp
 */
	mov	status,mar
	mov	mem,brg
	br7	1f
	br	dispret3
1:
/*
 * If a csr check has already been requested for this DZ
 * then go to delaycheck
 */
	br0	delaycheck
/*
 * Set the bit which indicates that a csr check has already
 * been scheduled
 */
	mov	1<<0,r1
	or	mem,r1,mem
/*
 * Insert the page for the current DZ at the end of the
 * csr-check-needed queue
 */
	mov	csrlink,mar
	mov	NIL,mem
	mov	tailcsrq,mar
	mov	%tailcsrq,%mar
	mov	mem,r0
	brz	1f
	mov	r8,mem
	mov	%dzst,brg
	add	brg,r0,%mar
	mov	csrlink,mar
	mov	r8,mem
	br	2f
1:
	mov	headcsrq,mar
	mov	r8,mem|mar++
	mov	r8,mem
2:
/*
 * Set global flag to indicate that a csr check has been requested
 */
	mov	CHKCSRQ,brg
	or	brg,r14
delaycheck:
/*
 * Set page register to page for this DZ
 */
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Decrement the 1 msec timer.  If the result is 
 * non-negative then reutrn
 */
	mov	clock1,mar
	mov	mem,r0
	dec	r0,mem
	brc	dispret3
/*
 * The 1 msec timer has expired, reset it and test
 * for input time out.
 */
	mov	2,mem
/*
 * Input time-out test
 *
 * Loop through all eight lines for this DZ.  For each line with a non-empty
 * input queue, if the timer has expired then put the line on the 
 * input-ready queue
 */
	mov	7,r7
deltst:
	mov	r7,mar
	mov	mem,r10
/*
 * See if timer is active
 */
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	TIMACT,brg
	orn	brg,r0,-
	brz	1f
	br	2f
1:
/*
 * Timer is active, decrement ITIME
 */
	mov	ITIME,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,mem
	brc	2f
/*
 * Timer has expired, set IDRAIN
 */
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~TIMACT,brg
	and	brg,r0,mem
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	IDRAIN,brg
	or	brg,r0,mem
/*
 * If there is a user buffer call drain
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	2f
	CALL(drain,retdra)
	mov	%dzst,brg
	add	brg,r8,%mar
2:
	dec	r7
	brc	deltst

/*
 * See if the 16.7 msec timer has expired
 */
	mov	clock2,mar
	mov	mem,r0
	dec	r0,mem
	brc	dispret3
/*
 * Reset the 16.7 msec timer
 */
	mov	17,mem
/*
 * The 16.7 msec timer has expired.  Therefore we need to decrement
 * the count field for each lte which is currently in the twait
 * state (delaying)
 */
/*
 * Initialize registers for loop which follows
 */
	mov	5,r5
	mov	7,r7|mar
/*
 * The following loop is executed once for each lte
 * in the page for the selected DZ.
 *
 * If we find a line which is in the twait state
 * then we decrement the count field of the lte.
 * If the result is negative then we put the line
 * back into state 1 and enable the line.
 */
delaych1:
/*
 * Get the address of the lte
 *
 * If state == 5 (twait) then go to delaych3
 *
 * Note that the following code assumes that STATE is the
 * first cell in the lte and that the memory-address
 * register (mar) has already been loaded from r7
 */
	mov	mem,r10|mar
	addn	mem,r5,-
	brz	delaych3
/*
 * Test for end of loop
 */
delaych2:
/*
 * Decrement the current entry number.  If the result is
 * non-negative then go to delaych1
 */
	dec	r7,r7|mar
	brc	delaych1
	br	dispret3
/*
 * We get here when we find a line with STATE == 5 (twait)
 */
delaych3:
/*
 * Decrement the count field of the lte.  If the result is
 * non-negative then go to delaych2
 */
	mov	COUNT,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,mem
	brc	delaych2
/*
 * The time has elapsed, if a break was being transmitted call ubreak
 */
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	OUTBRK,brg
	orn	brg,r0,-
	brz	1f
	br	2f
1:
/*
 * Turn off treak bit
 */
	CALL(ubreak,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	br	delaych2
2:
/*
 * Continue sending data.
 */
/*
 * Set state = 1 (sending normal data)
 */
	mov	STATE,brg
	add	brg,r10,mar
	mov	1,mem
/*
 * Turn on the enable bit for this line
 */
	CALL(enable,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Go to delaych2
 */
	br	delaych2
/*
 * End of above loop
 */
/*
 * The following code checks for output character completions
 * and input data ready in the silo
 */
csrcheck:
/*
 * If the check-CSR queue is empty then error
 */
	mov	headcsrq,mar
	mov	%headcsrq,%mar
	mov	mem,r8
	brz	csrcout
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Set up a unibus request for the current value of CSR
 */
	mov	csrget,mar
	mov	mem,ial|mar++
	mov	mem,iah|mar++
	mov	mem,r0
	asl	r0
	asl	r0
	mov	NRQ,brg
	or	brg,r0,npr
/*
 * Wait for the unibus transfer to complete.
 * If an error occurs then go to buserr3.
 */
	BUSWAIT(buserr3)
/*
 * If there is an input character available then go to csrchk2
 */
	mov	idl,brg
	br7	csrchk2
/*
 * If there is an output data request then go to csrchk4
 */
	mov	idh,brg
	br7	csrchk4
/*
 * Clear the bit that indicates that a csr check has been scheduled for this page
 */
	mov	status,mar
	mov	 ~(1<<0),brg
	mov	brg,r0
	and 	mem,r0,mem
/*
 * Remove the current DZ from the csr-check-needed queue;
 * if the queue is empty then clear the CHKCSRQ bit in r14
 */
	mov	csrlink,mar
	mov	mem,brg
	mov	%headcsrq,%mar
	mov	headcsrq,mar
	mov	brg,mem|mar++
	brz	1f
	br	2f
1:
	mov	brg,mem
csrcout:
	mov	~CHKCSRQ,brg
	and	brg,r14
2:
/*
 * Go to disp
 */
	br	dispret3
/*
 * Input data available--jump to input segment
 */
csrchk2:
	mov	%inchar,brg
	mov	brg,pcr
	jmp	inchar
/*
 * Output data request--jump to output segment
 */
csrchk4:
	mov	%outreq,brg
	mov	brg,pcr
	jmp	outreq

dispret3:
	mov	%disp,brg
	mov	brg,pcr
	jmp	disp
/*
 * The following code queues and error report
 */
erqueue:
/*
 * If an error has already been queued for this dz just return
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	status,mar
	mov	mem,r0
	mov	1<<5,brg
	orn	brg,r0,-
	brz	2f
	or	brg,r0,mem
/*
 * Increment the error counter
 */
	mov	errcnt,mar
	mov	mem,r0
	inc	r0,mem
/*
 * Get the driver's idea of the DZ number and combine it with the line
 * number to get the minor device number
 */
	mov	dznr,mar
	mov	mem,r0
	asl	r0
	asl	r0
	asl	r0
	mov	07,brg
	and	brg,r7,brg
	or	brg,r0
/*
 * Queue an error for this line
 */
	GETQ(r4,queuerr3)
	mov	NIL,mem|mar++
/*
 * Put dznr<<3|line number in second byte
 */
	mov	r0,mem|mar++
/*
 * Put error number into third byte
 */
	mov	r1,mem|mar++
/*
 * Save r8
 */
	mov	r8,mem
	mov	%tailerrq,%mar
	mov	tailerrq,mar
	mov	mem,r0
	brz	1f
	mov	r4,mem
	mov	03,brg
	and	brg,r0,%mar
	mov	~03,brg
	and	brg,r0,mar
	mov	r4,mem
	br	2f
1:
	mov	headerrq,mar
	mov	r4,mem|mar++
	mov	r4,mem
/*
 * Indicat an error queued
 */
2:
	mov	CHKERRQ,brg
	or	brg,r14
	RETURN(ret1)

/*
 * Unibus error
 */
buserr3:
/*
 * Clear the non-existent memory bit
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO|NEM),brg
	and	brg,r0,nprx
	ERROR(BUSERR3)
	br	dispret3
queuerr3:
/*
 * No queue entries to queue an error, tough
 */
	HALT

endseg3:
