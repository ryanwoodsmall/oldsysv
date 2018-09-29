/*	@(#)mmtest.c	1.1	*/
	/* Address registers */
#define KISAR0	((struct I *)0172340)
#define KDSAR0	((struct I *)0172360)
#define SISAR0	((struct I *)0172240)
#define SDSAR0	((struct I *)0172260)
#define UISAR0	((struct I *)0177640)
#define UDSAR0	((struct I *)0177660)

	/* Descriptor registers */

#define KISDR0	((struct I *)0172300)
#define KDSDR0	((struct I *)0172320)
#define SISDR0	((struct I *)0172200)
#define SDSDR0	((struct I *)0172220)
#define UISDR0	((struct I *)0177600)
#define UDSDR0	((struct I *)0177620)
#define PDESCRIP	077406
#define PDMASK		077417
#define PDLIMIT		0100000
#define destest(x)	desreg++; rtest(x); desreg = 0

	/* Memory mgmt control register stuff */

#define MMR0	((struct I *)0177572)
#define MMR3	((struct I *)0172516)
#define KDSPACE		04
#define SDSPACE		02
#define UDSPACE		01
#define ALLD		07
#define BIT22		020
#define UBMAP		040
#define MMOFF		0100
#define mmoff()		MMR0->integ  = 0; MMR3->integ = 0
#define mmon()		MMR0->integ++
#define setmmr3(x)	MMR3->integ = x & 077

#define CSW	((struct I *)0177570)
#define KL	((struct cons *)0177560)

#ifdef CPU45
#define IOPAGE	07600
#endif
#ifndef CPU45
#define IOPAGE	0177600
#endif

#define INCR		1		/* Increment in memory blocks	*/
#define ITER		10		/* Number of tries		*/

struct I {
	int	integ;
};

struct M {
	int	r[1];
};

struct cons {
	int	rsr;
	int	rbr;
	int	xsr;
	int	xbr;
};

int	desreg	= 0;

int	anyerrs	= 0;

main()
{
	printf("Memory Management Unit Test\n\n");
	init(KISAR0,KISDR0);
	printf("\nTest supervisor address regs\n\n");
	rtest(SISAR0);
	printf("\nTest supervisor descriptor regs\n\n");	/*testdes*/
	destest(SISDR0);	/*testdes*/
	printf("\nTest user address regs\n\n");
	rtest(UISAR0);
	printf("\nTest user descriptor regs\n\n");	/*testdes*/
	destest(UISDR0);	/*testdes*/
	init(SISAR0,SISDR0);
	printf("\nTest kernel address regs\n\n");
	runsuper(KISAR0);
	printf("\nTest kernel descriptor regs\n\n");	/*testdes*/
	desreg++;	/*testdes*/
	runsuper(KISDR0);	/*testdes*/
	desreg = 0;	/*testdes*/
	if (anyerrs)
			printf("\n\nErrors have been detected.\n");
		else
			printf("\n\nNo errors detected.\n");
	printf("\nDone\n\n");
}
init(ap,dp)
register struct M *ap, *dp;
{
	register *ip;

	mmoff();
	ap->r[0] = 0;
	ap->r[8] = 0;
	ap->r[7] = IOPAGE;
	ap->r[15] = IOPAGE;
	for(ip = (int *)dp; ip < &dp->r[16];)
		*ip++ = PDESCRIP;
}
rtest(base)
register *base;
{
	run("Mem mgmt off",base,MMOFF);
	run("Mem mgmt on - 18 bit",base,0);
	run("Mem mgmt on - 18 bit I and D",base,ALLD);
#ifndef CPU45
	run("Mem mgmt on - 22 bit",base,BIT22);
	run("Mem mgmt on - 22 bit unibus map",base,BIT22|UBMAP); /*testubmap*/
	run("Mem mgmt on - 22 bit I and D",base,BIT22|ALLD);
	run("Mem mgmt on - 22 bit I and D unibus map",base,BIT22|ALLD|UBMAP); /*testubmap*/
#endif
}
run(name,base,mmr3)
char *name;
register mmr3;
int *base;
{
	register unsigned val, limit;

	mmoff();
	printf("\n%s\n\n",name);
	setmmr3(mmr3);
	if((mmr3 & MMOFF) == 0)
		mmon();
	limit = desreg ? PDLIMIT : IOPAGE;
	for(val = 0; val < limit; val = val + INCR) {
/*		if((val&0777) == 0)	/*printon*/
/*			printf("test value base %o\n",val);	/*printon*/
		stuff(base,desreg ? val&PDMASK : val);
	}
}
stuff(base,val)
struct M *base;
register val;
{
	register *ip, *last;
	int k, err, cnt ;

	last = &base->r[16];
	err = 0;
	CSW->integ = val;
	for(k = ITER; k > 0; k--) {
		for(ip = (int *)base; ip < last; )
			*ip++ = val;
		for(ip = (int *)base; ip < last; ip++)
			if(*ip != val) {
				cnt = 10 - k ;
				error(ip-(int *)base,*ip,val, cnt);
				err++;
			}
		if(err)
			return;
	}
}
error(i,n,val,cnt)
{
	anyerrs = 1;
	printf("%s reg ",i < 8 ? "instruction" : "data");
	printf("%d ",i < 8 ? i : i-8);
	printf("read %o after writing %o  (after %d retries)\n",n,val,cnt);
}
printf(fmt,x1)
char fmt[];
{
	register char *s;
	register *adx, c;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if((c == 'd')&&(*adx < 0)){
		*adx = -*adx;
		putchar('-');
	}
	if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10);
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	}
	else if(c == 'c')
		putchar(*adx);
	adx++;
	goto loop;
}
printn(n, b)
{
	register a;

	if(a = ldiv(n, b))
		printn(a, b);
	putchar(lrem(n, b) + '0');
}
putchar(c)
{
	register rc, s, timo;

	rc = c;
	if(CSW->integ == 0)
		return;
	timo = 30000;
	while((KL->xsr&0200)==0 && --timo!=0);
	if(rc == 0)
		return;
	s = KL->xsr;
	KL->xsr = 0;
	KL->xbr = rc;
	if(rc == '\n') {
		putchar('\r');
		putchar(0177);
		putchar(0177);
	}
	putchar(0);
	KL->xsr = s;
}
