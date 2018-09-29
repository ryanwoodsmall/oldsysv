/*	@(#)ckipl.c	1.1	*/
extern int intrflg;
ckipl()
{
	int i,j,ipl,oipl,xipl;

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
}
