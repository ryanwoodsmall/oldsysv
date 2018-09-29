/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/scr_dump.c	1.4"
/*
	Dump an image of the screen into the named file.

	The format of the screen image follows. All numbers that specify the
	characters and its attributes on the screen are unsigned shorts
	(chtype). The name of the tty is 20 characters long and the mod time
	(the mtime of the tty that this is an image of) is of the type time_t.

		<magic number>
		<name of tty>
		<mod time of tty>
		<columns> <lines>
		<line length> <chars in line>
		<line length> <chars in line>
		   .
		   .
		   .

	Only as many characters as are in a line will be listed. For example,
	if the line length is zero, there will be no characters following
	that short.

 */
#include "curses.ext"
#include <sys/types.h>
#include <sys/stat.h>

extern char *ttyname();

static void output (i, file)
chtype i;
register FILE *file;
{
    (void) fwrite ( (char *) &i, sizeof (i), 1, file);
}

int scr_dump (filename)
register char *filename;
{
    register int i, j, l;
    register FILE *file;
    register struct line **cb;
    char *tty;
    struct stat ttystat;

    file = fopen (filename, "w");
    if (file == NULL)
	{
	(void) fprintf (stderr, "cannot open '%s'.\n", filename);
	return ERR;
	}

    output ((chtype) (MAGNUM+1), file);		/* magic number */
    tty = ttyname (fileno(SP->term_file));	/* name of /dev/tty */
    ttystat.st_mtime = 0;
    if (tty)
	{
	(void) fwrite (tty, 20, 1, file);
	(void) stat(tty, &ttystat);
	}
    else
	(void) fwrite ("", 20, 1, file);
    (void) fwrite ((char *) &ttystat.st_mtime, sizeof(time_t), 1, file);

    output ((chtype) columns, file);
    output ((chtype) lines, file);

    /*
     * for each line, output the length followed by the characters in
     * the line.
     */
    cb = SP->cur_body;
    for (i = 1; i <= lines; i++)
	if (cb[i] != NULL && cb[i]->length > 0)
	    {
	    chtype *chptr = cb[i]->body;
	    l = cb[i]->length;
	    output ((chtype) l, file);
	    for (j = 0; j < l; j++)
		{
		chtype ch = chptr[j];
		if (ch == (chtype) -1)
		    output ((chtype) ' ', file);
		else
		    output (ch, file);
		}
	    }
	else
	    output ((chtype) 0, file);

    /* If there are soft labels being maintained, */
    /* output the number of labels and their width, */
    /* followed by the labels themselves. This is done */
    /* irregardless of whether the labels have already */
    /* been output as part of the screen. */
    if (SP->slk != NULL && SP->slk->window == NULL)
	output ((chtype) TRUE, file);
    else
	output ((chtype) FALSE, file);
    if (SP->slk)
	{
	register struct slkdata *SLK = SP->slk;
	register int len = SLK->len;
#define LABMAX 8
	output ((chtype) LABMAX, file);		/* number of labels */
	output ((chtype) len, file);		/* width of labels */
	for (i = 0; i < LABMAX; i++)
	    for (j = 0; j < len; j++)
		output ((chtype) SLK->scrlabel[i][j], file);
	}
    output ((chtype) SP->phys_y, file);
    output ((chtype) SP->phys_x, file);

    (void) fclose (file);
    return OK;
}
