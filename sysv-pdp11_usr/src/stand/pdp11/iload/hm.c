/*	@(#)hm.c	1.2	*/
struct	{	char lobyte, hibyte;};

#define	HM	0176700
#define NSECT	32
#define NTRAC	19
hmwrite(a,b,c,d)
char *a, *b;
{
	struct	{ int cs1, wc, ba, da, cs2, ds, er1, as,
		la, db, mr, dt, sn, of, dc, cc;};
	register i, st, unit;
	long	p;

	unit = d&07;
	HM->cs2 = 040;
	HM->cs2 = unit;
	if ((HM->ds&0100)==0) {
		HM->cs1 = 021;
		HM->of = 010000;
	}
	for(i=0;i<10;i++) {
		p = (unsigned)a;
		HM->dc = p/(NSECT*NTRAC);
		st = p%(NSECT*NTRAC);
		HM->da = ((st/NSECT)<<8)|(st%NSECT);
		HM->wc = -((c>>1)&077777);
		HM->ba = b;
		HM->cs1 = 061;
		while(HM->cs1.lobyte>=0);
		if((HM->cs1&040000)==0)
			return(0);
		printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
		HM->cs2 = 040;
		while(HM->cs1.lobyte>=0);
		HM->cs2 = unit;
	}
	return(1);
}
