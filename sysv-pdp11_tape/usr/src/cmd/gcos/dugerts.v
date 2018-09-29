#	@(#)dugerts.v	1.2
#	3.0 SID #	1.1
define(ERROR0,0)
define(ERROR1,1)
define(ERROR2,2)
define(ERROR3,3)
define(ERROR4,4)
define(ERROR5,5)
define(ERROR6,6)
define(ERROR7,7)
define(ERROR8,8)
define(ERROR9,9)
define(ERROR10,10)
define(ERROR11,11)
define(ERROR12,12)
define(ERROR13,13)
define(ERROR14,14)
define(ERROR15,15)
define(ERROR16,16)
define(ERROR17,17)
define(ERROR18,18)
define(ERROR19,19)
define(BIT8,0200)
define(A,0301)
define(SOH,01)
define(STX,02)
define(ETX,03)
define(SYNC,026)
define(pxmt,	z = $1;
		if(testop(z) == 0)
			z ^= BIT8;
		xmt(z);)
define(prcv,	rcv($1);
		if(testop($1) == 0)
			$1 |= BIT8;
		else
			$1 &= ~BIT8;)
array xbuf[3];
array rbuf[3];

function gerts()
dsrwait();
	repeat{
		while(getxbuf(xbuf));
		xmtblk();
		rtnxbuf(xbuf);
		while(getrbuf(rbuf));
		recblk();
		rtnrbuf(rbuf);
	}
end

function xmtblk()
	repeat{
		if(get(c))
			exit(ERROR1);
	}until(c != SYNC)
	xsom(SYNC);
	while(c != SYNC){
		pxmt(c);
		if(get(c))
			exit(ERROR2);
	}
	xeom(A);
end

function recblk()
	if(timeout(255))
		exit(ERROR3);
	rsom(SYNC);
	repeat{
		prcv(c);
		if(c == 0)
			c |= BIT8;
		if(put(c))
			exit(ERROR4);
	}until(c == ETX)
	prcv(c);
	if(put(c))
		exit(ERROR5);
end
