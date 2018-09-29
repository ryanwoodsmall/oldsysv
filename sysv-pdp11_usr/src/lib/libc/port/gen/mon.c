/*	@(#)mon.c	2.5	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <mon.h>

extern int creat(), write(), close();
extern void profil(), perror();

static int ssiz;
static struct hdr *sbuf;

void
monitor(lowpc, highpc, buffer, bufsize, nfunc)
char	*lowpc, *highpc;	/* boundaries of text to be monitored */
WORD	*buffer;	/* ptr to space for monitor data (WORDs) */
int	bufsize;	/* size of above space (in WORDs) */
int	nfunc;		/* max no. of functions whose calls are counted
					(default nfunc is 300) */
{
	int scale;
	long text, lsize;

	if (lowpc == NULL) {		/* true only at the end */
		if (sbuf != NULL) {
			int fd;

			profil((char *)NULL, 0, 0, 0);
			if ((fd = creat(MON_OUT, 0666)) < 0 ||
			    write(fd, (char *)sbuf, (unsigned)ssiz) != ssiz)
				perror(MON_OUT);
			if (fd >= 0)
				(void) close(fd);
		}
		return;
	}
	sbuf = NULL;
	ssiz = (sizeof(struct hdr) + nfunc * sizeof(struct cnt))/sizeof(WORD);
	if (ssiz >= bufsize || lowpc >= highpc)
		return;
	sbuf = (struct hdr *)buffer;	/* for writing buffer at the wrapup */
	sbuf->lpc = lowpc;		/* initialize the first */
	sbuf->hpc = highpc;		/* region of the buffer */
	sbuf->nfns = nfunc;
	buffer += ssiz;			/* move ptr past 2'nd region */
	bufsize -= ssiz;		/* no. WORDs in third region */
					/* no. WORDs of text */
	text = (highpc - lowpc + sizeof(WORD) - 1)/sizeof(WORD);
	lsize = (bufsize > text) ? text : bufsize;	/* minimum */
	scale = (int)(((lsize << 16) - 1) / text);
	bufsize *= sizeof(WORD);		/* bufsize into no. bytes */
	ssiz = ssiz * sizeof(WORD) + bufsize;	/* size into no. bytes */
	profil((char *)buffer, bufsize, (int)lowpc, scale);
}
