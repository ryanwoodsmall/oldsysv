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
#include	"hpio.h"
#include	"stdio.h"
#include	"termio.h"
#include	"signal.h"
#include	"sys/types.h"
#include	"sys/stat.h"

#define	ENABLE	'\021'	/* Enable character. (DC1) */
#define	ENQ	'\005'	/* Enquiry character. (ENQ) */
#define	RS	'\036'	/* Record separator. (RS) */

/*
**	The "1" in Skip, Td, and Ts means left tape unit, changing it to 
**	"2" means right tape unit.
*/

char	Skip[] = "\033&p1u2C",	/* Tape file skip sequence. */
	Td[] = "\033&p1D",	/* Tape destination selection sequence. */
	Ts[] = "\033&p1S",	/* Tape source selection sequence. */
	Sreset[] = "\033g",	/* Soft reset of terminal. */
	check,			/* Checksum. */
	*drive = "left",	/* Selected tape drive name. */
	opts[] = "acin:ort";	/* Valid option letters. */

struct	FH	Fh;	/* File header. */
struct	stat	statbuf;/* Fstat buffer. */
struct	termio	Oterm,	/* Original terminal settings. */
		Nterm;	/* New terminal settings. */
int		aflg,	/* ask before input */
		cflg,	/* checksum */
		iflg,	/* input from tape */
		ncount,	/* number of input files */
		nflg,	/* use count of input files */
		oflg,	/* output to tape */
		tflg,	/* table of contents */	
		errflg,	/* option error detected */
		mode,	/* file access modes */
		ttymode;/* terminal access modes */
char		*ttynam;/* terminal file name */
extern int	optind;	/* argv index from getopt */
extern char	*optarg;/* option ptr from getopt */

int	catch();
char	*ttyname();

main(argc, argv)
int	argc;	/* arg count */
char	*argv[];/* arg vector */
{
	register FILE	*fp;	/* i/o file pointer */
	register char	c,	/* input character */
			*p;	/* ptr to buffer being checksummed */
	register int	i,	/* loop control */
			size2;	/* current record size */
	long		size;	/* current file size */
	char		buf[256];	/* i/o buffer */

	/* Accumulate options. */
	while((i = getopt(argc, argv, opts)) != EOF)
	switch(i) {
	case 'a':
		aflg = 1;
		break;
	case 'c':
		cflg = 'c';
		break;
	case 'i':
		iflg = 1;
		if(!nflg)
			ncount = 1;
		break;
	case 'n':
		nflg = 1;
		sscanf(optarg, "%d", &ncount);
		if(ncount <= 0)
			errflg++;
		break;
	case 'o':
		oflg = 1;
		break;
	case 'r':
		drive = "right";
		Td[3] = Ts[3] = Skip[3] = '2';
		break;
	case 't':
		tflg = 1;
		break;
	case '?':
		errflg++;
		break;
	}
	if(errflg || !(iflg ^ oflg)) {
		printf("Usage:\t%s\n\t%s\n",
			"hpio -i[art] [-n count]",
			"hpio -o[cr] file ...");
		exit(1);
	}

	/* Set up unbufferred raw mode with no echo. */
	setbuf(stdout, NULL);
	if(ioctl(fileno(stdout), TCGETA, &Oterm)) {
		printf("Can't get tty options on stdout.\n");
		exit(1);
	}
	ttynam = ttyname(fileno(stdout));
	fstat(fileno(stdout), &statbuf);
	ttymode = statbuf.st_mode;
	chmod(ttynam, ttymode & ~077);
	signal(SIGINT, catch);
	Nterm.c_iflag = BRKINT;
	Nterm.c_oflag = 0;
	Nterm.c_cflag = (Oterm.c_cflag & ~PARENB) | CS8;
	Nterm.c_lflag = NOFLSH;
	Nterm.c_line = 0;
	Nterm.c_cc[VMIN] = 6;
	Nterm.c_cc[VTIME] = 1;
	ioctl(fileno(stdout), TCSETAW, &Nterm);

	/* Determine desired action and perform it. */
	if(iflg) {

		/* Select source tape and loop through selected files. */
		for(printf("%s", Ts);ncount--;hskip()) {

			/* Read the header structure from the tape. */
			hread(&Fh, sizeof(Fh));
			sscanf(Fh.FHsize, SIZEFMT, &size);
			printf("%ld bytes of `%s'", size, Fh.FHname);
			sscanf(Fh.FHmode, MODEFMT, &mode);
			if(tflg) {

				/* Table of contents only. */
				printf(" mode: %#o with%s checksum.\r\n", mode,
					Fh.FHchkf ? "" : "out");
				continue;
			}
			if(aflg) {

				/* Ask before copying file in. */
				printf("? ");
				c = getchar();
				if(c != 'Y' && c != 'y') {
					putchar(c);
					while(c != '\r')
						putchar(c = getchar());
					putchar('\n');
					continue;
				}
				while(c != '\r') {
					putchar(c);
					c = getchar();
				}
			}
			printf("\r\n");

			/* Create the file to be read in. */
			if((fp = fopen(Fh.FHname, "w")) == NULL) {
				printf("Can't create `%s'.\r\n", Fh.FHname);
				continue;
			}

			/* Copy the file from the tape to disk. */
			check = 0;
			while(size > 0) {
				size2 = size > 256 ? 256 : size;
				hread(buf, size2);
				fwrite(buf, 1, size2, fp);
				if(Fh.FHchkf)
					for(i = size2, p = buf;i--;
						check ^= *p++);
				size -= size2;
			}
			fclose(fp);
			printf("`%s' read from %s tape.\r\n",
				Fh.FHname, drive);
			if(Fh.FHchkf) {

				/* Check checksum. */
				hread(buf, 5);
				sscanf(buf, CHKFMT, &errflg);
				printf("Checksum match %s.\r\n",
					(errflg == (check & 0377)) ?
					"succeeded" : "\007failed");
			}
			if(mode)
				chmod(Fh.FHname, mode);
		}
	} else {

		/* Select destination tape and copy files to tape. */
		for(printf("%s", Td), i = optind;i < argc;i++) {

			/* Setup file header and open the input file. */
			strcpy(Fh.FHname, argv[i]);
			Fh.FHchkf = cflg;
			check = 0;
			if((fp = fopen(Fh.FHname, "r")) == NULL) {
				printf("Can't open `%s'.\r\n", Fh.FHname);
				continue;
			}
			fstat(fileno(fp), &statbuf);
			if((statbuf.st_mode & S_IFMT) != S_IFREG) {
				printf("`%s' not a regular file.\r\n",
					Fh.FHname);
				fclose(fp);
				continue;
			}
			sprintf(Fh.FHmode, MODEFMT, statbuf.st_mode & 07777);
			sprintf(Fh.FHsize, SIZEFMT, size = statbuf.st_size);

			/* Write header record. */
			hwrite(&Fh, sizeof(Fh));

			/* Write file. */
			while(size > 0) {
				size2 = size > 256 ? 256 : size;
				if(fread(buf, 1, size2, fp) != size2) {
					sprintf(buf, "Read fail on `%s'.",
						Fh.FHname);
					exitp(buf);
				}
				hwrite(buf, size2);
				size -= size2;
				if(cflg)
					for(p = buf;size2--;check ^= *p++);
			}
			fclose(fp);

			/* Write checksum. */
			if(cflg) {
				sprintf(buf, CHKFMT, check & 0377);
				hwrite(buf, 5);
			}
			hmark();
			printf("%s bytes of `%s' written to %s tape.\r\n",
				Fh.FHsize, Fh.FHname, drive);
		}
	}
	ioctl(fileno(stdout), TCSETAW, &Oterm);
	chmod(ttynam, ttymode);
	exit(0);
}

/*
**	hwrite - Write to HP cassette drive.
**
**	Write a record of the given size from the given output buffer.
*/

hwrite(buf, cnt)
char	*buf;	/* ptr to output buffer */
int	cnt;	/* # of bytes to write */
{

	/* Put out write command and get go ahead signal. */
	printf("\033&p%dW%c", cnt, ENQ);
	getchar();

	/* Write the data and get completion status. */
	fwrite(buf, 1, cnt, stdout);
	putchar(ENABLE);
	if(getchar() != 'S')
		exitp("Write failed.");
	getchar();
	return(cnt);
}

/*
**	hskip - Skip to start of next file on HP cassette drive.
**
**	This routine requests a skip to the next file on the cassette and
**	waits for completion of the operation.
*/

hskip()
{
	printf("%s%c", Skip, ENABLE);
	if(getchar() != 'S')
		exitp("Skip to next file failed.");
	getchar();
}

/*
**	hread - Read from HP cassette drive.
**
**	Read the next record from the current file on the tape drive.
**	It must be the the same size as requested.
*/

hread(buf, cnt)
register char	*buf;	/* ptr to buffer to receive the record */
int		cnt;	/* size of desired record */
{
	register int	i,	/* loop control */
			cnt2;	/* # of bytes left to be read */
	register char	c;	/* input character */

	/* Request record read and get byte count for the record. */
	printf("\033&p2R%c", ENABLE);
	for(cnt2 = 0, i = 4;i--;cnt2 = cnt2 * 16 + c - '0')
		if((c = getchar()) == RS)
			exitp("End of Tape.");
	getchar();
	if(cnt2 != cnt) {
		sprintf(buf, "Readcnt = %d, termcnt = %d.", cnt, cnt2);
		exitp(buf);
	}

	/* Enable data record transmission and read it into buf. */
	putchar(ENABLE);
	for(;cnt2--;*buf++ = getchar());
	return(cnt);
}

/*
**	hmark - Write tape mark on HP cassette drive.
**
**	Write a tape mark and get the completion status of the operation.
*/

hmark()
{
	printf("\033&p5C%c", ENABLE);
	if(getchar() != 'S')
		exitp("Tape mark write failed.");
	getchar();
}

/*
**	catch	Catch break.
**
**	This routine catches the interrupt signal to reset terminal
**	options and exit.
*/

catch()
{
	signal(SIGINT, SIG_IGN);
	exitp("BREAK");
}

/*
**	exitp - Error exit.
**
**	Reset terminal modes, print error message, and exit.
*/

exitp(mp)
char	*mp;	/* ptr to error message */
{
	printf("%s", Sreset);
	sleep(2);
	printf("%s\r\n", mp);
	ioctl(fileno(stdout), TCSETAW, &Oterm);
	chmod(ttynam, ttymode);
	exit(1);
}

/* <@(#)hpio.c	1.1> */
