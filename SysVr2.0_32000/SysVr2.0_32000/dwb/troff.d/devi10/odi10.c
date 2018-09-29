char	xxxdi10[] = "@(#)di10.c	1.1";

/*
 *	dcan or di10
 *	driver for impress/imagen canon laser printer
 *	Modified to run on 3B20
 *      Nature of change involves conversion of characters used
 *	integer arithmetic, to deal with lack of sign extension on 
 *	3B20
 *	Modified from dcan code of 12/82 
 */

/*
output language from troff:
all numbers are character strings

sn	size in points
fn	font as number from 1-n
cx	ascii character x
Cxyz	funny char xyz. terminated by white space
Hn	go to absolute horizontal position n
Vn	go to absolute vertical position n (down is positive)
hn	go n units horizontally (relative)
vn	ditto vertically
nnc	move right nn, then print c (exactly 2 digits!)
		(this wart is an optimization that shrinks output file size
		 about 35% and run-time about 15% while preserving ascii-ness)
Dt ...\n	draw operation 't':
	Dl x y		line from here by x,y
	Dc d		circle of diameter d with left side here
	De x y		ellipse of axes x,y with left side here
	Da x y r	arc counter-clockwise by x,y of radius r
	D~ x y x y ...	wiggly line by x,y then x,y ...
nb a	end of line (information only -- no action needed)
	b = space before line, a = after
p	new page begins -- set v to 0
#...\n	comment
x ...\n	device control functions:
	x i	init
	x T s	name of device is s
	x r n h v	resolution is n/inch
		h = min horizontal motion, v = min vert
	x p	pause (can restart)
	x s	stop -- done for ever
	x t	generate trailer
	x f n s	font position n contains font s
	x H n	set character height to n
	x S n	set slant to N

	Subcommands like "i" are often spelled out like "init".
*/

#include	<stdio.h>
#include	<signal.h>
#include	<math.h>
#include	<ctype.h>
#include	<time.h>
#include	"../dev.h"
#include	"canon.h"
#include	"glyph.h"
#include	"spectab.h"

#define abs(n)  ((n) >= 0 ? (n) : -(n))

#define	NFONT	60

#define	TRUE	1
#define	FALSE	0

int	xx, yy;
int	xoffset	= 100;	/* shift output right by this amount */
int	yoffset = -0;	/* shift down (!) by this amount (was 120) */
			/* -120 is 1/2 inch on canon */
			/* allows for 10 inch page -- */
			/* toss away 1/2 inch on top and bottom */
float	xfac	= 1.0;	/* scaling factor for x */
float	yfac	= 1.0;	/* scaling factor for y */
int	inputarea	= 5;	/* input area = 5 * 8k bytes */
int	rotate	= 0;	/* 0 => portrait, 1 => landscape */
int	output	= 0;	/* do we do output at all? */
int	pageno	= -1;	/* output page number */
int	nolist	= 0;	/* output page list if > 0 */
int	olist[20];	/* pairs of page numbers */

int	erase	= 1;
float	aspect	= 1;	/* default aspect ratio */

struct dev dev;
struct font *fontbase[NFONT+1];
short	*pstab;
int	nsizes	= 1;
int	nfonts;
int	smnt;	/* index of first special font */
int	nchtab;
char	*chname;
short	*chtab;
char	*fitab[NFONT+1];
char	*widthtab[NFONT+1];	/* widtab would be a better name */
char	*codetab[NFONT+1];	/* device codes */

#define	FATAL	1
#define	BMASK	0377

#define	BAN_LENGTH	130	/* max number of chars in banner */
#define	BAN_SIZE	12	/* banner printed in this point size */
int	print_banner = 0;	/* don't print our banner page if zero */
char	*banstr;		/* this string will be the banner */
char	ban_buf[BAN_LENGTH+1];	/* banners from files go here */
int	ban_split = 0;		/* used in banner(), set with -H option - dumb */

int	dbg	= 0;
long	lineno	= 0;
int	res	= 972;		/* input assumed computed according to this resolution */
				/* initial value to avoid 0 divide */
FILE	*tf	= NULL;		/* output file pointer */

FILE	*fp_error = stderr;	/* error messages written to this file */
FILE	*fp_acct = stderr;	/* accounting stuff  written here */

char	*tempfile;
char	*fontdir	= "/usr/lib/font";
char	*bitdir		= "/usr/lib/font/devi10/rasti10";
char	*acctfile	= "";
int	acctpages	= 0;
int	xstat		= 0;	/* program's exit status */
int	copies		= 1;
/* TLH
char	*username	= "???";
char	*getlogin();
*/
char	*username;
char	*di10name;		/*name of di10 program, for error comments*/
char    *cuserid();
extern char devname[];

FILE *fp	= stdin;	/* input file pointer */
extern int DX, DY, maxdots;

#define	DO_ACCT		1	/* still have to do accounting if on */
#define	FILE_STARTED	2	/* file started but not completed yet */
#define	NO_ACCTFILE	8	/* couldn't open the accounting file */


main(argc, argv)
char *argv[];
{
	char buf[BUFSIZ];
	char *mktemp();
	int done();
	FILE *fp_ban;

	/* TLH
	username = getlogin();
	banstr = username;
	*/
	banstr = cuserid((char *)0);
	if (banstr == NULL)
	    banstr = "???";
	username = banstr;

	di10name = argv[0];
	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'c':
			copies = atoi(&argv[1][2]);
			break;
		case 'r':
			rotate = !rotate;
			break;
		case 't':
			tf = stdout;
			break;
		case 'x':
			xoffset = atoi(&argv[1][2]);
			break;
		case 'y':
			yoffset = atoi(&argv[1][2]);
			break;
		case 'f':
			bitdir = argv[2];
			argv++;
			argc--;
			break;
		case 'a':
			aspect = atof(&argv[1][2]);
			break;
		case 'e':
			erase = 0;
			break;
		case 'o':
			outlist(&argv[1][2]);
			break;
		case 'i':	/* set input area parameter */
			inputarea = atoi(&argv[1][2]);
			if (inputarea < 1)
				inputarea = 1;
			else if (inputarea > 5)
				inputarea = 5;
			break;
		case 'p':	/* pixels of resolution */
			DX = DY = atoi(&argv[1][2]);
			if (DX == 0)
				DX = DY = 1;
			break;
		case 'n':	/* number of dots in object */
			maxdots = atoi(&argv[1][2]);
			if (maxdots <= 0)
				maxdots = 32000;
			break;
		case 'b':
			fprintf(stderr, "It's never busy!");
			exit(0);
			break;
		case 'd':
			dbg = atoi(&argv[1][2]);
			if (dbg == 0) dbg = 1;
			tf = stdout;
			break;

		/* These options were added for the MHCC */

		case 'B':
			print_banner = 1;
			break;

		case 'h':
			banstr = &argv[1][2];
			print_banner = 1;
			break;
		case 'H':
			if ( (fp_ban = fopen(&argv[1][2], "r")) == NULL )
				error(FATAL, "can't open banner file %s", &argv[1][2]);
			fgets(ban_buf, sizeof(ban_buf), fp_ban);
			banstr = ban_buf;
			fclose(fp_ban);
			print_banner = 1;
			break;
		case 'L':
			if ( (fp_error = fopen(&argv[1][2], "a")) == NULL )  {
				fp_error = stderr;
				error(FATAL, "can't open log file %s", argv[1][2]);
			}
			break;
		case 'A':
			xstat |= DO_ACCT;
			break;
		case 'S':
			ban_split = atoi(&argv[1][2]);
			break;
		}
		argc--;
		argv++;
	}

	signal(SIGINT, done);
	signal(SIGHUP, done);
	signal(SIGQUIT, done);

	tempfile = mktemp("/tmp/dcanXXXXX");
	if (tf != stdout)
		if ((tf = fopen(tempfile, "w")) == NULL)
			error(FATAL, "can't open temporary file %s", tempfile);

	acct_file();

	if (argc <= 1)
		conv(stdin);
	else
		while (--argc > 0) {
			if (strcmp(*++argv, "-") == 0)
				fp = stdin;
			else if ((fp = fopen(*argv, "r")) == NULL)
				error(FATAL, "can't open %s", *argv);
			conv(fp);
			fclose(fp);
		}
	if ( print_banner )
		banner(banstr);
	t_wrapup();
	fclose(tf);
	sprintf(buf, "./bin/ipr -Limpress -r -o '-Downer \"'\"%s\"'\"'\", jobheader on, pagecollation on, pagereversal on\" %s 0</dev/null 1>/dev/null 2>&1 &",
		banstr, tempfile);
	if(dbg){fprintf(stderr, "executing %s\n", buf); done();}
	if (tf != stdout)
		system(buf);
	done();
}

acct_file()

{

	if ( *acctfile )  {		/* we have an accounting file */
		if ( (fp_acct = fopen(acctfile, "a")) == NULL )  {
			fp_acct = stderr;
			xstat |= NO_ACCTFILE;
			error(FATAL, "can't open accounting file");
			exit(xstat);
		}
		if ( tf != stdout )
			xstat |= DO_ACCT;
	}
}

outlist(s)	/* process list of page numbers to be printed */
char *s;
{
	int n1, n2, i;

	nolist = 0;
	while (*s) {
		n1 = 0;
		if (isdigit(*s))
			do
				n1 = 10 * n1 + *s++ - '0';
			while (isdigit(*s));
		else
			n1 = -9999;
		n2 = n1;
		if (*s == '-') {
			s++;
			n2 = 0;
			if (isdigit(*s))
				do
					n2 = 10 * n2 + *s++ - '0';
				while (isdigit(*s));
			else
				n2 = 9999;
		}
		olist[nolist++] = n1;
		olist[nolist++] = n2;
		if (*s != '\0')
			s++;
	}
	olist[nolist] = 0;
	if (dbg)
		for (i=0; i<nolist; i += 2)
			printf("%3d %3d\n", olist[i], olist[i+1]);
}

in_olist(n)	/* is n in olist? */
int n;
{
	int i;

	if (nolist == 0)
		return(1);	/* everything is included */
	for (i = 0; i < nolist; i += 2)
		if (n >= olist[i] && n <= olist[i+1])
			return(1);
	return(0);
}

conv(fp)
register FILE *fp;
{
	register int c, k;
	int m, n, i, n1, m1;
	char str[100], buf[300];

	lineno = 1;			/* line in current file */
	xstat |= FILE_STARTED;
	while ((c = getc(fp)) != EOF) {
		switch (c) {
		case '\n':		/* just count this line */
			lineno++;
			break;
		case ' ':	/* when input is text */
		case 0:		/* occasional noise creeps in */
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			/* two motion digits plus a character */
			hmot((c-'0')*10 + getc(fp)-'0');
			put1(getc(fp));
			break;
		case 'c':	/* single ascii character */
			put1(getc(fp));
			break;
		case 'C':
			fscanf(fp, "%s", str);
			put1s(str);
			break;
		case 'D':	/* draw function */
			fgets(buf, sizeof(buf), fp);
			lineno++;
			switch (buf[0]) {
			case 'l':	/* draw a line */
				sscanf(buf+1, "%d %d", &n, &m);
				t_line(n, m, ".");
				break;
			case 'c':	/* circle */
				sscanf(buf+1, "%d", &n);
				drawcirc(n);
				break;
			case 'e':	/* ellipse */
				sscanf(buf+1, "%d %d", &m, &n);
				drawellip(m, n);
				break;
			case 'a':	/* arc */
				sscanf(buf+1, "%d %d %d %d", &n, &m, &n1, &m1);
				drawarc(n, m, n1, m1);
				break;
			case '~':	/* wiggly line */
				drawwig(buf+1);
				break;
			default:
				error(FATAL, "unknown drawing function %s", buf);
				break;
			}
			break;
		case 's':
			fscanf(fp, "%d", &n);	/* ignore fractional sizes */
			setsize(t_size(n));
			break;
		case 'f':
			fscanf(fp, "%s", str);
			setfont(t_font(str));
			break;
		case 'H':	/* absolute horizontal motion */
			fscanf(fp, "%d", &n);
			hgoto(n);
/*	We had some trouble with negative numbers here.
			while ((c = getc(fp)) == ' ')
				;
			k = 0;
			do {
				k = 10 * k + c - '0';
			} while (isdigit(c = getc(fp)));
			ungetc(c, fp);
			hgoto(k);
*/
			break;
		case 'h':	/* relative horizontal motion */
			fscanf(fp, "%d", &n);
			hmot(n);
/*	Same potential problems with negative numbers
			while ((c = getc(fp)) == ' ')
				;
			k = 0;
			do {
				k = 10 * k + c - '0';
			} while (isdigit(c = getc(fp)));
			ungetc(c, fp);
			hmot(k);
*/
			break;
		case 'w':	/* word space */
			break;
		case 'V':
			fscanf(fp, "%d", &n);
			vgoto(n);
			break;
		case 'v':
			fscanf(fp, "%d", &n);
			vmot(n);
			break;
		case 'p':	/* new page */
			fscanf(fp, "%d", &n);
			t_page(n);
			break;
		case 'n':	/* end of line */
			while ( (c = getc(fp)) != '\n'  &&  c != EOF )
				;
			t_newline();
			lineno++;
			break;
		case '#':	/* comment */
			while (getc(fp) != '\n')
				;
			lineno++;
			break;
		case 'x':	/* device control */
			devcntrl(fp);
			break;
		default:
			error(!FATAL, "unknown input character %o %c", c, c);
			done();
		}
	}
	xstat &= ~FILE_STARTED;
}

devcntrl(fp)	/* interpret device control functions */
FILE *fp;
{
        char str[20], str1[50], buf[50];
	int c, n;

	fscanf(fp, "%s", str);
	switch (str[0]) {	/* crude for now */
	case 'i':	/* initialize */
		fileinit();
		t_init(0);
		break;
	case 'T':	/* device name */
		fscanf(fp, "%s", devname);
		break;
	case 't':	/* trailer */
		t_trailer();
		break;
	case 'p':	/* pause -- can restart */
		t_reset('p');
		break;
	case 's':	/* stop */
		t_reset('s');
		break;
	case 'r':	/* resolution assumed when prepared */
		fscanf(fp, "%d", &res);
		break;
	case 'f':	/* font used */
		fscanf(fp, "%d %s", &n, str);
		fgets(buf, sizeof buf, fp);	/* in case there's a filename */
		ungetc('\n', fp);	/* fgets goes too far */
		str1[0] = 0;	/* in case there's nothing to come in */
		sscanf(buf, "%s", str1);
		loadfont(n, str, str1);
		break;
	/* these don't belong here... */
	case 'H':	/* char height */
		fscanf(fp, "%d", &n);
		t_charht(n);
		break;
	case 'S':	/* slant */
		fscanf(fp, "%d", &n);
		t_slant(n);
		break;
	}
	while ((c = getc(fp)) != '\n')	/* skip rest of input line */
		if (c == EOF)
			break;
	lineno++;
}

fileinit()	/* read in font and code files, etc. */
{
	int i, fin, nw;
	char *malloc(), *filebase, *p;
	char temp[60];

	/* open table for device,
	/* read in resolution, size info, font info, etc.
	/* and set params
	*/
/*	strcpy(devname, "202");	/* this is the only char set we have */
				/* the resolution, etc., is already in */
	sprintf(temp, "%s/dev%s/DESC.out", bitdir, devname);
	if ((fin = open(temp, 0)) < 0)  {
		sprintf(temp, "%s/dev%s/DESC.out", fontdir, devname);
		if ((fin = open(temp, 0)) < 0)
			error(FATAL, "can't open tables for %s", temp);
	} else fontdir = bitdir;
	read(fin, &dev, sizeof(struct dev));
	nfonts = dev.nfonts;
	nsizes = dev.nsizes;
	nchtab = dev.nchtab;
	if ( dev.spare2 <= 0  ||  dev.spare2 > nfonts )
		dev.spare2 = nfonts;
	filebase = malloc(dev.filesize);	/* enough room for whole file */
	read(fin, filebase, dev.filesize);	/* all at once */
	pstab = (short *) filebase;
	chtab = pstab + nsizes + 1;
	chname = (char *) (chtab + dev.nchtab);
	p = chname + dev.lchname;
	for (i = 1; i <= nfonts; i++) {
		fontbase[i] = (struct font *) p;
		nw = *p & BMASK;		/* width count comes first */

		if ( i <= dev.spare2 ) {
			fontbase[i] = NULL;
			widthtab[i] = codetab[i] = fitab[i] = NULL;
			p += 3 * nw + dev.nchtab + 128 - 32 + sizeof(struct font);
			continue;
		}	/* End if */

		p += sizeof(struct font);	/* font structure written first */
		widthtab[i] = p;		/* then the width table */
		codetab[i] = p + 2 * nw;	/* skip kern to get code table */
		fitab[i] = p + 3 * nw;		/* font's index table */

		p += 3 * nw + dev.nchtab + 128 - 32;	/* next font starts here */

		t_fp(i, fontbase[i]->namefont, fontbase[i]->intname);

		if ( dbg ) fontprint(i);

	}	/* End for */
	close(fin);
}

fontprint(i)	/* debugging print of font i (0,...) */
{
	int j, k, n;
	char *p;

	printf("font %d:\n", i);
	p = (char *) fontbase[i];
	n = fontbase[i]->nwfont & BMASK;
	printf("base=0%o, nchars=%d, spec=%d, name=%s, widtab=0%o, fitab=0%o\n",
		p, n, fontbase[i]->specfont, fontbase[i]->namefont, widthtab[i], fitab[i]);
	printf("widths:\n");
	for (j=0; j <= n; j++) {
		printf(" %2d", widthtab[i][j] & BMASK);
		if (j % 20 == 19) printf("\n");
	}
	printf("\ncodetab:\n");
	for (j=0; j <= n; j++) {
		printf(" %2d", codetab[i][j] & BMASK);
		if (j % 20 == 19) printf("\n");
	}
	printf("\nfitab:\n");
	for (j=0; j <= dev.nchtab + 128-32; j++) {
		printf(" %2d", fitab[i][j] & BMASK);
		if (j % 20 == 19) printf("\n");
	}
	printf("\n");
}

loadfont(n, s, s1)	/* load font info for font s on position n (0...) */
int n;
char *s, *s1;
{
	char temp[60];
	int fin, nw, norig;

	if (n < 0 || n > NFONT)
		error(FATAL, "illegal fp command %d %s", n, s);
	if (fontbase[n] != NULL && strcmp(s, fontbase[n]->namefont) == 0)
		return;
	if (s1 == NULL || s1[0] == '\0')
		sprintf(temp, "%s/dev%s/%s.out", fontdir, devname, s);
	else
		sprintf(temp, "%s/%s.out", s1, s);
	if ((fin = open(temp, 0)) < 0) {
		error(!FATAL, "can't open font table %s", temp);
		return;
	}
	if (fontbase[n] != NULL)
		free(fontbase[n]);
	fontbase[n] = (struct font *) malloc(3*255 + dev.nchtab +
				(128-32) + sizeof(struct font));
	if (fontbase[n] == NULL)
		error(FATAL, "Out of space in loadfont %s", s);
	read(fin, fontbase[n], 3*255 + nchtab+128-32 + sizeof(struct font));
	close(fin);
	if (smnt == 0 && fontbase[n]->specfont == 1)
		smnt = n;
	nw = fontbase[n]->nwfont & BMASK;
	widthtab[n] = (char *) fontbase[n] + sizeof(struct font);
	codetab[n] = (char *) widthtab[n] + 2 * nw;
	fitab[n] = (char *) widthtab[n] + 3 * nw;
	t_fp(n, fontbase[n]->namefont, fontbase[n]->intname);
	if (dbg > 1) fontprint(n);
}


error(f, s, a1, a2, a3, a4, a5, a6, a7) {
	fprintf(fp_error, "%s: ", di10name);
	fprintf(fp_error, s, a1, a2, a3, a4, a5, a6, a7);
	fprintf(fp_error, " - near line %ld\n", lineno);
	if (f)
		done();
}


/*
	Here beginneth all the stuff that really depends
	on the canon (we hope).
*/

#define	RES	240		/* resolution of canon */
#define	SLOP	1	/* how much positioning error is allowed? */
#define	MAXX	(8*RES + RES/2)		/* 8-1/2 inches? */
#define	MAXY	(11 * RES)
#define	WIDTH	8
#define	LOGWID	3
#define	K	* 1024	/* clever, so watch out */

char	devname[20]	= "i10";

int	cancenter = TRUE;		/* improve char placement if TRUE */
int	center = 0;			/* if cancenter == TRUE and char is alphanumeric */
int	nglyph		= 0;	/* number of glyphs loaded */
int	totglyph	= 0;	/* total space used by glyphs sent down */
int	maxglyph	= 16 K;	/* maximum space for glyphs */

#define	oput(c)	if (output) xychar(c); else;

/* input coordinate system: */

int	size	= 1;
int	font	= 1;		/* current font */
int	hpos;		/* horizontal position where we are supposed to be next (left = 0) */
int	vpos;		/* current vertical position (down positive) */
int	lastw	= 0;	/* width of last input character */
int	DX	= 7;	/* step size in x for drawing */
int	DY	= 7;	/* step size in y for drawing */

/* canon coordinate system: */

int	lastsize	= -1;
int	lastfont	= -1;
int	lastx		= -1;
int	lasty		= -1;
int	lastfam		= -1;

int	drawdot	= '.';	/* draw with this character */
int	drawsize = 1;	/* shrink by this factor when drawing */

/*
 *
 *  This stuff was added to handle the pre-page and post-page parts of
 *  the input files. Every page that we print will have all its glyphs
 *  defined in the pre-page part, so that the memory used in the
 *  input area will be deallocated as soon as the glyph is entered in
 *  the glyph table. All the rest of the commands for the page will
 *  be done in the post-page part.
 *
 */

#ifdef  u3b
#define CONVINT(C) convint(C)
#else
#define CONVINT(C) C
#endif
/* this routine converts char > 127 to proper 2's comp int */
convint(c)
#ifdef u3b
unsigned char c;
{
	int i;
	if ( c & 0200 )
	{
		i = ( 256 - c ) * -1;
	}
	else
		i = c;
	return(i);
}
#else
{
	return(c);
}
#endif
FILE	*fp_pre;		/* glyph definitions go in this file */
FILE	*fp_post;		/* everything else (almost) goes here */

int	fd_pre;			/* pre-page file descriptor - used for copy */
int	fd_post;		/* post-page file descriptor */


t_init(reinit)	/* initialize device */
int reinit;
{
	int i;
	FILE	*tmpfile();

	if (! reinit) {
		fp_pre = tf;
		if ( (fp_post = tmpfile()) == NULL )
			error(FATAL, "can't open post-page temporary file");

		fd_pre = fileno(fp_pre);
		fd_post = fileno(fp_post);

		for (i = 0; i < nchtab; i++)
			if (strcmp(&chname[chtab[i]], "l.") == 0)
				break;
		if (i < nchtab) {
			drawdot = i + 128;
			drawsize = 1;
		} else {
			drawdot = '.';
			drawsize = 2;	/* half size */
		}

		/* some Imagen-specific junk: */
		fprintf(fp_pre, "%1d", inputarea);	/* their kludge for setting */
						/* input area to x * 8k */
		maxglyph = (52 -  inputarea - 4) K;
						/* glyph area = 52K - input */
		fprintf(fp_pre, "  %s\n", username);
		putc(0, fp_pre);		/* terminate this [so they say] */
		fprintf(fp_pre, "%8.8s", "dcan1/24");	/* padding 8 bytes */
						/* ignored but needed */
		xfac = (float) RES / res * aspect;
		yfac = (float) RES / res;
	}
	hpos = vpos = 0;
	setsize(t_size(10));	/* start somewhere */
}

t_line(n, m, s)
	int n, m;
	char *s;
{
	if (m == 0) {	/* horizontal rule needed */
		if (n > 0) {
			t_rule((int) (n * xfac + 0.5), 2);
			hmot(n);	/* finish at the end */
		} else {
			hmot(n);
			t_rule((int) (-n * xfac + 0.5), 2);
		}
	} else if (n == 0) {	/* vertical rule */
		if (m > 0) {
			vmot(m);	/* finish at the end */
			t_rule(2, (int) (m * yfac + 0.5));
		} else {
			t_rule(2, (int) (-m * yfac + 0.5));
			vmot(m);
		}
	} else {
		drawline(n, m, s);
	}
}

t_page(pg)	/* do whatever new page functions */
{
	register int i, j, n;
	register unsigned char *p;
	static int firstpage = 1;

	pageno = pg;
	if(dbg)fprintf(stderr, "t_page %d, output=%d\n", pg, output);
	if (output != 0) {
		/* beginning of first page, or */
		/* have just printed something, and seen p<n> for next one */
		/* ought to read in entire page, select needed glyphs */
		putc(AEND, fp_post);
		firstpage = 0;
	}
	copy_post();
	output = in_olist(pg);
	if (output) {
		if (totglyph >= maxglyph) {
			clearglyphs();
			totglyph = 0;
		}
		putc(APAGE, fp_post);	/* self defeating if put in *fp_pre */
		acctpages++;
	}
	lastx = lasty = -1;
	t_init(1);
}

t_newline()	/* do whatever for the end of a line */
{
	hpos = 0;
}

t_size(n)	/* convert integer to internal size number*/
int n;
{
	int i;

	if (n <= pstab[0])
		return(1);
	else if (n >= pstab[nsizes-1])
		return(nsizes);
	for (i = 0; n > pstab[i]; i++)
		;
	return(i+1);
}

t_charht(n)	/* set character height to n */
int n;
{
	/* punt for now */
}

t_slant(n)	/* set slant to n */
int n;
{
	/* punt for now */
}

t_font(s)	/* convert string to internal font number */
char *s;
{
	int n;

	n = atoi(s);
	if (n < 0 || n > nfonts)
		n = 1;
	return(n);
}

t_reset(c)
{

}

t_wrapup()
{
	putc(AEND, fp_post);
	copy_post();
	putc(AEOF, fp_pre);
}

account()	/* record paper use */
{
	FILE *f = NULL;

	if ( xstat & DO_ACCT )  {
		if ( username == NULL )
			username = "???";
		fprintf(fp_acct, " user = %-10s", username);
		fprintf(fp_acct, " paper = %-10d", acctpages);
		xstat &= ~DO_ACCT;
		fprintf(fp_acct, " exit_status = 0%-6o", xstat);
		fprintf(fp_acct, " type = t ");
		if ( tf == stdout )
			fprintf(fp_acct, "  ??");
		fprintf(fp_acct, "\n");
	}
}

banner(s)
	char *s;
{
	long t, time();
	char *ctime();
	char *bp;
	int i;

	output = 1;			/* always print a banner */
	bp = s;
	i = 0;
	while ( *bp )
		if ( *bp++ < ' '  ||  ++i > BAN_LENGTH )
			*(--bp) = '\0';

	time(&t);
	putc(AEND, fp_post); /* clean up previous page */
	copy_post();
	putc(APAGE, fp_post);	/* again don't write it to *fp_pre */
	setsize(t_size(BAN_SIZE));
	loadfont(1, "CW", "");
	setfont(1);
	lastx = lasty = -1;
	vgoto((3 * res) / 4);
	hgoto(res / 4);

	/* This is kind of dumb, but I was asked to do the banner this way */

	if ( ban_split > 0  &&  ban_split < BAN_LENGTH )  {
		setsize(t_size(BAN_SIZE + 4));
		bp = s + ban_split;
		while ( *bp )  {
			put1(*bp++);
			hmot(lastw);
		}
		setsize(t_size(BAN_SIZE));
	}
	vmot(res / 4);
	hgoto(res / 4);
	t_rule(1080, 12);
	vmot(res / 2);
	hgoto(res / 2);
	while ( *s != '\0'  &&  *s != '\n' )  {
		put1(*s++);
		hmot(lastw);
	}
	vmot( res / 2);
	hgoto(res / 4);
	t_rule(1080, 12);
	vgoto(10 * res);
	hgoto(4 * res + res / 4);
	s = ctime(&t);
	while (*s) {
		put1(*s++);
		hmot(lastw);
	}
}

t_rule(w, h)
{
	hvflush();
	putc(ABRULE, fp_post);
	putint(w, fp_post);
	putint(h, fp_post);
	putint(-h, fp_post);
}


t_trailer()
{
}

hgoto(n)
{
	hpos = n;	/* this is where we want to be */
			/* before printing a character, */
			/* have to make sure it's true */
}

hmot(n)	/* generate n units of horizontal motion */
int n;
{
	hpos += n;
}

vgoto(n)
{
	vpos = n;
}

vmot(n)	/* generate n units of vertical motion */
int n;
{
	vgoto(vpos + n);	/* ignores rounding */
}

put1s(s)	/* s is a funny char name */
	register char *s;
{
	static int i = 0;

	if (!output)
		return;
	if (dbg) printf("%s ", s);
	if (strcmp(s, &chname[chtab[i]]) != 0)
		for (i = 0; i < nchtab; i++)
			if (strcmp(&chname[chtab[i]], s) == 0)
				break;
	if (i < nchtab)
		put1(i + 128);
	else
		i = 0;
}


struct {
	char *name;
	int number;
} fontname[NFONT+1];

put1(c)	/* output char c */
	register int c;
{
	char *pw;
	register char *p;
	register int i, j, k;
	int ofont, code, w;
	int num, l;

	if (!output)
		return;

	if ( cancenter == TRUE )
		center = isalnum(c);

	c -= 32;
	if (c <= 0) {
		if (dbg) printf("non-exist 0%o\n", c+32);
		return;
	}
	k = ofont = font;
	i = fitab[font][c] & BMASK;

/*
 *
 * The next few lines were added to handle the lookups on the extra
 * default fonts.
 *
 */

	num = fontname[font].number;
	l = num & BMASK;
	while ( i == 0 && l <= nfonts && fontname[l].number == num )
		if ( (i = fitab[l][c]) != 0 )  {
			if ( dbg > 2 )  {
				fprintf(stderr, "found 0%o on font %d\n", c, l);
			}
			if ( strcmp(fontname[font].name, fontname[l].name) == 0 )  {
				buildchar(c, l);
				return;
			}    /* End if */
			setfont(l);
		} else l++;

	if (i != 0) {	/* it's on this font */
		p = codetab[font];
		pw = widthtab[font];
	} else if (smnt > 0) {		/* on special (we hope) */
		for (k=smnt, j=0; j <= nfonts; j++, k = (k+1) % (nfonts+1))  {
			if ( k == 0 )  continue;
			if ((i = fitab[k][c] & BMASK) != 0) {
				p = codetab[k];
				pw = widthtab[k];
				setfont(k);
				break;
			}
		}
	}
	if (i == 0 || (code = p[i] & BMASK) == 0 || k > nfonts) {
		if (dbg) printf("not found 0%o\n", c+32);
		if ( font != ofont ) setfont(ofont);
		return;
	}
	lastw = pw[i] & BMASK;
	lastw = (lastw * pstab[size-1] + dev.unitwidth/2) / dev.unitwidth;
	if (dbg) {
		if (isprint(c+32))
			printf("%c %d\n", c+32, code);
		else
			printf("%03o %d\n", c+32, code);
	} else
		oput(code);
	if (font != ofont)
		setfont(ofont);
}

buildchar(c, f)


	int	c;			/* build this character up */
	int	f;			/* it's on this font - internal num */


{


	char	ch[10];			/* character string for c goes here */
	int	ohpos, ovpos;		/* position right now */
	int	cwidth;			/* c's width from font f */
	int	pos;			/* c's position on this font */
	int	i;			/* for loop index */


	ohpos = hpos;			/* save the current position */
	ovpos = vpos;
	pos = fitab[f][c] & BMASK;	/* c's position on font f */
	cwidth = widthtab[f][pos] & BMASK;
	cwidth = (cwidth * pstab[size-1] + dev.unitwidth/2) / dev.unitwidth;

	fontname[f].number++;		/* prevent recursion for now */
	c += 32;

	if ( c < 128 )  {		/* was a single ASCII character */
		ch[0] = c;		/* so build up the ch[] string */
		ch[1] = '\0';
	} else strcpy(ch, &chname[chtab[c-128]]);

	for ( i = 0; spectab[i] != 0; i += 2 )
		if ( strcmp(spectab[i], ch) == 0 )  {
			dostring(spectab[i+1], cwidth);
			break;
		}    /* End if */

	if ( hpos != ohpos )  hgoto(ohpos);	/* back to original position */
	if ( vpos != ovpos )  vgoto(ovpos);

	lastw = cwidth;
	fontname[f].number--;

}    /* End of buildchar */

dostring(str, cwidth)


	char	*str;			/* control string for current char */
	int	cwidth;			/* char's width from font tables */


{


	int	hstep, vstep;		/* move this far from current pos */
	char	c;			/* character that is printed next */


	if ( dbg > 2 )
		fprintf(stderr, "cwidth = %d\n", cwidth);

	while ( c = *str++ )  {
		if ( dbg > 2 )
			fprintf(stderr, "string = %s\n", str);
		if ( c == '(' )  {	/* have a relative motion */
			hstep = cwidth * (atoi(str) / 100.0);
			while ( *str++ != ',' ) ;
			vstep = cwidth * (atoi(str) / 100.0);
			while ( *str++ != ')' ) ;
			if ( hstep != 0 )  hmot(hstep);
			if ( vstep != 0 )  vmot(vstep);
			c = *str++;
		}    /* End if */
		if ( dbg > 2 )  {
			fprintf(stderr, "char = %c (0%0), ", *str, *str);
			fprintf(stderr, "hstep = %d, vstep = %d, ", hstep, vstep);
			fprintf(stderr, "hpos = %d, vpos = %d\n", hpos, vpos);
		}    /* End if */
		if ( c != '\0' )  put1(c);
	}    /* End while */

}    /* End of dostring */



setsize(n)	/* set point size to n (internal) */
int n;
{
	size = n;
}

/* font position info: */

t_fp(n, s, si)	/* font position n now contains font s, intname si */
int n;
char *s, *si;
{
	fontname[n].name = s;
	fontname[n].number = atoi(si);
	if ( n == lastfont ) lastfont = -1;	/* force getfontdata() call */
}

setfont(n)	/* set font to n */
int n;
{
	if (!output)
		return;
	if (n < 0 || n > NFONT)
		error(FATAL, "illegal font %d", n);
	font = n;
}

done()
{
	account();
	exit(xstat);
}

/*
	The following things manage raster font information.
	The big problem is mapping desired font + size into
	available font + size.  For now, a file RASTERLIST
	contains entries like
		R 6 8 10 14 999
		I 8 10 12 999
		...
	This data is used to create an array "fontdata" that
	describes legal fonts and sizes, and pointers to any
	data from files that has actually been loaded.
*/

struct fontdata {
	char	name[4];	/* e.g., "R" or "PA" */
	int	size[10];	/* e.g., 6 8 10 14 0 */
	struct	fontset	*fsp[10];	/* either NULL or block of data */
};

#define	MAXFONT	60	/* no more than this many fonts forever */

struct	fontdata	fontdata[MAXFONT];
int	maxfonts	= 0;	/* how many actually used; set in initfontdata() */

struct	Fontheader	fh;
struct	fontset {
	int	size;
	int	family;
	struct	Charparam *chp;
	unsigned char	*cdp;	/* char data pointer */
	unsigned char	*chused;	/* bit-indexed; 1 if char downloaded */
};

/* A global variable for the current font+size */
struct	fontset	*fs;
int	nfamily		= 0;	/* number of "families" (font+size) */

initfontdata()	/* read RASTERLIST information */
{
	char name[100];
	FILE *fp;
	int i, j, n;

	sprintf(name, "%s/RASTERLIST", bitdir);
	if ((fp = fopen(name, "r")) == NULL)
		error(FATAL, "can't open %s", name);
	maxfonts = 0;
	while (fscanf(fp, "%s", fontdata[maxfonts].name) != EOF) {
		i = 0;
		while (fscanf(fp, "%d", &n) != EOF && n < 100) {
			fontdata[maxfonts].size[i] = n;
			fontdata[maxfonts].fsp[i] = NULL;
			i++;
		}
		fontdata[maxfonts].size[i] = 999;
		if (++maxfonts > MAXFONT)
			error(FATAL, "Too many fonts in RASTERLIST");
	}
	fclose(fp);
	if (dbg) {
		fprintf(stderr, "initfontdata():  maxfonts=%d", maxfonts);
		for (i = 0; i < maxfonts; i++) {
			fprintf(stderr, "%.4s ", fontdata[i].name);
			for (j = 0; fontdata[i].size[j] < 100; j++)
				fprintf(stderr, " %3d", fontdata[i].size[j]);
			fprintf(stderr, "\n");
		}
	}
}

getfontdata(f, s)	/* causes loading of font information if needed */
	char *f;
	int s;
{
	int fd, n, i, j;
	char name[100];
	static int first = 1;

	if (first) {
		initfontdata();
		first = 0;
	}

	for (i = 0; i < maxfonts; i++)
		if (strcmp(f, fontdata[i].name) == 0)
			break;
	if (i >= maxfonts)	/* the requested font wasn't there */
		i = 0;		/* use the first one (probably R) */

	/* find the best approximation to size s */
	for (j = 1; s >= fontdata[i].size[j]; j++)
		;
	j--;

	/* open file if necessary */
	if (fontdata[i].fsp[j] == NULL) {
		fs = (struct fontset *) malloc(sizeof(struct fontset));
		fontdata[i].fsp[j] = fs;
		fs->chp = (struct Charparam *) malloc(256*sizeof(struct Charparam));
		sprintf(name, "%s/%s.%d%s", bitdir,
			f, fontdata[i].size[j], rotate? "r" : "");
		fd = open(name, 0);
		if (fd == -1)
			error(FATAL, "can't open %s", name);
		read(fd, &fh, sizeof(struct Fontheader));
		read(fd, fs->chp, 256*sizeof(struct Charparam));
		fs->size = fontdata[i].size[j];
		fs->family = nfamily;
		nfamily += 2;	/* even-odd leaves room for big fonts */
		fs->cdp = (unsigned char *) malloc(fh.f_size);
		fs->chused = (unsigned char *) malloc(256/8);
		for (n = 0; n < 256/8; n++)
			fs->chused[n] = 0;
		n = read(fd, fs->cdp, fh.f_size);
		close(fd);
	}
	fs = fontdata[i].fsp[j];
}

xychar(c)
	register int c;
{
	register unsigned char *p;
	register struct Charparam *par;
	register int x, y;
	char hold;
	int i, n, rwid, ht, fam;
	int	w;

	x = hpos * xfac + 0.5;
	x += xoffset;
	y = vpos * yfac + 0.5;
	y += yoffset;

	if (font != lastfont || size != lastsize) {
		getfontdata(fontname[font].name, pstab[size-1]);
		lastsize = size;
		lastfont = font;
	}
	par = fs->chp + c;
	p = fs->cdp + par->c_addr;
	fam = fs->family;
	if (c > 127)
		fam++;
	if (fam != lastfam) {
		putc(AF, fp_post);
		putc(lastfam = fam, fp_post);
	}

	/* first cut:  ship each glyph as needed. */
	/* ignore memory use, efficiency, etc. */

	if ( !bit(fs->chused, c) ) {	/* 1st use of this character */
		nglyph++;
		totglyph += glspace(par);
		setbit(fs->chused, c);
		putc(ASGLY, fp_pre);
		putint((fam << 7) | c, fp_pre);
		par->c_width = (char)((lastw * RES) / res);
 		putc(par->c_width, fp_pre);	/* character width */
		hold=(char) (w = CONVINT(par->c_left) + CONVINT(par->c_right) + 1);
		putc(hold,fp_pre);
		if ( center )
			putc((w - CONVINT(par->c_width)) / 2, fp_pre);
		else putc(par->c_left, fp_pre);
  /* this nonsense fixes a bug in output produced by rec.c: */
  /* when up is < 0 (and = 0?) size is one too big */
		rwid = (1 + CONVINT(par->c_left) + CONVINT(par->c_right) + WIDTH-1) / WIDTH;
		ht = par->c_size / rwid;
		par->c_down = (char)(ht - CONVINT(par->c_up));
		hold=(char)(CONVINT(par->c_down) + CONVINT(par->c_up));
		putc(hold, fp_pre);
		putc(par->c_up, fp_pre);
		for (i = par->c_size; i--; )
			putc(*p++, fp_pre);
	}

	if (y != lasty) {
		putc(AV, fp_post);
		putint(y<<1, fp_post);
		lasty = y;
	}

	if (abs(x-lastx) > 127) {
		putc(AH, fp_post);
		putint(x<<1, fp_post);
		lastx = x + (par->c_width & BMASK);
	} else if (abs(x-lastx) > SLOP) {
		putc(AM, fp_post);
		putc(x-lastx, fp_post);
		putc(AM, fp_post);
		lastx = x + (par->c_width & BMASK);
	} else {
		lastx += (par->c_width & BMASK);
	}

	if (c <= 127)
		putc(c, fp_post);	/* fails if c > 127, probably disastrously */
	else
		putc(c-128, fp_post);
}

hvflush()	/* force position recorded in hpos,vpos */
{
	int x, y;

	x = hpos * xfac + xoffset + 0.5;
	y = vpos * yfac + yoffset + 0.5;
	if (y != lasty) {
		putc(AV, fp_post);
		putint(y<<1, fp_post);
		lasty = y;
	}
	if (abs(x-lastx) > 127) {
		putc(AH, fp_post);
		putint(x<<1, fp_post);
		lastx = x;
	} else if (abs(x-lastx) > SLOP) {
		putc(AM, fp_post);
		putc(x-lastx, fp_post);
		putc(AM, fp_post);
		lastx = x;
	}
}

glspace(par)
	struct Charparam *par;
{
	int n;

	/* works only for small glyphs right now */

	n = 12
	  + ((CONVINT(par->c_left)+CONVINT(par->c_right)+1+15)/16 )
		* (CONVINT(par->c_up)+CONVINT(par->c_down))
	  + 2;
	return n;
}

clearglyphs()	/* remove "used" bits from all glyphs */
		/* delete all families */
		/* very conservative policy */
{
	int i, j, k;
	struct fontset *f;

	if (fp_pre == stdout) fprintf(stderr, "clear %d glyphs (%d/%d) on page %d\n",
		nglyph, totglyph, maxglyph, pageno);
	for (i = 0; i < maxfonts; i++)
		for (j = 0; fontdata[i].size[j] < 999; j++) {
			f = fontdata[i].fsp[j];
			if (f != NULL) {
				putc(ADELF, fp_pre);
				putc(f->family, fp_pre);
				for (k = 0; k < 256/8; k++)
					f->chused[k] = 0;
			}
		}
}

bit(p, n)	/* return n-th bit of p[] */
	char *p;
	int n;
{
	return (p[n/8] >> (7 - n%8)) & 01;
}

setbit(p, n)	/* set bit n of p[] */
	char *p;
	int n;
{
	p[n/8] |= 01 << (7 - n%8);
}

putint(n, f)
	int n;
	FILE *f;
{
	putc(n >> 8, f);
	putc(n & 0377, f);
}

#define	BUFF_SIZE	512		/* size of buffer for file copy */

copy_post()


{


	long	pos;
	long	cur_pos;
	int	size;
	char	buff[BUFF_SIZE];


	fflush(fp_post);
	fflush(fp_pre);
	if ( (pos = lseek(fd_post, 0L, 1)) == -1 )
		error(FATAL, "can't get position in post-page temp file");

	if ( lseek(fd_post, 0L, 0)  == -1 )
		error(FATAL, "can't seek to start of post-page temp file");

	cur_pos = 0;
	while ( cur_pos < pos )  {
		size = (pos - cur_pos < BUFF_SIZE) ? pos - cur_pos
						   : BUFF_SIZE;

		if ( read(fd_post, buff, size) < size )
			error(FATAL, "problem reading from post-page file");

		if ( write(fd_pre, buff, size) < size )
			error(FATAL, "problem writing to pre-page file");

		cur_pos += size;
	}

	fflush(fp_pre);

	if ( lseek(fd_post, 0L, 0) == -1 )
		error(FATAL, "can't seek to start of post-page temp file");

}
