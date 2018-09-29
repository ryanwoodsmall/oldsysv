/*	report.s 1.3 of 2/10/82
	@(#)report.s	1.3
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
 *	csr2	2-0	Command type (0-7)
 *	csr3	5-0	Line number (0-63)
 *	csr4-csr7	Content depends on report type.
 */
reportc:
/*
 * If the receive-buffer-return queue is non-empty then go to rbufout
 */
	mov	rbrq,mar
	mov	%rbrq,%mar
	mov	mem,-
	brz	1f
	br	rbufout
1:
/*
 * If the transmit-buffer-return queue is non-empty then go to xbufout
 */
	mov	xbrq,mar
	mov	%xbrq,%mar
	mov	mem,-
	brz	1f
	br	xbufout
1:
/*
 * Get report flags
 */
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
/*
  Schedule a report
 */
	mov	~OKOUT,brg
	or	brg,r0,-
	brz	okout
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
	mov	~CMDACK,brg
	or	brg,r0,-
	brz	cmdack
    /* check other byte */
	mov	mar,r0
	inc	r0
	mov	r0,mar
	mov	mem,r0
	mov	~ERRCNT1,brg
	or	brg,r0,-
	brz	errcnt1
	mov	~BASEOUT,brg
	or	brg,r0,-
	brz	baseout
 /*
 * Clear the report-needed bit in r14
 */
	mov	~REPORT,brg
	and	brg,r14
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
	mov	r8,brg
	mov	brg,csr4
	mov	r9,brg
	mov	brg,csr5
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
	and	brg,r0,mem
/*
 * Put the report type in brg
 */
	mov	RTNOK,brg
/*
 * Go to reporte
 */
	br	reporte
/*
 * Send a BASEACK report to driver
 */
baseout:
	and	brg,r0,mem
	mov	BASEACK,brg
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
	mov	%regs,%mar
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
	mov	%regs,%mar
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
 * Send a STARTUP report to the driver
 */
startout:
	and	brg,r0,mem
	mov	NXB,brg
	mov	brg,csr4
	mov	NRB,brg
	mov	brg,csr5
	mov	startcode,mar
	mov	%startcode,%mar
	mov	mem,csr6
	mov	0,brg
	mov	brg,csr7
	mov	STARTUP,brg
	br	reporte
stopout:
	and	brg,r0,mem
	mov	stopcode,mar
	mov	%stopcode,%mar
	mov	mem,csr6
	mov	RTNSTOP,brg
	br	reporte		
/*
 * Send an error count report to the driver
 */
errcnt1:
	and	brg,r0,mem
	mov	nerr0,mar
	mov	%nerr0,%mar
	mov	mem,csr4|mar++
	mov	mem,csr5|mar++
	mov	mem,csr6|mar++
	mov	mem,csr7
	mov	nerr0,mar
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem
	mov	errset,mar
	mov	0,mem
	mov	RTNER1,brg
	br	reporte
/*
 * Wrapup of report generation
 */
reporte:
/*
 * Put device number in csr3
 */
	mov	device,mar
	mov	%device,%mar
	mov	mem,csr3
/*
 * Put the report type in csr2 and set RDYO
 */
	mov	brg,r0
	mov	RDYO,brg
	or	brg,r0,csr2
/*
 * If IEO is set then go to ieoset3
 */
	mov	csr0,brg
	br4	ieoset3
/*
 * Set dialog state = 3
 */
	mov	3,brg
	mov	brg,r13
/*
 * Go to disp
 */
	br	dispret3
/*
 * Continuation of driver-report processing
 *
 * We come here when the driver sets IEO (in response to
 * our setting RDYO)
 */
ieoset3:
	mov	%ieoset,brg
	mov	brg,pcr
	jmp	ieoset
/*
 * UNIBUS error while in segment one
 */
buserr3:
	mov	BUSERR,brg
	br	errterm3
/*
 * Receive-buffer error while in segment one
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
	mov	termcode,mar
	mov	%termcode,%mar
	mov	brg,mem
	mov	reports,mar
	mov	%reports,%mar
	mov	mem,r0
	mov	ERROUT,brg
	or	brg,r0,mem
	mov	%errterm,brg
	mov	brg,pcr
	jmp	errterm

/*
 * Bridge to dispatcher loop in segment one
 */
dispret3:
	mov	%disp,brg
	mov	brg,pcr
	jmp	disp
/*
 * *** end of segment one ***
 */
endseg3:
