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
/*
 *      dcutest.c:  program to test the hard disk
 */

#include <setjmp.h>
#include <sys/disk.h>
#include "dcusaio.h"

#define	PAGSIZE	512
#define	PAGMAX	511
#define	PAGSHFT	9
#define	MAXSEC	300		/* maximum transfer length */
#define	MAXLOG	100		/* maximum sectors remembered for errors */
#define	MORESIZE	20	/* number of lines per display */
#define	READ	0
#define	WRITE	1
#define	TTYSIZE	20
#define	EOL	'\n'
#define	DEL	'\177'
#define	LINERUB	(033)
#define	CHARRUB	('H'-'@')
#define HASH(seed)	(((seed) * 1103515245 + 12345) & MAXINT)
#define	MAXINT	2147483647

#define	TESTREAD	0		/* kinds of tests */
#define	TESTWRITE	1
#define	TESTBOTH	2

#define	IOSEQUENTIAL	0		/* ways we want to test doing IO */
#define	IOREPEAT	1
#define	IORANDOM	2

#define	ERRORIGNORE	0		/* actions to take on errors */
#define	ERRORTYPE	1
#define	ERRORSTOP	2

#define	WRITELOCK	0		/* how to handle disk writes */
#define	WRITEENABLE	1
#define	WRITEASK	2

#define	LOGREAD		0		/* types of logged errors */
#define	LOGWRITE	1
#define	LOGVERIFY	2
#define	LOGFORMAT	3

#define	INITFREQUENCY	1000		/* initial parameters */
#define	INITLIMIT	MAXINT		
#define	INITERROR	ERRORIGNORE
#define	INITTEST	TESTREAD
#define	INITINCREMENT	1		
#define	INITIOMODE	IORANDOM
#define	INITIOSIZE	1
#define	INITWRITE	WRITEASK
#define	INITFORMATTRIES	10

#ifndef BSIZE
#  define BSIZE 1024
#endif

char	*helptext[] = {
	"Commands:",
	"<del>   Type status and exit.",
	"<sp>    Type status without stopping.",
	"s       Type status and stop.",
	"p       Proceed from stopped state.",
	"o       Do just one more operation.",
	"l<n>    Setup limit of <n> operations (default infinite).",
	"ua      Attempt auto configuration of disk.",
	"uc<c><t><s>  Configure unit as <c>,<t>,<s> (cyl/trk/sec).",
	"u<n>    Select unit number <n>.",
	"we      Allow writes to current unit.",
	"wl      Disable writes to current unit.",
	"wa      Ask about writing to current unit.",
	"tr      Setup to test reading (default).",
	"tw      Setup to test writing.",
	"tb      Setup to test writing and reading (safely).",
	"b<l><u> Set lower and upper sector bounds for test.",
	"il<n>   Setup for <n> sector transfers (default 1).",
	"ir<n>   Setup for IO to random sectors (default).",
	"is      Setup for IO to sequential sectors.",
	"i<n>    Set next sector for IO to <n>.",
	"ii<n>   Setup for IO to sectors incrementing by <n>.",
	"f<n>    Set frequency of typeout to <n> (default 1000).",
	"ep      Type status and proceed on errors.",
	"es      Type status and stop on errors.",
	"ei      Ignore errors (default).",
	"d<n>    Dump data contained in sector <n>.",
	"db      Dump data buffer contents.",
	"de<n>   Dump meaning of error status.",
	"di      Dump last IOCB with an error.",
	"dl      Dump list of logged errors.",
	"dp      Dump unit partition table.",
	"ds      Dump data about units after selftest.",
	"dt      Dump all known disk types.",
	"du      Dump data about units.",
	"dv      Dump current values of parameters.",
	"c<n>    Convert from sector number to cyl/trk/sec number.",
	"xh      Write disk header to block 0.",
	"xi<n>   Set interleave formatting value.",
	"xr<n>   Set maximum retries for formatting.",
	"xp<z><nz>  Set PLOZ and PLONZ formatting values.",
	"x<l><u> Format the disk from cylinders <l> to <u>.",
	"z       Zero status counters and error log.",
	"r       Reset parameters to initial values.",
	"h       Type this text.",
	"",
	"Commands are executed as soon as they have been completely typed.",
	"Those which require numeric arguments use the RETURN key to",
	"terminate the command.  Partial commands can be aborted with the",
	"<esc> key, or edited with the <bs> key.  Sector numbers are of",
	"the form \"c<n>t<n>s<n>\", where c<n>, t<n>, and s<n> are optional",
	"cylinder, track and sector numbers (negative values indicate values",
	"measured from the end of the region.  The program continues to test",
	"the disk while commands are being typed, unless it is in the",
	"stopped state.",
	0
};


struct	log {
	long	sector;			/* sector which had error */
	long	status;			/* error status */
	int	type;			/* type of error */
};

/*
 *	The basic disk configuration data
 */

struct dcon {
	short	dt_nsec;	/* # of sectors per track */
	short	dt_ntr;		/* # of tracks per cylinder */
	short	dt_ncyl;	/* # of cylinders per unit */
	short	dt_spcyl;	/* # of sectors per cylinder */
	long	dt_spunit;	/* # of sectors per unit */
};


/*
 *	Default configurations used by stand alone driver for
 *	auto-configure feature. An array of these exists in
 *	dcusaio.c, one entry per known drive type.
 */

struct unitdata {
	short	ud_ploz;	/* formatting data for sector zero */
	short	ud_plonz;	/* formatting data for non-zero sectors */
	struct	dcon dc;	/* the configuration data */
	struct	partition *ud_pt;/* pointer to disk partition table*/
	char   *ud_name;	/* name of disk type */
};

#define d_nsectors		dc.dt_nsec
#define d_ntracks		dc.dt_ntr
#define d_ncylinders		dc.dt_ncyl
#define d_secpercyl		dc.dt_spcyl
#define d_secperunit		dc.dt_spunit


#define dt_nsectors	dt_nsec
#define dt_ntracks	dt_ntr
#define dt_ncylinders	dt_ncyl
#define dt_secpercyl	dt_spcyl
#define dt_secperunit	dt_spunit
#define dt_partition	dt_part


extern	long	end[2];
extern	int	interleave;
extern	struct	unitdata unitsizetable[];
extern	struct	{ unsigned char *_base; char _flag; char _file; } _iob[];
extern	int	dcerror;
jmp_buf	jumpbuf;
long	allocptr;
long	logcount;		/* count of logged sectors */
int	logover;		/* overflow counter */
struct	log	*logbuf;	/* buffer for logging errors */
char	*page1;
char	*page2;
char	*pagecrock;
long	cylunit;
long	trkunit;
long	trkcyl;
long	secunit;
long	seccyl;
long	sectrk;
long	seed;
long	iosize;
long	sector;
long	readcount;
long	writecount;
long	compcount;
long	readstatus;
long	writestatus;
long	readerrorcount;
long	writeerrorcount;
long	readerrorsector;
long	writeerrorsector;
long	comperrorcount;
long	comperrorsector;
long	formattries;
long	frequency;
long	frequencycount;
long	sectorincrement;
long	unitnumber;
long	limitcount;
long	lowsector;
long	highsector;
char	erroraction;
char	iomode;
char	morecount;
char	charreadyflag;
char	promptflag;
char	stoppedflag;
char	testaction;
char	failed;
char	locktab[MAXUNITS];
char	charbuffer[TTYSIZE];
long	charsstored;
long	charsread;
long	gotnum;
char	*alloc();
int	logcmp();


#define		CP_Usart	0xfffa00
#define 	CP_OptBuffer	0xfff400

struct dev2651 {			/* 2651 serial io device */
	unsigned char	data;		/* This depends on word alignment */
	unsigned short	status;		/* of structure members.... */
	unsigned short	mode;
	unsigned short	cmd;
};

/*	Signetics 2651 mode and command bits */

/* Mode register 1 bit definitions */
#define X01	0x1		/* 01x clock rate */
#define X16	0x2		/* 16x clock rate */
#define X64	0x3		/* 64x clock rate */
#define BITS8	0xc		/* 8 bits per char*/
#define BITS7	0x8		/* 7 bits per char*/
#define PENABLE	0x10		/* parity enable  */
#define	EPAR	0x20		/* even parity	  */
#define STOP1	0x40		/* 1 stop bit     */
#define STOP2	0xc0		/* 2 stop bits	  */

#define MODE1	X16 | BITS8 | STOP1 & ~PENABLE
#define MODE2	0		/* external clocking */

/* Command register bit definitions */
#define TxEN	0x1		/* transmitter enable */
#define DTR	0x2		/* assert data terminal ready */
#define RxEN	0x4		/* receiver enable */
#define BRK	0x8		/* force a "break" */
#define ERESET	0x10		/* reset error flags */
#define RTS	0x20		/* assert request to send */

#define CMD	TxEN | DTR | RxEN | ERESET | RTS	/* command reg */

/* Status register bit definitions */
#define TxRDY	0x1		/* transmitter ready */
#define RxRDY	0x2		/* receiver ready */
#define TxEMT	0x4		/* transmitter empty/dschng */
#define PE	0x8		/* parity error */
#define OE	0x10		/* overrun error */
#define FE	0x20		/* framing error */
#define DCD	0x40		/* data carrier detect */
#define DSR	0x80		/* data set ready */

#define ERROR	PE | OE | FE	/* error bit mask */

/* special characters */
#define dbell	7
#define bs	8
#define lf	10	
#define cr	13	
#define dc1 	0x11	
#define dc3 	0x13	

getchar()
{
register char thechar;
int echolf, echoc;
	echoc = echolf = *((short *) CP_OptBuffer);
	echolf &= 2;	/* option switch 6 on pack in upper corner */
	echoc &= 2;	/* option switch 6 on pack in upper corner */
	while (1) {
		thechar = sget();
		switch (thechar) {
		    default: if ((thechar >= ' ') && (echoc))
			     	sput(thechar);
			     return(thechar);
		    case lf: if (echolf) continue;
			     return(thechar);
		    case cr: if (echoc) sput(thechar);
			     if (echolf) sput(lf);
			     return(thechar);
		    case bs: if (echoc) {
				sput(bs);
				sput(' ');
				sput(bs);
			     }
			     return(thechar);
		}
	}
}

putchar(c)
char c;
{
int echolf;
        if (c == lf) {
		echolf = *((short *) CP_OptBuffer);
		echolf &= 2;	/* option switch 6 on pack in upper corner */
		if (echolf) sput(cr);
	}
	sput(c);
	return(0);
}

sget()
{
register struct dev2651 *dp = (struct dev2651 *) CP_Usart;
register char s;
char inchar;
	while (1) {
	    if ((s = dp->status) & RxRDY) {		/* a char is present */
		    inchar = dp->data;
		    if ((s&FE) || (s&OE)) {		/* framing error or */
			    inchar = 0;			/* overrun error */
			    dp->cmd |= ERESET;		/* reset errors */
		     } else 	inchar &= 0x7f;
		    return(inchar);
	    }
	}
}

sput(c)
char c;
{
register struct dev2651 *dp = (struct dev2651 *) CP_Usart;
register char s;
char inchar;
	while (1) {
	    if ((s = dp->status) & RxRDY) {		/* a char is present */
		    inchar = dp->data;
		    if ((s&FE) || (s&OE)) {		/* framing error or */
			    inchar = 0;		/* overrun error */
			    dp->cmd |= ERESET;		/* reset errors */
		    } else 	inchar &= 0x7f;
		    if (inchar == dc3) 		/* tandem mode */
			    while (sget() != dc1);
	    }
	    if ((dp->status) & TxRDY) {	/* a char can go out */
		    dp->data = c;
		    return(0);
	    }
	}
}

prstr(s)
char *s;
{
	for( ; *s; s++)
		putchar(*s);
}

beep(x)
{
	sput(dbell);
}

char
checki() /* check input */
{
register struct dev2651 *dp = (struct dev2651 *) CP_Usart;
	if ((dp->status) & RxRDY) return(1);
	return(0);
}


/*
 *		Beginning of main program
 */


main()
{
	setbuf(&_iob[1], 0);	/* Unbuffered stdout */
	printf("\nDCU test program\n");
	noretries = 1;		/* tell dcusaio not to retry */
	for (unitnumber = 0; unitnumber < MAXUNITS; unitnumber++)
		locktab[unitnumber] = INITWRITE;
	unitnumber = -1;
	allocptr = (int) end;
	logbuf = (struct log *) alloc(MAXLOG * sizeof(*logbuf));
	page1 = alloc(MAXSEC * PAGSIZE);
	page2 = alloc(MAXSEC * PAGSIZE);
	charreadyflag = 0;
	clearcounts();
	resetparameters();
	dumpunits();
	cmdinit();
	printf("\nType 'p' to begin the test or 'h' for help.\n");
	while (1) {
		typeprompt();
		while (stoppedflag || readcheck()) {
			typeprompt();
			charsread = 0;
			docommand();
		}
		switch (testaction) {
			case TESTREAD:
				failed = doio(READ, sector, iosize, page1);
				break;

			case TESTWRITE:
				fill(page1, iosize, sector);
				failed = doio(WRITE, sector, iosize, page1);
				break;

			case TESTBOTH:
				failed = doio(READ, sector, iosize, page1);
				if (failed) break;
				failed = doio(WRITE, sector, iosize, page1);
				if (failed) break;
				failed = doio(READ, sector, iosize, page2);
				if (failed) break;
				failed = compare(page1, page2, iosize);
				if (failed) logerror(sector, 0, LOGVERIFY);
				break;
		}
		if ((--limitcount == 0)) stoppedflag = 1;
		if (failed) beep();
		if (failed && (erroraction == ERRORSTOP)) stoppedflag = 1;
		if ((failed && (erroraction != ERRORIGNORE)) ||
			((++frequencycount % frequency) == 0))
				dostatus(); 
		if (failed && (erroraction != ERRORIGNORE)) {
			printf("\n");
			dumpsense(erroriocb.dcuio_sense);
		}
		advancesector();
	}
}


/* Non-blocking, restartable terminal character input routine.  This works by
 * returning a character if one is available, otherwise does a longjump back.
 * If further characters are then read, all previously read characters are
 * once again returned, followed by any new terminal characters.  This process
 * continues until the caller completes a command.
 *
 * NOTE: For this obscureness to work correctly, the caller must make sure
 * that no permanent state is changed until a command is parsed totally, that
 * characters will be parsed identically each pass, and that no prompting is
 * done.
 */
readchar()
{
	char	ch;
	register char	*crock;

	if (charsread < charsstored) {
		crock = (char *)((int)(charbuffer) + charsread);
		ch = *crock;
		charsread++;
		if ((ch >= 'A') && (ch <= 'Z')) ch += 'a' - 'A';
		return(ch);
	}
	if (readcheck() == 0)
		longjmp(jumpbuf, 1);
	charreadyflag = 0;
	ch = getchar() & 0xff;
	if (charsstored >= TTYSIZE) badcmd("Command too long");
	if (ch == LINERUB) badcmd("Command aborted");
	if (ch == CHARRUB) {
		if (charsstored > 0) charsstored--;
		if (charsread > 0) charsread--;
		longjmp(jumpbuf, 1);
	}
	if (ch == '\015') ch = EOL;
	crock = (char *)((int)(charbuffer) + charsstored);
	*crock = ch;
	charsstored++;
	charsread++;
	if ((ch >= 'A') && (ch <= 'Z')) ch += 'a' - 'A';
	return(ch);
}


/* Check to see if a terminal character is available. */
readcheck()
{
	if (charreadyflag == 0) charreadyflag = checki();
	return(charreadyflag);
}



/* Mark a command as being started.  Clear the character buffer, and
 * remember to prompt when we can.
 */
cmdinit()
{
	charsread = 0;
	charsstored = 0;
	promptflag = 1;
}



/* Here on a bad command.  Type a message, kill the command, and return. */
badcmd(str)
	char	*str;
{
	if (str) printf("\n%s.\n", str);
	cmdinit();
	longjmp(jumpbuf, 1);
}



/* Read a nonnegative integer, and use a default value if none was given.
 * The number must be terminated with a EOL or a space.
 */
readnumber(defval)
	int	defval;
{
	register char	ch;
	register long	num;

	while ((ch = readchar()) == ' ');
	num = 0;
	gotnum = 0;
	while ((ch >= '0') && (ch <= '9')) {
		num = (num * 10) + ch - '0';
		gotnum++;
		ch = readchar();
	}
	if (!gotnum) num = defval;
	num &= MAXINT;
	if (ch == ' ') return(num);
	if (ch != EOL) badcmd("Bad number typed");
	readbackup();
	return(num);
}



/* Read a hex number */
readhex()
{
	register char	ch;
	register long	num;

	while ((ch = readchar()) == ' ');
	num = 0;
	while (1) {
		if ((ch >= '0') && (ch <= '9')) {
			num = (num * 16) + ch - '0';
			ch = readchar();
			continue;
		}
		if ((ch < 'a') || (ch > 'f')) break;
		num = (num * 16) + ch - 'a' + 10;
		ch = readchar();
	}
	if (ch == ' ') return(num);
	if (ch != EOL) badcmd("Bad number typed");
	readbackup();
	return(num);
}



/* Read a possibly negative number, and ignore termination characters */
readinteger()
{
	register char	ch;
	register long	num;
	register int	negfac;

	num = 0;
	negfac = 1;
	ch = readchar();
	if (ch == '-') {
		negfac = -1;
		ch = readchar();
	}
	while ((ch >= '0') && (ch <= '9')) {
		num = (num * 10) + ch - '0';
		ch = readchar();
	}
	readbackup();
	return(num * negfac);
}


/* Read a sector number, which is of the form "c<n>t<n>s<n>", where c, t,
 * and s (along with their numeric argument) are optional.  This allows easy
 * selection of sectors.  For example, "c3s20" indicates sector 20 within
 * cylinder 3.  Negative numbers indicate sectors measured backwards from
 * the end of the appropriate range.  For example, "c0s-1" means the last
 * sector within cylinder 0.
 */
readsectornumber(defval)
{
	register int	cyl;
	register int	trk;
	register int	sec;
	register int	max;
	register char	ch;
	char	gotcyl, gottrk;

	cyl = 0;
	trk = 0;
	gotcyl = 0;
	gottrk = 0;
	gotnum = 0;
	while ((ch = readchar()) == ' ');
	if (ch == EOL) {
		readbackup();
		return(defval);
	}
	gotnum++;
	if (ch == 'c') {		/* cylinder specification */
		gotcyl = 1;
		cyl = readinteger();
		if (cyl < 0) cyl += cylunit;
		if ((cyl < 0) || (cyl >= cylunit)) badcmd("Bad cylinder value");
		ch = readchar();
	}
	if (ch == 't') {		/* track specification */
		gottrk = 1;
		trk = readinteger();
		max = trkunit;
		if (gotcyl) max = trkcyl;
		if (trk < 0) {
			trk += max;
			if (gotcyl == 0) cyl = cylunit - 1;
		}
		if ((trk < 0) || (trk >= max)) badcmd("Bad track value");
		ch = readchar();
	}
	if (ch != 's') readbackup();
	sec = readinteger();		/* sector specification */
	max = secunit;
	if (gotcyl) max = seccyl;
	if (gottrk) max = sectrk;
	if (sec < 0) {
		sec += max;
		if (gotcyl == 0) cyl = cylunit - 1;
		else if (gottrk == 0) trk = trkcyl - 1;
	}
	if ((sec < 0) || (sec >= max)) badcmd("Bad sector value");
	ch = readchar();
	if ((ch != ' ') && (ch != EOL)) badcmd("Bad sector number");
	readbackup();
	return((cyl * seccyl) + (trk * sectrk) + sec);
}


/* Check that the current character is the beginning of a numeric command */
checknum(cmd, ch)
	register char	ch;
{
	if ((ch == ' ') || (ch == '-')) return;
	if ((ch >= '0') && (ch <= '9')) return;
	printf("\nIllegal '%c' command.\n", cmd);
	badcmd(0);
}


/* Restore the last read character for rereading. */
readbackup()
{
	if (charsread > 0) charsread--;
}



/* Type the prompt to the user, only if it is necessary. */
typeprompt()
{
	if (promptflag == 0) return;
	prstr("\nDCUTEST (");
	prstr(stoppedflag ? "stopped) > " : "running) > ");
	promptflag = 0;
}


/* Read a single command from the user.   Multiple-character commands are
 * aborted when not enough characters are ready, and when we next enter this
 * routine, they will restart and proceed further.
 */
docommand()
{
	register char	ch;
	register long	value, value2;

	if (setjmp(jumpbuf)) return;
	ch = readchar();
	switch (ch) {

	case EOL:			/* null command */
		break;

	case 'h':
	case '?':			/* give help text */
		dohelp();
		break;

	case 'p':			/* proceed with test */
		printf("\nProceeding...\n");
		stoppedflag = 0;
		break;

	case 'o':			/* do one more operation */
		printf("\nDoing one operation.\n");
		stoppedflag = 0;
		limitcount = 1;
		break;

	case 'l':			/* set limit counter */
		value = readnumber(INITLIMIT);
		printf("\nLimit on operations now %ld.\n", value);
		limitcount = value;
		break;

	case 'z':			/* clear counters */
		printf("\nCounters cleared.\n");
		clearcounts();
		break;

	case 'f':			/* set status frequency */
		value = readnumber(INITFREQUENCY);
		if (value == 0) value = 1;
		frequency = value;
		printf("\nStatus frequency now %ld.\n", frequency);
		frequencycount = 0;
		break;

	case ' ':			/* give nonstopping status */
		dostatus();
		break;

	case DEL:			/* quit program */
		stoppedflag = 0;
		dostatus();
		printf("\nExiting test.\n");
		asm("bpt");
		break;

	case 's':			/* stop until proceeded */
		stoppedflag = 1;
		dostatus();
		break;

	case 'u':			/* unit operations */
		ch = readchar();
		switch (ch) {
		case 'a':		/* unit autoconfigure from table */
			dcuautoconfig(unitnumber);
			break;
		case 'c':		/* unit configure from user */
			reconfigure(unitnumber);
			break;
		default:		/* specify unit to test */
			checknum('u',ch);
			readbackup();
			value = readnumber(-1);
			if (setunit(value)) badcmd(0);
			printf("\nUsing unit %ld.\n", value);
			break;
		}
		break;

	case 'c':			/* convert sector number */
		value = readsectornumber(0);
		printf("\nSector %d  =  cylinder %d  track %d  sector %d.\n",
			value, value/seccyl, value%seccyl/sectrk, value%sectrk);
		break;

	case 'b':			/* set bounds for test */
		value = readsectornumber(0);
		value2 = readsectornumber(secunit-1);
		if (setbounds(value, value2)) badcmd(0);
		printf("\nSector boundaries now %d to %d.\n",
			lowsector, highsector);
		break;

	case 'w':			/* set write action */
		switch (readchar()) {
		case 'e':
			printf("\n\nWriting enabled on unit %d.\n",
				unitnumber);
			locktab[unitnumber] = WRITEENABLE;
			break;
		case 'l':
			printf("\n\nWrites disabled on unit %d.\n",
				unitnumber);
			locktab[unitnumber] = WRITELOCK;
			break;
		case 'a':
			printf("\n\nWrites asked about on unit %d.\n",
				unitnumber);
			locktab[unitnumber] = WRITEASK;
			break;
		default:
			badcmd("Illegal 'w' command");
		}
		break;

	case 't':			/* set test type */
		switch (readchar()) {
		case 'r':
			printf("\n\nTest set to read only.\n");
			testaction = TESTREAD;
			break;

		case 'w':
			switch (locktab[unitnumber]) {
			case WRITEASK:
				printf("\007\n\n\
WARNING:  This test destroys the data currently on the disk.\n\
If you still want to write on unit %d, type 'y' : ", unitnumber);
				if (getchar() != 'y') {
					badcmd("Command aborted");
				}
			case WRITEENABLE:
				break;
			default:
				badcmd("\nWrites are not enabled");
			}
			printf("\n\nTest set to write only.\n");
			testaction = TESTWRITE;
			break;

		case 'b':
			printf("\n\nTest set to read and write.\n");
			testaction = TESTBOTH;
			break;

		default:
			badcmd("Illegal 't' command");
		}
		break;

	case 'i':			/* set sector selection */
		ch = readchar();
		switch (ch) {
		case 's':
			printf("\nSequential IO mode.\n");
			iomode = IOSEQUENTIAL;
			sectorincrement = 1;
			break;

		case 'i':
			value = readsectornumber(0);
			if ((value <= -secunit) ||
				(value >= secunit)) {
				badcmd("Illegal increment");
			}
			printf("\nIncrementing by %ld sectors.\n", value);
			iomode = IOSEQUENTIAL;
			sectorincrement = value;
			break;

		case 'r':
			value = readnumber(0);
			if (value == 0) value == 17;
			seed = value;
			printf("\nRandom IO mode with seed %ld.\n", value);
			iomode = IORANDOM;
			break;

		case 'l':
			value = readsectornumber(INITIOSIZE);
			if ((value <= 0) || (value > MAXSEC)) {
				badcmd("Illegal sector count");
			}
			printf("\nSector transfer count now %ld.\n", value);
			iosize = value;
			break;

		default:
			checknum('i', ch);
			readbackup();
			value = readsectornumber(0);
			if ((value < 0) ||
				(value >= secunit)) {
				badcmd("Illegal sector number");
			}
			sector = value;
			printf("\nNext sector for IO is %ld.\n",
				sector);
			break;
		}
		break;

	case 'e':			/* actions for errors */
		switch (readchar()) {
		case 'p':
			printf("\nType and proceed on errors.\n");
			erroraction = ERRORTYPE;
			break;

		case 's':
			printf("\nType and stop on errors.\n");
			erroraction = ERRORSTOP;
			break;

		case 'i':
			printf("\nIgnore errors.\n");
			erroraction = ERRORIGNORE;
			break;

		default:
			badcmd("Illegal 'e' command");
		}
		break;

	case 'x':			/* formatting commands */
		switch(readchar()) {
		case 'h':
			switch(locktab[unitnumber]) {
				case WRITEASK:
					printf("\007\n\
WARNING: Verify unit %d header information. If correct, type 'y' : ",
					unitnumber);
					if (getchar() != 'y')
						badcmd("Command aborted");
				case WRITEENABLE:
					writeheader(unitnumber);
					break;
				default:
					badcmd("Writes are not enabled");
			}
			break;
		case 'p':
			value = readnumber(unittable[unitnumber]->dt_ploz);
			value2 = readnumber(unittable[unitnumber]->dt_plonz);
			setploz(unitnumber, value, value2);
			break;

		case 'r':
			value = readnumber(INITFORMATTRIES);
			printf("\nRetries maximum is %d, old value was %d.\n",
				value, formattries);
			formattries = value;
			break;

		case 'i':
			value = readnumber(0);
			if (gotnum == 0) {
				printf("\nInterleave value is %d.\n",
					interleave);
				break;
			}
			value2 = interleave;
			if (dointerleave(value)) badcmd("Cannot set interleave");
			printf("\nInterleave value is %d, old value was %d.\n",
				interleave, value2);
			break;

		default:
			readbackup();
			value = readnumber(0);
			value2 = readnumber(gotnum ? value : cylunit-1);
			switch (locktab[unitnumber]) {
				case WRITEASK:
					printf("\007\n\
WARNING: Formatting can destroy the data on the disk.  If you still\n\
want to format unit %d (cylinders %d to %d), type 'y' : ",
						unitnumber, value, value2);
					if (getchar() != 'y')
						badcmd("Command aborted");
				case WRITEENABLE:
					break;
				default:
					badcmd("Writes are not enabled");
			}
			format(value, value2);
		}
		break;

	case 'd':		/* dump data to user */
		ch = readchar();
		switch (ch) {
		case 'b':	/* dump buffer data */
			printf("\n");
			dumppage(page1, sector);
			break;

		case 't':	/* dump all disk types */
			dumptypes();
			break;

		case 'e':	/* dump error code */
			value = readhex();
			dumpsense(value);
			break;

		case 'i':	/* dump last error iocb */
			dumpiocb(&erroriocb);
			break;

		case 'l':	/* dump error log */
			dumplog();
			break;

		case 'p':		/* dump unit partition table */
			dumppartitions(unitnumber);
			break;

		case 's':	/* dump units after selftest */
			printf("\n");
			failed = doselftest();
			if (failed) badcmd("Selftest failed");
			dumpunits();
			break;

		case 'u':	/* dump units */
			printf("\n");
			dumpunits();
			break;

		case 'v':	/* dump current values */
			dumpvalues();
			break;

		default:
			checknum('d', ch);
			readbackup();
			value = readsectornumber(-1);
			if ((value < 0) || (value >= secunit)) {
				badcmd("Illegal sector number");
			}
			failed = doio(READ, value, 1, page1);
			if (failed) printf("\nRead failed.\n");
			else dumppage(page1, value);
			break;
		}
		break;

	case 'r':		/* reset all parameter values */
		resetparameters();
		printf("\n\nParameters reset.\n");
		break;

	default:
		printf("\n\nUnknown command '%c'. Type h for help.\n", ch);
	}
	cmdinit();
}

/*
 *	Read the parameters of the currently selected unit which will be
 *	in the form of c<n>t<n>s<n>, (in any order) where c,t, and s are
 *	optional. If any field is missing, that field of the disk parameters
 *	is not changed. The last value typed for c, t, or s will override
 *	any previous values.
 */

reconfigure(unit)
{
	register struct disktab *dt;
	int cyl,trk,sec;
	char ch;

	cyl = trk = sec = 0;

	while ((ch = readchar()) == ' ');

	while (ch != EOL) {
		switch (ch) {
		case 'c':		/* cylinder specification */
			cyl = readinteger();
			if (cyl <= 0) badcmd("Bad cylinder value");
			break;
		case 't':		/* track specification */
			trk = readinteger();
			if (trk <= 0) badcmd("Bad track value");
			break;
		case 's':		/* sector specification */
			sec = readinteger();
			if (sec <= 0) badcmd("Bad sector value");
			break;
		case ' ':
		case ',':		/* allow white space ...*/
			break;
		default:
			badcmd("Bad configuration value");
			break;
		}
		ch = readchar();
	}
	dt = unittable[unit];
	if (cyl == 0) cyl = dt->dt_ncylinders;
	if (trk == 0) trk = dt->dt_ntracks;
	if (sec == 0) sec = dt->dt_nsectors;
	dt->dt_ncylinders = cyl;
	dt->dt_ntracks = trk;
	dt->dt_nsectors = sec;
	dt->dt_secpercyl = sec * trk;
	dt->dt_secperunit = dt->dt_secpercyl * cyl;
	readbackup();
}

/*
 * Write the drive header to block 0 of the selected unit.
 */

writeheader(unit)
{
	register struct disktab *dp,*dt;
	register int np;
	register long	error;
	char buffer[BSIZE+PAGSIZE];

	dt = unittable[unit];
	dp = (struct disktab*)((((int)buffer)+PAGSIZE) & ~PAGMAX);

	if (dt == 0) {
		badcmd("\007\nUnit not found");
	}
	dt->dt_magic = DMAGIC;
	*dp = *dt;
	np = 1;		/* used to be np = BSIZE/PAGSIZE */

	if (dodiskio(WRITE,unit,0,np,dp) != np) {
		printf("\007\n\nUnit %d header write failed, error %d\n",
			unit,dcerror);
	} else {
		printf("\n\nUnit %d header written\n",unit);
	}
	return(error);
}

dumppartitions(unit)
register int unit;
{
	register struct partition *pp;
	register struct disktab *dt;
	register int i;

	dt = unittable[unit];
	if (dt == 0) return;
	pp = dt->dt_partition;
	printf("\n\nUnit %d partitions",unit);
	printf("\n\
partition     cyloff  nblocks\n");
	for(i = 0; i < 8; i++) {
		printf("    %1d         %5d   %6d\n",
		i, pp[i].cyloff, pp[i].nbl);
	}
}

/* Give the help output */
dohelp()
{
	register char	**s;
	register int	i;
	register char	ch;

	printf("\n\n");
	s = helptext;
	morecount = MORESIZE;
	while (*s) {
		if (more()) return;
		printf("%s\n", *s);
		s++;
		i--;
	}
}


/* Provide a "more" feature for long displays. Returns nonzero if should quit.
 * Before first use, the morecount variable should be set to MORESIZE.
 */
more()
{
	int	i;
	char	ch;

	if (morecount-- > 0) return(0);
	morecount = MORESIZE;
	prstr("--more--");
	ch = getchar();
	for (i = 0; i < 12; i++) prstr("\b \b");
	return(ch != ' ');	/* continue if typed space */
}


/* Allocate memory.  Returns data which is guaranteed to fit on one page,
 * unless it cannot, in which case it is page aligned.  The data is also
 * word aligned.
 */
char *
alloc(size)
{
	if (allocptr & 1) allocptr++;
	if (((allocptr & PAGMAX) != 0)  &&	
		((allocptr ^ (allocptr + size - 1)) >= PAGSIZE)) {
			allocptr = (allocptr | PAGMAX) + 1;
	}
	allocptr = allocptr + size;
	return ((char *) (allocptr - size));
}



/* compute a random integer from 0 to range - 1. */
random(range)
{
	seed = HASH(seed);
	return (seed % range);
}



/* Advance to the next sector on the disk, sequentially or randomly. */
advancesector()
{
	register long	minsector;
	register long	maxsector;
	register long	sectorrange;
	register long	i;

	minsector = lowsector;
	maxsector = highsector - iosize + 1;
	sectorrange = maxsector - minsector + 1;
	switch (iomode) {
		case IOSEQUENTIAL:
			sector = sector + sectorincrement;
			i = minsector - sector;
			if (i > 0) sector = maxsector - i + 1;
			i = sector - maxsector;
			if (i > 0) sector = minsector + i - 1;
			break;

		case IORANDOM:
			sector = minsector + random(sectorrange);
			break;

	}
}



/* Reset all parameters to their default value. */
resetparameters()
{
	register int	unit;

	lowsector = 0;
	highsector = secunit - 1;
	iosize = INITIOSIZE;
	frequency = INITFREQUENCY;
	erroraction = INITERROR;
	testaction = INITTEST;
	sectorincrement = INITINCREMENT;
	iomode = INITIOMODE;
	limitcount = INITLIMIT;
	formattries = INITFORMATTRIES;
	sector = 0;
	seed = 17;
	stoppedflag = 1;
	for (unit = 0; unit < MAXUNITS; unit++) locktab[unit] = INITWRITE;
}



/* Clear all error counters. */
clearcounts()
{
	readcount = 0;
	writecount = 0;
	compcount = 0;
	readstatus = 0;
	writestatus = 0;
	readerrorcount = 0;
	writeerrorcount = 0;
	comperrorcount = 0;
	readerrorsector = -1;
	writeerrorsector = -1;
	comperrorsector = -1;
	frequencycount = 0;
	logcount = 0;
	logover = 0;
}



/* Insert an error into the error log */
logerror(sector, status, type)
	register long	sector;
{
	register struct	log *lp;
	register struct	log *endlp;

	lp = logbuf;
	endlp = &lp[logcount];
	while ((lp < endlp) && (lp->sector != sector)) lp++;
	if (lp >= endlp) {
		if (logcount >= MAXLOG) {
			logover++;
			return;
		}
		logcount++;
	}
	lp->sector = sector;
	lp->status = status;
	lp->type = type;
}


/* Dump the current status of the test. */
dostatus()
{
	if ((readcount == 0) && (writecount == 0)) {
		printf("\n\nNo reads/writes done yet.\n");
	} else printf("\n\n\
        Pages   Errors  Badsec  Status,  current sector: %ld\n\
        -----------------------\n", sector);

	if (readcount) {
		printf("\
Read:   %5d   %5d   %5d     %x\n",
		readcount, readerrorcount, readerrorsector, readstatus);
	}

	if (writecount) {
		printf("\
Write:  %5d   %5d   %5d     %x\n",
		writecount, writeerrorcount, writeerrorsector, writestatus);
	}

	if (compcount) {
		printf("\
Verify: %5d   %5d   %5d     -\n",
		compcount, comperrorcount, comperrorsector);
	}

	frequencycount = 0;
}


/* Dump out a page of data */
dumppage(address, sector)
	register long *address;
{
	register offset;

	printf("\nData in sector %ld:\n", sector);
	for (offset = 0; offset < PAGSIZE / 4; offset++) {
		if ((offset % 4) == 0) printf("\n%ld/", offset * 4);
		printf("\t%.8x", *address++);
	}
	printf("\n");
}



/* Fill a number of pages of data with random data. */
fill(addr, iosize, seed)
	register long	*addr;		/* pointer to data */
{
	register long	*endaddr;	/* last address */

	endaddr = addr + ((PAGSIZE / 4) * iosize);
	while (addr < endaddr) {
		*addr++ = seed;
		seed = HASH(seed);
	}
}



/* Compare two arrays of data to see if they contain the same data. */
compare(addr1, addr2, iosize)
	long	*addr1, *addr2;		/* pointers to data */
{
	compcount += iosize;
	iosize *= (PAGSIZE / 4);
	while ((iosize -= 4) >= 0) {
		if (*addr1 != *addr2) {
			comperrorcount++;
			comperrorsector = sector;
			return (1);
		}
	}
	return (0);
}


/* Read or write the disk.  Returns nonzero on an error, and logs it. */
doio(writeflag, sector, iosize, address)
	register int	iosize;
{
	register long	error;
	register long	resid;

	error = dodiskio(writeflag, unitnumber, sector, iosize, address);
	if (error != iosize) {
		resid = erroriocb.dcuio_resid - 1;
		if (resid < 0) resid = 0;
		if (resid >= iosize) resid = iosize - 1;
		iosize -= resid;
	}
	error = dcerror;
	if (writeflag) {
		writecount += iosize;
	} else {
		readcount += iosize;
	}
	if (error == 0) return(0);
	if (writeflag) {
		writestatus = error;
		writeerrorcount++;
		writeerrorsector = sector;
	} else {
		readstatus = error;
		readerrorcount++;
		readerrorsector = sector;
	}
	logerror(sector, error, writeflag ? LOGWRITE : LOGREAD);
	return (1);
}


/* Set ploz and plonz values for formatting */
setploz(unit, ploz, plonz)
	register int	unit;
{
	register struct disktab *dt;
	int	oldploz, oldplonz;

	if (dc_initunit(unit)) {
		printf("Cannot find unit %d.\n", unit);
		return(1);
	}
	dt = unittable[unit];
	oldploz = dt->dt_ploz;
	oldplonz = dt->dt_plonz;
	printf("\nPLOZ is %d, PLONZ is %d.\n", ploz, plonz);
	if ((ploz == oldploz) && (plonz == oldplonz)) return(0);
	dt->dt_ploz = ploz;
	dt->dt_plonz = plonz;
	printf("Old PLOZ was %d, old PLONZ was %d.\n", oldploz, oldplonz);
	return(0);
}


/* Format specified cylinders of the current disk unit. */
format(lowcyl, highcyl)
{
	register int	cyl;
	register int	trk;
	register int	error;
	register int	hardcount;
	register int	softcount;
	int	cylcount;
	int	cyltotal;
	int	errstat;

	if (dc_initunit(unitnumber) != 0) {
		printf("\nCannot find unit %d.\n", unitnumber);
		return(-1);
	}
	if (lowcyl < 0) lowcyl = 0;
	if (highcyl >= cylunit) highcyl = cylunit - 1;
	cyltotal = highcyl - lowcyl + 1;
	hardcount = 0;
	softcount = 0;
	cylcount = 0;
	printf("\n\nFormatting cylinders %d to %d on unit %d.\n",
		lowcyl, highcyl, unitnumber);
	for (cyl = lowcyl; cyl <= highcyl; cyl++) {
		if (readcheck() && (getchar() == DEL)) {
			printf("\nFormat aborted on cylinder %d.\n", cyl);
			break;
		}
		for (trk = 0; trk < trkcyl; trk++) {
			error = formattrack(unitnumber, cyl, trk, &errstat);
			if (error < 0) hardcount++;
			if (error > 0) softcount++;
		}
		if ((++cylcount % 10) == 0) {
			printf(" Completed %d cylinders (%d%c).\n",
				cylcount, (cylcount*100)/cyltotal, '%');
		}
	}
	printf("Format completed (%d hard error%s, %d soft error%s).\n",
		hardcount, hardcount==1?"":"s", softcount, softcount==1?"":"s");

}


/* Format a particular track on a cylinder.  Returns 0 if it was successful,
 * positive if successful after some retrying, or negative if it failed.
 * Error status is returned through reterror.
 */
formattrack(unit, cyl, trk, reterror)
	register int	unit;
	int	*reterror;
{
	register int	tries;
	register int	error;
	register char	*str;
	int	type;
	int	sector;
	int	sectrk;
	int	softerr;

	*reterror = 0;
	if (dc_initunit(unit)) return(-1);
	sectrk = unittable[unit]->dt_nsectors;
	sector = (cyl * unittable[unit]->dt_secpercyl) + (trk * sectrk);
	error = 0;
	dodiskio(0, unit, sector, sectrk, page1);	/* try to keep data */
	for (tries = 0; tries <= formattries; tries++) {
		softerr = error;
		str = "Format failed";
		type = LOGFORMAT;
		if (error = doformat(unit, cyl, trk)) continue;
		str = "Write failed";
		type = LOGWRITE;
		if (dodiskio(1, unit, sector, sectrk, page1) != sectrk) {
			error = dcerror;
			continue;
		}
		str = "Read failed";
		type = LOGREAD;
		if (dodiskio(0, unit, sector, sectrk, page1) != sectrk) {
			error = dcerror;
			continue;
		}
		if (dodiskio(0, unit, sector, sectrk, page1) != sectrk) {
			error = dcerror;
			continue;
		}
		if (tries == 0) return(0);
		printf("Needed %d ", tries);
		str = "retries";
		if (tries == 1) str = "retry";
		error = softerr;
		break;
	}
	printf("%s for cyl %d trk %d (status %x).\n", str, cyl, trk, error);
	if (tries > formattries) {	/* hard error */
		tries = -1;
		logerror(sector, error, type);
	}
	*reterror = error;
	return(tries);
}


/* Set the unit number for doing future IO to. */
setunit(unit)
{
	register struct disktab *dt;

	if ((unit < 0) || (unit >= MAXUNITS)) {
		printf("\nUnit number %d is illegal.\n", unit);
		return(1);
	}
	if (dc_initunit(unit) != 0) {
		printf("\nCannot find unit %d.\n", unit);
		return(1);
	}
	dt = unittable[unit];
	unitnumber = unit;
	cylunit = dt->dt_ncylinders;
	trkcyl = dt->dt_ntracks;
	secunit = dt->dt_secperunit;
	trkunit = cylunit * trkcyl;
	seccyl = secunit / cylunit;
	sectrk = seccyl / trkcyl;
	lowsector = 0;
	highsector = secunit - 1;
	sector = 0;
	return(0);
}



/* Set the lower and upper bounds for testing. */
setbounds(low, high)
{
	if ((low < 0) || (high >= secunit) || (low > high)) {
		printf("\nIllegal sector boundary.\n");
		return(1);
	}
	lowsector = low;
	highsector = high;
	sector = lowsector;
	return(0);
}


/* Find all of the disk units on the system. */
dumpunits()
{
	register int count;
	register int unit;
	register struct disktab *dt;
	register char	*str;

	count = 0;
	for (unit = 0; unit < MAXUNITS; unit++) {
		if (dc_initunit(unit)) continue;
		if (unitnumber < 0) setunit(unit);
		count++;
	}
	if (count == 0) {
		printf("\nNo drives found.\n");
		return;
	}
/*
	printf("\n\
unit    type    sectrk  trkcyl  cylunit  seccyl secunit writes\n");
unit    type   sectrk trkcyl cylynit seccyl  secunit writes\n"); */
	printf("\nunit  type  sectrk trkcyl cylunit seccyl secunit writes\n");
	for (unit = 0; unit < MAXUNITS; unit++) {
		dt = unittable[unit];
		if (dt == 0) continue;
		str = "  ";
		if (unit == unitnumber) str = "* ";
		printf("%.2s%1d   %-5s   %2d     %-2d    %4d    %3d   %6d   ",
			str, unit, dt->dt_name, dt->dt_nsectors, dt->dt_ntracks,
			dt->dt_ncylinders, dt->dt_secpercyl, dt->dt_secperunit);
		switch(locktab[unit]) {
			case WRITEENABLE:
				printf("%-3s\n", "YES");
				break;
			case WRITEASK:
				printf("%-3s\n", "ASK");
				break;
			default:
				printf("%-3s\n", "NO");
		}
	}
}


/* Dump all known disk types */
dumptypes()
{
	register struct	unitdata *ud;

/*	printf("\n\n\
type    sectrk  trkcyl  cylunit seccyl  secunit ploz    plonz\n"); */
	printf("\n\ntype  sectrk trkcyl cylunit seccyl secunit ploz plonz\n");
	for (ud = unitsizetable; ud->d_nsectors; ud++) {
		printf("%-5s   %2d     %-2d    %4d    %3d   %6d  %3d   %3d\n",
			ud->ud_name, ud->d_nsectors, ud->d_ntracks,
			ud->d_ncylinders, ud->d_secpercyl, ud->d_secperunit,
			ud->ud_ploz, ud->ud_plonz);
	}
}


/* Dump out all of the current parameter values. */
dumpvalues()
{
	printf("\n\nUnit being tested: %d.\n", unitnumber);
	printf("Current test: ");
	switch (testaction) {
		case TESTBOTH:	printf("Write and read.\n");
				break;
		case TESTREAD:	printf("Read only.\n");
				break;
		case TESTWRITE:	printf("Write only.\n");
				break;
	}
	printf("Sector selection: ");
	switch (iomode) {
		case IOSEQUENTIAL:
			printf("Sequential, increment %d.\n", sectorincrement);
				break;
		case IOREPEAT:	printf("Repeat sector %d.\n", sector);
				break;
		case IORANDOM:	printf("Random.\n");
				break;
	}
	printf("Sector range for testing: %d to %d.\n", lowsector, highsector);
	printf("Sectors per transfer: %ld\n", iosize);
	printf("Current sector: %ld\n", sector);
	printf("Random seed value: %ld\n", seed);
	printf("Frequency of status typeout: %ld.\n", frequency);
	printf("Action on an error: ");
	switch (erroraction) {
		case ERRORIGNORE:
				printf("Ignore it.\n");
				break;
		case ERRORTYPE:	printf("Type status and proceed.\n");
				break;
		case ERRORSTOP:	printf("Type status and stop.\n");
				break;
	}
	if (limitcount <= 0)
		printf("No transfer limit.\n");
	else
		printf("Transfer limit: %ld\n", limitcount);
	printf("Maximum retries when formatting: %d\n", formattries);
}


/* Dump the error log */
dumplog()
{
	register struct	log	*lp;
	register struct	log	*endlp;
	register char	*str;

	if (logcount == 0) {
		printf("\n\nNo errors logged.\n");
		return;
	}
	printf("\n\n\
Sector   Cyl/Trk  Function  Status\n");
	qsort(logbuf, logcount, sizeof(*logbuf), logcmp);
	morecount = MORESIZE-1;
	for (lp = logbuf; lp < &logbuf[logcount]; lp++) {
		switch (lp->type) {
			case LOGREAD:
				str = "read";
				break;
			case LOGWRITE:
				str = "write";
				break;
			case LOGFORMAT:
				str = "format";
				break;
			case LOGVERIFY:
				str = "verify";
				break;
			default:
				str = "???";
		}
		if (more()) return;
		printf("%5d   %4d/%-4d  %-6s     %x\n", lp->sector,
			lp->sector / seccyl, (lp->sector % seccyl) / sectrk,
			str, lp->status);
	}
	if (logover) {
		if (more()) return;
		printf("Buffer overflow, %d unlogged errors.\n", logover);
	}
}

/* Comparison routine for qsort */
logcmp(e1, e2)
	struct	log	*e1, *e2;
{
	return(e1->sector - e2->sector);
}


/* dump out an IOCB block */
dumpiocb(io)
	register struct iocb *io;
{
	printf("\n\nOpcode: %x\n", io->dcuio_opcode);
	printf("Drive param: %x\n", io->dcuio_drivepar);
	printf("Sector count: %ld\n", io->dcuio_count);
	printf("Cylinder: %ld\n", io->dcuio_cylinder);
	printf("Drive: %ld,   Head: %ld,   Sector: %ld\n", io->dcuio_drhdsec>>13,
		(io->dcuio_drhdsec>>8) & 0x1f, io->dcuio_drhdsec & 0xff);
	printf("Buffer address: %x", io->dcuio_buffer);
	dumpsense(io->dcuio_sense);
	if (io->dcuio_ecc) printf("ECC bytes 3-0: %X\n", io->dcuio_ecc);
	if (io->dcuio_chain) printf("Chain address: %x\n", io->dcuio_chain);
	printf("Ending status: %x\n", io->dcuio_status);
	bitout("Drive check", io->dcuio_status & DCUES_DRIVCHECK);
	bitout("Controller check", io->dcuio_status & DCUES_CTRLCHECK);
	bitout("Bus check", io->dcuio_status & DCUES_BUSCHECK);
	bitout("Command acknowledge", io->dcuio_status & DCUES_CHAIN);
	bitout("Completion", io->dcuio_status & DCUES_DONE);
	printf("Residual: %ld\n", io->dcuio_resid);
}



/* dump out the sense bytes */
dumpsense(data)
	register long	data;
{
	register long	code;

	printf("\nSense bytes 3-0: %X\n", data);
	bitout("Sequence error", data & DCUSB_SEQERR);
	code = BUSFAULTCODE(data);
	if (code != DCUBF_NOERROR) {
		printf("  Bus fault code: %ld (", code);
 		switch (code) {
		case DCUBF_DRIVETIME: 	printf("Drive acknowledge timeout");
					break;
		case DCUBF_MESAERROR: 	printf("Mesa interface error");
					break;
		case DCUBF_PARITY: 	printf("CBUS input parity error");
					break;
		case DCUBF_INDEXTIME: 	printf("Index/sector pulse timeout");
					break;
		case DCUBF_WORDTIME: 	printf("Word flag timeout");
					break;
		case DCUBF_DMATIME: 	printf("DMA transfer timeout");
					break;
		case DCUBF_CBUSBUSY: 	printf("CBUS busy");
					break;
		}
		printf(")\n");
	}
	code = CTLFAULTCODE(data);
	if (code != DCUCF_NOERROR) {
		printf("  Control fault code: %ld (", code);
		switch (code) {
		case DCUCF_OCCUPIED: 	printf("Drive occupied");
					break;
		case DCUCF_NOSECTOR: 	printf("Sector not found");
					break;
		case DCUCF_ECCHEADER: 	printf("ECC error in header field");
					break;
		case DCUCF_ECCDATA: 	printf("ECC error in data field");
					break;
		case DCUCF_ILLPARAM: 	printf("Illegal DCU parameter");
					break;
		case DCUCF_ILLCMMD: 	printf("Illegal DCU command");
					break;
		case DCUCF_REJECT: 	printf("Rejected DCU command");
					break;
		case DCUCF_WRITELOCK: 	printf("Write protection error");
					break;
		case DCUCF_MEMERROR: 	printf("Memory read error");
					break;
		case DCUCF_SYNCERROR: 	printf("Sync error in data field");
					break;
		case DCUCF_DRIVEERR: 	printf("Drive fault");
					break;
		case DCUCF_NODRIVES:	printf("No available drive");
					break;
		case DCUCF_DRIVEADDR:	printf("Incorrect drive address");
					break;
		default:		printf("Undefined error code");
					break;
		}
		printf(")\n");
	}
	bitout("Device not ready", data & DCUSB_NOTREADY);
	bitout("Control bus error", data & DCUSB_BUSERROR);
	bitout("Illegal command", data & DCUSB_ILLCMMD);
	bitout("Illegal parameter", data & DCUSB_ILLPARAM);
	bitout("Sense 2 nonzero", data & DCUSB_SENSE2);
	bitout("Sense 3 nonzero", data & DCUSB_SENSE3);
	bitout("Busy executing", data & DCUSB_BUSY);
	bitout("Normal completion", data & DCUSB_NORMAL);
	bitout("Seek error", data & DCUSB_SEEKERR);
	bitout("Read/write fault", data & DCUSB_RWFAULT);
	bitout("Power fault", data & DCUSB_POWER);
	bitout("Rd/wrt violation",data & DCUSB_VIOLATION);
	bitout("Speed error", data & DCUSB_SPEEDERR);
	bitout("Command reject", data & DCUSB_REJECT);
	bitout("Other errors", data & DCUSB_OTHERERR);
	bitout("Vendor errors", data & DCUSB_VENDORERR);
	bitout("Initial state", data & DCUSB_INITSTATE);
	bitout("Ready transition", data & DCUSB_RDYTRANS);
	bitout("Reserved to this port", data & DCUSB_THISPORT);
	bitout("Forced release", data & DCUSB_RELEASED);
	bitout("Reserved to alternate port", data & DCUSB_ALTPORT);
	bitout("Attribute table modified", data & DCUSB_ATTMOD);
	bitout("In write protected area", data & DCUSB_WRITELOCK);
	bitout("Vendor unique attentions", data & DCUSB_VENDATTN);
}


/* print a string depending if data is nonzero */
bitout(string, data)
	char	*string;
{
	if (data) printf("  %s\n", string);
}
