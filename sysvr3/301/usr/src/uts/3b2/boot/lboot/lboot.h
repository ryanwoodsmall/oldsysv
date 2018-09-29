/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/lboot.h	10.3"

/*
 * Note -- header files must be included in the following order:
 *
 *	sys/types.h
 *	a.out.h		(if required)
 *	sys/param.h	(if required)
 *	sys/dir.h
 *	lboot.h
 *	sys/param.h	(if not above and io.h is included)
 *	sys/dir.h	(if not above and io.h is included)
 *	sys/filsys.h	(if io.h is included)
 *	sys/inode.h	(if io.h is included)
 *	sys/ino.h	(if io.h is included)
 *	io.h		(if required)
 *	<other headers>
 */


#include <sys/sys3b.h>
#include <sys/edt.h>

#ifdef u3b2
#include <sys/firmware.h>
#include <sys/csr.h>
#include <sys/boot.h>

#undef ON	/* Undef ON so we can have our own ON() */
#endif

/*
 * OFFSET TO BEGINNING OF FILE SYSTEM
 */
#define FSO_OFFSET	100


/*
 * Boolean data type
 */
typedef	unsigned char	boolean;

#define	TRUE	1
#define	FALSE	0


/*
 * Data type for address arithmetic
 */
typedef	unsigned long	address;


/*
 * Location counter
 */
typedef	struct	{
		address	v_locctr;	/* virtual location counter */
		address	v_origin;	/* virtual origin */
		address	p_origin;	/* real origin */
		}
		LOCCTR;

#define	REAL(vaddr,locctr)	((vaddr)-locctr.v_origin+locctr.p_origin)

/*
 * Symbol table entries
 */
#ifdef SYMENT
typedef	struct	{
		char			*name;			/* symbol name */
		address			value;			/* value of symbol */
		unsigned long		size;			/* sizeof symbol */
		unsigned char		flag;
		}
		SYMBOL;


#define EXTERN		0x80	/* symbol must be DEFINED by lboot */
#define STATIC		0x40	/* symbol is declared STATIC;
				 * it will not be found by Ksym_name()
				 * or Xsym_name() */
#define	BSS		0x20	/* symbol is defined in BSS section */
#define PASSTHRU	0x10	/* This symbol is only here to pass
				 * thru to the sys3b symbol table.
				 * It should not be used for and
				 * relocation.			  */
#define	DEFER		0x08	/* do not attempt to find a routine
				 * name to resolve this symbol
				 */
#define	NOINIT		0x04	/* driver data structure is not to be
				 * initialized */
#define	DRIVER		0x02	/* symbol is defined within a driver */
#define	DEFINED		0x01	/* symbol is defined */
#endif


/*
 * ON Condition
 */
typedef	struct	{
		char	name[9];	/* condition name */
		}
		CONDITION;

#define ON(condition)		if (setjmp(on(&condition)))
#define REVERT(condition)	revert(&condition)
#define SIGNAL(condition)	signal(&condition)

/*
 * Error table entry
 */
struct	errortab
	{
	short	msgnum;		/* the message number from error.h */
	short	action;		/* the action to take */
	char	*text;		/* the actual message text */
	};

#define _PANIC_	 	0x8000		/* Flags:  panic() */
#define _SILENT_	0x4000		/*         do not print message text */
#define _PERROR_	0x2000		/*         perror() */
					/*       else printf() */

#define _CODE_		0x0FFF		/* Action codes		*/
#define _RETURN_	1		/*	return		*/
#define _RETURNTRUE_	2		/*	return(TRUE)	*/
#define _RETURNFALSE_	3		/*	return(FALSE)	*/
#define _DYNAMIC_	4095		/*	error_action() will determine ultimate action	*/

/*
 * Global variables
 */
extern int              errno;			/* C library error number */
extern CONDITION	ERROR;			/* universal error condition */
extern CONDITION	IOERROR;		/* read_and_check() or seek_and_check() routine failed */
extern CONDITION	ABORT;			/* load of bootprogram failed */

extern address          virtual0;		/* real address of virtual 0 */
extern LOCCTR           text_locctr;		/* .text */
extern LOCCTR           data_locctr;		/* .data */
extern LOCCTR           bss_locctr;		/* .bss */
extern int              interrupts;		/* number of interrupt routines required */
extern int              IRTNSIZE;		/* size of an interrupt routine */
extern int              driver_text;		/* total size of driver text */
extern int              driver_symbol;		/* maximum number of symbols for any one driver */

extern int		fstypcnt;
extern int		fsincnt;
extern int              cdevcnt;
extern int              bdevcnt;
extern int		fmodcnt;
extern short		nfstyp;
extern struct fstypsw  *fstypswp;
extern struct fsinfo   *fsinfop;
extern struct cdevsw   *cdevswp;		/* real address */
extern struct bdevsw   *bdevswp;		/* real address */
extern struct fmodsw   *fmodswp;
extern char            *MAJOR;			/* real address */
extern char            *MINOR;			/* real address */

#ifdef SYMENT
extern struct	rtname				/* functions defined in the UNIX kernel */
		{
		char   *name;
		SYMBOL *symbol;
		}
			rtname[];
#endif

extern char             etcsystem[];		/* /etc/system path name */
extern struct s3bboot   bootprogram;		/* /unix path name */
extern char             slash_boot[];		/* /boot directory name */
extern dev_t            rootdev;
extern dev_t            pipedev;
extern dev_t            dumpdev;
extern dev_t            swapdev;
extern daddr_t          swplo;
extern int              nswap;


/*
 * Use the IAU functions
 */

#ifdef u3b5
#define	bzero		(*(IAU_FNCTS)->pfi[BZERO])
#define	restart		(*(IAU_FNCTS)->pfi[RESTART])
#define	scanf		(*(IAU_FNCTS)->pfi[SCANF])
#define	strcmp		(*(IAU_FNCTS)->pfi[STRCMP])
#define	strncmp		(*(IAU_FNCTS)->pfi[STRNCMP])
#define	init_uart	(*(IAU_FNCTS)->pfi[INIT_UART])

#define	SCANF_OK	0	/* value returned by scanf() for success */


/*
 * Is the AUTOBOOT switch on?
 */
#define ISAUTO	(BOOT_DEVP->auto_boot == AUTO_BOOT)
#endif

#ifdef u3b2
#define	scanf		GETS
#define	restart() {RST = 1; while (1) ; }

#define	SCANF_OK	1	/* value returned by scanf() for success */

#define EDT_START	P_EDT	/* origin of EDT */

/*
 * Is this an automatic boot of /unix?  (Equivalent to 3B5's AUTOBOOT switch)
 */
#define ISAUTO		(P_CMDQ->b_type == UNIXBOOT)
#endif

/*
 * Stripped-down STDIO C library interface stuff
 */

#ifndef NULL
#define NULL	0
#endif

typedef struct	{
		int		_cnt;
		unsigned char	*_ptr;
		unsigned char	*_base;
		char		_flag;
		char		_file;
		}
		FILE;

#define _IOREAD		0x01		/* _iob[] entry in use */
#define _IOMYBUF	0x02		/* _filbuf() malloc'ed buffer */
#define _IOEOF		0x04
#define _IOERR		0x08

#define EOF		(-1)

#ifndef lint
#define getc(p)		((--((p)->_cnt) >= 0)?\
			((int) *((p)->_ptr)++) : _filbuf(p))
#endif
#define feof(p)		(((p)->_flag & _IOEOF) != 0)
#define ferror(p)	(((p)->_flag & _IOERR) != 0)
#define fileno(p)	p->_file

/*
 * All external functions
 */
extern void            alloc_loadmap();		/* allocate load map array */
extern address         alloc_string();		/* allocate a character string in the data section */
extern address         allocate();		/* allocate memory for a symbol */
extern void            bcopy();			/* copy memory-to-memory */
extern boolean         blankline();		/* is a line entirely blank? */
extern boolean         break_hit();		/* has BREAK been hit on console? */
#ifdef u3b2
extern void            bzero();			/* set memory to zeros */
#endif
#ifdef SYMENT
extern SCNHDR         *coff_section();		/* read common object file section headers */
extern SYMENT         *coff_symbol();		/* read common object file symbol table entries */
extern SYMBOL         *define();		/* define the value of a symbol */
#endif
#ifdef DIRSIZ
extern struct direct  *directory();		/* read directory entries */
#endif
extern void            Drelocate();		/* do relocation for a driver object file */
extern void            edtscan();		/* scan all EDT's in the system */
extern int             error();			/* error handling routine */
extern long            eval();			/* evaluate a prefix polish expression */
extern void            exclude();		/* mark a driver to be excluded */
extern void            findsystem();		/* get the path name of the bootprogram or system file */
extern void            free();			/* return malloc'ed memory to free status; see malloc(3C) */
extern boolean         fsinit();		/* initialize the UNIX file system */
extern void            fsystem();		/* process system file from opened FILE */
extern void            generate();		/* define, allocate and initialize data structures */
extern void            ignore();		/* remember EDT entries to be ignored */
extern void            include();		/* mark a driver to be included */
extern char           *itoa();			/* convert integer to ASCII string */
extern boolean         Kload();			/* load a section from the bootprogram object file */
#ifdef SYMENT
extern SYMBOL         *Ksym_name();		/* lookup bootprogram symbol by name */
extern SYMBOL         *Ksym_number();		/* lookup bootprogram symbol by number */
#endif
extern void            Ksymread();		/* read and initialize bootprogram symbol table */
extern char	      *lcase();			/* convert uppercase string to lower case */
extern void            loadunix();		/* do actual self-configuration */
extern void            longjmp();		/* non-local goto; see setjmp(3C) */
extern void            lreset();		/* reset local variables in loadunix.c */
extern void            ls();			/* a clone of the ls(1) command */
extern void            make_loadmap();		/* add an entry to the load map array */
extern char           *malloc();		/* storage allocator; see malloc(3C) */
extern long            max();			/* return maximum of two values */
extern long            min();			/* return minimum of two values */
extern int            *on();			/* enable an ON condition handler */
extern void            print_loadmap();		/* print the completed load map array */
extern void            printf();		/* call firmware printf() routine */
extern void            read_and_check();	/* read(2) system call with error checking */
extern void            revert();		/* disable an ON condition handler */
extern boolean         routine();		/* search driver boothdr's for `routine' names */
extern void            seek_and_check();	/* lseek(2) system call with error checking */
extern int             setjmp();		/* non-local goto; see setjmp(3C) */
extern void            signal();		/* invoke an ON condition handler */
extern void            sreset();		/* reset local variables in subr.c */
extern char           *strcat();		/* character string concatenate; see string(3C) */
#ifdef u3b2
extern int             strcmp();		/* character string compare; see string (3C) */
#endif
extern char           *strcpy();		/* character string copy; see string(3C) */
extern int             strcspn();		/* character string search; see string(3C) */
extern int             strlen();		/* compute character string length; see string(3C) */
extern char           *strncat();		/* character string concatenate; see string(3C) */
#ifdef u3b2
extern int             strncmp();		/* character string compare; see string (3C) */
#endif
extern char           *strncpy();		/* character string copy; see string(3C) */
extern char           *strpbrk();		/* character string search; see string(3C) */
extern int             strspn();		/* character string search; see string(3C) */
extern char           *strtok();		/* character string parse; see string(3C) */
extern long            strtol();		/* convert ASCII string to integer */
extern void            system();		/* process system file */
extern void            treset();		/* reset local variables in tables.c */
extern address         umax();			/* return maximum of two unsigned values */
extern address         umin();			/* return minimum of two unsigned values */
extern void            Xrelocate();		/* do relocation for external symbol file */
extern void            Xsym_copy();		/* copy external symbol table to kernel data for sys3b(2) system call */
#ifdef SYMENT
extern SYMBOL         *Xsym_name();		/* search external symbol table by name */
#endif
extern void            Xsym_resolve();		/* resolve all undefined symbols in external symbol table */
extern void            Xsym_walk();		/* do an in-order traversal of the external symbol table */

extern int             chdir();			/* chdir(2) system call */
extern void            close();			/* close(2) system call */
extern void            dfcread();		/* read routine for IDFC */
extern boolean         dfinit();		/* initialize IDFC */
extern char           *filename();		/* system call; return path name associated with an opened file descriptor */
extern long            lseek();			/* lseek(2) system call */
extern int             open();			/* open(2) system call */
extern int             read();			/* read(2) system call */
extern int             stat();			/* stat(2) system call */

extern int             fdclose();		/* close a buffered stream */
extern FILE           *fdopen();		/* open a buffered stream */
extern char	      *fgets();			/* read a line from a buffered stream */
extern int            _filbuf();		/* fill a buffer for a stream */
extern int             fread();			/* read a buffered stream */
extern int             fseek();			/* reposition a file pointer for a buffered stream */


/*
 * Generate() routine commands
 *
 *			         WHAT        #ARGS
 *			   ----------------  -----  ------------ARGUMENTS------------- */
#define	G_TEXT	0	/*             text     3   char* name, long size, char* init  */
#define	G_DATA	1	/* initialized data     3   char* name, long size, char* init  */
#define	G_UDATA	2	/*     uninit. data     2   char* name, long size              */
#define	G_BSS	3	/*              bss     2   char* name, long size              */
#define	G_PCB	4	/*              pcb     4   char* name, char* entry, ipl, vector#  */
#define	G_IRTN	5	/* interupt routine     3   char* name, char* entry, minor     */
#define	G_IOSYS	6	/*    I/O subsystem     0                                      */

/*
 * Debugging options
 *
 * TEST must be defined to enable any debugging features.  The preprocessor
 * names which can be used are listed below.  Note the hiarchy of names; for
 * example, DEBUG1 must be defined in order for DEBUG1a to have any effect.
 *
 *	TEST:		PROGRAM(name), panic, time, CC console
 *	  DEBUG1:	prt[], shell commands time,ls,exit,panic,help,symbol,system,stat
 *	    DEBUG1a:	shell commands Xprint,Xdepth
 *	    DEBUG1b:	stack height monitoring, shell command stack
 *	    DEBUG1c:	shell command cat
 *	    DEBUG1d:	shell commands cd,pwd
 *	    DEBUG1e:	shell command driver
 *	    DEBUG1f:	collect system activity statistics, shell command sar
 *	    DEBUG1g:	shell command file
 *	    DEBUG1h:	shell commands malloc,free
 *	    DEBUG1i:	shell command memory
 *	  DEBUG2:	malloc integrity checks
 *
 *
 * The following table show the dependencies of the source files on each
 * preprocessor define.  For example, if DEBUG1c is changed, then
 * main.c must be recompiled; since DEBUG1 is the only define in dfcstand.c,
 * then dfcstand.c may be compiled without TEST at all to save space.
 *
 *	basicio.c:	DEBUG1 DEBUG1b DEBUG1f DEBUG1g
 *	clibrary.c:	DEBUG1 DEBUG1b DEBUG1i DEBUG2
 *	dfcstand.c:	DEBUG1
 *	io.h:		DEBUG1f
 *	lboot.h:	DEBUG1 DEBUG1a DEBUG1b DEBUG1c DEBUG1d DEBUG1e
 *			DEBUG1f DEBUG1g DEBUG1h DEBUG1i DEBUG2
 *	loadunix.c:	DEBUG1 DEBUG1b DEBUG1e
 *	main.c:		DEBUG1 DEBUG1a DEBUG1b DEBUG1c DEBUG1d DEBUG1e
 *			DEBUG1f DEBUG1g DEBUG1h DEBUG1i DEBUG2
 *	on.c:		DEBUG1 DEBUG1b
 *	subr.c:		DEBUG1 DEBUG1b
 *	tables.c:	DEBUG1 DEBUG1a DEBUG1b
 */

#if !TEST

#undef DEBUG1
#undef DEBUG1a
#undef DEBUG1b
#undef DEBUG1c
#undef DEBUG1d
#undef DEBUG1e
#undef DEBUG1f
#undef DEBUG1g
#undef DEBUG1h
#undef DEBUG1i
#undef DEBUG2

#else	TEST is defined

#if !DEBUG1
#undef DEBUG1a
#undef DEBUG1b
#undef DEBUG1c
#undef DEBUG1d
#undef DEBUG1e
#undef DEBUG1f
#undef DEBUG1g
#undef DEBUG1h
#undef DEBUG1i
#endif

#endif



#if TEST
/*
 * Declare a pointer to the function name as the first word on the stack frame
 */

#if DEBUG1b
extern char *_spmonitor();
#define	PROGRAM(name)	char *_0fp = _spmonitor("name");
#else
#define	PROGRAM(name)	char *_0fp = "name";
#endif

#if DEBUG1
/*
 * Setup for the prt[] switches
 */
extern boolean prt[];

#define	_BASICIO	0
#define	_CLIBRARY	1
#define	_DFCSTAND	2
#define	_LOADUNIX	3
#define	_MAIN		4
#define	_ON		5
#define	_SUBR		6
#define	_TABLES		7
#define	_MAXFILES	8	/* total number of source files */
#endif

#else	!TEST
#define	PROGRAM(name)
#endif
