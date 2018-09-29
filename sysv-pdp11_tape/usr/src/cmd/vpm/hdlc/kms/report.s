/*	report.s 1.4 of 2/12/82
	@(#)report.s	1.4
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
 * If an OK report is needed then go to okout
 */
	mov	~OKOUT,brg
	or	brg,r14,-
	brz	okout
/*
 * If an base-in acknowledgment report is needed then go to baseout
 */
	mov	~BASEOUT,brg
	or	brg,r14,-
	brz	baseout
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
 * Send an I_AM_OK report to the driver
 */
okout:
/*
 * Clear the bit that says to send this report
 */
	and	brg,r14
/*
 * Get saved buffer address
 */
	mov	okbuf,mar
	mov	%okbuf,%mar
	mov	mem,r1|mar++
	mov	mem,r2|mar++
	mov	mem,r3
/*
 * If the buffer address is zero then go to okout2
	mov	0,brg
	dec	r0,-
	subc	brg,r1,-
	subc	brg,r2,-
	brc	okout2
 */
	dec	r1,-
	brc	1f
	dec	r2,-
	brc	1f
	dec	r3,-
	brc	1f
	br	okout2
1:
/*
 * Copy activity counts to specified buffer
 */
	mov	LACTBUF/2-1,brg
	mov	brg,r4
	mov	actbuf,mar
1:
	mov	mem,odl|mar++
	mov	mem,odh|mar++
	mov	r1,brg
	mov	brg,oal
	mov	r2,brg
	mov	brg,oah
	mov	3,brg
	and	brg,r3
	mov	nprx,r0
	mov	VEC4,brg
	and	brg,r0
	mov	r3,brg
	mov	brg,r5
	asl	r5
	asl	r5,brg
	or	brg,r0,nprx
	mov	OUT|NRQ,brg
	mov	brg,npr
	BUSWAIT(buserr3)
	dec	r4
	brc	2f
	br	3f
2:
	mov	2,brg
	add	brg,r1
	adc	r2
	adc	r3
	br	1b
3:
okout2:
/*
 * Reset line activity counts
 */
	mov	LACTBUF-1,brg
	mov	brg,r4
	mov	actbuf,mar
1:
	mov	0,mem|mar++
	dec	r4
	brc	1b
/*
 * Set device number = 0
 */
	mov	0,brg
	mov	brg,csr3
/*
 * Put the report type in brg
 */
	mov	RTNOK,brg
	mov	brg,csr2
/*
 * Go to reportf
 */
	br	reportf
/*
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
 * Error processing a report
 */
rpterr1:
	mov	0144,brg
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
	mov	0146,brg
	br	crash3
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
