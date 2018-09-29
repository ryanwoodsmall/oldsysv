/*	@(#)ckdz.c	1.1	*/
extern int intrflg, clruba();
ckdz(addr)
unsigned short *addr;
#define DZ_CLR 0x10
#define DZ_SAE 0x1000
#define DZ_RIE 0x40
#define DZ_TIE 0x4000
#define DZ_MAINT 0x8
#define DZ_MSCE 0x20
#define DZ_TXRDY 0x8000
#define DZ_RXON 0x1000
#define DZ_9600 0xe00
#define DZ_TXL0 1
#define DZ_CS8 0x18
{
	struct dzr {
		short csr, lpr, tcr, tdr;
	} *dz;
	int i,j,ipl,oipl,xipl;
	printf(" (%x)",addr);
#ifdef STANDALONE
	dz = (struct dzr*)addr;
	dz->csr = DZ_CLR;
				/* wait for end of clear cycle */
	i = 100;
	while((dz->csr & DZ_CLR) && (--i > 0));
	if(i <= 0){
		printf("\n\t\tDevice inoperable or not DZ\n\t\t regs:");
		goto dzdmp;
	}
	dz->csr = DZ_MAINT;
	if((dz->csr | dz->lpr | dz->tcr | dz->tdr) == 0)
		return(0);
/*	look for XMIT interrupt vector */
	clruba();
	intrflg = 0;
	printf("\n\t\t\tXMIT ");
	dz->csr = DZ_MSCE | DZ_TIE | DZ_RIE;
	dz->lpr = DZ_RXON | DZ_9600 | DZ_CS8;
	dz->tcr = 0x101;	/* line 0 enable */
	oipl = getipl();
	for(ipl = 0x16;(ipl > 0x12) && !intrflg; ipl--){
		xipl = splx(ipl);
		for( i = 0; (i <= 1000) && !intrflg; i++);
	}
	xipl = splx(oipl);
	if(i >= 50){
		printf(" ___");
		intrflg = 1;
	}
	else printf("Vector-%o IPL-%d",intrflg,ipl - 0xf +1);

/*	look for RCVR interrupt vector */
	printf("\n\t\t\tRCVR ");
	clruba();
	intrflg = 0;
	dz->csr = DZ_MSCE | DZ_RIE | DZ_MAINT;
	dz->lpr = DZ_RXON | DZ_9600 | DZ_CS8;
	dz->tcr = 0x101;	/* line 0 enable */
	for(i = 0;(i <= 20) && !intrflg; i++) {
		j = 0;
		while(!(dz->csr & DZ_TXRDY))
			if(j++ > 1000){
				printf("\t\t\nLoopback test failed");
				goto dzdmp;
			}
		dz->tdr = 'T';
		for(ipl = 0x16; (ipl>0xf) && !intrflg; ipl--){
			xipl = splx(ipl);
		}
		xipl = splx(oipl);
	}
	splx(oipl);
	if(!intrflg){
		printf(" ___");
		intrflg = 1;
	}
	else{
		printf("Vector-%o ",intrflg);
		dz->csr = DZ_CLR;
		return(intrflg);  /* return receiver vector address */
	}
#endif
	return(1);
dzdmp:
	printf("  csr: %4x",dz->csr & 0xffff);
	printf("  rbuf: %x",dz->lpr & 0xffff);
	printf("  tcr: %x",dz->tcr & 0xffff);
	printf("  msr: %x\n",dz->tdr & 0xffff);
	return(1);
}
