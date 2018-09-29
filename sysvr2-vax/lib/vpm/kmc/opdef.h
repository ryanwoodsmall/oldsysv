/*	opdef.h 1.4 of 2/13/83
	@(#)opdef.h	1.4
 */

#define	JMP	0200
#define	JMPEQZ	0220
#define	JMPNEZ	0240
#define	CALL	0260
#define	MOV	1
#define MOVX	1 + 192
#define	AND	2
#define ANDX	2 + 192
#define	OR	3
#define ORX	3 + 192
#define	XOR	4
#define XORX	4 + 192
#define	ADD	5
#define ADDX	5 + 192
#define	SUB	6
#define SUBX	6 + 192
#define	LSH	7
#define LSHX	7 + 192
#define	RSH	8
#define RSHX	8 + 192
#define	BIC	9
#define BICX	9 + 192
#define	TSTEQL	10
#define TSTEQLX	10 + 192
#define	TSTNEQ	11
#define TSTNEQX	11 + 192
#define	TSTGTR	12
#define TSTGTRX	12 + 192
#define	TSTLSS	13
#define TSTLSSX	13 + 192
#define	TSTGEQ	14
#define TSTGEQX	14 + 192
#define	TSTLEQ	15
#define TSTLEQX	15 + 192
#define	MOVM	16
#define MOVMX	16 + 192
#define	ANDM	17
#define ANDMX	17 + 192
#define	ORM	18
#define ORMX	18 + 192
#define	XORM	19
#define XORMX	19 + 192
#define	ADDM	20
#define ADDMX	20 + 192
#define	SUBM	21
#define SUBMX	21 + 192
#define	LSHM	22
#define LSHMX	22 + 192
#define	RSHM	23
#define RSHMX	23 + 192
#define	BICM	24
#define BICMX	24 + 192
#define	ATOE	25
#define ATOEX	25 + 192
#define	ETOA	26
#define ETOAX	26 + 192
#define	CRC16	27
#define CRC16X	27 + 192
#define	GETBYTE	28
#define GETBYTEX	28 + 192
#define	RCV	29
#define RCVX	29 + 192
#define	PUTBYTE	30
#define PUTBYTEX	30 + 192
#define	XMT	31
#define XMTX	31 + 192
#define	GETXFRM	32
#define GETXFRMX	32 + 192
#define	RTNXFRM	33
#define RTNXFRMX	33 + 192
#define	XMTFRM	34
#define XMTFRMX	34 + 192
#define	SETCTL	35
#define SETCTLX	35 + 192
#define	HALT	36
#define HALTX	36 + 192
#define	TESTOP	37
#define TESTOPX	37 + 192
#define	TIMEOUT	38
#define TIMEOUTX	38 + 192
#define	TIMER	39
#define TIMERX	39 + 192
#define	TRACE1	40
#define TRACE1X	40 + 192
#define	TRACE2	41
#define TRACE2X	41 + 192
#define	CLR	42
#define CLRX	42 + 192
#define	INC	43
#define INCX	43 + 192
#define	DEC	44
#define DECX	44 + 192
#define	RETURN	45
#define RETURNX	45 + 192
#define	MOVI	46
#define	ANDI	47
#define	ORI	48
#define	XORI	49
#define	ADDI	50
#define	SUBI	51
#define	LSHI	52
#define	RSHI	53
#define	BICI	54
#define	TSTEQLI	55
#define	TSTNEQI	56
#define	TSTGTRI	57
#define	TSTLSSI	58
#define	TSTGEQI	59
#define	TSTLEQI	60
#define	ATOEI	61
#define	ETOAI	62
#define	CRC16I	63
#define	PUTBYTEI	64
#define	XMTI	65
#define	HALTI	66
#define	TIMEOUTI	67
#define	TIMERI	68
#define	TRACE1I	69
#define	TRACE2I	70
#define	RETURNI	71
#define	XEOMI	72
#define	XSOMI	73
#define	RSOMI	74
#define	RTNRBUF	75
#define	RTNXBUF	76
#define	GETXBUF	77
#define	GETRBUF	78
#define	RSRBUF	79
#define	RSXBUF	80
#define	CRCLOC	81
#define	DSRWAIT	82
#define	RCVFRM	83
#define	RSRFRM	84
#define	RTNRFRM	85
#define	XMTBUSY	86
#define	XMTCTL	87
#define	RSXMTQ	88
#define	ABTXFRM	89
#define	NORBUF	90
#define	GETCMD	91
#define	RTNRPT	92
#define	SNAP	93
#define	PAUSE	94
#define	GETOPT	95
#define TESTOPI	96
#define GETXFRMI	97
#define XMTFRMI	98
#define RTNXFRMI	99
#define LDA	100
#define LDBYTE	101
