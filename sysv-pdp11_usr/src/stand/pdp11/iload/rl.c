/*	@(#)rl.c	1.1	*/
#
struct {	char lobyte, hibyte;};
#define RL	0174400
rlwrite(a,b,c,d)
unsigned a;
char *b;
{
	struct	{ int cs, ba, da, mp;};
	register i, cmd, unit;
	int pos;

	unit = (d&03)<<8;
	for(i=0;i<10;i++) {
		RL->cs = unit | 010;
		while(RL->cs.lobyte>=0);
		cmd = ((a/20)<<6) | ((a%20)<<1);
		pos = (RL->mp&~0177) - (cmd&~0177);
		RL->da = (pos < 0 ? -pos | 4 : pos ) | 1 | (cmd&0100)>>2;
		RL->cs = unit | 6;
		while(RL->cs.lobyte>=0);
		RL->da = cmd;
		RL->ba = b;
		RL->mp = -((c>>1)&077777);
		RL->cs = unit | 012;
		while(RL->cs.lobyte>=0);
		if(RL->cs>0)
			return(0);
		if(RL->cs & 040000) {
			RL->da = 3;
			RL->cs = unit | 4;
			while(RL->cs.lobyte>=0);
			if(!RL->mp&01000)
				printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
			RL->da = 013;
			RL->cs = unit | 4;
			while(RL->cs.lobyte>=0);
		} else
			printf("disk error, range %d - %d blocks\n", a, a+(c>>9)-1);
	}
	return(1);
}
