/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:xtraces.c	2.5"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Routine to print 'xt' driver traces
 */
#include "stdio.h"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/xtproto.h"
#include "sys/xt.h"

#if XTRACE == 1
#define min(x, y)	((x) < (y)? (x) : (y))
#define CNTL(tp)	(((tp)->pktpart[0]>>6)&01)
#define SEQ(tp)		((tp)->pktpart[0]&07)
#undef  CHAN		/* from xt.h */
#define CHAN(tp)	(((tp)->pktpart[0]>>3)&07)
#define DSIZE(tp)	((tp)->pktpart[1])

struct Tbuf Traces;
extern long time();
extern char *ctime();
char *mtype[] = {
	"", "SENDCHAR", "NEW", "UNBLK", "DELETE", "EXIT",
	"DEFUNCT", "SENDNCHARS", "RESHAPE",
};	
#endif XTRACE

void
xtraces(cfd, ofd)
register FILE *ofd;
{
#if XTRACE == 1
	register struct Tpkt *tp;
	register Pbyte *cp;
	register int i, j;
	register time_t date;
	long t;

	if (ioctl(cfd, XTIOCTRACE, &Traces) == -1)
		return;
	if ((i = Traces.used) == 0 || (Traces.flags&TRACEON) == 0)
		return;
	(void)time(&t);
	(void)fprintf(ofd, "\n%15.15s Xt Packet Traces:-\n", ctime(&t)+4);
	if (Traces.flags&TRACELOCK)
		(void)fprintf(ofd, "(Trace Locked!)\n");
	if ((j = Traces.index - i) < 0)
		j += PKTHIST;
	tp = &Traces.log[j];
	date = tp->time;
	do {
		j = tp->time - date;
		(void)fprintf(ofd, "%3d.%2.2d  ", j/HZ, j%HZ);
		(void)fprintf(ofd, "Ch %d ", CHAN(tp));
		(void)fprintf(ofd, tp->flag == XMITLOG? "Xmt " : "Rcv ");
		(void)fprintf(ofd, "%d:  ", SEQ(tp));
		if (CNTL(tp) && DSIZE(tp) == 0) {
			(void)fprintf(ofd, "ACK\n");
			goto next;
		}
		j = min(DSIZE(tp), sizeof tp->pktpart - 2);
#if vax | u3b
		cp = &tp->pktpart[2];	/* first byte of data */
#else
		if (tp->flag == RECVLOG)
			cp = &tp->pktpart[4];	/* first byte of data */
		else
			cp = &tp->pktpart[2];	/* first byte of data */
#endif
		if (CNTL(tp) && DSIZE(tp) > 0) {
			(void)fprintf(ofd, *cp == ACK? "ACK " :
				*cp == NAK? "NAK " : "<%#o> ", *cp&0377);
			cp++;
			j--;
			DSIZE(tp)--;
		}
		if (tp->flag == RECVLOG && C_SENDCHAR <= *cp && *cp <= C_RESHAPE){
			(void)fprintf(ofd, "%s ", mtype[*cp++]);
			j--;
			DSIZE(tp)--;
		} 
		while (j-- > 0) {
			(void)fprintf(ofd, (' ' < *cp && *cp < '~')?
				"%c " : "<%#o> ", *cp);
			cp++;
			DSIZE(tp)--;
		}
		(void)fprintf(ofd, DSIZE(tp) > 0? "[more]\n" : "\n");
	next:
		if (++tp >= &Traces.log[PKTHIST])
			tp = Traces.log;
	} while (--i);
	(void)fflush(ofd);
#endif XTRACE
}
