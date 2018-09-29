/*	opsw.s 1.4 of 2/13/83
	@(#)opsw.s	1.4
 */

jumpsw:
	br	jump		/* 0 - JMP */
	br	jumpeqz		/* 1 - JMPEQZ */
	br	jumpnez		/* 2 - JMPNEZ */
	br	pushj		/* 3 - CALL */
	br	illegal		/* 4 - unused */
	br	illegal		/* 5 - unused */
	br	illegal		/* 6 - unused */
	br	illegal		/* 7 - unused */
opsw:
	br	illegal		/* unused op code */
	br	movd		/* MOV */
	br	andd		/* AND */
	br	ord		/* OR */
	br	xord		/* XOR */
	br	addd		/* ADD */
	br	subd		/* SUB */
	br	lshd		/* LSH */
	br	rshd		/* RSH */
	br	bicd		/* BIC */
	br	tsteqld		/* TSTEQL */
	br	tstneqd		/* TSTNEQ */
	br	tstgtrd		/* TSTGTR */
	br	tstlssd		/* TSTLSS */
	br	tstgeqd		/* TSTGEQ */
	br	tstleqd		/* TSTLEQ */
	br	movm		/* MOVM */
	br	andm		/* ANDM */
	br	orm		/* ORM */
	br	xorm		/* XORM */
	br	addm		/* ADDM */
	br	subm		/* SUBM */
	br	lshm		/* LSHM */
	br	rshm		/* RSHM */
	br	bicm		/* BICM */
	br	atoed		/* ATOE */
	br	etoad		/* ETOA */
	br	crc16		/* CRC16 */
	br	getbyte		/* GETBYTE */
	br	rcv		/* RCV */
	br	putbyte		/* PUTBYTE */
	br	xmt		/* XMT */
	br	getxfrm		/* GETXFRM */
	br	rtnxfrm		/* RTNXFRM */
	br	xmtfrm		/* XMTFRM */
	br	setctli		/* SETCTL */
	br	exit		/* EXIT */
	br	testop		/* TESTOP */
	br	timeout		/* TIMEOUT */
	br	timerd		/* TIMER */
	br	trace1		/* TRACE1 */
	br	trace2		/* TRACE2 */
	br	clrm		/* CLR */
	br	incm		/* INC */
	br	decm		/* DEC */
	br	popj		/* RETURN */
last_arrop:
	br	movi		/* MOVI */
	br	andi		/* ANDI */
	br	ori		/* ORI */
	br	xori		/* XORI */
	br	addi		/* ADDI */
	br	subi		/* SUBI */
	br	lshi		/* LSHI */
	br	rshi		/* RSHI */
	br	bici		/* BICI */
	br	tsteqli		/* TSTEQLI */
	br	tstneqi		/* TSTNEQI */
	br	tstgtri		/* TSTGTRI */
	br	tstlssi		/* TSTLSSI */
	br	tstgeqi		/* TSTGEQI */
	br	tstleqi		/* TSTLEQI */
	br	atoei		/* ATOEI */
	br	etoai		/* ETOAI */
	br	crc16i		/* CRC16I */
	br	putbytei	/* PUTBYTEI */
	br	xmti		/* XMTI */
	br	exiti		/* EXITI */
	br	timeouti		/* TIMEOUTI */
	br	timeri		/* TIMERI */
	br	trace1i		/* TRACE1I */
	br	trace2i		/* TRACE2I */
	br	popji		/* RETURNI */
	br	xeomi		/* XEOMI */
	br	xsomi		/* XSOMI */
	br	rsomi		/* RSOMI */
	br	rtnrbuf		/* RTNRBUF */
	br	rtnxbuf		/* RTNXBUF */
	br	getxbuf		/* GETXBUF */
	br	getrbuf		/* GETRBUF */
	br	illegal		/* RSRBUF */
	br	illegal		/* RSXBUF */
	br	crcloc		/* CRCLOC */
	br	dsrwait		/* DSRWAIT */
	br	rcvfrm		/* RCVFRM */
	br	rsrfrm		/* RSRFRM */
	br	rtnrfrm		/* RTNRFRM */
	br	xmtbusy		/* XMTBUSY */
	br	xmtctl		/* XMTCTL */
	br	rsxmtq		/* RSXMTQ */
	br	abtxfrm		/* ABTXFRM */
	br	norbuf		/* NORBUF */
	br	getcmd		/* GETCMD */
	br	rtnrpt		/* RTNRPT */
	br	snap		/* SNAP */
	br	pause		/* PAUSE */
	br	getopt		/* GETOPT */
	br	testopi		/* TESTOPI */
	br	getxfrmi	/* GETXFRMI */
	br	xmtfrmi		/* XMTFRMI */
	br	rtnxfrmi	/* RTNXFRMI */
	br	lda		/* LDA */
	br	ldbyte		/* LDBYTE */
lastop:
