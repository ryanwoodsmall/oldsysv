/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkunix:mkunix.c	1.3"
/*
 *		Copyright 1984 AT&T
 */

#include	<stdio.h>
#include	<a.out.h>

/*
 *  Undo inconsistency between <a.out.h> and <nlist.h>
 */
#undef n_name
#define syment_name _n._n_name

#include	<fcntl.h>
#include	<ldfcn.h> 
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/sys3b.h>

/*
 * structure filled by loadsym() for a symbol in the sys3b(2) symbol table
 */
struct	s3bs
	{
	char	*name;	/* -> symbol name */
	long	value;	/* symbol's value */
	}
	*symindex;

#define	TRUE		1
#define	FALSE		0

#ifdef u3b5
#define PSTARTADR	0x800000
#endif
#ifdef u3b2
#define PSTARTADR	0x2000000
#endif

struct	nlist	nl[] = {
	{"crashsw"},
	{"sboot"},
	{"stext"},
	{"sdata"},
	{"sbss"},
	{"bootsize"},
	{0},
};

#define	CRASHSW		0	/* indexes to special symbols */
#define	SBOOT		1
#define	STEXT		2
#define	SDATA		3
#define	SBSS		4
#define	BOOTSIZE	5
#define	LAST_SYM	5

extern char	*ctime();
extern void	exit();
extern void	free();
extern int	getopt();
extern int	ldclose();
extern int	ldfhread();
extern char	*ldgetname();
extern LDFILE	*ldopen();
extern int	ldshread();
extern long	lseek();
extern char	*malloc();
extern int	nlist();
extern void	perror();
extern char	*strcat();
extern char	*strcpy();
extern int	strlen();
extern char	*strncpy();
extern int	sys3b();
extern long	time();

void		addsect();
void		copysects();
void		copysyms();
void		dumpshdr();
void		endjob();
void		error();
void		exit_msg();
struct s3bs	*findsym();
void		loadconfig();
void		loadsyms();

char	*cmdname;
char	*if_name = NULL;
char	*of_name = NULL;
char	namebuf[sizeof(struct s3bboot)] = { '/', '\0' };

LDFILE	*ldptr = NULL;
FILE	*outfile = NULL;

#define	NSCNS		6	/* number of output sections */

#define	GATE_SECTION	1	/* output section numbers */
#define	TEXT_SECTION	2
#define	DATA_SECTION	3
#define	BSS_SECTION	4
#define	EDT_SECTION	5
#define	BOOT_SECTION	6

SCNHDR	section[NSCNS];  /*  section headers for ABSOLUTE file  */

FILHDR	filehead;
int	scn_gate, scn_text, scn_data, scn_bss, scn_bott, scn_botd;
struct s3bsym	*sbase;
struct s3bconf	*cbase;

int	debug = 0;

main(argc,argv)
int	argc;
char	*argv[];
{
	extern	char	*optarg;
	extern	int	optind;

	int	i;
	int	c;
	long	sectionseek, symseek;
	char	*opthdr;
	long	zero	=	0;
	SCNHDR	sectionhead;
	struct	stat	if_stat, of_stat;
	struct	s3bboot	unix_name;


	cmdname = argv[0];
	
	while ((c = getopt(argc, argv, "di:o:")) != EOF)
		switch (c) {

		case 'd':		/* debug flag */
			++debug;
			break;

		case 'i':		/* original symbol table file */
			if (if_name == NULL)
				if_name = optarg;
			else
				exit_msg("Multiple input file names specified");
			break;

		case 'o':		/* output symbol table file */
			if (of_name == NULL)
				of_name = optarg;
			else
				{
				of_name = NULL;
				exit_msg("Multiple output file names specified");
				}
			break;
		default:		/* illegal option letter */
			fprintf(stderr,"Usage: %s [-o a.out] [[-i] bootprogram]\n",cmdname);
			exit(1);
		}

	/* if any arguments remain, interpret as an input file name */
	if (optind < argc) {
		if (if_name == NULL)
			if_name = argv[optind];
		else
			{
			of_name = NULL;
			exit_msg("Multiple input file names specified");
			}
	}

	/* Get the actual boot program name */

	if (sys3b(S3BBOOT,&unix_name) != 0)
		{
		of_name = NULL;
		exit_msg("Cannot get boot program name");
		}

	/* Check if default names needed */

	if (if_name == NULL) {
		/* use the boot program file name */
		if ( unix_name.path[0] == '/' )
			if_name = strcpy(namebuf,unix_name.path);
		else
			if_name = strcat(namebuf,unix_name.path);
		printf("Using %s for input file\n", if_name);
	}

	if (of_name == NULL)
		of_name = "a.out";

	if (debug > 0)
		fprintf(stderr, "if_name=\"%s\", of_name=\"%s\"\n", if_name, of_name);

	/*  Verify files used.  */

	if (stat(if_name, &if_stat) != 0)
		{
		of_name = NULL;
		exit_msg("Cannot stat input file");
		}
	if (stat(of_name, &of_stat) == 0 && if_stat.st_ino == of_stat.st_ino)
		{
		of_name = NULL;
		exit_msg("Input file and output file have same inode numbers");
		}

	/*
	 *	open source and target files
	 */
	if ((ldptr = ldopen(if_name, ldptr)) == NULL) {
		of_name = NULL;
		exit_msg("Cannot open %s\n",if_name);
	}
	if ((outfile = fopen(of_name, "w")) == NULL) {
		exit_msg("Cannot open %s\n",of_name);
	}

	if (debug > 0)
		fprintf(stderr, "All files opened\n");

	/*	copy file header and check magic number */
	if (ldfhread(ldptr, &filehead) == FAILURE)
		exit_msg("Cannot read file header");
	if ( filehead.f_magic != FBOMAGIC )
		exit_msg("Bad magic on input file");

	if (debug > 0)
		fprintf(stderr,"magic number okay\n");

	if (filehead.f_timdat != unix_name.timestamp)
		{
		error("Warning -- boot program \"%s\" timestamp= %s", unix_name.path, ctime(&unix_name.timestamp) );
		error("           input file   \"%s\" timestamp= %s", if_name, ctime(&filehead.f_timdat) );
		error("           *** Results are unpredictable! ***" );
		}

	/*
	 *	save space in target file for file header after it is updated
	 */
	if (fseek(outfile,(long)FILHSZ,0) != 0)
		exit_msg("Bad seek on file:  %s\n",of_name);

	/*
	 *	copy optional header from source to target file,
	 */
	if (filehead.f_opthdr != 0) {
		if ((opthdr = malloc((unsigned int)filehead.f_opthdr)) == NULL)
			exit_msg("Not enough memory for optional header");

		if (FREAD(opthdr,(int)filehead.f_opthdr,1,ldptr) != 1)
			exit_msg("Cannot read optional header for file: %s\n",if_name);
		/*  On 3b2 there is a problem with the size of the
		 *  the optional header when booting from floppy
		 *  using the old lboot.  This should go away in 
		 *  1I3.1
		 */
#ifdef u3b2
		if (fwrite(opthdr,(int)sizeof(AOUTHDR),1,outfile) != 1)
			exit_msg("Cannot write optional header for file:  %s\n",of_name);
#else
		if (fwrite(opthdr,(int)filehead.f_opthdr,1,outfile) 1= 1)
			exit_msg("Cannot write optional header for file:  $s\n",of_name);
#endif
		free(opthdr);
	}

	if (debug > 0)
		fprintf(stderr, "Optional header (size 0x%x) copied\n", filehead.f_opthdr );

	/*
	 * get original section indexes
	 */
	for( i=1; i<=filehead.f_nscns; ++i )
		{
		if (ldshread(ldptr,(unsigned short)i,&sectionhead) == FAILURE)
			exit_msg("Cannot read section header for file: %s\n", if_name);
		if ( 0 == strcmp(".gate",sectionhead.s_name) )
			scn_gate = i;
		if ( 0 == strcmp("boott",sectionhead.s_name) )
			scn_bott = i;
		if ( 0 == strcmp("bootd",sectionhead.s_name) )
			scn_botd = i;
		if ( 0 == strcmp(_TEXT,sectionhead.s_name) )
			scn_text = i;
		if ( 0 == strcmp(_DATA,sectionhead.s_name) )
			scn_data = i;
		if ( 0 == strcmp(_BSS,sectionhead.s_name) )
			scn_bss = i;
		}
	if ( scn_gate == 0 )
		exit_msg("Section \".gate\" not found");
	if ( scn_bott == 0 )
		exit_msg("Section \"boott\" not found");
	if ( scn_botd == 0 )
		exit_msg("Section \"bootd\" not found");
	if ( scn_text == 0 )
		exit_msg("Section \"%s\" not found",_TEXT);
	if ( scn_data == 0 )
		exit_msg("Section \"%s\" not found",_DATA);
	if ( scn_bss == 0 )
		exit_msg("Section \"%s\" not found",_BSS);

	/*
	 *	save space for section headers in new file
	 */

	sectionseek = ftell(outfile);

	if (fseek(outfile,(long)(NSCNS * SCNHSZ),1) != 0)
		exit_msg("Cannot write section headers");
	
	if (nlist(if_name, nl) != 0)
		exit_msg("Cannot get special symbols from input file");

	for (i = 0; i <= LAST_SYM; ++i) {
		if (debug > 0)
			fprintf(stderr,"symbol=\"%s\", type=%x, value=%x\n",
				nl[i].n_name, nl[i].n_type, nl[i].n_value);
		if (nl[i].n_value == 0)
			exit_msg("Cannot locate special symbol \"%s\"\n", nl[i].n_name);
	}
	
	/*
	 * get the symbol table and the configuration table built by lboot
	 */
	loadsyms();
	loadconfig();

	/*
	 * write the sections in the output file
	 */
	copysects();

	/*
	 * build the symbol table in the output file
	 */
	symseek = ftell(outfile);

	copysyms();

	ldclose(ldptr);
	ldptr = NULL;

	/* round output file size to word boundry */
	while( ftell(outfile) % sizeof(int) )
		putc( 0, outfile );

	/*
	 * update file header
	 */
	filehead.f_nscns = NSCNS;
	filehead.f_flags |= F_EXEC | F_RELFLG | F_LNNO;
	filehead.f_timdat = time((long *)0);
	filehead.f_symptr = symseek;
	/*  following should go away in 1I3.1 */
	filehead.f_opthdr = sizeof(AOUTHDR);

	if (fseek(outfile,0L,0) != 0)
		exit_msg("Cannot seek to file header in %s\n",of_name);
	if (fwrite((char *)&filehead,FILHSZ,1,outfile) != 1)
		exit_msg("Cannot write file header in %s\n",of_name);

	/* update section headers */
	if (fseek(outfile,sectionseek,0) != 0)
		exit_msg("Cannot seek to section headers in %s\n",of_name);
	if (fwrite((char *)section,NSCNS * SCNHSZ,1,outfile) != 1)
		exit_msg("Cannot write section headers in %s\n",of_name);

	/* update the special variables */
	if (fseek(outfile, section[BOOT_SECTION-1].s_scnptr, 0) != 0)
		error("Could not seek to start of boot section\n");
	else {
		if (debug > 0) {
			fprintf(stderr,"Updating crash word: boot section offset is %x, crashsw offset is %x\n",
				section[BOOT_SECTION-1].s_scnptr, nl[CRASHSW].n_value - nl[SBOOT].n_value);
			fprintf(stderr,"File at %x\n", ftell(outfile));
		}
		if (fseek(outfile, nl[CRASHSW].n_value - nl[SBOOT].n_value, 1) != 0) {
			error("Could not seek to update \"crashsw\"\n");
			goto closeout;
		}
		if (debug > 0)
			fprintf(stderr,"File at %x\n", ftell(outfile));
		if (fwrite((char *)&zero, sizeof(zero), 1, outfile) != 1) {
			error( "Could not write 0 to \"crashsw\"\n");
			goto closeout;
		}
		if (debug > 0)
			fprintf(stderr,"File at %x\n", ftell(outfile));
	}
closeout:
	fclose(outfile);

	/* make the new file executable */
	if (chmod(of_name,0744) != 0)
		error("Cannot change mode of %s\n", of_name);

	return( 0 );
}

/*
 *	findsym(name) - return a pointer to the incore symbol entry for 'name'
 *		return NULL if not found.
 */

struct s3bs *
findsym(name)
char	*name;
{
	register struct s3bs *s3bs;
	register i;

	for( i=0, s3bs=symindex; i<sbase->count; ++i, ++s3bs )
		if ( 0 == strcmp(s3bs->name,name) )
			return( s3bs );

	return((struct s3bs *) NULL);
}

/*
 *	loadsyms() - load the incore symbol table and build the symindex[] for
 *		later use by copysect and copysyms functions
 */

void
loadsyms()
{
	unsigned int	size;
	int		count;
	char		*p;
	struct s3bs	*sp;

	/* get length of incore symbol table */
	sys3b(S3BSYM,&size,sizeof(size));

	if ( ((sbase = (struct s3bsym *)malloc(size)) == NULL) )
		exit_msg("Not enough memory to acquire incore symbol table");

	/*  acquire incore symbol table  */
	sys3b(S3BSYM,sbase,size);

	/* build symindex[] */
	if ( (symindex = (struct s3bs *)malloc(sbase->count*sizeof(struct s3bs))) == NULL )
		exit_msg("Not enough memory for incore symbol table index");

	for( count = 0, p = sbase->symbol, sp = symindex;
		count < sbase->count;
		++count, p += (strlen(p)+1+sizeof(long) + sizeof(long)-1) & ~(sizeof(long)-1), ++sp )
		{
		sp->name = p;
		sp->value = * (long*) (p + ((strlen(p)+1 + sizeof(long)-1) & ~(sizeof(long)-1)));
		}
}

/*
 *	loadconfig() - load the configuration table for later use by copysect
 *		function
 */

void
loadconfig()
{
	extern	struct s3bconf	*cbase;

	int		num_entries;
	unsigned	tab_size;

	sys3b(S3BCONF,&num_entries,sizeof(num_entries));	/* get length of configuration table */

	/*  set tab_size to the size required for the configuration table  */
	tab_size = ((num_entries-1) * sizeof(struct s3bc) + sizeof(struct s3bconf));

	if ( (cbase = (struct s3bconf*)malloc(tab_size)) == NULL)
		exit_msg("Not enough memory for configuration table");

	/*  acquire incore symbol table  */
	sys3b(S3BCONF,cbase,tab_size);
}

/*
 *	copysects() - copy section data from the file and from memory into
 *		the result file.
 */

void
copysects()
{
	extern	FILE	*outfile;
	extern	SCNHDR	section[NSCNS];

	unsigned	pstart;
	unsigned	vstart;
	unsigned	size;
	struct s3bs	*scptr;


	/* .gate */
	vstart = 0;
	if ((scptr = findsym("Sgate")) != NULL)
		pstart = scptr->value;
	else
		exit_msg("Cannot find start of gate section");
	if ((scptr = findsym("gateSIZE")) != NULL)
		size = scptr->value;
	else
		exit_msg("Cannot find size of gate section");
	/* load the gate table to the file */
	addsect(&section[GATE_SECTION-1],".gate",pstart,vstart,size, STYP_REG | STYP_DATA,TRUE);

	/* .text */
	vstart = nl[STEXT].n_value;
	if ((scptr = findsym("Stext")) != NULL)
		pstart = scptr->value;
	else
		exit_msg("Cannot find start of text section");
	if ((scptr = findsym("textSIZE")) != NULL)
		size = scptr->value;
	else
		exit_msg("Cannot find size of text section");
	addsect(&section[TEXT_SECTION-1],_TEXT,pstart,vstart,size, STYP_REG | STYP_TEXT,TRUE);

	/* .data */
	vstart = nl[SDATA].n_value;
	if ((scptr = findsym("Sdata")) != NULL)
		pstart = scptr->value;
	else
		exit_msg("Cannot find start of data section");
	if ((scptr = findsym("dataSIZE")) != NULL)
		size = scptr->value;
	else
		exit_msg("Cannot find size of data section");
	addsect(&section[DATA_SECTION-1],_DATA,pstart,vstart,size, STYP_REG | STYP_DATA, TRUE);

	/* .bss */
	vstart = nl[SBSS].n_value;
	if ((scptr = findsym("Sbss")) != NULL)
		pstart = scptr->value;
	else
		exit_msg("Cannot find start of bss section");
	if ((scptr = findsym("bssSIZE")) != NULL)
		size = scptr->value;
	else
		exit_msg("Cannot find size of bss section");
	addsect(&section[BSS_SECTION-1],_BSS,pstart, vstart,size, STYP_NOLOAD | STYP_BSS, FALSE);

	/* <EDT> */
	addsect(&section[EDT_SECTION-1],"<EDT>",(unsigned)0, (unsigned)cbase, sizeof(struct s3bconf) + (cbase->count-1)*sizeof(struct s3bc), STYP_DSECT, FALSE);

	/* boot */
	pstart = nl[SBOOT].n_value;
	vstart = pstart;
	size = nl[BOOTSIZE].n_value;
	addsect(&section[BOOT_SECTION-1],"boot",pstart,vstart,size,STYP_REG,TRUE);
}

/*
 *	addsect(scnptr, name, pstart, vstart, size, flags, copyflag) - add the section
 *		"name" to the file with the section flags "flags"
 *		if copyflag == TRUE, then read the section from physical memory
 *		else don't read the section, just update the section pointers
 *		(a special case is <EDT>; pstart==0, vstart==&s3bconf -- copy the
 *		configuration table)
 */

void
addsect( scnptr, name, pstart, vstart, size, flags, copyflag)
SCNHDR		*scnptr;
char		*name;
unsigned	pstart;
unsigned	vstart;
unsigned	size;
unsigned	flags;
int		copyflag;
{

	unsigned left, msize;
	int	retval;
	int	mem;
	char	*buf;

	if (debug > 0)
		fprintf(stderr,"addsect( section[%d]: %s, ps=%x, vs=%x, sz=%x, flags=%x, copy=%x) called\n",
			scnptr-section, name, pstart, vstart, size, flags, copyflag);
	
	if (copyflag == TRUE) {
		scnptr->s_scnptr = ftell(outfile);

		/*
		 * copy contents of section from memory
		 */
		left = msize = size;
		while (msize != 0 && (buf = malloc(msize)) == NULL)
			msize /= 2;
		if (msize == 0)
			exit_msg( "Cannot get memory for reading \"/dev/mainstore\"");

		if (debug > 0)
			fprintf(stderr,"Memory for copy available in %x byte buffer\n",msize);
		if ((mem = open("/dev/mainstore", O_RDONLY)) == -1)
			exit_msg("Cannot open \"/dev/mainstore\" for reading");
		if (debug > 0)
			fprintf(stderr,"/dev/mainstore file opened for reading\n");

		if (lseek(mem,(long)((pstart>=PSTARTADR)?pstart-PSTARTADR:pstart),0) == -1) {
			close(mem);
			exit_msg("Cannot seek on \"/dev/mainstore\"");
		}
		if (debug > 0) {
			fprintf(stderr,"Seek to %lx in memory complete\n", (long)((pstart>=PSTARTADR)?pstart-PSTARTADR:pstart));
			fprintf(stderr,"buffer size used is %x at loc %x\n", msize, buf);
		}
		while (left != 0) {
			if (debug > 0)
				fprintf(stderr,"buffer size is %x, left to go %x\n", msize, left);

			if (msize > left)
				msize = left;

			/* read msize bytes from memory, write to outfile */
			if ((retval = read(mem, buf, msize)) != msize) {
				fprintf(stderr,"Return code from read is %d\n",retval);
				perror("Reading from mem");
				close(mem);
				exit_msg("Cannot read from \"/dev/mainstore\"");
			}
			if (fwrite(buf, (int)msize, 1, outfile) != 1) {
				close(mem);
				exit_msg("Cannot write to output file");
			}
			left -= msize;
		}

		free(buf);
		close(mem);
	}
	else
		/*
		 * copyflag == FALSE
		 */
		if (pstart==0 && flags==STYP_DSECT) {
			/*
			 * <EDT> section
			 */
			scnptr->s_scnptr = ftell(outfile);

			if ( fwrite((char*)vstart, (int)size, 1, outfile) != 1 )
				exit_msg("Cannot write to output file");

			vstart = 0;
		}
		else
			scnptr->s_scnptr = 0;


	/*
	 * update the rest of the section header
	 */

	strncpy(scnptr->s_name, name, sizeof(scnptr->s_name));
	scnptr->s_paddr = pstart;
	scnptr->s_vaddr = vstart;
	scnptr->s_size = size;
	scnptr->s_relptr = 0;
	scnptr->s_lnnoptr = 0;
	scnptr->s_nreloc = 0;
	scnptr->s_nlnno = 0;
	scnptr->s_flags = flags;

	if (debug > 0)
		dumpshdr(scnptr);
}

/*
 *	dumpshdr(scnptr) - dump the section header
 */

void
dumpshdr(scnptr)
SCNHDR	*scnptr;
{
	fprintf(stderr,"Section header dump for section #%d", scnptr-section);
	fprintf(stderr,"    name=%s\n",scnptr->s_name);
	fprintf(stderr,"    paddr=%lx",scnptr->s_paddr);
	fprintf(stderr,"    vaddr=%lx",scnptr->s_vaddr);
	fprintf(stderr,"    size=%lx",scnptr->s_size);
	fprintf(stderr,"    scnptr=%lx\n",scnptr->s_scnptr);
	fprintf(stderr,"    relptr=%lx",scnptr->s_relptr);
	fprintf(stderr,"    lnnoptr=%lx",scnptr->s_lnnoptr);
	fprintf(stderr,"    nreloc=%x",scnptr->s_nreloc);
	fprintf(stderr,"    nlno=%x",scnptr->s_nlnno);
	fprintf(stderr,"    flags=%lx\n",scnptr->s_flags);
}

/*
 *	copysyms() - merge the file symbol table and the self-config symbol
 *		table from memory and write to the result file
 */

void
copysyms()
{
	extern	FILE	*outfile;

	SYMENT	symbol;
	int	n_aux;
	int	i;
	char	*old_base;
	int	old_num;
	int	old_len;
	char	*new_base;
	int	new_num;
	int	new_len;
	char	*name;
	int	total_size;
	struct s3bs	*scptr;
	long	Sdata;	/* virtual address */
	long	Sbss;	/* virtual address */

	/*
	 *	get start of data and bss sections for later use 
	 */
	Sdata = nl[SDATA].n_value;
	Sbss = nl[SBSS].n_value;

	/*
	 *	create merged symbol table:
	 *	1. read a symbol from symbol table in original file
	 *	2. check for update from incore symbol table
	 *	3. write (updated) symbol to new file
	 *	4. copy auxiliary entries if present
	 */

	if (FSEEK(ldptr,filehead.f_symptr,0) != 0)
		exit_msg("Cannot seek to symbol table in %s\n",if_name);

	old_num = -1;
	while (++old_num < filehead.f_nsyms) {
 		if (FREAD(&symbol,SYMESZ,1,ldptr) != 1)
			exit_msg("Cannot read symbol table entry from input file");

		/*
		if(symbol.n_sclass != C_STAT	&&
		   symbol.n_sclass != C_USTATIC	&&
		   symbol.n_sclass != C_TPDEF	&&
		   symbol.n_sclass != C_MOS	&&
		   symbol.n_sclass != C_MOU	){
		*/
		if(symbol.n_sclass == C_EXT){
			if ((name = ldgetname(ldptr, &symbol)) == NULL)
				exit_msg("Cannot read symbol name from input file");

			if ((scptr = findsym(name)) != NULL) {
				/*  establish new value field  */
				symbol.n_value = scptr->value;
				/* mark name as already used  */
				scptr->name = "";
			}
		}

		/* update section pointers */
		if (symbol.n_scnum == scn_gate) symbol.n_scnum = GATE_SECTION;
		else if (symbol.n_scnum == scn_botd) symbol.n_scnum = BOOT_SECTION;
		else if (symbol.n_scnum == scn_bott) symbol.n_scnum = BOOT_SECTION;
		else if (symbol.n_scnum == scn_text) symbol.n_scnum = TEXT_SECTION;
		else if (symbol.n_scnum == scn_data) symbol.n_scnum = DATA_SECTION;
		else if (symbol.n_scnum == scn_bss) symbol.n_scnum = BSS_SECTION;
		else if (symbol.n_scnum > 0) symbol.n_scnum = N_ABS;

		if ( symbol.n_scnum == 0 )
			if (symbol.n_value < Sbss) {
				if (symbol.n_value < Sdata)
					symbol.n_scnum = TEXT_SECTION;
				else
					symbol.n_scnum = DATA_SECTION;
				}
			else
				symbol.n_scnum = BSS_SECTION;

		if (fwrite(&symbol,SYMESZ,1,outfile) != 1)
			exit_msg("Cannot write symbol table entry in %s\n",of_name);

		old_num += (n_aux = symbol.n_numaux);

		while (n_aux-- > 0) {
			if (FREAD(&symbol,SYMESZ,1,ldptr) != 1)
				exit_msg("Cannot read auxiliary symbol table entry in %s\n",if_name);
			if (fwrite(&symbol,SYMESZ,1,outfile) != 1)
				exit_msg("Cannot write auxiliary symbol table entry in %s\n",of_name);
		}
	}

	/*
	 *	All existing symbol entries from source file
	 *	have been updated with data from the incore symbol table
	 *
	 *	Now add any remaining entries from the incore symbol table
	 *	to the end of the original symbol table and construct the
	 *	portion of the string table reflecting these remaining
	 *	entries.
	 */

	printf("Read %d symbols from %s\n", old_num, if_name);

	if (debug > 0 && old_num != filehead.f_nsyms)
		fprintf(stderr,"Should have been %d\n", filehead.f_nsyms);

	if (FREAD((char*)&old_len, sizeof(old_len), 1, ldptr) != 1)
		exit_msg("Cannot read length of string table from input file");

	new_num = 0;
	new_len = 0;
	if ( (new_base = malloc( sbase->size - sbase->count*sizeof(long) )) == NULL )
		exit_msg("Not enough memory for string table");

	for ( i=0, scptr=symindex; i < sbase->count; ++i, ++scptr) {

		if (strlen(scptr->name) == 0)
			continue;

		if ((strcmp(scptr->name, _BSS)  == 0) ||
		    (strcmp(scptr->name, _DATA) == 0) ||
		    (strcmp(scptr->name, _TEXT) == 0))
			continue;

		/* add this symbol to the symbol table */

		++new_num;

		if (strlen(scptr->name) <= SYMNMLEN)
			strncpy(symbol.syment_name, scptr->name, SYMNMLEN);
		else {
			symbol.n_zeroes = 0;
			symbol.n_offset = old_len + new_len;
			strcpy( new_base+new_len, scptr->name );
			new_len += strlen(scptr->name) + 1;
		}

		symbol.n_value = scptr->value;

		if (symbol.n_value < Sbss) {
			if (symbol.n_value < Sdata)
				symbol.n_scnum = TEXT_SECTION;
			else
				symbol.n_scnum = DATA_SECTION;
		} else
			symbol.n_scnum = BSS_SECTION;

		symbol.n_type = T_INT;
		symbol.n_sclass = C_EXT;
		symbol.n_numaux = 0;

		if (fwrite(&symbol, SYMESZ, 1, outfile) != 1)
			exit_msg( "Cannot write new symbol to %s\n", of_name);
	}

	printf("%d symbols added\n", new_num);

	filehead.f_nsyms += new_num;

	/*
	 *	Construct combined string table and write out to outfile
	 */
	if (debug > 0)
		fprintf(stderr,"Buffer for file string table %d bytes\n",old_len);

	if ((old_base = malloc((unsigned)old_len)) == NULL)
		exit_msg("Not enough memory for string table");

	if (FREAD(old_base, (int)(old_len-sizeof(old_len)), 1, ldptr) != 1)
		exit_msg("Cannot read string table from input file");

	total_size = old_len + new_len;
	if (debug > 0) {
		fprintf(stderr,"New string table size:  %x\n",total_size);
		fprintf(stderr,"Filepos before write:  %x\n",ftell(outfile));
	}

	if (fwrite((char*)&total_size, sizeof(total_size), 1, outfile) != 1)
		exit_msg("Cannot write string table size to %s\n", of_name);

	if (fwrite(old_base, (int)(old_len-sizeof(old_len)), 1, outfile) != 1)
		exit_msg("Cannot write old string table to %s\n", of_name);

	if (fwrite(new_base, new_len, 1, outfile) != 1)
		exit_msg("Cannot write new string table to %s\n", of_name);

	if (debug > 0)
		fprintf(stderr,"Filepos after write:  %x\n",ftell(outfile));

	free(old_base);
	free(new_base);
}

/*
 *	error(string,args...) - fprintf the string on stderr
 */

 /*VARARGS1*/
void
error(string,arg1,arg2,arg3)
char	*string;
int	arg1, arg2, arg3;
{
	char buffer[256];

	sprintf( buffer, string, arg1, arg2, arg3 );

	fprintf(stderr, "%s: %s\n", cmdname, buffer);
}

/*
 *	exit_msg(string,arg1,arg2,arg3) - print the exit string and exit(1)
 */

 /*VARARGS1*/
void
exit_msg(string,arg1,arg2,arg3)
char	*string;
{
	error(string,arg1,arg2,arg3);
	endjob();
}

/*
 *	endjob() - closes the source and target files
 *		before exitting with exit status code of 1
 */

void
endjob()
{
	extern	FILE	*outfile;

	if ( outfile != NULL )
		fclose(outfile);

	if ( of_name != NULL )
		unlink(of_name);

	if ( ldptr != NULL )
		ldclose(ldptr);

	exit(1);
}
