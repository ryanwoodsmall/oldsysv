/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/read_fifo.c	1.3"

/*LINTLIBRARY*/

#include "errno.h"
#include "memory.h"

#include "msgs.h"

extern char	Resync[];
extern char	Endsync[];
static int	Had_Full_Buffer = 0;
int		Garbage_Bytes	= 0;
int		Garbage_Messages= 0;

#if	defined(TRACE_MESSAGES)
int		trace_messages	= 0;
#include "stdio.h"
#endif

/*
** A real message is written in one piece, and the write
** is atomic. Thus, even if the O_NDELAY flag is set,
** if we read part of the real message, we can continue
** to read the rest of it in as many steps as we want
** (up to the size of the message, of course!) without
** UNIX returning 0 because no data is available.
** So, a real message doesn't have to be read in one piece,
** which is good since we don't know how much to read!
**
** Fake messages, or improperly written messages, don't
** have this nice property.
**
** INTERRUPTED READS:
**
** If a signal occurs during an attempted read, we can exit.
** The caller can retry the read and we will correctly restart
** it. The correctness of this assertion can be seen by noticing
** that at the beginning of each READ below, we can go back
** to the first statement executed (the first READ below)
** and correctly reexecute the code.
**
** If the last writer closed the fifo, we'll read 0 bytes
** (at least on the subsequent read). If we were in the
** middle of reading a message, we were reading a bogus
** message (but see below).
**
** If we read less than we expect, it's because we were
** reading a fake message (but see below).
**
** HOWEVER: In the last two cases, we may have ONE OR MORE
** REAL MESSAGES snuggled in amongst the trash!
**
** All this verbal rambling is preface to let you understand why we
** buffer the data (which is a shame, but necessary).
*/


/*
** WARNING: This routine uses a static buffer, so only ONE fifo
** can be read per process. Don't be fooled by the first argument
** (the file descriptor for the open fifo).
*/


/*
** read_fifo() - READ A BUFFER WITH HEADER AND CHECKSUM
*/

# define	SAVE_SIZE	MSGMAX

static char	save[SAVE_SIZE],
		*psave		= save,
		*psave_end	= save;



static int
_buffer (fifo)
int	fifo;
{
    register int	nbytes,
			n;

    /*
    ** As long as we get real messages, and if we chose
    ** SAVE_SIZE well, we shouldn't have to move the data
    ** in the "else" branch below: Each time we call "read"
    ** we aren't likely to get as many bytes as we ask for,
    ** just as many as are in the fifo, AND THIS SHOULD
    ** REPRESENT AN INTEGRAL NUMBER OF MESSAGES. Since
    ** the "read_fifo" routine reads complete messages,
    ** it will end its read at the end of the message,
    ** which (eventually) will make "psave_end" == "psave".
    */

    /*
    ** If the buffer is empty, there's nothing to move.
    */
    if (psave_end == psave)
	psave = psave_end = save;	/* sane pointers! */

    /*
    ** If the buffer has data at the high end, move it down.
    */
    else if (psave != save)		/* sane pointers! */
    {
	register char	*p	= save,
			*ps	= psave;


	/*
	** Move the data still left in the buffer to the
	** front, so we can read as much as possible into
	** buffer after it.
	**
	** Don't try to use memcpy here!
	** See CAVEATS under memory(3C).
	*/

	while (ps < psave_end)
	    *p++ = *ps++;

	psave = save;	/* sane	pointers! */
	psave_end = p;
    }


    /*
    ** The "psave" and "psave_end" pointers must be in a sane
    ** state when we get here, in case the "read()" gets interrupted.
    ** When that happens, we return to the caller who may try
    ** to restart us! Sane: psave == save (HERE!)
    */

    nbytes = SAVE_SIZE - (psave_end - save);

    if ((n = read(fifo, psave_end, nbytes)) > 0)
	psave_end += n;

    Had_Full_Buffer = (nbytes == n);

    return (n);
}


# define	SYNCD	(1)

/*
** As long as we get real messages, we can avoid needless function calls.
** The SYNC argument in this macro should be set if the resynch. bytes
** have been read--i.e. if the rest of the message is trying to be read.
** In this case, if we had not read a full buffer last time, then we
** must be in the middle of a bogus message.
*/

#define READ(N,SYNC) \
    if (psave_end - psave < N) \
    { \
	if (SYNC && !Had_Full_Buffer) \
	    goto SyncUp; \
	switch (_buffer(fifo)) \
	{ \
	    case -1: \
		return (-1); \
	    case 0: \
		if (psave_end > psave) \
		    goto SyncUp; \
		return (0); \
	} \
    } else


int
read_fifo (fifo, buf, size)
int		fifo;
char		*buf;
unsigned int	size;
{
    register unsigned int	real_chksum,
				chksum,
				real_size;

    /*
    ** Make sure we start on a message boundary. The first
    ** line of defense is to look for the resync. bytes.
    **
    ** The "SyncUp" label is global to this routine (below this point)
    ** and is called whenever we determine that we're out
    ** of sync. with the incoming bytes.
    */

    READ (HEAD_RESYNC_LEN, !SYNCD);
    while (*psave != Resync[0] || *(psave + 1) != Resync[1])
    {
SyncUp:
#if	defined(TRACE_MESSAGES)
	if (trace_messages) {
		register FILE	*fp = fopen("./trace", "a");
		fprintf (fp, "DISCARD %c\n", *psave);
		fclose (fp);
	}
#endif
	psave++;
	Garbage_Bytes++;
	READ (HEAD_RESYNC_LEN, !SYNCD);
    }


    /*
    ** We're sync'd, so read the full header.
    */

    READ (HEAD_LEN, SYNCD);


    /*
    ** If the header size is smaller than the minimum size for a header,
    ** or larger than allowed, we must assume that we really aren't
    ** synchronized.
    */

    real_size = stoh(psave + HEAD_SIZE);
    if (real_size < CONTROL_LEN || SAVE_SIZE < real_size)
    {
#if	defined(TRACE_MESSAGES)
	if (trace_messages) {
		register FILE	*fp = fopen("./trace", "a");
		fprintf (fp, "BAD SIZE\n");
		fclose (fp);
	}
#endif
	goto SyncUp;
    }

    /*
    ** We have the header. Now we can finally read the rest of the
    ** message...
    */

    READ (real_size, SYNCD);


    /*
    ** ...but did we read a real message?...
    */

    if
    (
	   *(psave + TAIL_ENDSYNC(real_size)) != Endsync[0] 
	|| *(psave + TAIL_ENDSYNC(real_size) + 1) != Endsync[1] 
    )
    {
#if	defined(TRACE_MESSAGES)
	if (trace_messages) {
		register FILE	*fp = fopen("./trace", "a");
		fprintf (fp, "BAD ENDSYNC\n");
		fclose (fp);
	}
#endif
	Garbage_Messages++;
	goto SyncUp;
    }

    chksum = stoh(psave + TAIL_CHKSUM(real_size));
    CALC_CHKSUM (psave, real_size, real_chksum);
    if (real_chksum != chksum)
    {
#if	defined(TRACE_MESSAGES)
	if (trace_messages) {
		register FILE	*fp = fopen("./trace", "a");
		fprintf (fp, "BAD CHKSUM\n");
		fclose (fp);
	}
#endif
	Garbage_Messages++;
	goto SyncUp;
    }

    /*
    ** ...yes!...but can the caller handle the message?
    */

    if (size < real_size)
    {
	errno = E2BIG;
	return (-1);
    }


    /*
    ** Yes!! We can finally copy the message into the caller's buffer
    ** and remove it from our buffer. That wasn't so bad, was it?
    */

#if	defined(TRACE_MESSAGES)
    if (trace_messages) {
	register FILE	*fp = fopen("./trace", "a");
	fprintf (fp, "MESSAGE:\n");
	fwrite (psave, 1, real_size, fp);
	fputs ("\n", fp);
	fclose (fp);
    }
#endif
    (void)memcpy (buf, psave, real_size);
    psave += real_size;
    return (real_size);
}
