/*	@(#)hthp.c	1.1	*/
#
struct	{	char lobyte, hibyte;};
#define	HT	0172440
htread(a,b,c,d)
char *a,*b;
{
	struct	{ int cs1, wc, ba, fc, cs2, ds, er1, as,
		ck, db, mr, dt, sn, tc;};
	static int tapa 0;
	register i, unit;

	unit = d&07;
	HT->cs2 = 040;
	HT->tc = 01300|unit;
	if (a==0&&b==0&&c==0) {
		HT->cs1 = 07;
		tapa = 0;
	}
	if (a!=tapa) {
		if (a>tapa) {
			HT->fc = a-tapa;
			HT->cs1 = 031;
		} else {
			HT->fc = tapa-a;
			HT->cs1 = 033;
		}
		tapa = a;
	}
	while(HT->cs1.lobyte>=0);
	if (c==0)
		return;
	while(HT->ds&020000);
	for(i=0;i<10;i++) {
		HT->wc = -((c>>1)&077777);
		HT->ba = b;
		HT->cs1 = 071;
		while(HT->cs1.lobyte>=0);
		if((HT->cs1&040000)==0) {
			tapa++;
			return(0);
		}
		printf("tape error at record %d\n",a);
		HT->cs2 = 040;
		HT->tc = 1300|unit;
		HT->fc = -1;
		HT->cs1 = 033;
		while(HT->cs1.lobyte>=0);
	}
	return(1);
}

#define	HP	0176700
hpwrite(a,b,c,d)
char *a, *b;
{
	struct	{ int cs1, wc, ba, da, cs2, ds, er1, as,
		la, db, mr, dt, sn, of, dc, cc;};
	register i, st, unit;
	long	p;

	unit = d&07;
	HP->cs2 = 040;
	HP->cs2 = unit;
	if ((HP->ds&0100)==0) {
		HP->cs1 = 021;
		HP->of = 010000;
	}
	for(i=0;i<10;i++) {
		p = (unsigned)a;
		HP->dc = p/418;
		st = p%418;
		HP->da = ((st/22)<<8)|(st%22);
		HP->wc = -((c>>1)&077777);
		HP->ba = b;
		HP->cs1 = 061;
		while(HP->cs1.lobyte>=0);
		if((HP->cs1&040000)==0)
			return(0);
		printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
		HP->cs2 = 040;
		while(HP->cs1.lobyte>=0);
		HP->cs2 = unit;
	}
	return(1);
}
