/*     rjeproto.c 1.2 of 3/17/82     */
/*     @(#)rjeproto.c	1.2                */

#define INIT	020
#define SENTACK	021
#define SENTBLK	022
#define DLE	0x10
#define ENQ	0x2d
#define ETB	0x26
#define SOH	0x01
#define NAK	0x3d
#define NULL	0x00
#define PAD	0xff
#define X70	0x70
#define STX	0x02
#define SYNC	0x32
#define CKSEQ	0x80
#define NOCK	0x90
#define RSSEQ	0xa0
#define WAITBIT	0x40
#define INITFLT	1
#define ERRORS	2
#define R_ACK	1
#define R_WAIT	2
#define R_ENQ	3
#define R_ERBLK	5
#define R_NAK	6
#define R_OKBLK	7
#define R_SEQER	010
#define R_ERROR	011
#define TIMEOUT	012
#define X_ENQ	013
#define X_NAK	014
#define X_ACK	015
#define X_OKBLK	016
#define X_ERBLK	017
#define xmtctl(X,Y)	{c1 = X; c2 = Y;	xmtctl1();}
#define xmtcrc(X)	{crc16(X); xmt(X);}
#define incmod(X,Y)	{++X; if(X >= Y)X = 0;}
char	crc[2];
char	rtype[3];
char	xtype[3];
char	xfcs0, xfcs1, rfcs0, rfcs1;
char	byte, rcvs, xbcb, rbcb, xseq;
char	c1, c2, count, punch, rseq, actseq;

short	wait, exitval, state, seqst;
short	ct0, ct1;

hasp()
{
	trace(0);
	while(getxbuf(xtype) != 0);
	state = INIT;
	xbcb = 0xa0;
	xfcs0 = 0x8f;
	xfcs1 = 0xcf;
	xmtctl(SOH,ENQ);
	trace(X_ENQ);
	for(;;) {
		rcvs = rec();
		timeout(0);
		trace(rcvs);
		switch(state){
		case INIT:
			switch(rcvs){
			case R_ACK:
				xmtrsp();
				break;
			default:
				if(ct0<5){
					++ct0;
					xmtctl(SOH,ENQ);
					trace(X_ENQ);
					state = INIT;
				}else{
					exitval = INITFLT;
					abort();
				}
			}
			break;
		case SENTACK:
			switch(rcvs){
			case R_ACK:
				ct1 = 0;
				wait = 0;
				seqst = R_ACK;
				xmtrsp();
				break;
			case R_OKBLK:
				ct1 = 0;
				rseqck();
				seqst = R_OKBLK;
				break;
			case R_ERBLK:
				ct1 = 0;
				xmtctl(NAK,NULL);
				trace(X_NAK);
				state = SENTACK;
				break;
			case R_NAK:
				ct1 = 0;
				seqst = R_NAK;
				xmtctl(DLE,X70);
				trace(X_ACK);
				state = SENTACK;
				break;
			default:
				++ct1;
				if(ct1 >= 6){
					exitval = ERRORS;
					abort();
				}else{
					xmtctl(NAK,NULL);
					trace(X_NAK);
					state = SENTACK;
				}
			}
			break;
		case SENTBLK:
			switch(rcvs){
			case R_ACK:
				ct1 = 0;
				seqst = R_ACK;
				xbcb = 0x80|xseq;
				incmod(xseq,16);
				rtnxbuf(xtype);
				xmtrsp();
				break;
			case R_OKBLK:
				ct1 = 0;
				xbcb = 0x80|xseq;
				incmod(xseq,16);
				rtnxbuf(xtype);
				rseqck();
				seqst = R_OKBLK;
				break;
			case R_ERBLK:
				ct1 = 0;
				xmtctl(NAK,NULL);
				trace(X_NAK);
				state = SENTBLK;
				break;
			case R_NAK:
				ct1 = 0;
				seqst = R_NAK;
				getxbuf(xtype);
				xmtblk();
				state = SENTBLK;
				break;
			default:
				++ct1;
				if(ct1 >= 6){
					exitval = ERRORS;
					abort();
				}else{
					xmtctl(NAK,NULL);
					trace(X_NAK);
					state = SENTBLK;
				}
			}
		}
	}
}
xmtblk()
{
	crcloc(crc);
	xsom(SYNC);
	xmt(DLE);
	xmt(STX);
	xmtcrc(xbcb);
	xmtcrc(xfcs0);
	xmtcrc(xfcs1);
	while(get(&byte) == 0){
		if(byte == DLE)
			xmt(DLE);
		xmtcrc(byte);
	}
	xmt(DLE);
	xmtcrc(ETB);
	xmt(crc[0]);
	xmt(crc[1]);
	xeom(PAD);
	trace(X_OKBLK);
}
xmtdat()
{
	if(byte == DLE)
		xmt(DLE);
	xmtcrc(byte);
}
xmtctl1()
{
	xsom(SYNC);
	xmt(c1);
	if(c2 != NULL)
		xmt(c2);
	xeom(PAD);
}
rec()
{
	crcloc(crc);
	if(timeout(30))
		return(TIMEOUT);
	rsom(SYNC);
	rcv(&byte);
	switch(byte){
	case NAK:
		rcv(&byte);
		byte &= 017;
		if(byte == 017)
			return(R_NAK);
		else
			return(R_ERROR);
		break;
	case DLE:
		rcv(&byte);
		switch(byte){
		case X70:
			rcv(&byte);
			byte &= 017;
			if(byte == 017)
				return(R_ACK);
			else
				return(R_ERROR);
			break;
		case STX:
			while(getrbuf(rtype))
				;
			rbcb = rcvdat();
			put(0);
			rfcs0 = rcvdat();
			put(rfcs0);
			rfcs1 = rcvdat();
			put(rfcs1);
			for(;;) {
				byte = rcvdat();
				if((byte&0x05) == 0x05)
					punch = 1;
				else
					punch = 0;
				put(byte);
				if(byte == 0)
					break;
				byte = rcvdat();
				put(byte);
				for(;;) {
					byte = rcvdat();
					put(byte);
					if(byte == 0)
						break;
					switch(byte&0xc0) {
					case 0x40:
						continue;
						break;
					case 0x80:
						if(byte&0x20) {
							byte = rcvdat();
							if(!punch)
								byte = etoa(byte);
							put(byte);
						}
						break;
					case 0xc0:
						count = byte&0x3f;
						for(;count;--count) {
							byte = rcvdat();
							if(!punch)
								byte = etoa(byte);
							put(byte);
						}
					}
				}
			}
			byte = rcvdat();
			if(byte != ETB) {
				if(!punch) {
					/* trace(byte); */
					byte = rcvdat();
					/* trace(byte); */
					byte = rcvdat();
					/* trace(byte); */
					byte = rcvdat();
					/* trace(byte); */
				}
				if(byte != ETB)
					return(R_ERROR);
			}
			rcvcrc();
			rcvcrc();
			rcv(&byte);
			byte &= 017;
			if(byte != 017)
				return(R_ERROR);
			if(crc[0] != 0)
				return(R_ERBLK);
			if(crc[1] != 0)
				return(R_ERBLK);
			return(R_OKBLK);
			break;
		default:
			return(R_ERROR);
		}
		break;
	default:
		return(R_ERROR);
	}
}
rcvcrc()
{
	rcv(&byte);
	crc16(byte);
	return(byte);
}
rcvdat()
{
	for(;;) {
		rcv(&byte);
		if(byte != DLE) {
			crc16(byte);
			return(byte);
		} else {
			rcv(&byte);
			if(byte == SYNC)
				continue;
			crc16(byte);
			return(byte);
		}
	}
}
xmtserr()
{
	crcloc(crc);
	xsom(SYNC);
	xmt(DLE);
	xmt(STX);
	xmtcrc(actseq);
	xmtcrc(xfcs0);
	xmtcrc(xfcs1);
	xmtcrc(0xe0);
	xmtcrc(rseq);
	xmtcrc(0);
	xmtcrc(0);
	xmt(DLE);
	xmtcrc(ETB);
	xmt(crc[0]);
	xmt(crc[1]);
	xeom(PAD);
	trace(X_OKBLK);
	state = SENTACK;
}
xmtrsp()
{
	if(wait){
		xmtctl(DLE,X70);
		trace(X_ACK);
		state = SENTACK;
		return;
	}
	if(getxbuf(xtype)){
		xmtctl(DLE,X70);
		trace(X_ACK);
		state = SENTACK;
	}else{
		xmtblk();
		state = SENTBLK;
	}
}
rseqck()
{
	if(rfcs0&WAITBIT){
		wait = 1;
		trace(R_WAIT);
	}else{
		wait = 0;
	}
	switch(rbcb&0xf0){
		case CKSEQ:
			actseq = rbcb&017;
			if(actseq == rseq){
				incmod(rseq,16);
				rtnrbuf(rtype);
				xmtrsp();
			}else{
				if(seqst == R_NAK) {
					trace(0xff);
					xmtrsp();
				} else {
					trace(R_SEQER);
					xmtserr();
				}
			}
			break;
		case NOCK:
			rtnrbuf(rtype);
			xmtctl(DLE,X70);
			trace(X_ACK);
			state = SENTACK;
			return;
			break;
		case RSSEQ:
			rseq = rbcb&0x0f;
			rtnrbuf(rtype);
	}
}

abort()
{
	while(getrbuf(rtype))
		;
	put(exitval);
	rtnrbuf(rtype);
	while(1)
	  ;
}
