/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include <sys/signal.h>

int (*_realcatch[NSIG+1])();

int (*signal(sig, fun))()
int (*fun)();
{
	int (*old)(), (*ret)();
	extern int (*_signal())();
	extern int _catch();

	if (sig <= 0 || sig > NSIG) {
		return (_signal(sig, fun));
	}
	old = _realcatch[sig];
	if (fun == SIG_DFL || fun == SIG_IGN) {
		_realcatch[sig] = 0;
	} else {
		_realcatch[sig] = fun;
		fun = _catch;
	}
	ret = _signal(sig, fun);
	if (ret == _catch)
		ret = old;
	return (ret);
}

asm("# C library -- signal");
asm(".text");

asm("	.set	signal,48");
asm("	.globl	__signal");
asm("	.globl	cerror");
asm("");
asm("	.align	2");
asm("__signal:");
asm("	addr	signal,r0");
asm("	addr	4(sp),r1");
asm("	svc");
asm("	bfc 	noerror");
asm("	jump	cerror");
asm("noerror:");
asm("	ret	0");
asm("");
asm("# this routine catches signals");
asm("# it calls the real signal function");
asm("");
asm("	.globl	__catch");
asm("	.align	2");
asm("__catch:");
asm("	save	[r0,r1,r2,r3]");
asm("	sprd	upsr,r3");
asm("	movd	24(sp),r0	# arg");
asm("	movd	r0,tos");
asm("	ashd	$2,r0");
asm("	movd	__realcatch(r0),r0");
asm("	jsr	r0		# call real function");
asm("	adjspb	$-4");
asm("	lprd	upsr,r3");
asm("	restore	[r0,r1,r2,r3]");
asm("	rxp	4");
