/*	trace.h 1.1 of 7/21/81
	@(#)trace.h	1.1
 */

/*
 * Per trace structure
 */
struct trace {
	struct 	clist tr_outq;
	short	tr_state;
	short	tr_chbits;
	short	tr_rcnt;
	unsigned char	tr_chno;
	char	tr_ct;
};
