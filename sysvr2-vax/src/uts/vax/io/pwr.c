/* @(#)pwr.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/var.h"


pwr()
{
	register (**clr)();
	register	s;
	register struct proc *pp;

	s = spl7();
	printf("\377\377\377\n\nPower fail #%d\n\n", pwr_cnt);
	pwr_act = 1;
	/* clear adaptors */
	mbaclr();
	ubaclr();
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

	pwr_act = 0;
	splx(s);
}
