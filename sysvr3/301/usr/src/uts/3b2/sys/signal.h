/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/signal.h	10.9"

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQUIT	3	/* quit (ASCII FS) */
#define	SIGILL	4	/* illegal instruction (not reset when caught)*/
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define SIGABRT 6	/* used by abort, replace SIGIOT in the  future */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGUSR1	16	/* user defined signal 1 */
#define	SIGUSR2	17	/* user defined signal 2 */
#define	SIGCLD	18	/* death of a child */
#define	SIGPWR	19	/* power-fail restart */

			/* SIGWIND and SIGPHONE only used in UNIX/PC */
/*#define SIGWIND 20*/	/* window change */
/*#define SIGPHONE 21*/	/* handset, line status change */

#define SIGPOLL 22	/* pollable event occured */

#define	NSIG	23	/* The valid signal number is from 1 to NSIG-1 */
#define MAXSIG	32	/* size of u_signal[], NSIG-1 <= MAXSIG*/
			/* MAXSIG is larger than we need now. */
			/* In the future, we can add more signal */
			/* number without changing user.h */

#if lint
#define SIG_ERR (void(*)())0
#else
#define SIG_ERR	(void(*)())-1
#endif
#define	SIG_DFL	(void(*)())0
#if lint
#define	SIG_IGN	(void (*)())0
#else
#define	SIG_IGN	(void (*)())1
#endif
#if lint
#define SIG_HOLD (void(*)())0
#else
#define SIG_HOLD (void(*)())2
#endif

/* The following definitions are only used by kernel */
#ifdef INKERNEL
#define SIGNO_MASK	0xff
#define SIGDEFER	0x100
#define SIGHOLD		0x200
#define SIGRELSE	0x400
#define SIGIGNORE	0x800
#define SIGPAUSE	0x1000
#endif
