/*	@(#)intr.c	1.1	*/
#include <stdio.h>
extern int intrflg;
int mbadev;
mbaintr()
{
	printf("\n*** MBA interrupt ***");
}
ubasrv()
{
	printf("\n*** UBA service ??? ***");
}
ubastray(vec)
int vec;
{
	if(intrflg) printf("\n*** mba_int at %o ***",vec);
	else{
		printf("#");
		intrflg = vec;
	}
}
conrint()
{
	printf("\n*** CONS rcvr intrerrupt ***");
}
conxint()
{
	printf("\n*** CONS xmtr intrerrupt ***");
}
clock()
{
	printf("\n*** CLOCK interrupt ***");
}
pwr()
{
	printf("POWER FAIL\n\n\n");
}
bkpt()
{
	printf("\nbkpt.. c?");
	while(getc(stdin) != 'c');
}
