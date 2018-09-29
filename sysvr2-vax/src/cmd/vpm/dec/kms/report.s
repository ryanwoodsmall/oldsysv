/*	report.s 1.6 of 4/14/83
	@(#)report.s	1.6
 */

/*
 * VPM interpreter
 *
 * This version of the interpreter supports both BISYNC and HDLC.
 *
 * The desired protocol class is selected by the #define at the beginning
 * of main.s
 *
 * Segment three
 *
 * This segment contains report-generation code
 *
 * Entered from segment one via label reportc
 */
	.org	3*1024
seg3:
/*
 * The KMC has something to report to the driver
 *
 * (We get here from dialog state 0 when the report-needed bit in r14
 * is set.)
 *
 * Reports are passed to the driver via the shared-memory interface
 * as follows:
 *
 *	Byte	Bits		Contents
 *
 *	csr2	2-0	Report type (0-7)
 *	csr3	2-0	Line number (0-7)
 *	csr4-csr7	Content depends on report type.
 */
reportc:
/*
 * If a crash has occurred then go to reportn
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	reportn
/*
 * Set r12 = NIL
 */
	mov	NIL,brg
	mov	brg,r12
/*
 * If a base-in acknowledgment report is needed then go to baseout
 */
	mov	~BASEOUT,brg
	or	brg,r14,-
	brz	baseout
/*
 * If a size error report is needed then go to szerr_out
 */
	mov	~SIZE_ERR,brg
	or	brg,r14,-
	brz	szerr_out
/*
 * Get the pointer to the head of the report-needed queue
 * If the queue is empty then go to reportn
 */
	mov	%reportq,%mar
	mov	reportq,mar
	mov	mem,r12|%mar
	brz	reportn
/*
 * If r12 does not contain a valid page number then go to
 * rpterr2
 */
	LINECHK(rpterr2);
/*
 * Remove the entry at the head of the report-needed queue
 */
	mov	rptlink,mar
	mov	mem,r0
	mov	%reportq,%mar
	mov	reportq,mar
	mov	r0,mem|mar++
	brz	1f
	br	2f
1:
	mov	NIL,mem
2:
/*
 * Set the page register for this line
 */
	mov	r12,%mar
/*
 * Clear the flag that says that this line is in the report-needed queue
 */
	mov	~REPORTL,brg
	mov	brg,r0
	mov	flags,mar
	and	mem,r0,mem
/*
 * If the process is halted or idle then go to reporta
 */
	mov	state,mar
	mov	mem,r0
	mov	HALTED,brg
	addn	brg,r0,-
	brz	reporta
	mov	IDLE,brg
	addn	brg,r0,-
	brz	reporta
/*
 * If the receive-buffer-return queue is non-empty then go to rbufout
 */
	mov	rbrq,mar
	mov	mem,-
	brz	1f
	br	rbufout
1:
/*
 * If the transmit-buffer-return queue is non-empty then go to xbufout
 */
	mov	xbrq,mar
	mov	mem,-
	brz	1f
	br	xbufout
1:
reporta:
/*
 * Get report flags
 */
	mov	reports,mar
	mov	mem,r0
/*
 * If an ERRTERM report is needed then go to errout;
 * else if a TRACE report is needed then go to traceout;
 * else if a SNAP report is needed then go to snapout;
 * else if a RPT report is needed then go to rptout;
 * else if a CMDACK report is needed then go to cmdack;
 * else if a STARTUP report is needed then go to startout;
 * else if an ERRCOUNT report is needed then go to ercnt1;
 */
	mov	~STARTOUT,brg
	or	brg,r0,-
	brz	startout
	mov	~STOPOUT,brg
	or	brg,r0,-
	brz	stopout
	mov	~ERROUT,brg
	or	brg,r0,-
	brz	errout
	mov	~TRACEOUT,brg
	or	brg,r0,-
	brz	traceout
	mov	~SNAPOUT,brg
	or	brg,r0,-
	brz	snapout
	mov	~RPTOUT,brg
	or	brg,r0,-
	brz	rptout
/*
 * Check second byte
 */
	mov	reports+1,mar
	mov	mem,r0
	mov	~CMDACK,brg
	or	brg,r0,-
	brz	cmdack
	mov	~ERRCNT1,brg
	or	brg,r0,-
	brz	ercnt1
	mov	~ERRCNT2,brg
	or	brg,r0,-
	brz	ercnt2
/*
 * If process state == READY then reactivate the line
 */
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0,-
	brz	1f
	br	2f
1:
	SCALL(act);
2:
/*
 * Return to the dispatcher
 */
	br	dispret3
/*
 * Send an RTNXBUF report to the driver
 *
 * The format of an RTNXBUF report is as follows:
 *
 *	Byte	Contents
 *
 *	csr2 - report number
 *	csr3 - line number
 *	csr4 - bits 7-0 of the descriptor address
 *	csr5 - bits 15-8 of the descriptor address
 *	csr6 - bits 17-16 of the descriptor address
 *	csr7 - unused
 */
xbufout:
/*
 * Get the xbt entry
 * at the head of the transmit-buffer-return queue
 */
	REMOVEQ(xbrq,r5,xbuferr3)
/*
 * If buffer state != 3 then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r0
	mov	3,brg
	addn	brg,r0,-
	brz	1f
	br	xbuferr3
1:
/*
 * Set buffer state = 0
 */
	mov	0,mem
/*
 * Put the address of the buffer descriptor into csr4-csr6
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6
/*
 * Attach the buffer to the end of the unused-transmit-buffer-entry
 * queue
 */
	APPENDQ(uxbq,r5)
/*
 * Put the report number into brg
 */
	mov	RRTNXBUF,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send a RTNRBUF report to the driver
 *
 * The format of a RTNRBUF report is as follows:
 *
 *	Byte	Contents
 *
 *	csr2 - report number
 *	csr3 - line number
 *	csr4 - bits 7-0 of the descriptor address
 *	csr5 - bits 15-8 of the descriptor address
 *	csr6 - bits 17-16 of the descriptor address
 *	csr7 - unused
 */
rbufout:
/*
 * Get the rbt entry at the head of the receive-buffer-return
 * queue
 */
	REMOVEQ(rbrq,r5,rbuferr3)
/*
 * If buffer state != 5 then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r0
	mov	5,brg
	addn	brg,r0,-
	brz	1f
	br	rbuferr3
1:
/*
 * Set buffer state = 0
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	0,mem
/*
 * Put the descriptor address into csr4-csr6
 */
	mov	DESC,brg
	add	brg,r5,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6
/*
 * Append the rbt entry to the unused-rbt-entry queue
 */
	APPENDQ(urbq,r5)
/*
 * Put the report type in brg
 */
	mov	RRTNRBUF,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send a TRACE report to the driver
 */
traceout:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Put the trace parameters in csr6 and csr7
 */
	mov	traceval,mar
	mov	mem,csr6|mar++
	mov	mem,csr7
/*
 * Put the reentry address in csr4-csr5
 */
	mov	r8,brg
	mov	brg,csr4
	mov	r9,brg
	mov	brg,csr5
/*
 * Put the report type in brg
 */
	mov	RTRACE,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send an ERRTERM report to the driver
 */
errout:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Put the termination code in csr6
 */
	mov	termcode,mar
	mov	mem,csr6
/*
 * Put the exit parameter, if any, in csr7
 */
	mov	errcode,mar
	mov	mem,csr7
/*
 * Put reentry address in csr4-csr5
 */
	mov	psave+2,mar
	mov	mem,csr4|mar++
	mov	mem,csr5
/*
 * Put the report type in brg
 */
	mov	ERRTERM,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send a SNAP report to the driver
 */
snapout:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Get saved address of report parameters
 */
	mov	traceval,mar
	mov	mem,mar
/*
 * Copy report parameters to csr4-csr7
 */
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6|mar++
	mov	mem,csr7
/*
 * Put the report type in brg
 */
	mov	RTNSNAP,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Return a script report to the driver
 */
rptout:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Get saved address of report parameters
 */
	mov	traceval,mar
	mov	mem,mar
/*
 * Copy report parameters to csr4-csr7
 */
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6|mar++
	mov	mem,csr7
/*
 * Put the report type in brg
 */
	mov	RTNSRPT,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send a command-received report to the driver
 *
 * This report is used to tell the driver that the script has accepted
 * the command
 */
cmdack:
	and	brg,r0,mem
	mov	RTNACK,brg
	br	reporte
/*
 * Send a basein-acknowledgment report to the driver
 */
baseout:
	and	brg,r14
	mov	0,brg
	mov	brg,csr3
	mov	brg,csr4
	mov	brg,csr5
	mov	brg,csr6
	mov	brg,csr7
	mov	BASEACK,brg
	mov	brg,csr2
	br	reportf
/*
 * Send a SIZERR termination report to the driver
 */
szerr_out:
/*
 * Determine if the command was a run command -- if so, then two
 * reports must be generated in response to the size error: a
 * STARTUP report and an ERRTERM report- these reports will
 * require two iterations through the report code; otherwise only an
 * ERRTERM report must be generated.
 */
/*
 * Put the start code (termination code) in csr6
 */
	mov	SIZERR,brg
	mov	brg,csr6
	mov	0,brg
	mov	brg,csr7
/*
 * Put the device number in csr3
 */
	mov	%size_err,%mar
	mov	size_err,mar
	mov	mem,brg
	mov	brg,csr3|mar++
/*
 * If command that caused the SIZERR problem was a run command,
 * then	clear the contents of size_err+1
 *	set the report type to ERRTERM (load this value into csr2)
 *	do not clear the SIZERR bit of r14 !!!!!
 *
 * else clear the SIZERR bit of r14 and set the report type to
 * 	ERRTERM.
 */
	mov	mem,r0
	mov	RUNCMD,brg
	addn	brg,r0,-
	brz	1f
	mov	~SIZE_ERR,brg
	and	brg,r14
	mov	ERRTERM,brg
	mov	brg,csr2
	br	2f
1:
	mov	0,brg
	mov	brg,mem
	mov	STARTUP,brg
	mov	brg,csr2
	mov	NXB,brg
	mov	brg,csr4
	mov	NRB,brg
	mov	brg,csr5
2:
/*
 * Branch to report continuation code
 */
	br	reportf
/*
 * Send a STARTUP report to the driver
 */
startout:
	and	brg,r0,mem
	mov	NXB,brg
	mov	brg,csr4
	mov	NRB,brg
	mov	brg,csr5
	mov	startcode,mar
	mov	mem,csr6
	mov	0,brg
	mov	brg,csr7
	mov	STARTUP,brg
	br	reporte
stopout:
	and	brg,r0,mem
	mov	stopcode,mar
	mov	mem,csr6
	mov	RTNSTOP,brg
	br	reporte
/*
 * Send an ERRCNT1 report to the driver
 */
ercnt1:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Copy error counts to csr regs
 */
	mov	errcnt,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6|mar++
	mov	mem,csr7
/*
 * Reset error counts
 */
	mov	errcnt,mar
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem
/*
 * Put report type in brg
 */
	mov	RTNER1,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send an ERRCNT2 report to the driver
 */
ercnt2:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r0,mem
/*
 * Copy error counts to csr regs
 */
	mov	errcnt+4,mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6|mar++
	mov	mem,csr7
/*
 * Reset error counts
 */
	mov	errcnt+4,mar
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem
/*
 * Put report type in brg
 */
	mov	RTNER2,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Wrapup of report generation
 */
reporte:
/*
 * Put the report type in csr2
 */
	mov	brg,csr2
/*
 * Put the device number in csr3
 */
	mov	device,mar
	mov	mem,csr3
reportf:
/*
 * Set RDYO
 */
	mov	csr2,r0
	mov	RDYO,brg
	or	brg,r0,csr2
/*
 * Set dialog state = 3
 */
	mov	3,brg
	mov	brg,r13
/*
 * If a crash has occurred then return to the dispatcher
 */
	mov	CRASHED,brg
	orn	brg,r14,-
	brz	dispret3
/*
 * If r12 == NIL then go to dispret3
 */
	mov	r12,-
	brz	dispret3
/*
 * Put the line on the end of the report-needed queue
 */
	mov	0,r0
	SCALL(rptschd);
/*
 * Return to the dispatcher
 */
	br	dispret3
/*
 * Nothing in the report-needed queue
 */
reportn:
 /*
 * Clear the report-needed bit in r14
 */
	mov	~REPORTG,brg
	and	brg,r14
/*
 * Return to the dispatcher
 */
	br	dispret3
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
	LINECHK(rpterr2);
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
 * Entered with page number in r12 adn report flag in r0
 * This entry is used when teh report flag is in the second by
 * reports cell
 */
rptschdb:
/*
 * If r12 does not contain a valid page number then go to
 * rpterr
 */
	LINECHK(rpterr2);
/*
 * Set the page register to the page for this line
 */
	mov	r12,%mar
/*
 * Or in the flag for this report
 */
	mov 	reports+1,mar
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
 * If the line is currently active then set acline = NIL and
 * store the virtual-machine registers
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
 * Read the DSR bit in the modem control unit line
 * status register for each virtual process in the
 * dsrneed queue.
 * If the state of the virtual process is WAITDSR
 *    if the DSR bit is set
 * 	 delete that process's element in the dsrneed queue
 *	 activate the process
 * else
 *    if the DSR bit is not set
 *	 terminate the process (DSR lead dropped)
 */
dsrcheckc:
	mov	%dsrneedq,brg|%mar
	mov	brg,r1
	mov	dsrneedq,mar
dsrloc1:
	mov	mem,r12
	brz	dsrloc6
	BUSWAIT(buserr3)
	mov	NZERO,brg
	xor	brg,r12,brg
	mov	%modemcsr,%mar
	MODEMWRITE(brg,modemcsr)
	BUSWAIT(buserr3)
	MODEMREAD(modemlsr)
	BUSWAIT(buserr3)
	mov	idl,r0
	mov	r12,%mar
	mov	state,mar
	mov	mem,r3
	mov	WAITDSR,brg
	addn	brg,r3,-
	brz	dsrloc3
/*
 * This code executed if state of process is not WAITDSR.
 * If DSR is not set, then 
 *	reset the DSRNEEDL bit in flags2
 *	remove the process's element from the dsrneed queue
 * 	terminate the process.
 */
	mov	DSR,brg
	orn	brg,r0,-
	brz	dsrloc5
	br	dsrloc2
dsrloc2:
	mov	~DSRNEEDL,brg
	mov	flags2,mar
	mov	mem,r0
	and	brg,r0,mem
	REM_NEEDQ(dsrneedq,dsrneedlink)
	mov	HANGUP,brg
	br	errterm3
/*
 * This code executed if state is WAITDSR.
 * If DSR is set, then activate the virtual process.
 */
dsrloc3:
	mov	DSR,brg
	orn	brg,r0,-
	brz	dsrloc4
	br	dsrloc5
dsrloc4:
	mov	state,mar	
	mov	READY,brg
	mov	brg,mem
	SCALL(act);
	mov	r12,%mar
/* 
 * Now move on to the next queue element and then jump to the
 * top of the loop.
 */
dsrloc5:
	mov	r12,brg
	mov	brg,r1
	mov	dsrneedlink,mar	
	br	dsrloc1
/*
 * When execution reaches this point, the entire queue has been
 * traversed, so reset the DSRNEED bit in r14 and go to disp.
 */
dsrloc6:
	mov	~DSRNEED,brg
	and	brg,r14
	br	dispret3
/*
 * DSRWAIT:  Wait for modem-ready
 */
dsrwaitc:
/*
 * First test if this virtual process is already in the dsrneedq
 *	if so, then branch to dispret3
 *	else continue
 */
	mov	flags2,mar
	mov	mem,r0
	mov	DSRNEEDL,brg
	orn	brg,r0
	brz	dispret3
#ifdef HDLC
/*
 * A virtual process may run on the HDLC kms interpreter without the
 * previous invocation of the dmkset command only if the following
 * conditions are met:
 *	1. DSRMODE is not a interpreter option for that process
 *	2. No dsrwait primitives are executed in the script for
 *	    that process
 * Test if the address of a modem control unit was sent to the inter-
 * preter as part of the basein command (address specified by dmkset
 * command). If not, then ERROR.
 */
	mov	%modemcsr,%mar
	mov	modemcsr,mar
	mov	mem,r1|mar++
	mov	mem,r2
	dec	r1,-
	brc	1f
	dec	r2,-
	brc	1f
	mov	NODMKDEV,brg
	br	errterm3
1:
	mov	r12,%mar
#endif
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
 * Change process state to WAITDSR
 */
	mov	r12,%mar
	mov	state,mar
	mov	WAITDSR,mem
/*
 * Set DSRNEEDL bit in flags2
 */
	mov	flags2,mar
	mov	mem,r0
	mov	DSRNEEDL,brg
	or	brg,r0,mem
	SEGJMP(deact);
#ifdef HDLC
/*
 * rsxmtq() - disassign (reset) all transmit sequence numbers
 *
 * Remove the sequence number assignment from any outstanding 
 * transmit buffers and put the buffers back onto the 
 * transmit-buffer-ready queue for reassignment via <getxfrm>
 */
rsxmtqc:
/*
 * Transfer any buffers on the transmit-buffer-ready queue to
 * the end of the transmit-buffer-open queue
 */
1:
	REMOVEQ(xbgq,r5,1f)
	APPENDQ(xboq,r5)
	br	1b
1:
/*
 * Now transfer all the buffers on the open queue back to the ready queue,
 * changing the buffer state appropriately on each buffer
 */
1:
	REMOVEQ(xboq,r5,1f)
	mov	STATE,brg
	add	brg,r5,mar
	mov	1,mem
	APPENDQ(xbgq,r5)
	br	1b
1:
/*
 * Remove the buffer assignment from all sequence numbers
 */
	mov	7,brg
	mov	brg,r0
	mov	oxbt,mar
1:
	mov	NIL,mem|mar++
	dec	r0
	brc	1b
/*
 * Merge with code for abtxfrm primitive
 */
	br	abtxfrmc
/*
 * abtxfrm() - abort the current transmission, if any
 */
abtxfrmc:
/*
 * Frame-abort not feasible with this line unit
 */
	br	dispret3
#endif
#ifdef BISYNC
/*
 * XBUFGET: Open the transmit buffer;
 *	    set MEM[Y] = RTYPE;
 *	    set MEM[Y+1] = RDEV;
 *	    set MEM[Y+2] = RSTA
 */
xbufget:
	mov	r5,brg
	mov	brg,r7
/*
 * If there is already a transmit buffer then go to xbufget2
 */
	mov	xmtbuf,mar
	mov	mem,r5
	brz	1f
	br	xbufget2
1:
/*
 * If the transmit-buffer-ready queue is empty then go to true;
 * otherwise remove the xbt entry from the head of the queue
 */
	REMOVEQ(xbgq,r5,true3)
/*
 * Record the address of the xbt entry
 */
	mov	xmtbuf,mar
	mov	r5,mem
/*
 * If STATE neq 1 then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r0
	mov	1,brg
	addn	brg,r0,-
	brz	1f
	br	xbufer3
1:
/*
 * Set state = 2
 */
	mov	2,mem
/*
 * Attach the buffer to the end of the transmit-buffer-open
 * queue
 */
	APPENDQ(xboq,r5)
xbufget2:
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
 * (total of twelve bytes)
 */
	mov	BDLENGTH/2,brg
	mov	brg,r4
	mov	xmtdesc,mar
	mov	r2,brg
	mov	3,r3
	and	brg,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3
	br	2f
1:
	BUSWAIT(buser3)
	mov	idl,mem|mar++
	mov	idh,mem|mar++
2:
	mov	r0,ial
	mov	r1,brg
	mov	brg,iah
	mov	r3,brg
	mov	brg,npr
	mov	2,brg
	add	brg,r0
	adc	r1
	adc	r2
	mov	r2,brg
	mov	3,r3
	and	brg,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3
	dec	r4
	brc	1b
/*
 * If the buffer address is odd then go to xbufer3
 */
	mov	xmtcp,mar
	mov	mem,brg
	br0	xbufer3
/*
 * Set npc = 0
 */
	mov	xmtnpc,mar
	mov	0,mem|mar++
	mov	0,mem
/*
 * Copy type, station, and device to user data area
 */
	mov	xbparam,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	r7,mar
	mov	r0,mem|mar++
	mov	r1,mem|mar++
	mov	r2,mem
/*
 * Clear the AC, then go to fetch
 */
	br	false3
/*
 * RBUFGET: Open the receive buffer, if any;
 *	    set MEM[Y] = RTYPE;
 *	    set MEM[Y+1] = RDEV;
 *	    set MEM[Y+2] = RSTA
 */
rbufget:
	mov	r5,brg
	mov	brg,r7
/*
 * If there is already a receive buffer then go to rbufget2
 */
	mov	rcvbuf,mar
	mov	mem,r5
	brz	1f
	br	rbufget2
1:
/*
 * Get an rbt entry from the queue of empty receive buffers.
 * If none is available then go to true
 */
	REMOVEQ(erfq,r5,true3)
/*
 * If buffer state != 1 then error
 */
	mov	STATE,brg
	add	brg,r5,mar
	mov	mem,r0
	mov	1,brg
	addn	brg,r0,-
	brz	1f
	br	rbufer3
1:
/*
 * Set buffer state = 2
 */
	mov	2,mem
/*
 * Save the address of the new receive buffer
 */
	mov	rcvbuf,mar
	mov	r5,mem
rbufget2:
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
 * (total of twelve bytes)
 */
	mov	BDLENGTH/2,brg
	mov	brg,r4
	mov	rcvdesc,mar
	mov	r2,brg
	mov	3,r3
	and	brg,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3
	br	2f
1:
	BUSWAIT(buser3)
	mov	idl,mem|mar++
	mov	idh,mem|mar++
2:
	mov	r0,ial
	mov	r1,brg
	mov	brg,iah
	mov	r3,brg
	mov	brg,npr
	mov	2,brg
	add	brg,r0
	adc	r1
	adc	r2
	mov	r2,brg
	mov	3,r3
	and	brg,r3
	asl	r3
	asl	r3
	mov	NRQ,brg
	or	brg,r3
	dec	r4
	brc	1b
/*
 * If the buffer address is odd then go to rbufer3
 */
	mov	rcvcp,mar
	mov	mem,brg
	br0	rbufer3
/*
 * Set npc = 0
 */
	mov	rnrc,mar
	mov	0,mem|mar++
	mov	0,mem
/*
 * Copy type, station, and device to user data area
 */
	mov	rbparam,mar
	mov	mem,r0|mar++
	mov	mem,r1|mar++
	mov	mem,r2
	mov	r7,mar
	mov	r0,mem|mar++
	mov	r1,mem|mar++
	mov	r2,mem
/*
 * Clear the AC, then go to fetch
 */
	br	false3
/*
 * Code for jumping to true in fetch.s
 */
true3:
	SEGJMP(true);
/*
 * Code for jumping to false in fetch.s
 */
false3:
	SEGJMP(false);
/*
 * UNIBUS error while executing xbufget or rbufget
 * ( routines displaced from segment 2 )
 */
buser3:
	mov	BUSERR,brg
	br	errterm3
/*
 * Receive-buffer error while executing rbufget
 * ( a routine displaced from segment 2 )
 */
rbufer3:
	mov	RBUFERR1,brg
	br	errterm3
/*
 * Transmit-buffer error while executing xbufget
 * ( a routine displaced from segment 2 )
 */
xbufer3:
	mov	XBUFERR1,brg
	br	errterm3
#endif
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
 * If (process state == READY || process state == WAITDSR)
 *  then go to react2
 */
	mov	state,mar
	mov	mem,r0
	mov	READY,brg
	addn	brg,r0,-
	brz	react2
	mov	WAITDSR,brg
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
	br	crash3
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
 * Act called with invalid page number in r12
 */
acterr:
	mov	0166,brg
	br	crash3
/*
 * react called with invalid page number in r12
 */
reacterr:
	mov	0165,brg
	br	crash3
/*
 * Error processing a report
 */
rpterr2:
	mov	0145,brg
	br	crash3
/*
 * UNIBUS error while in segment three
 */
buserr3:
#ifdef BISYNC
	mov	~NEM,brg
	mov	nprx,r0
	and	brg,r0,nprx
#endif
	mov	BUSERR,brg
	br	errterm3
/*
 * Receive-buffer error while in segment three
 */
rbuferr3:
	mov	RBUFERR,brg
	br	errterm3
/*
 * Transmit-buffer error while in segment 0
 */
xbuferr3:
	mov	XBUFERR,brg
	br	errterm3
/*
 * Error stop for debugging
 */
hang3:
	mov	brg,r15
	mov	HANG,brg
	br	errterm3
/*
 * Bridge to error-termination code in segment one
 */
errterm3:
	mov	brg,r0
	SEGJMP(errterm);
/*
 * Bridge to top of dispatcher loop in segment one
 */
dispret3:
	SEGJMP(disp);
/*
 * Bridge to global-error-termination code in segment one
 */
crash3:
	mov	brg,r15
	SEGJMP(crash);
/*
 * *** end of segment three ***
 */
endseg3:
