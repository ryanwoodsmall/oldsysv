/* @(#)pwr.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/var.h"


pwr()
{
	register (**clr)();
	register struct proc *pp;

	if (pwrlock())
		return;
	printf("\377\377\377\n\nPower fail #%d\n\n", pwr_cnt/2);
	pwr_act = 1;
	/* call clear and start routines */
	clr = &pwr_clr[0];
	while (*clr) {
		(**clr)();
		clr++;
	}
	/* if none - halt */
	if (clr == &pwr_clr[0]) {
		printf("Stopped\n");
		for (;;);
	}
	/* if panic - halt */
	if (panicstr) {
		printf("Panicked\n");
		for (;;);
	}
	/* Send SIGPWR to all processes */
	for (pp = &proc[1]; pp<(struct proc *)v.ve_proc; pp++)
		if (pp->p_stat)
			psignal(pp, SIGPWR);

	/* restart clock */
	clkreld();
	pwr_act = 0;
	pwrunlock();
}
