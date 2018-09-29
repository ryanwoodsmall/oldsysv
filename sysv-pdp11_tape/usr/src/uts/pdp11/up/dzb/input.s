/* @(#)input.s	1.2 */
/*
 * Input segment
 *
 * This segment is entered from the main segment when it has been
 * determined that a character has been received
 */
	.org	2048
seg2:
/*
 * Define error bits in DZ-11 status register
 */
#define	PARERR	1<<4
#define	FRMERR	1<<5
#define	OVRRUN	1<<6
/*
 * Enter here from main segment
 */
inchar:
/*
 * Read RBUF
 */
	mov	csrget,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	2,brg
	add	brg,r0,ial
	adc	r1,brg
	mov	brg,iah
	adc	r2
	asl	r2
	asl	r2
	mov	NRQ,brg
	or	brg,r2,brg
	mov	brg,npr
	BUSWAIT(buserr2)
/*
 * If data-valid bit is not set then go to dispret2
 */
	mov	idh,brg
	br7	1f
	br	dispret2
1:
/*
 * Set r7 = line number; set r10 = address of line table entry
 */
	mov	idh,brg
	mov	7,r7
	and	brg,r7,r7|mar
	mov	mem,r10
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Put IFLAG1 into oal and IFLAG2 into oah
 */
	mov	IFLAG1,brg
	add	brg,r10,mar
	mov	mem,oal|mar++
	mov	mem,oah
/*
 * Get bits 8-15 of RBUF
 */
	mov	idh,r0
/*
 * If a framing error has occurred
 */
	mov	FRMERR,brg
	orn	brg,r0,-
	brz	1f
	br	2f
1:
/*
 * See if it is a break by seeing if data is zero, else
 * treat as a parity error
 */
	mov	idl,r1
	dec	r1
	brc	parerr
/*
 * A break has been detected, see if breaks are to be ignored
 */
	mov	oal,r6
	mov	IGNBRK,brg
	orn	brg,r6,-
	brz	dispret2
/*
 * Breaks are not being ignored, see if signal should be sent
 */
	mov	BRKINT,brg
	orn	brg,r6,-
	brz	sigbrk
/*
 * Put a null character out
 */
	mov	NUL,brg
	mov	brg,r0
	br	putit

2:
/*
 * If an overrun for this dz has occurred and not previously reported do so
 */
	mov	OVRRUN,brg
	orn	brg,r0,-
	brz	1f
	br	2f
/*
 * See if previous report 
 */
1:
	mov	status,mar
	mov	mem,r2
	mov	(1<<3),brg
	orn	brg,r2,-
	brz	parerr
/*
 * Set previous report bit and queue error
 */
	or	brg,r2,mem
	ERROR(ERRB)
	mov	%dzst,brg
	add	brg,r8,%mar
2:
/*
 * If parity is being checked and a parity error has occurred then translate
 * it to 0377, 0, X 
 */
	mov	oal,r6
	mov	INPCK,brg
	orn	brg,r6,-
	brz	1f
	br	2f
1:
	mov	PARERR,brg
	orn	brg,r0,-
	brz	parerr
	br	2f

parerr:
	mov	oal,r6
/*
 * See if parity errors are being ignored
 */
	mov	IGNPAR,brg
	or	brg,r6,-
	brz	dispret2
/*
 * Mark the character a a parity error
 */
	mov	0377,brg
	mov	brg,r0
	CALL(putc,ret1)
	mov	0,r0
	CALL(putc,ret1)
	mov	idl,r0
	br	putit
2:
/*
 * Fetch input char
 */
	mov	idl,r0
/*
 * See if in 7 or 8 bit mode
 */
ckstrip:
	mov	oal,r6
	mov	ISTRIP,brg
	orn	brg,r6,-
	brz	1f
	br	2f
1:
	mov	0177,brg
	and	brg,r0
	mov	r0,idl
2:
/*
 * See if start/stop being done
 */
	mov	oah,r6
	mov	IXON,brg
	orn	brg,r6,-
	brz	1f
	br	2f
/*
 * Line is under control, see if line is stopped
 */
1:
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r2
	mov	TTSTOP,brg
	orn	brg,r2,-
	brz	stopped
/*
 * Line is not stopped, see if this is stop character
 */
	mov	CSTOP,brg
	addn	brg,r0,-
	brz	1f
	br	ckignor
/*
 * Stop the line
 */
1:
	mov	TTSTOP,brg
	or	brg,r2,mem
	br	dispret2

/*
 * If character is start character or IXANY is set restart line
 */
stopped:
	mov	CSTART,brg
	addn	brg,r0,-
	brz	start
	mov	IXANY,brg
	orn	brg,r6,-
	brz	start
	br	ckignor
/*
 * Restart line
 */
start:
	mov	~TTSTOP,brg
	and	brg,r2,mem
	CALL(enable,ret1)
/*
 * Restore values destroyed by enable
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	IFLAG1,brg
	add	brg,r10,mar
	mov	mem,oal|mar++
	mov	mem,oah
	mov	idl,r0
/*
 * Do not return start or stop character
 */
ckignor:
	mov	CSTART,brg
	addn	brg,r0,-
	brz	dispret2
	mov	CSTOP,brg
	addn	brg,r0,-
	brz	dispret2
2:
/*
 * If character is 0377 then escape it with another 0377
 */
	mov	r0,brg
	brz	1f
	br	2f
1:
	CALL(putc,ret1)
	br	putit
2:
/*
 * See if a NL
 */
	mov	NL,brg
	addn	brg,r0,-
	brz	1f
	br	ckcr
/*
 * See if NL mapped to CR
 */
1:
	mov	oal,r6
	mov	INLCR,brg
	orn	brg,r6,-
	brz	1f
	br	putit
1:
	mov	CR,brg
	mov	brg,r0
	br	putit
/*
 * See if a CR
 */
ckcr:
	mov	CR,brg
	addn	brg,r0,-
	brz	1f
	br	cklc
/*
 * Character is a CR, see if it should be ignored
 */
1:
	mov	oal,r6
	mov	IGNCR,brg
	orn	brg,r6,-
	brz	dispret2
/*
 * See if CR mapped to NL
 */
	mov	oah,r6
	mov	ICRNL,brg
	orn	brg,r6,-
	brz	1f
	br	putit
1:
	mov	NL,brg
	mov	brg,r0
	br	putit
/*
 * If IUCLC is set then translate upper-case letters to lower case
 */
cklc:
	mov	oah,r6
	mov	IUCLC,brg
	orn	brg,r6,-
	brz	1f
	br	3f
1:
	mov	'A',brg
	sub	brg,r0,-
	brc	1f
	br	3f
1:
	mov	'Z',brg
	addn	brg,r0,-
	brc	3f
2:
	mov	'a'-'A',brg
	add	brg,r0
3:

/*
 * Put out the character in R0
 */
putit:
	CALL(putc,ret1)


/*
 * ITIME is used time how long it has been since the last character
 * arrived.  It is initialized to the value in INITIM when a character
 * arrives.  It is decremented every milisecond and if it expires
 * the line is queued for input.  If INITIM is zero the line
 * is queued immediately.
 */

	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * If INITIM>0 then decrement and start timer, else set IDRAIN
 */
	mov	INITIM,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ITIME,brg
	add	brg,r10,mar
	dec	r1,mem
	brz	1f
/*
 * Indicate timer active
 */
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	TIMACT,brg
	or	brg,r1,mem
	br	2f
/*
 * No timer on input, set IDRAIN 
 */
1:
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	or	brg,r1,mem
/*
 * If there is a user buffer call drain
 */
2:
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	1f
/*
 * There is a buffer, call drain to input what is available
 */
	CALL(drain,retdra)
1:
	br	dispret2


/*
 * This routine copies data from the internal circular
 * buffer into main memory.  Data will be moved two bytes at
 * a time (except possibly the first and last bytes).
 * A single remaining byte will be left in the circular buffer until
 * another character arrives when both will be moved into memory.
 *
 * Exceptions:
 * 	If the buffer is at an odd address one character will
 * 	be placed in the odd location then two bytes will be
 * 	moved thereafter.
 *
 *	If IDRAIN is set all characters will be moved into memory
 * 	an a report to the host will be generated.
 */

drain:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get the residual byte count into r4, r5
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,r3
	mov	03,brg
	and	brg,r3,%mar
	mov	~03,brg
	and	brg,r3,mar
/*
 * Increment mar
 */
	mov	brg,brg|mar++
	mov	brg,brg|mar++
	mov	mem,r4|mar++
	mov	mem,r5
/* 
 * Mask off address portion 
 */
	mov	~03,brg
	and	brg,r4
/*
 * Main loop: Test for input data or room 
 */
dralup:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * See  if there is any more input
 */
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brz	1f
	br	2f
/*
 * No more input, if IDRAIN is set queue line
 */
1:
dranon:
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	orn	brg,r1,-
	brz	draque
/*
 * Just return
 */
	br	draret
2:
/*
 * There is more input, if there is room in the buffer
 * for only 1 more character put it in and queue line.
 * (Note: the wordcount low  byte<<2 is in r4 and high
 * byte in r5.
 */
	dec	r5,-
	brc	drarom
	mov	1<<2,brg
	addn	brg,r4,-
	brz	dramov
/*
 * Here there is more room and more input, if buffer
 * address is odd move in  one character and 
 * continue loop.
 */
drarom:
	mov	03,brg
	and	brg,r3,%mar
	mov	~03,brg
	and	brg,r3,mar
	mov	mem,brg
	br0	1f
	br	2f
/*
 * Move a character to an odd location
 */
1:
	CALL(getc,ret1)
	CALL(escproc,retesc)
	mov	r0,odh
/*
 * Decrement byte count by 1
 */
	mov	1<<2,brg
	sub	brg,r4
	mov	0,brg
	subc	brg,r5
/*
 * Move in one character and test for more room
 */
drmov1:
	CALL(mov1,ret1)
	br	dtstrm
2:
/*
 * See if there is more than 1 character remaining to be input
 */
	mov	1,brg
	addn	brg,r0,-
	brz	drat
/*
 * Here there are at least two characters of input,
 * there is at least room for two characters of input
 * at an even address.  Move in two characters.
 */
	CALL(getc,ret1)
	CALL(escproc,retesc)
	mov	r0,odl
/*
 * Decrement byte count by 1
 */
	mov	1<<2,brg
	sub	brg,r4
	mov	0,brg
	subc	brg,r5
/*
 * If the subroutine escproc reduced the number
 * of input characters to zero just send in the one in odl
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r1
	dec	r1,-
	brc	1f
/*
 * Ran out of characters, just send in the one
 */
	CALL(mov1,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	br	dranon
1:
	CALL(getc,ret1)
	CALL(escproc,retesc)
	mov	r0,odh
/*
 * Decrement byte count by 1
 */
	mov	1<<2,brg
	sub	brg,r4
	mov	0,brg
	subc	brg,r5
	CALL(mov2,ret1)
/*
 * If there is more room in the input buffer return to the loop,
 * else queue input
 */
dtstrm:
	dec	r5,-
	brc	dralup
	dec	r4,-
	brc	dralup
	br	draque
/*
 * Here there is only one more character of input.
 * If IDRAIN is set move it in and queue the line, esle
 * just return and wait for next character
 */
drat:
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	orn	brg,r1,-
	brz	dramov
	br	draret
/*
 * Move in one character
 */
dramov:
	CALL(getc,ret1)
	CALL(escproc,retesc)
	mov	r0,odl
	mov	r0,odh
	CALL(mov1,ret1)
/*
 * Queue the input
 */
draque:
	CALL(queue,ret1)
draret:
	RETURN(retdra)

/*
 * Stash the character in an input buffer
 */
putc:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * If the ICLOST flag is set continue tossing characters
 * until all input is drained.  This is not so drastic since it is
 * assumed this condition is a driver bug.
 */
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICLOST,brg
	orn	brg,r1,-
	brz	putret
/*
 * If there are MAXINCH characters in the input queue
 * for this line then set flag and toss character
 */
#define	MAXINCH	64
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	MAXINCH,brg
	addn	brg,r1,-
	brz	1f
	br	2f
/*
 * Set ICLOST, IDRAIN and return
 */
1:
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICLOST,brg
	or	brg,r1,mem
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	or	brg,r1,mem
	br	putret
2:

/*
 * Put character into circular buffer at offset (IFIRST+NICIRC)%MAXINCH
 */
	mov	IFIRST,brg
	add	brg,r10,mar
	mov	mem,r1|mar++
	add	mem,r1
	mov	MAXINCH,brg
	sub	brg,r1,-
	brc	1f
	br	2f
1:
	sub	brg,r1
2:
/*
 * r1 contains modulo offset, get absolute address
 */
	mov	ICIRC,brg
	add	brg,r10,mar
	mov	mem,r2
	mov	017,brg
	and	brg,r2,%mar
	mov	~017,brg
	and	brg,r2,brg
	add	brg,r1,mar
/*
 * Move character into memory
 */
	mov	r0,mem
/*
 * Increment the number of input characters
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r1
	inc	r1,mem
/*
 * Return to main line
 */
putret:
	RETURN(ret1)

/*
 * Get a character from the circular buffer
 */

getc:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * If there is no character just return, else decrement count
 */
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r1
	dec	r1,-
	brc	1f
	br	getret
1:
	dec	r1,mem
/*
 * Get character from circular buffer at offset IFIRST
 */
	mov	IFIRST,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICIRC,brg
	add	brg,r10,mar
	mov	mem,r2
	mov	017,brg
	and	brg,r2,%mar
	mov	~017,brg
	and	brg,r2,brg
	add	brg,r1,mar
	mov	mem,r0
/*
 * Set IFIRST = (IFIRST+1)%MAXINCH
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	IFIRST,brg
	add	brg,r10,mar
	mov	mem,r1
	inc	r1,mem
/*
 * Do modulo (-1 for inc above)
 */
	mov	MAXINCH-1,brg
	addn	brg,r1,-
	brz	1f
	br	2f
1:
	mov	0,mem
2:
getret:
	RETURN(ret1)

/*
 * Move the character in odl,odh into the next address
 * pointed to by IBUFF
 * (Note: both odl and odh contain the character to
 * handle even and odd addresses)
 */

mov1:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get the address into oal, oah & set high order bits
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,r6
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	mem,oal|mar++
	mov	mem,oah|mar++
	mov	mem,r0
	mov	03,brg
	and	brg,r0
	asl	r0
	asl	r0
	mov	~(BRQ|ACLO|(3<<2)),brg
	mov	nprx,r1
	and	brg,r1,brg
	or	brg,r0,nprx
/*
 * Set request and wait
 */
	mov	NRQ|OUT|BYTE,brg
	mov	brg,npr
	BUSWAIT(buserr2)
/*
 * Increment the address, decrement the word count
 */
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	mem,r0
	inc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
/*
 * Get the word count into a seperate reg
 */
	mov	mem,r0
	mov	~03,brg
	and	brg,r0,brg
	mov	brg,r1
/*
 * Continue incrementing address
 */
	adc	r0
/*
 * Mask off any overflow
 */
	mov	03,brg
	and	brg,r0
/*
 * Decrement the wordcount by 1 (note the count is shifted
 * left by two places)
 */
	mov	(1<<2),brg
	sub	brg,r1,brg
/*
 * Recombine the address and word count
 */
	or	brg,r0,mem|mar++
/*
 * Subtract the carry from the high order byte of the word count
 */
	mov	mem,r0
	mov	0,brg
	subc	brg,r0,mem

	RETURN(ret1)

/*
 * Move the characters in odl and odh into the next address
 * pointed to by IBUFF
 */

mov2:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Get the address into oal, oah & set high order bits
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,r6
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	mem,oal|mar++
	mov	mem,oah|mar++
	mov	mem,r0
	mov	03,brg
	and	brg,r0
	asl	r0
	asl	r0
	mov	~(BRQ|ACLO|(3<<2)),brg
	mov	nprx,r1
	and	brg,r1,brg
	or	brg,r0,nprx
/*
 * Set request and wait
 */
	mov	NRQ|OUT,brg
	mov	brg,npr
	BUSWAIT(buserr2)
/*
 * Increment the address by 2, decrement the word count by two
 */
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	2,brg
	mov	mem,r0
	add	brg,r0,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++
/*
 * Get the word count into a seperate reg
 */
	mov	mem,r0
	mov	~03,brg
	and	brg,r0,brg
	mov	brg,r1
/*
 * Continue incrementing address
 */
	adc	r0
/*
 * Mask off any overflow
 */
	mov	03,brg
	and	brg,r0
/*
 * Decrement the wordcount by 2 (note the count is shifted
 * left by two places)
 */
	mov	(2<<2),brg
	sub	brg,r1,brg
/*
 * Recombine the address and word count
 */
	or	brg,r0,mem|mar++
/*
 * Subtract the carry from the high order byte of the word count
 */
	mov	mem,r0
	mov	0,brg
	subc	brg,r0,mem

	RETURN(ret1)

enableit:
/*
 * If the line is inactive then enable it
 */
/*
 * Restore page regs
 */
	mov 	%dzst,brg
	add	brg,r8,%mar
	mov	STATE,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brc	1f
	mov	1,mem
	CALL(enable,ret1)
1:
/*
 * Return to the dispatcher
 */
	br	dispret2
/*
 * A break has been detected.  If there is a user input
 * buffer set ICBRK in the completion code and queue input.
 * Else send a break report to the host
 */
sigbrk:
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	sigrep
/*
 * Set ICBRK, IDRAIN and call drain
 */
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICBRK,brg
	or	brg,r1,mem
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	or	brg,r1,mem
	CALL(drain,retdra)
	br	dispret2

/*
 * Schedule a BREAK signal to the host
 */
sigrep:
/*
 * Get an unused queue entry.  If there is not one available
 * then go to queuerr2
 */
	GETQ(r4,queuerr2)
/*
 * Set link to next queue entry = NIL
 */
	mov	NIL,mem|mar++
/*
 * Put (dz nr<<3)|line nr in second byte of queue entry
 */
	mov	r8,brg
	mov	brg,r1
	asl	r1
	asl	r1
	asl	r1
	mov	7,brg
	and	brg,r7,brg
	or	brg,r1,mem|mar++
/*
 * Chain this queue entry to the end of the break-request-
 * report queue
 */
	mov	tailbrkq,mar
	mov	%tailbrkq,%mar
	mov	mem,r5
	brz	1f
	mov	r4,mem
	mov	3,brg
	and	brg,r5,%mar
	mov	~3,brg
	and	brg,r5,mar
	mov	r4,mem
	br	2f
1:
	mov	r4,mem
	mov	headbrkq,mar
	mov	r4,mem
2:
/*
 * Set a flag to indicate that the break-request-report
 * queue is non-empty
 */
	mov	CHKBRKQ,brg
	or	brg,r14
	br	dispret2

/*
 * If there is a user buffer set ICFLU and queue input.  Else
 * just reset number of input characters to zero and clean LTE
 */
iflush:
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,-
	brz	iflnob
/*
 * Set ICFLU, IDRAIN and call drain
 */
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICFLU,brg
	or	brg,r1,mem
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IDRAIN,brg
	or	brg,r1,mem
	CALL(drain,retdra)
	br	fluret
/*
 * Clean up the LTE
 */
iflnob:
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	0,mem
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	0,mem
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	~(IDRAIN|IPERR),brg
	and	brg,r1,mem
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	~TIMACT,brg
	and	brg,r1,mem
fluret:
	RETURN(retifl)

/*
 * Return to top of dispatcher loop in main segment
 */
dispret2:
	mov	%disp,brg
	mov	brg,pcr
	jmp	disp
queue:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * Put this line on the input-ready queue
 */
	mov	IBUFF,brg
	add	brg,r10,mar
	mov	mem,r6
	mov	NIL,mem
/*
 * Get an unused queue entry.  If there is not one available
 * then go to queuerr2
 */
	GETQ(r4,queuerr2)
/*
 * Set link to next queue entry = NIL
 */
	mov	NIL,mem|mar++
/*
 * Put the line number, DZ number and ptr to completion report
 * queue into the queue entry
 */
	mov	r7,mem|mar++
	mov	r8,mem|mar++
	mov	r6,mem
/*
 * Chain this queue entry to the end of the input-ready queue
 */
	mov	tailinrq,mar
	mov	%tailinrq,%mar
	mov	mem,r5
	brz	1f
	mov	r4,mem
	mov	3,brg
	and	brg,r5,%mar
	mov	~3,brg
	and	brg,r5,mar
	mov	r4,mem
	br	2f
1:
	mov	r4,mem
	mov	headinrq,mar
	mov	r4,mem
2:
/*
 * Get residual byte count into r4, r5
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	brg,brg|mar++
	mov	brg,brg|mar++
	mov	mem,r4|mar++
	mov	mem,r5|mar++
/*
 * Shift r4, r5 two places to the right
 */
	mov	r4,brg
	mov	r5,brg>>
	mov	brg,r4
	mov	r5,brg
	mov	0,brg>>
	mov	brg,r5
	mov	r4,brg
	mov	r5,brg>>
	mov	brg,r4
	mov	r5,brg
	mov	0,brg>>
	mov	brg,r5
/*
 * If ICBRG, ICEIB, or ICFLU is set then indicate in completion code
 * and clear them.  If overrun has occurred and the number of
 * input characters remaining is zero indicate.
 */
	mov	%dzst,brg
	add	brg,r8,%mar
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~ICBRK|ICEIB|ICFLU,brg
	and	brg,r0,mem
	mov	ICLOST|ICBRK|ICEIB|ICFLU,brg
	and	brg,r0
/*
 * If NICIRC is not zero clear overrun bit in r0.  Else
 * clean up LTE
 */
	mov	NICIRC,brg
	add	brg,r10,mar
	mov	mem,r1
	dec	r1,-
	brz	1f
/*
 * Turn off ICLOST
 */
	mov	~ICLOST,brg
	and	brg,r0
	br	2f
/*
 * Clean up LTE
 */
1:
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	~IDRAIN,brg
	and	brg,r1,mem
	mov	FLAGS1,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	~TIMACT,brg
	and	brg,r1,mem
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	0,mem
2:
/*
 * Set up completion report queue entry
 */
	mov	03,brg
	and	brg,r6,%mar
	mov	~03,brg
	and	brg,r6,mar
	mov	r4,mem|mar++
	mov	r5,mem|mar++
	mov	r0,mem
/*
 * Set a flag to indicate that the input-buffer-completion-report
 * queue is non-empty
 */
	mov	CHKINRQ,brg
	or	brg,r14
/*
 * Return to caller
 */
	RETURN(ret1)

/*
 * This subroutine is called with a character in r0 to
 * handle escape sequences properly.  All parity errors
 * are placed in the circular buffer in 0377, 0, X 
 * format.  If parmrk is not set this sequence must
 * be translated to 0.  The subroutine returns with
 * the next character to be placed in the user buffer
 * in r0.
 */

escproc:
	mov	%dzst,brg
	add	brg,r8,%mar
/*
 * If IPERR is set the third character of a parity
 * sequence (0377, 0, X) is being passed, do not
 * process it.
 */
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	~IPERR,brg
	or	brg,r1,-
	brz	1f
	br	2f
/*
 * Turn off IPERR and return
 */
1:
	and	brg,r1,mem
	br	escend
/*
 * See if the last character was an escape
 */
2:
	mov	~PREVESC,brg
	or	brg,r1,-
	brz	1f
/*
 * No, go see if this is an escape
 */
	br	escesc
1:
/*
 * Escape has been processed, turn off PREVESC
 */
	and	brg,r1,mem
/*
 * If this is the second character of an 0377, 0377
 * just pass it through
 */
esctst:
	inc	r0,-
	brc	escend
/*
 * If PARMRK is not set set current character to zero and
 * remove third character of the 0377, 0, X sequence and
 * set ICEIB, else set IPERR so third character will not
 * be processed next time through
 */
	mov	IFLAG1,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	PARMRK,brg
	orn	brg,r1,-
	brz	escerr
/*
 * Set ICEIB
 */
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICEIB,brg
	or	brg,r1,mem
/*
 * Remove next character from input queue
 */
	CALL(getc,ret1)
/*
 * Set current character to zero and return
 */
	mov	0,r0
	br	escend

/*
 * Set IPERR so the third character will be skipped
 */
escerr:
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	IPERR,brg
	or	brg,r1,mem
	br	escend
/*
 * See if this is an escape character
 */
escesc:
	inc	r0,-
	brc	1f
/*
 * No, just return current character
 */
	br	escend
/*
 * Escape character, see if PARMRK is set
 */
1:
	mov	IFLAG1,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	PARMRK,brg
	orn	brg,r1,-
	brz	escetc
/*
 * PARMRK is not set, get the second character and
 * go to esctst
 */
	CALL(getc,ret1)
	mov	%dzst,brg
	add	brg,r8,%mar
	br	esctst
/*
 * PARMRK is set and an 0377 has been found, set 
 * PREVESC and ICEIB
 */
escetc:
	mov	FLAGS,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	PREVESC,brg
	or	brg,r1,mem
	mov	ICCODE,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	ICEIB,brg
	or	brg,r1,mem

escend:
	RETURN(retesc)

/*
 * Ran out of input buffers
 */
flush:
	ERROR(FLUSH)
/*
 * little need to keep processing
 */
	HALT
/*
 * Unibus error
 */
buserr2:
/*
 * Clear the non-existent memory bit
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO|NEM),brg
	and	brg,r0,nprx
	ERROR(BUSERR2)
	br	dispret2
/*
 * Ran out of queue entries
 */
queuerr2:
	br	flush
endseg2:
