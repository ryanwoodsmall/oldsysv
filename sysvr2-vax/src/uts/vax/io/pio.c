/* @(#)pio.c	6.1 */
#include "sys/param.h"
#include "sys/mtpr.h"
#include "sys/page.h"

short piget(pad)
{
	extern char *pmavad;
	extern long *pmapte;
	register b;
	register short *p;

	b = (pad>>9)&0x7fffff;
	*pmapte = b|(PG_V|PG_KR);
	mtpr(TBIS, pmavad);
	p = (short *)&pmavad[pad&0x1ff];
	return(*p);
}

piput(pad, val)
{
	extern char *pmavad;
	extern long *pmapte;
	register b;
	register short *p;

	b = (pad>>9)&0x7fffff;
	*pmapte = b|(PG_V|PG_KW);
	mtpr(TBIS, pmavad);
	p = (short *)&pmavad[pad&0x1ff];
	*p = val;
}
