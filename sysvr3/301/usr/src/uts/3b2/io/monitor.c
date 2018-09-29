/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/monitor.c	10.5"
#include "sys/inline.h"
/*
 * Maintain an internal circular trace buffer in which to record significant
 * system events.  This trace can then be used for post-mortem debugging.
 * This scheme is modelled exactly after the trace buffer for IBM OS/VS370.
 *
 * The size of the trace buffer is determined at boot time by self-config.
 * The etc/master file parameter MONBUF should be modified appropriatly,
 * mkboot(1M) run, and the system re-booted using a kernel which was built
 * with the preprocessor symbol MONITOR #define'd.  Etc/master should contain
 * the following lines:
 *
 *
 *		monitor	ox
 *				monitor() {}
 *				_monitor[MONBUF] (%l%l%l%l)
 *				_ms(%l) ={&_monitor}
 *				_me(%l) ={&_monitor+#_monitor*MONBUF}
 *				_mp(%l) ={&_monitor}
 *
 *		$$
 *
 *		MONBUF	AUTO	2048
 *
 * The three words, _ms, _me and _mp can be used to locate and interpret the
 * trace buffer.  The symbol _ms is the address of the start of the buffer,
 * _me is the address of the first location past the end of the buffer, and
 * _mp is the address of the next entry that will be filled in.
 *
 * The trace records are four words long.  Each record is identified by an
 * ascii letter in the first byte.  The records now supported are:
 *
 * SYSTEM EVENT                    LETTER  WORD 1    WORD 2    WORD 3    WORD 4
 * ------------------------------  ------  --------- --------- --------- ---------
 * os/slp.c        pswtch() entry    Z     curproc   p_stat    p_flag
 * os/slp.c         pswtch() exit    P     curproc   %pc       %psw
 * os/trap.c               trap()    T     %pc       %psw
 * os/trap.c      systrap() entry    S     %pc       %psw      %r0       %r1
 * os/trap.c       systrap() exit    R     %pc       u_error   %r0       %r1
 * ml/ttrap.s             int_ret    I     %pcbp     repeat*   %pc       argument to C routine
 * os/trap.c              intsx()    X     %pc       %psw      %sp
 * ml/ttrap.s          int_syserr    E     %pc       %psw      CC csr
 *
 * The `repeat' count is the number of duplicate interrupts; i.e. if another
 * interrupt with the same %pcbp occurred without any intervening trace records,
 * then `repeat' is incremented by 1.
 */

/*
 * Define the number of trace records for pre-auto-config 3B2.
 */
#define	MONBUF	2048		/* Size of circular list */

/*
 * Trace records; actually defines space for pre-auto-config 3B2.
 */
struct {
	union {
		char	id[sizeof(int)];
		int	word1;
	} u;
	int	word2, word3, word4;
} _monitor[MONBUF], *_ms = _monitor, *_me = _monitor + MONBUF, *_mp = _monitor;


monitor(id,word1,word2,word3,word4)
	char id;
	register int word1,word2,word3,word4;
	{
	register psw;

	/*
	 * if an interrupt, then check for duplicates and bump repitition count
	 */
	if (id == 'I')
		{
		if (_mp[-1].u.id[0] == 'I' && (_mp[-1].u.word1&0xFFFFFF) == word1)
			{
			/*
			 * bump repeat count
			 */
			++_mp[-1].word2;
			return;
			}
		/*
		 * adjust arguments in order to set repeat count to zero
		 */
		word4 = word3;
		word3 = word2;
		word2 = 0;
		}

	psw = splhi();

	/*
	 * fill-in trace record
	 */
	_mp->u.word1 = word1;
	_mp->u.id[0] = id;
	_mp->word2 = word2;
	_mp->word3 = word3;
	_mp->word4 = word4;

	/*
	 * update trace pointer to next spot in buffer
	 */
	if (++_mp == _me)
		_mp = _ms;

	splx(psw);
	}
