/* @(#)config.pdp.c	1.2 */
#include "stdio.h"
#define	TSIZE	50		/* max configuration table */
#define	DSIZE	50		/* max device table */
#define	ASIZE	20		/* max alias table */
#define MSIZE	100		/* max address map table */
#define	PSIZE	64		/* max keyword table */
#define	BSIZE	16		/* max block device table */
#define	CSIZE	50		/* max character device table */
#define ONCE	0200		/* allow only one specification of device */
#define NOCNT	0100		/* suppress device count field */
#define SUPP	040		/* suppress interrupt vector (on print) */
#define REQ	020		/* required device */
#define	BLOCK	010		/* block type device */
#define	CHAR	04		/* character type device */
#define	FLT	02		/* interrupt vector in range 0300 - 0774 */
#define	FIX	01		/* interrupt vector in range 0 - 0274 */
#define	TTYS	0200		/* tty header exists */
#define INIT	0100		/* initialization routine exists */
#define PWR	040		/* power fail routine exists */
#define	OPEN	020		/* open routine exists */
#define	CLOSE	010		/* close routine exists */
#define	READ	04		/* read routine exists */
#define	WRITE	02		/* write routine exists */
#define	IOCTL	01		/* ioctl routine exists */

struct	t	{
	char	*devname;	/* pointer to device name */
	short	vector;		/* interrupt vector location */
	short	address;	/* device address */
	short	buslevel;	/* bus request level */
	short	addrsize;	/* number of bytes at device address */
	short	vectsize;	/* number of bytes for interrupt vector */
	short	type;		/* ONCE,NOCNT,SUPP,REQ,BLOCK,CHAR,FLT,FIX  */
	char	*handler;	/* pointer to interrupt handler */
	short	count;		/* sequence number for this device */
	short	blk;		/* major device number if block type device */
	short	chr;		/* major device number if char. type device */
	short	mlt;		/* number of devices on controller */
}	table	[TSIZE];

struct	t2	{
	char	dev[9];		/* device name */
	short	vsize;		/* interrupt vector size */
	short	asize;		/* device address size */
	short	type2;		/* ONCE,NOCNT,SUPP,REQ,BLOCK,CHAR,FLT,FIX  */
	short	block;		/* major device number if block type device */
	short	charr;		/* major device number if char. type device */
	short	mult;		/* max number of devices per controller */
	short	busmax;		/* max allowable bus request level */
	short	mask;		/* device mask indicating existing handlers */
	char	hndlr[5];	/* handler name */
	short	dcount;		/* number of controllers present */
	char	entries[3][9];	/* conf. structure definitions */
	short	acount;		/* number of devices */
}	devinfo	[DSIZE];

struct	t3	{
	char	new[9];		/* alias of device */
	char	old[9];		/* reference name of device */
}	alias	[ASIZE];

struct	map	{
	unsigned	start;	/* starting unibus address */
	int		length;	/* gap length */
}	map	[MSIZE]
{
	0160000, 017570
};

struct	t4	{
	char	indef[21];		/* input parameter keyword */
	char	oudef[21];		/* output definition symbol */
	char	value[21];		/* actual parameter value */
	char	defval[21];		/* default parameter value */
}	parms	[PSIZE];

struct	t	*lowptr[128]		/* low core double-word pointers */
{
	1,2,3,4,5,6,7,8,9,10,11,12,	/* trap vectors/fillers */
	0,0,0,0,			/* free entries */
	17,18,				/* trap vectors/fillers */
	0,				/* free entry */
	13,				/* trap vector/filler */
	0,0,0,0,0,0,0,0,0,0,		/* free entries */
	0,0,0,0,0,0,0,0,0,0,		/* free entries */
	14,15,16			/* trap vectors/fillers */
};

struct	t2	*bdevices[BSIZE];	/* pointers to block devices */
struct	t2	*cdevices[CSIZE];	/* pointers to char. devices */
struct	t	*p;			/* configuration table pointer */
struct	t2	*q;			/* master device table pointer */
struct	t3	*r;			/* alias table pointer */
struct	t4	*kwdptr;		/* keyword table pointer */
struct	t	*locate();
struct	t2	*find();
struct	t4	*lookup();
char	*uppermap();

short	power	= 0;	/* power fail restart flag */
short	eflag	= 0;	/* error in configuration */
short	tflag	= 0;	/* table flag */
short	nflag	= 0;	/* no separate I&D space flag */
short	bmax	= -1;	/* dynamic max. major device number for block device */
short	cmax	= -1;	/* dynamic max. major device number for char. device */
short	blockex	= 0;	/* dynamic end of block device table */
short	charex	= 0;	/* dynamic end of character device table */
short	subv	= 0;	/* low core double-word subscript */
short	subtmp	= 0;	/* temporary; for subscripting */
short	abound	= -1;	/* current alias table size */
short	dbound	= -1;	/* current device table size */
short	tbound	= -1;	/* current configuration table size */
short	pbound	= -1;	/* current keyword table size */
short	mbound	= 0;	/* current address map table size */
short	rtmaj	= -1;	/* major device number for root device */
short	swpmaj	= -1;	/* major device number for swap device */
short	pipmaj	= -1;	/* major device number for pipe device */
short	dmpmaj	= -1;	/* major device number for dump device */
short	rtmin	= -1;	/* minor device number for root device */
short	swpmin	= -1;	/* minor device number for swap device */
short	pipmin	= -1;	/* minor device number for pipe device */
short	dmpmin	= -1;	/* minor device number for dump device */
long	swplo	= -1;	/* low disc address for swap area */
short	nswap	= -1;	/* number of disc blocks in swap area */
FILE	*fdconf;	/* configuration file descriptor */
FILE	*fdlow;		/* low core file descriptor */
char	*dmphndlr;	/* pointer to dump handler routine */
char	*mfile;		/* master device file */
char	*cfile;		/* output configuration table file */
char	*lfile;		/* output low core file */
char	*infile;	/* input configuration file */
char	line[101];	/* input line buffer area */
char	dump[81];	/* trap filler for dump routine */
char	pwrbf[513];	/* power fail routine buffer */
char	initbf[513];	/* initialization routine buffer */
char	xbf[111];	/* buffer for external symbol definitions */
char	d[9];		/* buffer area */
char	capitals[9];	/* buffer area */
short	vec;		/* specified interrupt vector */
unsigned	add;	/* specified device address */
short	bl;		/* specified bus level */
short	ml;		/* specified device multiplicity */

char	*cputrap[]	/* trap/filler text */
{
	"",
	"\tjmp\tdstart\t\t/ Initial startup",
	"\ttrap4;\tbr7+0.\t\t/ Bus error",
	"\ttrap10;\tbr7+1.\t\t/ Illegal instruction",
	"\ttrap14;\tbr7+2.\t\t/ Bpt-trace trap",
	"\ttrap;\tbr7+3.\t\t/ Iot trap",
	"\ttrap24;\tbr7+4.\t\t/ Power failure",
	"\ttrap;\tbr7+5.\t\t/ Emulator trap",
	"\ttrap;\tbr7+6.\t\t/ System entry",
	"\t_utsname;\t\t/ Name of system\n\tbr .",
	dump,
	"\t.+2;\tbr .\t\t/ Trap catcher",
	"\t.+2;\tbr .",
	"\ttrap;\tbr7+10.\t\t/ Memory system error",
	"\ttrap240;\tbr7+7.\t\t/ Programmed interrupt",
	"\ttrap;\tbr7+8.\t\t/ Floating point",
	"\ttrap250;\t\t/ Segmentation violation\n\tbr7+9.",
	"\tclio;\tbr6+0.",
	"\tclio;\tbr6+1."
};

char	*zerotrap
{
	"\t42;\tbr ."
};


char	*out[2]			/* low core text */
{
	".globl\t_%srint, _%sxint\n%sin:\n\tjsr\tr0,call; %s _%srint\n",
	"%sou:\n\tjsr\tr0,call; %s _%sxint\n\n"
};

char	*opterror		/* option error message */
{
	"Option re-specification\n"
};

char	*dmpform		/* format for dump routine */
{
	"dump:\t\t\t\t/ Memory dump\n.globl\tdump_%s\n\tjmp\t*$dump_%s"
};

char	*sdev[] = {
	"dm11",
	"hisdm",
	NULL
};

main(argc,argv)
short argc;
char *argv[];
{
	register short i, l;
	register FILE *fd;
	short number;
	char input[21], bf[9], c, symbol[21];
	char *argv2;
	struct t *pt;
	argc--;
	argv++;
	argv2 = argv[0];
	argv++;
/*
 * Scan off the 'c', 'l', 'm', and 't' options.
 */
	while ((argc > 1) && (argv2[0] == '-')) {
		for (i=1; (c=argv2[i]) != NULL; i++) switch (c) {
/*
 * Output configuration file specification
 */
		case 'c':
			if (cfile) {
				printf(opterror);
				exit(1);
			}
			cfile = argv[0];
			argv++;
			argc--;
			break;
/*
 * Output low core file specification
 */
		case 'l':
			if (lfile) {
				printf(opterror);
				exit(1);
			}
			lfile = argv[0];
			argv++;
			argc--;
			break;
/*
 * Input master table specification
 */
		case 'm':
			if (mfile) {
				printf(opterror);
				exit(1);
			}
			mfile = argv[0];
			argv++;
			argc--;
			break;
/*
 * Table flag
 */
		case 't':
			tflag++;
			break;
/*
 * no I space flag
 */
		case 'n':
			nflag++;
			break;


		default:
			printf("Unknown option\n");
			exit(1);
		}
		argc--;
		argv2 = argv[0];
		argv++;
	}
	if (argc != 1) {
		printf("Usage: config [-t][-l file][-c file][-m file] file\n");
		exit(1);
	}
/*
 * Set up defaults for unspecified files.
 */
	if (cfile == 0)
		cfile = "conf.c";
	if (lfile == 0)
		lfile = "low.s";
	if (mfile == 0)
		mfile = "/etc/master";
	infile = argv2;
	fd = fopen (infile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",infile);
		exit(1);
	}
/*
 * Open configuration file and set modes.
 */
	fdconf = fopen (cfile,"w");
	if (fdconf == NULL) {
		printf("Open error for file -- %s\n",cfile);
		exit(1);
	}
	chmod (cfile,0644);
/*
 * Print configuration file heading.
 */
	fprintf(fdconf,"/*\n *  Configuration information\n */\n\n");
	p = table;
/*
 * Read in the master device table and the alias table.
 */
	file();
/*
 * Set up the console, which must be present.
 */
	q = find("dl11");
	enterdev(060,0177560,4,1,"dl11");
	if (addrcheck(0177560,q->asize)) {
		printf("Map table initialization error\n");
		exit(1);
	}
/*
 * Set up address map for kw11-l and kw11-p
 */
	if (addrcheck(0177546,2)) {
		printf("Map table initialization error\n");
		exit(1);
	}
	if (addrcheck(0172540,8)) {
		printf("Map table initialization error\n");
		exit(1);
	}
/*
 * Start scanning the input.
 */
	while (fgets(line,100,fd) != NULL) {
		if (line[0] == '*')
			continue;
		l = sscanf(line,"%20s",input);
		if (l == 0) {
			error("Incorrect line format");
			continue;
		}
		if (equal(input,"root")) {
/*
 * Root device specification
 */
			l = sscanf(line,"%*8s%8s%o",bf,&number);
			if (l != 2) {
				error("Incorrect line format");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error("Not a block device");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (rtmin >= 0) {
				error("Root re-specification");
				continue;
			}
			rtmin = number;
			rtmaj = pt->blk;
		} else
		    if (equal(input,"swap")) {
/*
 * Swap device specification
 */
			l = sscanf(line,"%*8s%8s%o%ld%d",
					bf,&number,
					&swplo,&nswap);
			if (l != 4) {
				error("Incorrect line format");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error("Not a block device");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (swpmin >= 0) {
				error("Swap re-specification");
				continue;
			}
			if (nswap < 1) {
				error("Invalid nswap");
				continue;
			}
			if (swplo < 0) {
				error("Invalid swplo");
				continue;
			}
			swpmin = number;
			swpmaj = pt->blk;
		}
		else
		    if (equal(input,"pipe")) {
/*
 * Pipe device specification
 */
			l = sscanf(line,"%*8s%8s%o",bf,&number);
			if (l != 2) {
				error("Incorrect line format");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error("Not a block device");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (pipmin >= 0) {
				error("Pipe re-specification");
				continue;
			}
			pipmin = number;
			pipmaj = pt->blk;
		}
		else
		    if (equal(input,"dump")) {
/*
 * Dump device specification
 */
			l = sscanf(line,"%*8s%8s%o",bf,&number);
			if (l != 2) {
				error("Incorrect line format");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ((pt->type & BLOCK) == 0) {
				error("Not a block device");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (dmpmin >= 0) {
				error("Dump re-specification");
				continue;
			}
			dmpmin = number;
			dmpmaj = pt->blk;
			dmphndlr = pt->handler;
		}
		else {
/*
 * Device or parameter specification other than root, swap, pipe, or dump.
 * If power fail recovery is specified, remember it.
 */
			kwdptr = lookup(input);
			if (kwdptr) {
				l = sscanf(line,"%20s%20s",input,symbol);
				if (l != 2) {
					error("Incorrect line format");
					continue;
				}
				if (strlen(kwdptr->value)) {
					error("Parameter re-specification");
					continue;
				}
				if (equal(input,"power") && equal(symbol,"1"))
					power++;
				strcpy(kwdptr->value,symbol);
				continue;
			}
			l = sscanf(line,"%8s%o%o%d%d",d,&vec,&add,&bl,&ml);
			if (l < 4) {
				error("Incorrect line format");
				continue;
			}
/*
 * Does such a device exist, and if so, do we allow its specification?
 */
			if ((q=find(d)) == 0) {
				error("No such device");
				continue;
			}
			if ((q->type2 & ONCE) && (locate(d))) {
				error("Only one specification allowed");
				continue;
			}
/*
 * Is the address valid?
 */
			if (q->asize) {
				if (add % 2) {
					error("Address alignment");
					continue;
				}
				if (add < 0160000) {
					error("Invalid address");
					continue;
				}
				if (addrcheck(add,q->asize)) {
					error("Address collision");
					continue;
				}
			}
			else {
				if (add) {
					error("Address not null");
					continue;
				}
			}
/*
 * If the vector size field is non-zero, process as a regular device.
 */
			if (q->vsize) {
/*
 * Is the interrupt vector on an appropriate boundary?
 */
				if (vec%q->vsize) {
					error("Vector alignment");
					continue;
				}
/*
 * Is the interrupt vector in low core?
 */
				if (vec<0 || vec>0774) {
					error("Vector not in low core range");
					continue;
				}
				switch (q->type2 & (FIX|FLT)) {
/*
 * This interrupt vector should be in the 0 - 0274 range.
 */
				case FIX:
					if (vec > 0274) {
						error("Vector not in 0-0274 range");
						continue;
					}
					break;
/*
 * This interrupt vector should be in the 0300 - 0774 range.
 */
				case FLT:
					if (vec < 0300) {
						error("Vector not in 0300-0774 range");
						continue;
					}
					break;
				}
/*
 * Is the bus request level a valid one for this device?
 */
				if (bl<4 || bl>q->busmax) {
					error("Invalid bus level");
					continue;
				}
/*
 * Get the double-word offset for the interrupt vector.
 */
				subv = vec/4;
/*
 * Make sure the interrupt vector is free.
 */
				if (lowptr[subv]) {
					error("Vector allocated");
					continue;
				}
				if (q->vsize == 8) {
					if (lowptr[subv+1]) {
						error("Vector collision");
						continue;
					}
				}
			}
/*
 * Device is not interrupt driven, so interrupt vector and
   bus request levels should be zero.
 */
			else {
				if (vec) {
					error("Interrupt vector not null");
					continue;
				}
				if (bl) {
					error("Bus level not null");
					continue;
				}
				subv = 0;
			}
/*
 * See if the optional device multiplier was specified, and if so, see
 * if it is a valid one.
 */
			if (l == 5) {
				if ((ml > q->mult) || (ml < 1)) {
					error("Invalid device multiplier");
					continue;
				}
			}
/*
 * Multiplier not specified, so take a default value from master device table.
 */
			else
				ml = q->mult;
/*
 * Fill in the contents of the configuration table node for this device.
 */
			enterdev(vec,add,bl,ml,d);
		}
	}
/*
 * Make sure that the root, swap, pipe and dump devices were specified.
 * Set up default values for tunable things where applicable.
 */
	if (rtmaj < 0) {
		printf("root device not specified\n");
		eflag++;
	}
	if (swpmaj < 0) {
		printf("swap device not specified\n");
		eflag++;
	}
	if (pipmaj < 0) {
		printf("pipe device not specified\n");
		eflag++;
	}
	if (dmpmaj < 0) {
		printf("dump device not specified\n");
		eflag++;
	}
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (strlen(kwdptr->value) == 0) {
			if (strlen(kwdptr->defval) == 0) {
				printf("%s not specified\n",kwdptr->indef);
				eflag++;
			}
			strcpy(kwdptr->value,kwdptr->defval);
		}
	}
/*
 * See if configuration is to be terminated.
 */
	if (eflag) {
		printf("\nConfiguration aborted.\n");
		exit (1);
	}
/*
 * Configuration is okay, so write the two files and quit.
 */
	sprintf(dump,dmpform,dmphndlr,dmphndlr);

	for (q=devinfo; q<= &devinfo[dbound]; q++) {
		if (q->type2 & REQ) {
			if (!(locate(q->dev))) {
				enterdev(0,0,0,q->mult,q->dev);
			}
		}
	}

	prtconf();
	prtlow();
	exit(0);
}

/*
 * This routine writes out the low core assembler program.
 */
prtlow()
{
	prtlhd();
	prtlvec("DZERO");
	if(!nflag) {
		dump[0] = 'D';
		cputrap[1] = zerotrap;
		prtlvec("TZERO");
		fprintf(fdlow,"\n.data\npowmm:\tinc\tSSR0\n\tjmp\tpowmm\n");
		fprintf(fdlow,"\ntstart:\tinc\tSSR0\n\tjmp\tdstart\n");
		fprintf(fdlow,"\n.text\n\tdec\tSSR0\n\tjmp\tpowback\n");
		fprintf(fdlow,"\nrestart:\tdec\tSSR0\n\tjmp\tstart\n");
	}
	prtlinter();
}

/*
 * This routine writes the low core assembler header.
 */
prtlhd()
{
/*
 * Open low core file and set modes.
 */
	fdlow = fopen (lfile,"w");
	if (fdlow == NULL) {
		printf("Open error for file -- %s\n",lfile);
		exit(1);
	}
	chmod (lfile,0644);
/*
 * Print some headings.
 */
	fprintf(fdlow,"\t\t/ UNIX/TS - Bell Laboratories /\n\n");
	fprintf(fdlow,"/ Low core\n\nbr4 = 200\nbr5 = 240\n");
	fprintf(fdlow,"br6 = 300\nbr7 = 340\n");
	fprintf(fdlow,"SSR0 = 177572\n\n");
	fprintf(fdlow,".globl\tpowmm, powback, tstart, restart, dstart, start, dump, call, _utsname\n");
	fprintf(fdlow,".globl\ttrap, trap4, trap10, trap14, trap24, trap240, trap250\n\n");
}

/*
 * This routine prints the low core vectors for either Ispace or Dspace
 */
prtlvec(space)
char	*space;
{
	register short i;
	register struct t *ptr;
	register struct t *ktemp;

	if(space[0] == 'T')
		fprintf(fdlow,"\n.text\n\n%s:\n\n", space);
	else
		fprintf(fdlow,"\n.data\n\n%s:\n\n", space);
/*
 * Go through low core in double-word increments.
 */
	for (i=0; i<128; i++) {
		ktemp = lowptr[i];
/*
 * If the pointer is 0, set vector up for stray interrupt catcher.
 */
		if (ktemp == 0)
			fprintf(fdlow,"\t.+2;\tbr .\n");
		else {
			fprintf(fdlow,". = %s+%o\n",space ,i*4);
/*
 * See if pointer refers to a filler message.
 */
			if (ktemp < 20)
				fprintf(fdlow,"%s\n",cputrap[(int)ktemp]);
/*
 * Pointer refers to a device, so set up the handler routine address with
 * the bus level and device sequence number, unless SUPP bit is on.
 */
			else {
				ptr = ktemp;
				if (ptr->vectsize == 8) {
					if (ptr->type & SUPP) {
						fprintf(fdlow,"\t.+2;\tbr .\n");
						fprintf(fdlow,"\t.+2;\tbr .\n");
					}
					else {
						fprintf(fdlow,"\t%sin;\tbr%d+%d.\n",
							ptr->handler,
							ptr->buslevel,
							ptr->count);
						fprintf(fdlow,"\t%sou;\tbr%d+%d.\n",
							ptr->handler,
							ptr->buslevel,
							ptr->count);
					}
					i++;
				}
				else {
					if (ptr->type & SUPP) {
						fprintf(fdlow,"\t.+2;\tbr .\n");
					}
					else {
						fprintf(fdlow,"\t%sio;\tbr%d+%d.\n",
							ptr->handler,
							ptr->buslevel,
							ptr->count);
					}
				}
			}
		}
	}
	fprintf(fdlow,". = %s+1000\n", space);
}

/*
 * Print headings for C interface code.
 */
prtlinter()
{
	char *jmp;
	register struct t *ptr;

	if(!nflag)
		jmp = "jmp ";
	else
		jmp = "";
	fprintf(fdlow,"\n\n/ Interface code to C\n\n");
	fprintf(fdlow,".globl\t_clock\nclio:\n\tjsr\tr0,call; %s _clock\n\n", jmp);
/*
 * Go through configuration table and print out all required handler references.
 * For devices that are not interrupt driven, and those that have the
 * SUPP bit on, the printing of the handler name is suppressed.
 */
	for (ptr=table; ptr->devname!=0; ptr++) {
		if (ptr->count == 0) {
			if ((ptr->type & SUPP) || (ptr->vectsize == 0))
				continue;
			if (ptr->vectsize == 4) {
				fprintf(fdlow,".globl\t_%sintr\n%sio:\n\tjsr\tr0,call; %s _%sintr\n\n",
					ptr->handler,
					ptr->handler,
					jmp,
					ptr->handler);
			}
			else {
				fprintf(fdlow,out[0],
					ptr->handler,
					ptr->handler,
					ptr->handler,
					jmp,
					ptr->handler);
				fprintf(fdlow,out[1],
					ptr->handler,
					jmp,
					ptr->handler);
			}
		}
	}
}

/*
 * This routine is used to read in the master device table and the
 * alias table.  In the master device file, these two tables are separated
 * by a line that contains an asterisk in column 1.
 */
file()
{
	register short l;
	register FILE *fd;
	fd = fopen(mfile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",mfile);
		exit(1);
	}
	q = devinfo;
	
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * alias table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		dbound++;
		if (dbound == DSIZE) {
			printf("Device table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%d%o%o%4s%d%d%d%d%d%8s%8s%8s",
			q->dev,&(q->vsize),&(q->mask),&(q->type2),
			q->hndlr,&(q->asize),&(q->block),
			&(q->charr),&(q->mult),&(q->busmax),
			q->entries[0],q->entries[1],q->entries[2]);
		if (l < 10) {
			printf("%s\nDevice parameter count\n",line);
			exit(1);
		}
		if (q->type2 & REQ) {
			if (q->vsize || q->asize || q->busmax) {
				printf("%s\nParameter inconsistency\n",line);
				exit(1);
			}
		}
/*
 * Update the ends of the block and character device tables.
 */
		if (q->type2 & BLOCK)
			if ((blockex = max(blockex,q->block)) >= BSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}
		if (q->type2 & CHAR)
			if ((charex = max(charex,q->charr)) >= CSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}
		q++;
	}
	r = alias;
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * keyword table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		abound++;
		if (abound == ASIZE) {
			printf("Alias table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%8s",r->new,r->old);
		if (l < 2) {
			printf("%s\nAlias parameter count\n",line);
			exit(1);
		}
		r++;
	}
	kwdptr = parms;
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		pbound++;
		if (pbound == PSIZE) {
			printf("Keyword table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%20s%20s%20s",kwdptr->indef,
			kwdptr->oudef,kwdptr->defval);
		if (l < 2) {
			printf("%s\nTunable parameter count\n",line);
			exit(1);
		}
		if (l == 2)
			*(kwdptr->defval) = NULL;
		kwdptr++;
	}
	return;
}

/*
 * This routine compares two null terminated strings for equality.
 */
equal(s1,s2)
register char *s1, *s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0)
			return(1);
	}
	return(0);
}

/*
 * This routine is used to print a configuration time error message
 * and then set a flag indicating a contaminated configuration.
 */
error(message)
char *message;
{
	printf("%s%s\n\n",line,message);
	eflag++;
	return;
}

/*
 * This routine is used to search through the master device table for
 * some specified device.  If the device is found, we return a pointer to
 * the device.  If the device is not found, we search the alias table for
 * this device.  If the device is not found in the alias table, we return a
 * zero.  If the device is found, we change its name to the reference name of
 * the device and re-initiate the search for this new name in the master
 * device table.
 */
struct t2 *
find(device)
char *device;
{
	register struct t2 *q;
	register struct t3 *r;
	for (;;) {
/*
 * Search the master device table for the specified device.
 */
		for (q=devinfo; q<= &devinfo[dbound]; q++) {
			if (equal(device,q->dev))
				return(q);
		}
/*
 * Since the device was not found in the master device table,
 * we now search through the alias table.
 */
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
/*
 * See if the device was found in the alias table. If it wasn't, return a 0.
 */
		if (r > &alias[abound])
			return(0);
/*
 * It was found in the alias table, so we change its name back to the
 * reference name of the device and re-initiate the master device table search.
 */
	}
}

/*
 * This routine is used to set the character and/or block table pointers
 * to point to an entry of the master device table.
 */
setq()
{
	register struct t2 *ptr;
/*
 * Do a switch on the type of device.
 */
	switch (q->type2 & (BLOCK|CHAR)) {
/*
 * Device is not a block or character device.
 */
	case 0:	break;
/*
 * Device is a character type device.
 */
	case CHAR:
		subtmp = q->charr;
		ptr = cdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subtmp = charex;
			}
		}
		cdevices[subtmp] = q;
		cmax = max (cmax,subtmp);
		break;
/*
 * Device is a block type device.
 */
	case BLOCK:
		subtmp = q->block;
		ptr = bdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf("Block table overflow\n");
					exit(1);
				}
				q->block = subtmp = blockex;
			}
		}
		bdevices[subtmp] = q;
		bmax = max (bmax,subtmp);
		break;
/*
 * Device is both a character and block type device.
 */
	case BLOCK|CHAR:
		subtmp = q->charr;
		ptr = cdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subtmp = charex;
			}
		}
		cdevices[subtmp] = q;
		cmax = max (cmax,subtmp);
		subtmp = q->block;
		ptr = bdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf("Block table overflow\n");
					exit(1);
				}
				q->block = subtmp = blockex;
			}
		}
		bdevices[subtmp] = q;
		bmax = max (bmax,subtmp);
		break;
	}
	return;
}

/*
 * This routine writes out the configuration file (C program.)
 */
prtconf()
{
	register short i, j, counter;
	short k;
	char *sname;
	struct t2 *ptr;
/*
 * Print some headings.
 */
	fprintf(fdconf,"\n");
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		fprintf(fdconf,"#define\t%s\t%s\n",
			kwdptr->oudef, kwdptr->value);
	}
	fprintf(fdconf,"\n#include\t\"sys/param.h\"\n");
	fprintf(fdconf,"#include\t\"sys/types.h\"\n");
	fprintf(fdconf,"#include\t\"sys/sysmacros.h\"\n");
	fprintf(fdconf,"#include\t\"sys/space.h\"\n");
	fprintf(fdconf,"#include\t\"sys/io.h\"\n");
	fprintf(fdconf,"#include\t\"sys/conf.h\"\n\n");

/*
 * Print the extern statement for the nodev and nulldev entries.
 */
	fprintf(fdconf,"extern nodev(), nulldev();\n");

/*
 * Search the configuration table and generate an extern statement for
 * any routines that are needed.
 */
	for (p=table; p<= &table[tbound]; p++) {
		if (p->count)
			continue;
		switch (p->type & (BLOCK|CHAR)) {
		case CHAR:
			q = find(p->devname);
			sprintf(xbf,"extern ");
			if (q->mask & OPEN) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"open(), ");
			}
			if (q->mask & CLOSE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"close(), ");
			}
			if (q->mask & READ) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"read(), ");
			}
			if (q->mask & WRITE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"write(), ");
			}
			if (q->mask & IOCTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"ioctl(), ");
			}
			if (q->mask & PWR)
				if (power) {
					strcat (xbf,q->hndlr);
					strcat (xbf,"clr(), ");
				}
			if (q->mask & INIT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"init(), ");
			}
			xbf[strlen(xbf)-2] = ';';
			if (q->mask & TTYS) {
				strcat(xbf,"extern struct tty ");
				strcat(xbf,q->hndlr);
				strcat(xbf,"_tty[];\n");
			}
			xbf[strlen(xbf)-1] = NULL;
			fprintf(fdconf,"%s\n",xbf);
			break;
		case BLOCK:
			fprintf(fdconf,"extern %sopen(), %sclose(),",
				p->handler,p->handler);
			fprintf(fdconf," %sstrategy(), %sprint();\n",
				p->handler,p->handler);
			break;
		case BLOCK|CHAR:
			q = find(p->devname);
			sprintf(xbf,"extern %sopen(), %sclose(), ",
				q->hndlr,q->hndlr);
			if (q->mask & READ) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"read(), ");
			}
			if (q->mask & WRITE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"write(), ");
			}
			if (q->mask & IOCTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"ioctl(), ");
			}
			if (q->mask & PWR)
				if (power) {
					strcat (xbf,q->hndlr);
					strcat (xbf,"clr(), ");
				}
			if (q->mask & INIT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"init(), ");
			}
			strcat (xbf,q->hndlr);
			strcat (xbf,"strategy(), ");
			strcat (xbf,q->hndlr);
			strcat (xbf,"print();");
			fprintf(fdconf,"%s\n",xbf);
			break;
		}
	}

	fprintf(fdconf,"\nstruct bdevsw bdevsw[] = {\n");
/*
 * Go through block device table and indicate addresses of required routines.
 * If a particular device is not present, fill in "nodev" entries.
 */
	for (i=0; i<=bmax; i++) {
		ptr = bdevices[i];
		fprintf(fdconf,"/*%2d*/\t",i);
		if (ptr) {
			fprintf(fdconf,"%sopen,\t%sclose,\t%sstrategy,\t%sprint,\n",
				ptr->hndlr,
				ptr->hndlr,
				ptr->hndlr,
				ptr->hndlr);
		}
		else {
			fprintf(fdconf,"nodev, \tnodev, \tnodev, \t0, \n");
		}
	}
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"struct cdevsw cdevsw[] = {\n");
/*
 * Go through character device table and indicate addresses of required
 * routines, or indicate "nulldev" if routine is not present.  If a
 * particular device is not present, fill in "nodev" entries.
 */
	for (j=0; j<=cmax; j++) {
		ptr = cdevices[j];
		fprintf(fdconf,"/*%2d*/",j);
		if (ptr) {
			if (ptr->mask & OPEN)
				fprintf(fdconf,"\t%sopen,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnulldev,");
			if (ptr->mask & CLOSE)
				fprintf(fdconf,"\t%sclose,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnulldev,");
			if (ptr->mask & READ)
				fprintf(fdconf,"\t%sread,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & WRITE)
				fprintf(fdconf,"\t%swrite,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & IOCTL)
				fprintf(fdconf,"\t%sioctl,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & TTYS)
				fprintf(fdconf,"\t%s_tty,\n",ptr->hndlr);
			else
				fprintf(fdconf,"\t0,\n");
		}
		else
			fprintf(fdconf,"\tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \t0,\n");
	}
/*
 * Print out block and character device counts, root, swap, and dump device
 * information, and the swplo, and nswap values.
 */
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"int\tbdevcnt = %d;\nint\tcdevcnt = %d;\n\n",i,j);
	fprintf(fdconf,"dev_t\trootdev\t{(%d<<8)|%#o};\n",rtmaj,rtmin);
	fprintf(fdconf,"dev_t\tpipedev\t{(%d<<8)|%#o};\n",pipmaj,pipmin);
	fprintf(fdconf,"dev_t\tdumpdev\t{(%d<<8)|%#o};\n",dmpmaj,dmpmin);
	fprintf(fdconf,"dev_t\tswapdev\t{(%d<<8)|%#o};\n",swpmaj,swpmin);
	fprintf(fdconf,"daddr_t\tswplo\t%lu;\n",swplo);
	fprintf(fdconf,"int\tnswap\t%d;\n\n",nswap);

/*
 * Initialize the power fail and init handler buffers.
 */
	sprintf(pwrbf,"\nint\t(*pwr_clr[])() = \n{\n");
	sprintf(initbf,"\nint\t(*dev_init[])() = \n{\n");
/*
 * Go through the block device table, ignoring any zero entries.
 * Add power fail and init handler entries to the buffers as appropriate.
 */
	if (tflag)
		printf("Block Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=bmax; i++) {
		if ((q=bdevices[i]) == 0)
			continue;
		if (power)
			if (q->mask & PWR)
				sprintf(&pwrbf[strlen(pwrbf)],"\t%sclr,\n",q->hndlr);
		if (q->mask & INIT)
			sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		counter = 0;
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		fprintf(fdconf,"\n");
		if (q->asize)
			fprintf(fdconf,"int\t%s_addr[]\n{\n",q->hndlr);
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(q->dev,p->devname)) {
				if (q->asize)
					fprintf(fdconf,"\t%#o,\n",p->address);
				counter += p->mlt;
			}
		}
		if (q->asize)
			fprintf(fdconf,"};\n");
		if (!(q->type2 & NOCNT))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
		q->acount = counter;
/*
 * Print any required structure definitions.
 */
		for (j=0; j<3; j++) {
			if (*(q->entries[j]) != NULL)
				fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Go through the character device table, ignoring any zero entries,
 * as well as those that are not strictly character devices.
 * Add power fail and init handler entries to the buffers as appropriate.
 */
	if (tflag)
		printf("Character Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=cmax; i++) {
		if ((q=cdevices[i]) == 0)
			continue;
		if ((q->type2 & (BLOCK|CHAR)) != CHAR) {
			if (tflag)
				printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,q->acount);
			continue;
		}
		if (power)
			if (q->mask & PWR)
				sprintf(&pwrbf[strlen(pwrbf)],"\t%sclr,\n",q->hndlr);
		if (q->mask & INIT)
			sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		counter = 0;
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		fprintf(fdconf,"\n");
		if (q->asize)
			fprintf(fdconf,"int\t%s_addr[] = {\n",q->hndlr);
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(q->dev,p->devname)) {
				if (q->asize)
					fprintf(fdconf,"\t%#o,\n",p->address);
				counter += p->mlt;
			}
		}
		if (q->asize)
			fprintf(fdconf,"};\n");
		if (!(q->type2 & NOCNT))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
/*
 * Print any required structure definitions.
 */
		for (j=0; j<3; j++) {
			if (*(q->entries[j]) != NULL)
				fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Repeat this swill for the weirdo devices.
 */
	for (k=0; (sname=sdev[k]) != NULL; k++)
		if (locate(sname)) {
			q = find(sname);
			counter = 0;
			fprintf(fdconf,"\nint\t%s_addr[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal (p->devname,sname)) {
					fprintf(fdconf,"\t%#o,\n",p->address);
					counter += p->mlt;
				}
			}
			fprintf(fdconf,"};\n");
			if (!(q->type2 & NOCNT))
				fprintf(fdconf,"int\t%s_cnt = %d;\n",
					q->hndlr,counter);
			for (j=0; j<3; j++) {
				if (*(q->entries[j]) != NULL)
					fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
						q->entries[j],q->hndlr,
						q->entries[j],counter);
			}
		}
/*
 * Write out NULL entry into power fail and init buffers.
 * Then write the buffers out into the configuration file.
 */
	sprintf(&pwrbf[strlen(pwrbf)],"\t(int (*)())0\n};\n");
	sprintf(&initbf[strlen(initbf)],"\t(int (*)())0\n};\n");
	fprintf(fdconf,"%s",pwrbf);
	fprintf(fdconf,"%s",initbf);
	return;
}

max(a,b)
short a, b;
{
	return (a>b ? a:b);
}

/*
 * This routine is used to search the configuration table for some
 * specified device.  If the device is found we return a pointer to
 * that device.  If the device is not found, we search the alias
 * table for this device.  If the device is not found in the alias table
 * we return a zero.  If the device is found, we change its name to
 * the reference name of the device and re-initiate the search for this
 * new name in the configuration table.
 */

struct t *
locate(device)
char *device;
{
	register struct t *p;
	register struct t3 *r;
	for (;;) {
/*
 * Search the configuration table for the specified device.
 */
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(device,p->devname))
				return(p);
		}
/*
 * Since the device was not found in the configuration table, we now
 * search through the alias table.
 */
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
/*
 * See if the device was found in the alias table. If it wasn't, return a 0.
 */
		if (r > &alias[abound])
			return(0);
/*
 * It was found in alias table, so we change its name back to the
 * reference name of the device and re-initiate the configuration
 * table search.
 */
	}
}

/*
 * This routine is used to search the parameter table
 * for the keyword that was specified in the configuration.  If the
 * keyword cannot be found in this table, a value of zero is returned.
 * If the keyword is found, a pointer to that entry is returned.
 */
struct t4 *
lookup(keyword)
char *keyword;
{
	register struct t4 *kwdptr;
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (equal(keyword,kwdptr->indef))
			return (kwdptr);
	}
	return(0);
}

/*
 * This routine is used to map lower case alphabetics into upper case.
 */
char *
uppermap(device,caps)
char *device;
char *caps;
{
	register char *ptr;
	register char *ptr2;
	ptr2 = caps;
	for (ptr=device; *ptr!=NULL; ptr++) {
		if ('a' <= *ptr && *ptr <= 'z')
			*ptr2++ = *ptr + 'A' - 'a';
		else
			*ptr2++ = *ptr;
	}
	*ptr2 = NULL;
	return (caps);
}

/*
 * This routine enters the device in the configuration table.
 */
enterdev(vec,add,bl,ml,name)
short	vec, bl, ml;
unsigned	add;
char	*name;
{
	tbound++;
	if (tbound == TSIZE) {
		printf("Configuration table overflow\n");
		exit(1);
	}
/*
 * Write upper case define statement and sequence number for device.
 */
	fprintf(fdconf,"#define\t%s_%d %d\n",
		uppermap (name,capitals),
		q->dcount, ml);
	if (q->vsize)
		setlow(vec);
	setq();
	p->devname = q->dev;
	p->vector = vec;
	p->address = add;
	p->buslevel = bl;
	p->addrsize = q->asize;
	p->vectsize = q->vsize;
	p->type = q->type2;
	p->handler = q->hndlr;
	p->count = q->dcount;
	p->mlt = ml;
	p->blk = q->block;
	p->chr = q->charr;
	p++;
	q->dcount++;
	return;
}

/*
 * This routine enters the appropriate pointer for the device
 * into low core.
 */
setlow(vec)
short vec;
{
	subv = vec/4;
	if (q->vsize == 8)
		lowptr[subv+1] = 20;
	lowptr[subv] = p;
	return;
}

addrcheck(add,size)
unsigned add;
short size;
{
	register struct map *mp;
	register struct map *hold;
	for (mp= &map[mbound]; mp >= map; mp--) {
		if (add >= mp->start) {
			if ((add+size) > (mp->start+mp->length))
				return(1);
			if ((add == mp->start) &&
				((size+add)==(mp->start+mp->length))) {
				mbound--;
				while (mp <= &map[mbound]) {
					mp->start = (mp+1)->start;
					mp->length = (mp+1)->length;
					mp++;
				}
				return(0);
			}
			if (add == mp->start) {
				mp->start += size;
				mp->length -= size;
				return(0);
			}
			if ((size + add) == (mp->start + mp->length)) {
				mp->length -= size;
				return(0);
			}
			hold = mp;
			if ((mbound + 1) == MSIZE) {
				printf("Address map table overflow\n");
				exit(1);
			}
			for (mp= &map[mbound]; mp >= hold; mp--) {
				(mp+1)->start = mp->start;
				(mp+1)->length = mp->length;
			}
			mp++;
			mp->length = add - mp->start;
			(mp+1)->start = size + add;
			(mp+1)->length = (mp+1)->length-(size+add)+mp->start;
			mbound++;
			return(0);
		}
	}
	return(1);
}
