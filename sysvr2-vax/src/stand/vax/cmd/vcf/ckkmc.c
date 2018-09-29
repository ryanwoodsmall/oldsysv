/*	@(#)ckkmc.c	1.1	*/
extern int intrflg, clruba(), ckipl(), wait();
unsigned short *addr;
#define sel(X) addr->csr.sel_[X/2]
#define bsel(X) addr->csr.bsel_[X]
#define MAR sel(4)	/* Maintenance Address Reg. */
#define MDIR sel(6)	/* Maintenance Data and Instruction Reg */
#define bsel1 bsel(1)
#define bsel2 bsel(2)
#define BASEA sel(4)
#define COUNT sel(6)
#define CNT(X) ((short)(((int)buff >> 2) & 0xc)) + X
#define RUN 0x80
#define MCLR 0x40
#define CWRT 0x20
#define LUB 0x10
#define LUA 8
#define ROMO 4
#define ROMI 2
#define STEP 1
#define IN 4
#define OUT 0
#define RDO 0x80
#define RDI 0x80
#define RQI 0x20
#define IEI 0x40
#define BACC 0
#define CTLI 1
#define BASEI 3
#define READ bsel(2)
#define WRIT bsel(0)

ckkmc(addr)
struct {
	union {
		char bsel_[8];
		short sel_[4];
	} csr;
	char	lur[8];
	char	reg[16];
	char	io[8];
	char	npr[2];
	char	brg;
	char	mem;
	char	fill[4];
} *addr;
{
	int i,j,ipl,oipl,xipl;
	char *buff = "test pattern";
#define NCHAR 12

	printf(" (%x)",addr);
#ifdef STANDALONE
	for(i = 0; i < 4; i++) addr->csr.sel_[i] = 0;
	i = 0;
	while(READ & RDO){
		j = bsel(4);
		j = bsel(6);	/* unload data port) */
		READ = 0;
		wait(20);
		if(++i > 100) break;
	}
	bsel1 = MCLR;
	wait(100);
	if(bsel1 == RUN){
		printf(" DMC");
		clruba();
		bsel1 = RUN | LUB;
		WRIT = RQI | IN;
		READ = IEI;
		printf("\n\t\tRCVR ");
		i = 100;
		while(i-- > 0);
		if(!(WRIT & RDI)){
			printf("not ready\n\t\t\t");
			goto kmcdmp;
		}
		BASEA = (int)buff;
		COUNT = CNT(NCHAR);
		WRIT = 0;	/* end of transfer, look for inerrupt */
	}
	else {
		printf(" KMC");
	}

	intrflg = 0;
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
	clruba();
#endif
	return(intrflg);

kmcdmp:
	printf("\t sel0: %4x",sel(0) & 0xffff);
	printf("  sel2: %4x",sel(2) & 0xffff);
	printf("  sel4: %4x",sel(4) & 0xffff);
	printf("  sel6: %4x\n",sel(6) & 0xffff);
	return(1);
}
