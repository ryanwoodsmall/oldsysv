/*	@(#)ckdn.c	1.1	*/
extern wait(), ckipl(), intrflg;
#define CRQ 1
#define DPR 2
#define MINAB 4
#define MAINT 8
#define PND 0x10
#define DSS 0x20
#define INAB 0x40
#define DONE 0x80
#define DLO 0x1000
#define ACR 0x4000
#define PWR 0x8000
struct device {
	short dn_csr[4];
};
ckdn(dn)
struct device *dn;
{
	int i,j,ipl,oipl,xipl;

	printf(" (%x)",dn);
#ifdef STANDALONE
	clruba();
	dn->dn_csr[0] = 0;
	for(i = 0; i < 4; i++){
		if(!(dn->dn_csr[i] & PWR)) break;
	}
	if(i ==  4){
		printf(" no acu");
		return(1);
	}
	dn->dn_csr[0] = 0;
	if(dn->dn_csr[i] & (DSS | DLO)){
		printf(" bad status");
	}
	dn->dn_csr[0] |= MINAB;
	dn->dn_csr[i] = MINAB | INAB | CRQ;
	intrflg = 0;
	oipl = getipl();
	for(ipl = 0x16;(ipl > 0x12) && !intrflg; ipl--){
		xipl = splx(ipl);
		for( i = 0; (i <= 1000) && !intrflg; i++);
	}
	xipl = splx(oipl);
	if(!intrflg){
		printf(" ___");
		intrflg = 1;
	}
	else printf("Vector-%o IPL-%d",intrflg,ipl - 0xf +1);
	dn->dn_csr[i] = 0;
	clruba();
#endif
	return(intrflg);
}
