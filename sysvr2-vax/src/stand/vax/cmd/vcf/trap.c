/*	@(#)trap.c	1.1	*/
#include <stdio.h>
#define	USER	040		/* user-mode flag added to type */
#define	NSYSENT	64

extern int intrflg;
/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(params, r0, r1, r2, r3, r4, r5 ,r6, r7, r8, r9, r10,
	r11, r12, r13, sp, type, code, pc, ps)
int * params;
{
	register i;

		printf("\nTRAP\n");
		printf("ps = %x\n", ps);
		printf("pc = %x\n", pc);
		printf("trap type %x\n", type);
		printf("code = %x\n", code);

}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
}

/*
 * Ignored system call
 */
nullsys()
{
}

stray(addr)
{
	if(intrflg){
		printf("stray interrupt at");
		printf(" %o\n", addr);
	}
	printf("*");
	intrflg = addr;
}
