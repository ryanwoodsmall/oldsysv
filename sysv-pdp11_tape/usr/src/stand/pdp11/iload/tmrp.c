/*	@(#)tmrp.c	1.1	*/
#
struct {	char lobyte, hibyte;};
#define TM	0172520
tmread(a,b,c,d)
char *a, *b;
{
	struct	{ int ds, cs, bc, ba, db, rd;};
	static int tapa 0;
	register i, unit;

	unit = (d&07)<<8;
	if (a==0&&b==0&&c==0) {
		TM->cs = 060017|unit;
		tapa = 0;
	}
	if (a!=tapa) {
		if (a>tapa) {
			TM->bc = a-tapa;
			TM->cs = 060011|unit;
		} else {
			TM->bc = tapa-a;
			TM->cs = 060013|unit;
		}
		tapa = a;
	}
	while((TM->cs.lobyte>=0));
	if (c==0)
		return;
	TM->cs = unit;
	while(TM->ds&02);
	for(i=0;i<10;i++) {
		TM->bc = -c;
		TM->ba = b;
		TM->cs = 060003|unit;
		while(TM->cs.lobyte>=0);
		if(TM->cs>0) {
			tapa++;
			return(0);
		}
		printf("tape error at record %d\n",a);
		while(TM->ds&010);
		TM->bc = -1;
		TM->cs = 060013|unit;
		while((TM->cs.lobyte>=0));
	}
	return(1);
}

#define RP	0176710
rpwrite(a,b,c,d)
char *a, *b;
{
	struct	{ int ds, er, cs, wc, ba, ca, da;};
	register i, st, unit;
	long	p;

	unit = (d&07)<<8;
	for(i=0;i<10;i++) {
		p = (unsigned)a;
		RP->ca = p/200;
		st = p%200;
		RP->da = ((st/10)<<8)|(st%10);
		RP->wc = -((c>>1)&077777);
		RP->ba = b;
		RP->cs = 03|unit;
		while(RP->cs.lobyte>=0);
		if(RP->cs>0)
			return(0);
		printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
		RP->cs = 01;
		while(RP->cs.lobyte>=0);
	}
	return(1);
}
