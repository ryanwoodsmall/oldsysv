/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/physical.c	1.13"

#include	<curses.h>
#include	<fcntl.h>
#include	"wish.h"
#include	"token.h"
#include	"message.h"

#ifdef	TIME_IT

#include	<sys/types.h>
#include	<sys/times.h>

#endif /* TIME_IT */

long	Cur_time;
extern char Semaphore[];
extern int Coproc_active;
extern int Vflag;

token
physical_stream(t)
register token	t;
{
	extern long	Mail_check;
	extern long	Interupt_pending;
	token	wgetchar();
	long	time();
	int fd;

#ifdef PHYSICAL

	struct tms	tms1, tms2;
	long		real1, real2;
	int		lcv1 = 10000;

	real1 = times( &tms1 )

	while ( lcv1-- )
	{

#endif /* PHYSICAL */
/*
fprintf( stderr, "Into physical_stream()\n\n" );
*/
	Cur_time = time(0L);
/*
	stopwatch(1);
*/
	ar_checkworld(FALSE);
	working(FALSE);
	if (Vflag)
		showmail( FALSE );
/*
	status_line();
*/

/* only called in this function: twice */
	mess_flush(FALSE);

/* new les */
	vt_flush();
/*
	t = TOK_NOP;
*/
	alarm((int) Mail_check);
	if (Coproc_active) {
		/*
		 * The call to open is to protect "vsig" from
		 * sending a signal to FACE when the screen
		 * is being painted ...
		 * Vsig will block on the semaphore until FACE
		 * is able to receive signals ..
		 * This code may be changed if a better solution
		 * is found ...
		 * Interupt_pending is definined in main.c and
		 * set in the interupt handler for a SIGUSR2 
		 * once a signal is encountered ...
		 */ 
		fd = open(Semaphore, O_RDONLY|O_NDELAY);
		if (Interupt_pending) {
			Interupt_pending = 0;
			ar_checkworld(TRUE);
		}
		t = wgetchar();
		close(fd);
	}
	else
		t = wgetchar();
	mess_unlock();		/* allow calls to mess_temp and mess_perm */
	if (t < 0)
		t = TOK_NOP;
	else if (t >= TOK_F(1) && t <= TOK_F(8))
		t = t - TOK_F(1) + TOK_SLK1;

	mess_flush(TRUE);
/*
	stopwatch(0);
*/
	return t;
}
