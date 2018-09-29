*	@(#)rk.c	1.1	*/
#
struct {	char lobyte, hibyte;};
#define RK	0177400
rkwrite(a,b,c,d)
unsigned a;
char *b;
{
	struct	{ int ds, er, cs, wc, ba, da, db;};
	register i, unit;

	unit = (d&07)<<13;
	for(i=0;i<10;i++) {
		RK->da = unit | ((a/12)<<4) | (a%12);
		RK->wc = -((c>>1)&077777);
		RK->ba = b;
		RK->cs = 03;
		while(RK->cs.lobyte>=0);
		if(RK->cs>0)
			return(0);
		printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
		RK->cs = 01;
		while(RK->cs.lobyte>=0);
	}
	return(1);
}
