/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fmthard:fmthard.c	1.5"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <sys/id.h>
#include <sys/open.h>

#define	nel(a)	(sizeof(a)/sizeof(*(a)))	/* Number of array elements */

#define SWAP_10	  3500
#define SWAP_32	  6020
#define SWAP_72	 10106
 
#define ROOT_10	  8928
#define ROOT_32	 12510
#define ROOT_72	 12636

#define USR_10	  9360
#define USR_32	 43920
#define USR_72	126684

#define SIZE_10	 21888
#define SIZE_32	 62550
#define SIZE_72	149526

#define BOOTSIZE   100

#define PDBLKNO		0x00
#define MFGBLKNO	0x01
#define DEFBLKNO	0x02
#define RELBLKNO	0x03
#define NUMOFREL	0x01
#define BLKSZ		512
#define FULLDISK	6

union datatype	{
	struct idsector0 o_sect0;
	struct pdsector sect0;
} datastruct;

unsigned long logical;
unsigned long size_newdsk;
unsigned long cyl_blks;
unsigned long swap;
unsigned long boot;
unsigned long root;
unsigned long usr;	
int lsize, ssize, sroot, lroot, sswap, lswap, part_size;

/*
 * External functions.
 */
extern int errno;
void	exit();
char	*strchr();
char	*strcpy();
char	*strncpy();

/*
 * Internal functions.
 */
void    coredsk();
void 	display();
void    dftdsk();
void	initial();
void	insert();
void	load();
void	minval();
void	mkfs();
void	pread();
void	pwrite();
int	round();
void	tinyvtoc();
void	upgrade();
void	usage();
void	validate();
void	values();

/*
 * Static variables.
 */
static char	*cflag;			/* Core system disk */
static char	*delta;			/* Incremental update */
static short	iflag;			/* Prints VTOC w/o updating */
static short	mflag;			/* Make empty filesystems */
static short	pflag;			/* Just print the vtoc */
static short	qflag;			/* Just check for a 1.0-format disk */
static short	rflag;			/* Just print minimum partition sizes */
static short	tflag;			/* Set up minimum partition size in VTOC */
static short	vflag;			/* Verbose */

main(argc, argv)
int	argc;
char	**argv;
{
	int		fd;
	int		c;
	char		*dfile, *vname;
	struct vtoc	bufvtoc;
	extern char	*optarg;
	extern int	optind;

	dfile = NULL;
	vname = NULL;
	while ((c = getopt(argc, argv, "c:d:imn:qrs:tvp")) != EOF)
		switch (c) {
		case 'c':
		 	cflag=optarg;
			break;
		case 'd':
			delta = optarg;
			break;
		case 'i':
			++iflag;
			break;
		case 'm':
			++mflag;
			break;
		case 'n':
			vname = optarg;
			break;
		case 'p':
			++pflag;
			break;
		case 'q':
			++qflag;
			break;
		case 'r':
			++rflag;
			break;
		case 's':
			dfile = optarg;
			break;
		case 't':
			++tflag;
			break;
		case 'v':
			++vflag;
			break;
		default:
			usage();
		}

	if (argc - optind != 1)
		usage();
	if(strncmp(argv[optind],"/dev/r",6) != 0)	{
		(void) fprintf(stderr, "fmthard:  Must specify a raw device.\n");
		exit(1);
	}	
	if ((fd = open(argv[optind], O_RDWR)) < 0) {
		(void) fprintf(stderr, "fmthard:  Cannot open device %s\n",argv[optind]);
		exit(1);
	}

	/* READ SECTOR 0 */
	pread(fd, PDBLKNO, BLKSZ, (char *) &datastruct);

	if (qflag)
		exit(datastruct.o_sect0.reserved == 0xfeedbeef ? 0 : 1);

	/*
	 * Upgrade a Release 1.0 disk. The alternate-case pwrite() is
	 * needed to initialize the driver in the case of a "new" disk.
	 */
	if (datastruct.o_sect0.reserved == 0xfeedbeef)
		upgrade(fd, &datastruct);
	else
		pwrite(fd, PDBLKNO, BLKSZ, (char *) &datastruct);

	logical = datastruct.sect0.pdinfo.logicalst;

	cyl_blks = datastruct.sect0.pdinfo.sectors * 
		   datastruct.sect0.pdinfo.tracks;

	size_newdsk = datastruct.sect0.pdinfo.sectors * 
		      datastruct.sect0.pdinfo.tracks * 
		      datastruct.sect0.pdinfo.cyls -
		      logical - cyl_blks ;

	if (rflag) {
		minval();
		exit(0);
	}
	if (delta) {
		pread(fd, logical + 1, sizeof(struct vtoc), (char *) &bufvtoc);
		if (bufvtoc.v_sanity != VTOC_SANE) {
			(void) fprintf(stderr, "%s: Invalid VTOC\n", argv[optind]);
			exit(1);
		}
		insert(delta, &bufvtoc);
		validate(&datastruct.sect0.pdinfo, &bufvtoc);
		pwrite(fd, logical + 1, sizeof(struct vtoc), (char *) &bufvtoc);
		exit(0);
	} else {
		initial(vname, &datastruct.sect0.pdinfo, &bufvtoc);
	}
	if (tflag) {
		tinyvtoc(size_newdsk, &bufvtoc);
		validate(&datastruct.sect0.pdinfo, &bufvtoc);
		pwrite(fd, logical + 1, sizeof(struct vtoc), (char *) &bufvtoc);
		exit(0);
	}
	if (dfile)
		load(dfile, &datastruct.sect0.pdinfo, &bufvtoc);
	else if (cflag)  {
		if(*cflag == '0' || *cflag == '1') 
			coredsk (size_newdsk, cyl_blks, &bufvtoc);
		else if (*cflag == '2')
			dftdsk (size_newdsk, cyl_blks, &bufvtoc);
		else {
			fprintf(stderr,"fmthard: argument for -c must be 0, 1 or2.\n");
			exit(1);
		}
	} else 
		dftdsk (size_newdsk, cyl_blks, &bufvtoc);

	if (iflag) {
		display(&bufvtoc, argv[optind]);
		exit(0);
	}
	if (pflag) {
		values();
		exit(0);
	}
	validate(&datastruct.sect0.pdinfo, &bufvtoc);
	pwrite(fd, logical + 1, sizeof(struct vtoc), (char *) &bufvtoc);
	(void) printf("fmthard:  New volume table of contents now in place.\n");
	if (mflag)
		mkfs(argv[optind], bufvtoc.v_part,
		  datastruct.sect0.pdinfo.sectors
		    * datastruct.sect0.pdinfo.tracks);
	exit(0);
}

/*
 *  coredsk()    
 *
 * Sets up the VTOC for a single core system disk or the first 
 * disk of dual disk system.  Contains partitions for boot, swap, 
 * and root (and usr on a single core disk.)
 */
static void
coredsk(size_newdsk, cyl_blks, vtoc)
unsigned long		size_newdsk;
unsigned long		cyl_blks;
struct vtoc	*vtoc;
{
	auto int 	part;
	auto int	tag;
	auto int 	flag;
	auto int 	start;
	auto int 	size;
	auto int 	remainder;
	int		last_used;
	int 		part_size;

	if (size_newdsk < SIZE_10) {
		(void) fprintf(stderr, "Error:  Disks smaller than 10M are not supported as core system disks.");

		exit(1);
	}
	else if (size_newdsk > SIZE_72) {
		swap = SWAP_72 * size_newdsk / SIZE_72;
		root = ROOT_72 * size_newdsk / SIZE_72;
	}
	else {
		if (size_newdsk >= SIZE_10 && size_newdsk < 
		SIZE_32) {

 			ssize = SIZE_10;
			lsize = SIZE_32;
			sswap = SWAP_10;
			lswap = SWAP_32;
			sroot = ROOT_10;
			lroot = ROOT_32;
		} else {
 			ssize = SIZE_32;
			lsize = SIZE_72;
			sswap = SWAP_32;
			lswap = SWAP_72;
			sroot = ROOT_32;
			lroot = ROOT_72;
		}
		swap = (size_newdsk - ssize) * (lswap - sswap) /
			(lsize - ssize) + sswap;
		root = (size_newdsk - ssize) * (lroot - sroot) /
			(lsize - ssize) + sroot;
    	}
	boot = BOOTSIZE;
	last_used = boot;
	swap = round (swap, last_used, cyl_blks);
	last_used = boot + swap;
	root = round (root, last_used, cyl_blks);
	last_used += root;
	remainder = size_newdsk - last_used;
	for (part=0; part < 16; ++part) {
		tag = 0;
		flag = V_UNMNT;
		start = 0;
		size = 0;
		switch (part) {
		case 0:
			tag = V_ROOT;
			flag = 00;
			start = boot + swap;
			size = root;
			break;
		case 1:
			tag = V_SWAP;
			start = boot;
			size = swap;
			break;
		case 2:
			if (cflag && *cflag == '0') {
				tag = V_USR;
				flag = 00;
				start = last_used; 
				usr = remainder;
				size = usr;
			}
			break;
		case 6:
			size = size_newdsk;
			break;
		case 7:
			size = boot;
			break;
		case 8:
			if (cflag && *cflag == '1') {
				flag = 00;
				start = last_used; 
				size = remainder;
			}
			break;
		}
		vtoc ->v_part[part].p_tag = tag;
		vtoc ->v_part[part].p_flag = flag;
		vtoc ->v_part[part].p_start = start;
		vtoc ->v_part[part].p_size = size;
	}
}

/*
 *  dftdsk()    
 *
 * Partitions a noncore disk or the second disk of dual core system. 
 * Contains partitions for boot and usr.
 */
static void
dftdsk (size_newdsk, cyl_blks, vtoc)
unsigned long	size_newdsk;
unsigned long	cyl_blks;
struct vtoc	*vtoc;
{
	auto int	part;
	auto int	tag;
	auto int 	flag;
	auto int 	start;
	auto int 	size;

	for (part=0; part<16; ++part) {
		tag = 0;
		flag = V_UNMNT;
		start = 0;
		size = 0;
		switch (part) {
		case 2:
			if (cflag && *cflag == '2') {
				tag = V_USR;
				flag = 00;
				start = cyl_blks;
				usr = size_newdsk - cyl_blks;
				size = usr;
			}
			break;
		case 6:
			size = size_newdsk;
			break;
		case 7:
			boot = cyl_blks;
			size = boot;
			break;
		case 8:
			if (!cflag) {
				flag = 00;
				start = cyl_blks;
				size = size_newdsk - cyl_blks;
			}
			break;
		}
		vtoc ->v_part[part].p_tag = tag;
		vtoc ->v_part[part].p_flag = flag;
		vtoc ->v_part[part].p_start = start;
		vtoc ->v_part[part].p_size = size;
	}
}

/*
 * display ()	
 *
 * display contents of VTOC without writing it to disk
 */
static void
display(buf, device)
struct vtoc	*buf;
char 		*device;
{
	register int	i;

	/* PRINT OUT THE VTOC */
	printf("* %s default partition map\n",device);
	if(*buf->v_volume)
		printf("* Volume Name:  %s\n",buf->v_volume);
	printf("*\n");
	printf("* Dimensions:\n");
	printf("*     %d bytes/sector\n",datastruct.sect0.pdinfo.bytes);
	printf("*      %d sectors/track\n",datastruct.sect0.pdinfo.sectors);
	printf("*       %d tracks/cylinder\n",datastruct.sect0.pdinfo.tracks);
	printf("*     %d cylinders\n",datastruct.sect0.pdinfo.cyls);
	printf("*     %d accessible cylinders\n",(datastruct.sect0.pdinfo.cyls - 1 - (logical/cyl_blks) ) );
	printf("*\n");
	printf("* Flags:\n");
	printf("*   1:  unmountable\n");
	printf("*  10:  read-only\n");
	printf("*\n");
	printf("\n* Partition    Tag     Flag	    First Sector    Sector Count\n");
	for(i=0;i<16;i++)	{
		if(buf->v_part[i].p_size > 0) 
			printf("    %d		%d	0%x		%d		%d\n",i,buf->v_part[i].p_tag, buf->v_part[i].p_flag, buf->v_part[i].p_start, buf->v_part[i].p_size);
	}
	exit(0);
}

/*
 * initial()
 *
 * Initialize a new VTOC.
 */
static void
initial(volume, pdinfo, vtoc)
char			*volume;
struct pdinfo		*pdinfo;
register struct vtoc	*vtoc;
{
	register int	i;

	for(i = 0; i < nel(vtoc->v_bootinfo); ++i)
		vtoc->v_bootinfo[i] = 0;
	vtoc->v_sanity = VTOC_SANE;
	vtoc->v_version = V_VERSION;
	for(i = 0; i < nel(vtoc->v_volume); ++i)
		vtoc->v_volume[i] = '\0';
	if (volume)
		(void) strncpy(vtoc->v_volume, volume, nel(vtoc->v_volume));
	vtoc->v_sectorsz = pdinfo->bytes;
	vtoc->v_nparts = V_NUMPAR;
	for(i = 0; i < nel(vtoc->v_reserved); ++i)
		vtoc->v_reserved[i] = 0;
	for(i = 0; i < nel(vtoc->v_part); ++i) {
		vtoc->v_part[i].p_tag = 0;
		vtoc->v_part[i].p_flag = V_UNMNT;
		vtoc->v_part[i].p_start = 0;
		vtoc->v_part[i].p_size = 0;
	}
}

/*
 * insert()
 *
 * Insert a change into the VTOC.
 */
static void
insert(data, vtoc)
char		*data;
struct vtoc	*vtoc;
{
	auto int	part;
	auto int	tag;
	auto int	flag;
	auto int	start;
	auto int	size;

	if (sscanf(data, "%d:%d:%x:%d:%d",
	    &part, &tag, &flag, &start, &size) != 5) {
		(void) fprintf(stderr, "Delta syntax error on \"%s\"\n", data);
		exit(1);
	}
	vtoc->v_part[part].p_tag = tag;
	vtoc->v_part[part].p_flag = flag;
	vtoc->v_part[part].p_start = start;
	vtoc->v_part[part].p_size = size;
}

/*
 * load()
 *
 * Load VTOC information from a datafile.
 */
static void
load(dfile, pdinfo, vtoc)
char		*dfile;
struct pdinfo	*pdinfo;
struct vtoc	*vtoc;
{
	FILE		*dstream;
	auto int	part;
	auto int	tag;
	auto int	flag;
	auto int	start;
	auto int	size;
	char		line[256];

	if (strcmp(dfile, "-") == 0)
		dstream = stdin;
	else if ((dstream = fopen(dfile, "r")) == NULL) {
		(void) fprintf(stderr, "Cannot open file %s\n", dfile);
		exit(1);
	}
	while (fgets(line, sizeof(line) - 1, dstream)) {
		if (line[0] == '\0' || line[0] == '\n' || line[0] == '*')
			continue;
		line[strlen(line) - 1] = '\0';
		if (sscanf(line, "%d %d %x %d %d",
		    &part, &tag, &flag, &start, &size) != 5) {
			(void) fprintf(stderr, "%s: Syntax error on \"%s\"\n",
			    dfile, line);
			exit(1);
		}
		if (part >= V_NUMPAR) {
			(void) fprintf(stderr,
			    "Error in datafile %s: No such partition %d\n",
			    dfile, part);
			exit(1);
		}
		vtoc->v_part[part].p_tag = tag;
		vtoc->v_part[part].p_flag = flag;
		vtoc->v_part[part].p_start = start;
		vtoc->v_part[part].p_size = size;
	}
	if (dstream != stdin && fclose(dstream) == EOF) {
		(void) fprintf(stderr, "I/O error reading datafile %s\n", dfile);
		exit(1);
	}
}

/*
 * minval()
 *
 * Prints the minimum partition sizes (for use by the instf command).
 */
static void
minval()
{
	printf("bootmin=%ld diskmin=%ld rootmin=%ld swapmin=%ld usrmin=%ld swapbase=%ld\n",
	    BOOTSIZE, SIZE_10, ROOT_10, SWAP_10, USR_10, BOOTSIZE);
}

/*
 * mkfs()
 *
 * Make a filesystem on each viable partition.
 */
static void
mkfs(devname, part, cylsize)
char *				devname;
register struct partition	*part;
unsigned long			cylsize;
{
	register int	idx;
	char		cmd[100];
	int		partno;

	partno = strlen(devname) -1;
	for (idx = 0; idx < V_NUMPAR; ++idx)
		if ((part[idx].p_flag & V_UNMNT) == 0 && 
                (part[idx].p_flag & V_RONLY) == 0 &&
    		part[idx].p_size) {
			devname[partno] = "0123456789abcdef"[idx];
			(void) sprintf(cmd,
			    "mkfs %s %ld 9 %lu",
			    devname, part[idx].p_size, cylsize);
			if (vflag)
				(void) printf("+(fmthard) %s\n", cmd);
			if (system(cmd)) {
				(void) fprintf(stderr,
				    "fmthard: \"%s\" failed\n", cmd);
				exit(1);
			}
		}
}

static void
pread(fd, block, len, buf)
int		fd;
unsigned long	block;
unsigned long	len;
char		*buf;
{
	struct io_arg	args;

	args.sectst = block;
	args.datasz = len;
	args.memaddr = (unsigned long) buf;
	args.retval = 0;
	if (ioctl(fd, V_PREAD, &args) == -1 || args.retval == V_BADREAD) {
		(void) fprintf(stderr, "fmthard: Cannot read block %lu\n", block);
		exit(1);
	}
}

static void
pwrite(fd, block, len, buf)
int		fd;
unsigned long	block;
unsigned long	len;
char		*buf;
{
	struct io_arg	args;

	args.sectst = block;
	args.datasz = len;
	args.memaddr = (unsigned long) buf;
	args.retval = 0;
	if(ioctl(fd,V_PWRITE,&args) == -1 || args.retval == V_BADWRITE) {
		if (errno == EPERM)  {
			(void) fprintf(stderr, "fmthard: Must have super-user privileges \n");
		}else  {
			(void) fprintf(stderr, "fmthard: Cannot write block %lu  errno = %d\n", block, errno);
		}
		exit(1);
	}
}

/*
 *  round ()
 *
 *  Rounds the root and swap partitions up to the next higher 
 *  cylinder boundary.
 */
int
round (part_size, last_used, cyl_blks)
int 	part_size;
int 	last_used;
int	cyl_blks;
{
	int 	end_sect;	
	int 	nrst_blk_bnd;

	end_sect = last_used + part_size;
	if (end_sect % cyl_blks) {
		nrst_blk_bnd = end_sect / cyl_blks * cyl_blks;
		part_size = nrst_blk_bnd - last_used + cyl_blks;
	}	
	return(part_size);
}

/*
 * tinyvtoc()	
 *
 * Sets up the minimum partition sizes (for use by the partition command).
 */

static void
tinyvtoc (size_newdsk, vtoc) 
unsigned long	size_newdsk;
struct vtoc	*vtoc;
{
	auto int	part;
	auto int	tag;
	auto int	flag;
	auto int	start;
	auto int	size;

	for (part=0; part < 16; ++part) {
		tag = 0;
		flag = V_UNMNT;
		start = 0;
		size = 0;
		switch (part) {
		case 0:
			tag = V_ROOT;
			flag = 00;
			start = BOOTSIZE + SWAP_10;
			size = ROOT_10;
			break;
		case 1:
			tag = V_SWAP;
			start = BOOTSIZE;
			size = SWAP_10;
			break;
		case 2:
			tag = V_USR;
			flag = 00;
			start = BOOTSIZE + SWAP_10 + ROOT_10; 
			size = USR_10;
			break;
		case 6:
			size = SIZE_10;
			break;
		case 7:
			size = BOOTSIZE;
			break;
		}
		vtoc->v_part[part].p_tag = tag;
		vtoc->v_part[part].p_flag = flag;
		vtoc->v_part[part].p_start = start;
		vtoc->v_part[part].p_size = size;
	}
}

/*
 * upgrade()
 *
 * Upgrade a 1.0 defect table.
 */
static void
upgrade(fd, olddata)
int		fd;
union datatype	*olddata;
{

	struct pdsector bufsect;
	struct defstruct bufdef;
	unsigned int bufrel[128];
	int i, j;
	struct defect *new, *old;
	register unsigned long sectno, cyl;

	/* SETUP NEW SECTOR 0 */

	bufsect.pdinfo.driveid = olddata->o_sect0.driveid;
	bufsect.pdinfo.sanity = VALID_PD;
	bufsect.pdinfo.version = V_VERSION;
	for (i=0;i<12;i++)
		bufsect.pdinfo.serial[i] = 0x00;
	bufsect.pdinfo.cyls = olddata->o_sect0.cyls;
	bufsect.pdinfo.tracks = olddata->o_sect0.tracks;
	bufsect.pdinfo.sectors = olddata->o_sect0.sectors;
	bufsect.pdinfo.bytes = olddata->o_sect0.bytes;
	logical = bufsect.pdinfo.tracks * bufsect.pdinfo.sectors;
	bufsect.pdinfo.logicalst = logical;
	bufsect.pdinfo.errlogst = (bufsect.pdinfo.tracks * bufsect.pdinfo.sectors) - 1;
	bufsect.pdinfo.errlogsz = bufsect.pdinfo.bytes;
	bufsect.pdinfo.mfgst = MFGBLKNO;
	bufsect.pdinfo.mfgsz = bufsect.pdinfo.bytes;
	bufsect.pdinfo.defectst = DEFBLKNO;
	bufsect.pdinfo.defectsz = bufsect.pdinfo.bytes;
	bufsect.pdinfo.relno = NUMOFREL;
	bufsect.pdinfo.relst = RELBLKNO;
	bufsect.pdinfo.relsz = (bufsect.pdinfo.tracks * bufsect.pdinfo.sectors) - 4;
	for (i=0;i<10;i++)
		bufsect.reserved[i] = 0x00;
	for (i=0;i<97;i++)
		bufsect.devsp[i] = 0x00;

	/* READ 1ST RELOCATION SECTOR */
	pread(fd, DEFBLKNO, bufsect.pdinfo.bytes, (char *) bufrel);

	/*CREATE NEW DEFECT MAP */
	j = 0;
	old = (struct defect *)&olddata->o_sect0.iddeftab[0];
	new = &bufdef.map[0];
	while (old->bad.full != 0xffffffff)	{
		new->bad.full = old->bad.full;
		new->good.full = old->good.full;
		new++;
		old++;
		j++;
	}

	bufsect.pdinfo.relnext = (bufsect.pdinfo.relst + j-1); 
	for(;j<64;j++)	{
		bufdef.map[j].bad.full = 0xffffffff;
		bufdef.map[j].good.full = 0xffffffff;
	}

	/* WRITE 1ST RELOCATION SECTOR TO END OF LIST - UPDATE DEFECT MAP */
	if (bufdef.map[0].bad.full != 0xffffffff)	{
		cyl = bufsect.pdinfo.relnext / (bufsect.pdinfo.tracks * bufsect.pdinfo.sectors);
		bufdef.map[0].good.part.pcnh = ((cyl >> 8) & 0xff);
		bufdef.map[0].good.part.pcnl = cyl & 0xff;
		sectno = bufsect.pdinfo.relnext % (bufsect.pdinfo.tracks * bufsect.pdinfo.sectors);
		bufdef.map[0].good.part.phn = sectno / bufsect.pdinfo.sectors;
		bufdef.map[0].good.part.psn = sectno % bufsect.pdinfo.sectors;

		pwrite(fd, bufsect.pdinfo.relnext, bufsect.pdinfo.bytes, (char *) bufrel);
		bufsect.pdinfo.relnext++;
	}

	/* SEND SECTOR 0 TO IOCTL FOR A WRITE */
	pwrite(fd, PDBLKNO, bufsect.pdinfo.bytes, (char *) &bufsect);

	/* SEND DEFECT MAP TO IOCTL FOR A WRITE */
	pwrite(fd, DEFBLKNO, bufsect.pdinfo.bytes, (char *) &bufdef);

	/* READ IN NEW SECTOR O */
	pread(fd, PDBLKNO, BLKSZ, (char *) &datastruct);
}

static void
usage()
{
	(void) fprintf(stderr, "\
Usage:	fmthard [ -i ] [ -m ] [ -n volumename ]\n\
	[ -c coredisktype ] [ -s datafile ] device\n");
	exit(2);
}

/*
 * validate()
 *
 * Validate the new VTOC.
 */
static void
validate(pdinfo, vtoc)
struct pdinfo	*pdinfo;
struct vtoc	*vtoc;
{
	register int	i, j;
	int	fullsz, total, endsect;
	int	isize, istart, jsize, jstart;

	fullsz = (
	    (pdinfo->cyls - 1)
	    * pdinfo->tracks
	    * pdinfo->sectors
	) - logical;
	for(i=0, total=0; i<V_NUMPAR; i++) {
		if(i==6) {
			if(vtoc->v_part[6].p_size <= 0) {
				(void) fprintf(stderr, "\
fmthard: Partition 6 specifies the full disk and cannot have a size of 0.\n\
\tThe full disk capacity is %lu sectors.\n", fullsz);
			exit(1);
			}
		}
		if(vtoc->v_part[i].p_size == 0)
			continue;	/* Undefined partition */
		if (vtoc->v_part[i].p_start > fullsz
		  || vtoc->v_part[i].p_start
		    + vtoc->v_part[i].p_size > fullsz) {
			(void) fprintf(stderr, "\
fmthard: Partition %d specified as %lu sectors starting at %lu\n\
\tdoes not fit. The full disk contains %lu sectors.\n",
			    i, vtoc->v_part[i].p_size,
			    vtoc->v_part[i].p_start, fullsz);
			exit(1);
		}

		if (i!=6)	{
			total += vtoc->v_part[i].p_size;
			if(total > fullsz)	{
				(void) fprintf(stderr, "\
fmthard: Total of %lu sectors specified within partitions\n\
\texceeds the disk capacity of %lu sectors.\n",
				    fullsz);
				exit(1);
			}
			for(j=0; j<V_NUMPAR; j++)	{
				isize = vtoc->v_part[i].p_size;
				jsize = vtoc->v_part[j].p_size;
				istart = vtoc->v_part[i].p_start;
				jstart = vtoc->v_part[j].p_start;
				if((i!=j) && (j!=6) && (isize != 0) && (jsize != 0))	{
					endsect = jstart + jsize -1;
					if((jstart <= istart) && (istart <= endsect))	{
						(void) fprintf(stderr,"\
fmthard: Partition %d overlaps partition %d. Overlap is allowed\n\
\tonly on partition 6 (the full disk).\n",
						    i, j);
						exit(1);
					}
				}
			}
		}
	}
}

/*
 * values()
 *
 * Print the VTOC values.
 */
static void
values ()
{
	char drive;

	if (cflag) {
		if(*cflag == '0' || *cflag == '1') {
			drive = '0';
			printf("rootsize=%ld swapsize=%ld ",root,swap);
			if (*cflag == '0')
				printf("usrsize_0=%ld ",usr);
		}else if (*cflag == '2')  {
			drive = '1';
			printf("usrsize_1=%ld ",usr);
		}
	}

	printf("bootsize_%c=%ld  disksize_%c=%ld\n",drive,boot,drive,size_newdsk);

}
