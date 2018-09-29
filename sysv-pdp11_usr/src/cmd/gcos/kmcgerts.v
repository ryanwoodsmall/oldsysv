#	@(#)kmcgerts.v	1.2
#	3.0 SID #	1.2
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
define(BIT8,0200)
define(SOH,01)
define(STX,02)
define(ETX,03)
define(SYNC,026)
define(SELFC,0103)
define(SEQ1,0101)
define(SEQ2,0102)
define(CDUM,0100)
define(OKRET,1)
define(ERRET,0-1)
define(TIMEOUT,0-2)
define(CKRET,0-3)

define(error,	z = $1;
		if(put(z))
			exit(ERROR1);
		goto gerror;)
define(getck,	if(get($1))
			exit(ERROR2);
		xck ^= $1;)
define(putck,	if(put($1))
			exit(ERROR3);
		rck ^= $1;)
define(pxmt,	z = $1;
		if(testop(z) == 0)
			z ^= BIT8;
		xmt(z);)
define(SCHAR,	bcc ^= $1;
		pxmt($1);)
define(prcv,	rcv($1);
		if(testop($1) == 0){
			goto reject;
		}
		else
			$1 &= ~BIT8;)
define(CCHAR,	prcv($1);
		if($1 == 0){
			goto reject;
		}
		bcc ^= $1;)
array xbuf[3];
array rbuf[3];

function gerts()
dsrwait();
repeat{

	for(errcnt = 0; errcnt < 5; ++errcnt){
	
		while(getxbuf(xbuf)) ;
		while(getrbuf(rbuf)) ;
		xck = 0;
		getck(ofc);
		getck(ooc);
		if(errcnt)
			ooc = roc;
		roc = ooc & 0707;
		if(ofc == SELFC)
			osc = SEQ1;
		bcc = 0;
		if(timeout(100)){
			error(TIMEOUT);
		}
		xsom(SYNC);
		pxmt(SOH);
		SCHAR(ofc);
		SCHAR(osc);
		SCHAR(CDUM);
		SCHAR(ooc);
		SCHAR(CDUM);
		if(ofc == SELFC){
			SCHAR(0111);
		}
		repeat{
			getck(c);
		}until(c == STX)
		SCHAR(STX);
		repeat{
			getck(c);
			SCHAR(c);
		}until(c == ETX)
		if(get(c) == 0)
			if(xck != c){
				error(CKRET);
			}
		pxmt(bcc);
		xeom(CDUM);
		timeout(0);

		if(timeout(255)){
			error(TIMEOUT);
		}
		rsom(SYNC);
		prcv(c);
		if(c != SOH)  goto reject;
		bcc = 0;
		rck = 0;
		CCHAR(ifc);
		putck(ifc);
		CCHAR(isc);
		if(isc != SEQ1)
			if(isc != SEQ2)
				goto reject;
		CCHAR(c);
		if(c != CDUM)  goto reject;
		CCHAR(ioc);
		putck(ioc);
		CCHAR(c);
		if(c != CDUM)  goto reject;
		CCHAR(c);
		if(c != STX)  goto reject;
		putck(STX);
		dflg = 0-1;
		repeat{
			++dflg;
			CCHAR(c);
			putck(c);
		}until(c == ETX)
		prcv(c);
		if(bcc != c)  goto reject;
		timeout(0);
		if(osc != isc)
			dflg = 0;
		if(dflg == 0)
			if(ioc & 070)
				goto accept;
		if(ioc == 0106)
			goto accept;
		if(++osc > SEQ2)
			osc = SEQ1;
		if(put(rck))
			exit(ERROR3);
		if(put(OKRET))
			exit(ERROR3);
		break;

reject:
		roc = ooc | 010;
accept:
		timeout(0);
		z = ERRET;
		if(put(z))
			exit(ERROR3);
	}



gerror:
	timeout(0);
	rtnxbuf(xbuf);
	rtnrbuf(rbuf);
}

end
