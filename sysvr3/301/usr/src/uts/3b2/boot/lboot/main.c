/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/main.c	10.6"

#include <sys/types.h>
#include <a.out.h>
#include <sys/param.h>
#include <sys/dir.h>
#include "lboot.h"
#include "error.h"

#if TEST
#if DEBUG1f
#include <sys/filsys.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include "io.h"
#endif
#if DEBUG1
#include <ctype.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#endif
#ifdef u3b5
#include <sys/termio.h>
#endif
#endif

#ifdef u3b5
#include <sys/cc.h>
#endif

#ifdef u3b2
#include <sys/sbd.h>
#include <sys/todc.h>
#endif



#if lint
/*
 * These are here to reduce the complaints from lint
 */
void		bcopy(s1,s2,l) char *s1,*s2; unsigned l; { s1=l?s1:s2; }
int		getc(f) FILE* f; { return(f->_cnt); }
void		longjmp(x) int *x; { if (setjmp(x))x=0; }
long		max(x,y) long x,y; { return(x-y); }
long		min(x,y) long x,y; { return(x-y); }
int		setjmp(x) int *x; { return(x-x); }
char		*strcpy(s1,s2) char *s1,*s2; { return(s1==s2?s1:s2); }
int		strlen(s) char *s; { return(s-s); }
unsigned long	umax(x,y) unsigned long x,y; { return(x-y); }
unsigned long	umin(x,y) unsigned long x,y; { return(x-y); }
int		edata[1], end[1];
int		stext[1];
#if TEST
void		splx(x) long x; { x=max(x,x); }
address		getsp() { return(0); }
int		etext[1];
#endif
#endif

/*
 * Magic mode flag; when set by findsystem() after the user typed "magic mode",
 * loadunix() will return to the IAU firmware instead of branching to MAINSTORE
 * At that point, developers can debug the kernel by using DEMON to set
 * breakpoints, etc.
 */
boolean MagicMode = FALSE;

/* Debug mode flag.  When set, it enables functions which are useful
** for debugging the kernel (not lboot).  Currently, it just
** causes a large sys3b symbol table to be created.
*/

boolean DebugMode = FALSE;

/*
 * Quiet mode flag; it will be set TRUE if the BREAK key is hit on the CC console
 * when either the configuration summary or the load map is being printed.  When
 * set, it will suppress further printing of these tables.
 */
boolean QuietMode = FALSE;

/*
 * Break key detection flag; it will be set TRUE if the BREAK key is hit on the
 * console.
 */
static boolean BreakHit = FALSE;

/*
 * memory allocation information
 */
address maxmem;		/* memory size from EDT */
address topboot;	/* highest address used by lboot */

/*
 * Static function declarations for this file
 */
#ifdef u3b5
static void	fix_10_edt();
#endif
#if TEST
static long	time();


/*
 * Record the debug settings
 */

char debug_options[] ={ 'L','B','O','O','T',' ','(','T','E','S','T',
#if DEBUG1
			',','1',
#endif
#if DEBUG1a
			',','1','a',
#endif
#if DEBUG1b
			',','1','b',
#endif
#if DEBUG1c
			',','1','c',
#endif
#if DEBUG1d
			',','1','d',
#endif
#if DEBUG1e
			',','1','e',
#endif
#if DEBUG1f
			',','1','f',
#endif
#if DEBUG1g
			',','1','g',
#endif
#if DEBUG1h
			',','1','h',
#endif
#if DEBUG1i
			',','1','i',
#endif
#if DEBUG2
			',','2',
#endif
			')','\0' };

extern char debug_version[];
#endif


#if DEBUG1
/*
 * Debug printing switch
 */
boolean prt[_MAXFILES];
#endif



/*
 * LBOOT
 *
 * Modus operandi:
 *
 *	main():
 *		zero bss
 *		size memory (maxmem and topboot)
 *		fsinit(): initialization
 *			binit(): buffer cache
 *			inoinit(): inode table and hashing
 *			finit(): file table
 *			dfinit(): IDFC
 *			iinit(): UNIX file system
 *		findsystem(): get either name of etc/system file or name of boot program
 *				(check for magic mode)
 *		loadunix():
 *			if etc/system file name known
 *				findrivers(): read /boot directory and build driver[] array
 *				fsystem(): parse etc/system file
 *			if boot program name not known
 *				system(): prompt for boot program
 *			read boot program file header
 *			if not F_EXEC
 *				alloc_loadmap(): allocate space for the loadmap[] array
 *				absolute(): load sections based upon physical addresses in section headers
 *			else
 *				if etc/system file not known
 *					findrivers(): read /boot directory and build driver[] array
 *					system(): prompt for INCLUDE and EXCLUDE
 *				system(): if necessary, prompt for rootdev, pipedev, ...
 *				flag all drivers to be ignored
 *				flag all drivers to be loaded (follow dependency chain)
 *				compute: number of character and block devices
 *					 number of interrupt routines required
 *					 total size of all text sections of drivers to be loaded
 *					 total number of drivers that are to be loaded
 *				Ksymread(): read boot program symbol table
 *						defined symbols put it Ksymtab[]
 *						external symbols put it *Xsymtab
 *				if not auto-boot
 *					print_configuration(): print the configuration table
 *				alloc_loadmap(): allocate space for the loadmap[] array
 *				relocatable():
 *					load real boot program sections (names do not begin with '.')
 *					load virtual boot program sections except .text and .data
 *						(names beginning with '.')
 *					assign location counters for text and data
 *					build_io_subsys():
 *						allocate MAJOR, MINOR, [cb]devcnt, [cb]devsw
 *						for each driver to be loaded
 *							allocate xx_addr[] array if necessary
 *							set MAJOR[] and MINOR[] entries
 *							generate interrupt routines and PCB's
 *						allocate rootdev, pipedev, ...
 *						get space to build io_init[] and io_start[] arrays
 *					load boot program .text section
 *					load boot program .data section
 *					for each driver to be loaded
 *						loadriver():
 *							set [bc]devsw table entries
 *							allocate driver specific data structures
 *							load driver .text section
 *							load driver .data section
 *							set io_init[] and io_start[] table entries
 *					allocate final io_init[] and io_start[] arrays
 *					Xsym_resolve(): resolve any remaining undefined symbols
 *						routine(): check routine names in unloaded drivers
 *					initdata(): initialize the driver data structures which
 *							require initialization
 *					allocate the sys3b symbol table
 *					Xsym_copy(): copy the *Xsymtab to the sys3b symbol table
 *				if not auto-boot
 *					print_loadmap(): print the load map table
 *			if magic mode
 *				return to IAU
 *			else
 *				call entry(absolute) or entry(relocatable)
 */
main()
	{PROGRAM(main)

	register struct edt *edtp;
	register i;
	extern int edata[], end[];

#if TEST
	(void) time();
#endif

	/*
	 * zero bss
	 */
	bzero(edata, (address)end - (address)edata);

	topboot = (address)end;

#ifdef u3b5
	/*
	 * initialize CC console uart from contents of unit_equip field
	 * of CC EDT entry
	 */
	init_uart();

	/*
	 * scan EDT to size mainstore and to fixup a release 1.0 IAU's EDT
	 */
	edtp = EDT_START->edtx;

	maxmem = 0;
	for (i=0; i<EDT_START->number; ++i, ++edtp)
		{
		if (0 == strcmp("MASC",edtp->dev_name))
			maxmem = edtp->unit_equip + 1;
		else
			if (0 == strcmp("CC",edtp->dev_name) && edtp->version == 0x0100)
				/*
				 * this is a 1.0 3B5; the EDT must be "adjusted"
				 */
				edtscan(EDT_START, 0, fix_10_edt);
		}

	if (maxmem == 0)
		/* MASC not found in EDT ? */
		maxmem = ((address)end + NCPS*NBPC - 1) & ~(NCPS*NBPC - 1);

#endif

#ifdef u3b2
	/*
	 * size mainstore
	 */
	maxmem = SIZOFMEM + MAINSTORE;
#endif

#if TEST
		{
		extern unsigned char Smd;	/* SMD # */
		extern unsigned char Dsk;	/* Disk # on SMD */
		extern long Fso;		/* File system offset */

		register flags;
		register char *p;
		char answer[81];

#ifdef u3b2
		static exception();

		/*
		 * setup exception handler
		 */
		EXC_HAND = exception;
#endif

		elapsed(debug_options);
		printf("Version: %s\n", debug_version);

		flags = 1;
		while (flags > 0)
			{
			switch (flags)
				{
				case 1:
					printf("Spl? ");
					if (scanf(answer) == SCANF_OK)
						if (answer[0] >= '0' && answer[0] <= '9')
							{
							splx(strtol(answer,(char**)NULL,0));
							++flags;
							}
					break;
				case 2:
#if DEBUG1
					printf("Prt? bcdlmost\b\b\b\b\b\b\b\b");
					if (scanf(answer) == SCANF_OK && strspn(answer,"yn") >= _MAXFILES)
						{
						int j;
						for (j=0; j<_MAXFILES; ++j)
							prt[j] = answer[j]=='y';
						++flags;
#ifdef u3b5
						printf("Boot device is IDFC(%d)\n", (int)sgnum((long)BOOT_DEVP->boot_addr));
#endif
#ifdef u3b2
						printf("P_CMDQ->b_dev = 0x%x\n", P_CMDQ->b_dev);
#endif
						}
					break;
				case 3:
#ifdef u3b5
					printf("Boot disk? ");
					if (scanf(answer) == SCANF_OK && (p=strtok(answer,"\r\n\t ")) != NULL)
						{
						i = strtol(p, (char**)NULL, 0);
						Smd = (i<2)? 0 : 1;
						Dsk = i & 0x01;
						++flags;
						}
#endif
#ifdef u3b2
					printf("P_CMDQ->b_dev? ");
					if (scanf(answer) == SCANF_OK)
						{
						if ((p=strtok(answer,"\r\n\t ")) != NULL)
							P_CMDQ->b_dev = strtol(p, (char**)NULL, 0);
						++flags;
						printf("Boot device is %s\n", (P_CMDQ->b_dev == FLOPDISK)? "IF" : "ID");
						printf("P_CMDQ->b_type = 0x%x\n", P_CMDQ->b_type);
						}
#endif
					break;
				case 4:
#ifdef u3b5
					++flags;
#endif
#ifdef u3b2
					printf("P_CMDQ->b_type? ");
					if (scanf(answer) == SCANF_OK)
						{
						if ((p=strtok(answer,"\r\n\t ")) != NULL)
							P_CMDQ->b_type = strtol(p, (char**)NULL, 0);
						++flags;
						}
#endif
					break;
				case 5:
					printf("Boot file system offset? ");
					if (scanf(answer) == SCANF_OK)
						{
						if ((p=strtok(answer,"\r\n\t ")) != NULL)
							Fso = strtol(p, (char**)NULL, 0);
						++flags;
						}
					break;
				case 6:
#endif
					printf("[Hit return to continue] ");
					scanf(answer);
					++flags;
					break;
				default:
					flags = -1;
				}
			}
		}
#endif

	/*
	 * initialize system tables and i/o drivers
	 */
	if (! fsinit())
		/* return to IAU - initialization failed */
		return;

#if TEST
	elapsed("Initialization complete");

#if DEBUG1
	shell();
#endif
#endif

	/*
	 * Loop until a bootable program is loaded and running
	 */
	while (TRUE)
		{
		/*
		 * Get the /etc/system file name
		 */
		findsystem();

		/*
		 * load the boot program
		 */
		loadunix();

		/*
		 * load failed; reset and try again
		 */
		if (chdir("/") == -1)
			panic("cannot chdir(\"/\")");

		errno = 0;
		BreakHit = FALSE;

		lreset();
		sreset();
		treset();
		}
	}

#ifdef u3b5
/*
 * Fix_10_edt()
 *
 * A release 1.0 EDT must be fixed
 *
 *	- the ioa_edti index must be initialized to zero (there can be no IOA's
 *	  on a 1.0 machine)
 */
 static
 void
fix_10_edt(edtp)
	register struct edt *edtp;
	{PROGRAM(fix_10_edt)

	edtp->ioa_edti = 0;
	}
#endif

/*
 * Break_hit()
 *
 * Return TRUE if the BREAK key was hit on the CC console, FALSE if not
 */
 boolean
break_hit()
	{PROGRAM(break_hit)

	return(BreakHit);
	}
 
/*
 * Invoke firmware PRINTF routine and check for break key being hit
 */
 /*VARARGS1*/
 void
printf(str,ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9)
	char *str;
	long ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9;
	{PROGRAM(printf)

#ifdef u3b2
	if (PRINTF(str,ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9) == -1)
		BreakHit = TRUE;
#endif

#ifdef u3b5
	if ((*(IAU_FNCTS)->pfi[PRINTF]) (str,ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9) == -1)
		BreakHit = TRUE;
#endif
	}

/*
 * Messages corresponding to errno; those messages that cannot occur
 * have been replaced by NULL pointers to conserve space
 */
static char *sys_errlist[] = {
			"Error 0",
        /* 1 EPERM */	0,	/* "Not owner", */
        /* 2 ENOENT */	"No such file or directory",
        /* 3 ESRCH */	0,	/* "No such process", */
        /* 4 EINTR */	0,	/* "interrupted system call", */
        /* 5 EIO */	"I/O error",
        /* 6 ENXIO */	"Special device cannot be used",	/* <== message changed */
        /* 7 E2BIG */	0,	/* "Arg list too long", */
        /* 8 ENOEXEC */	0,	/* "Exec format error", */
        /* 9 EBADF */	"Bad file number",
       /* 10 ECHILD */	0,	/* "No children", */
       /* 11 EAGAIN */	0,	/* "No more processes", */
       /* 12 ENOMEM */	0,	/* "Not enough core", */
       /* 13 EACCES */	0,	/* "Permission denied", */
       /* 14 EFAULT */	0,	/* "Bad address", */
       /* 15 ENOTBLK */	0,	/* "Block device required", */
       /* 16 EBUSY */	0,	/* "Mount device busy", */
       /* 17 EEXIST */	0,	/* "File exists", */
       /* 18 EXDEV */	0,	/* "Cross-device link", */
       /* 19 ENODEV */	"No such device",
       /* 20 ENOTDIR */	"Not a directory",
       /* 21 EISDIR */	0,	/* "Is a directory", */
       /* 22 EINVAL */	"Invalid argument",
       /* 23 ENFILE */	0,	/* "File table overflow", */
       /* 24 EMFILE */	"Too many open files",
       /* 25 ENOTTY */	0,	/* "Not a typewriter", */
       /* 26 ETXTBSY */	0,	/* "Text file busy", */
       /* 27 EFBIG */	"File too large",
		};
#define sys_nerr (sizeof(sys_errlist)/sizeof(sys_errlist[0]))

/*
 * Perror(message)
 */
perror(message)
	register char *message;
	{PROGRAM(perror)

	if (errno >= 0 && errno < sys_nerr && sys_errlist[errno])
		printf("%s: %s\n", message, sys_errlist[errno]);
	else
		printf("%s: errno=%d\n", message, errno);
	}

/*
 * Error(msgno, args ...)
 *
 * Print an error message, and determine the recovery action (if any) to
 * be taken.  The actions are itemized in errortab[] located in errortable.c
 */

extern struct errortab errortab[];

static int error_action();

/*VARARGS1*/
 int
error(msgnum, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
	register int msgnum;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int arg5;
	int arg6;
	int arg7;
	int arg8;
	int arg9;
	int arg10;
	{PROGRAM(error)

	register struct errortab *ep;
	register int code;
	register char *text;

	for (ep=errortab; code = ep->msgnum; ++ep)
		{
		if (code == msgnum)
			{
			/*
			 * determine recovery action
			 */
			if (((code = ep->action) & _CODE_) == _DYNAMIC_)
				code = error_action(msgnum);

			if ((text=ep->text) == NULL)
				text = "<NOTEXT>";

			if (code & _PANIC_)
				/*
				 * no deposit, no return
				 */
				panic(text);

			if (! (code & _SILENT_))
				/*
				 * either printf() or perror() will write the message
				 */
				if (code & _PERROR_)
					perror((char *)arg1);
				else
					{
					printf(text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
					printf("\n");
					}

			/*
			 * take the recovery action
			 */
			switch (code & _CODE_)
				{
			/*
			 * the caller will do the recovery
			 */
			case _RETURN_:
			/*
			 * return FALSE
			 */
			case _RETURNFALSE_:
				return(FALSE);
			/*
			 * return TRUE
			 */
			case _RETURNTRUE_:
				return(TRUE);
			/*
			 * better fix errortab[] ...
			 */
			default:
				panic("Illegal error action");
				}
			}
		}

	panic("Unknown error number");
	/*NOTREACHED*/
	}



/*
 * error_action(msgno)
 * 
 * This routine must decide what action to take for the error `msgno'.  A
 * valid action code must be returned.
 */
 static
 int
error_action(msg)
	register int msg;
	{PROGRAM(error_action)

	switch (msg)
		{
		/* Configured for more memory than available. */
	case ER77:
		/* Device <name> previously configured on LBE (board code <n>) at ELB board code <n> */
	case ER40:
		/* Device <name> previously configured at board code <n> */
	case ER41:

		if (ISAUTO)
			/* no message, go directly to /etc/system */
			return(_SILENT_ | _RETURNTRUE_);

		if (MagicMode)
			/* go ahead and load it */
			return(_RETURNFALSE_);
		else
			/* go back to system file prompt */
			return(_RETURNTRUE_);

		/* Configured for less memory than available. */
	case ER78:
		/* Device <name> (board code <n>) not configured */
	case ER42:
		/* Device <name> (LBE <n>, board code <n>) not configured */
	case ER43:
		if (ISAUTO)
			/* no message, go directly to /etc/system */
			return(_SILENT_ | _RETURNTRUE_);

		/* go ahead and load it */
		return(_RETURNFALSE_);
		}

	panic("error_action() failed");
	/*NOTREACHED*/
	}

#if DEBUG1
/*
 *	Shell()
 *
 *	A really stripped down imitation shell for testing.
 */
shell()
	{PROGRAM(shell)

	struct stat statbuf;
	register char *sp;
	char command[81];

	while (TRUE)
		{
		printf("# ");
		if (scanf(command) == -1)
			/*
			 * break key hit; return
			 */
			return;
		if ((sp=strtok(command,"\r\n\t ")) == NULL)
			continue;

		/*
		 * help
		 */
		if (0 == strcmp(sp,"help"))
			sp = "?";
		if (0 == strcmp(sp,"?"))
			{
			static char blanks[] ="                  : ";

			printf("Commands available: exit, help, stat, ls, time, panic, symbol, system\n");
#if DEBUG1a
			printf("%sXdepth, Xprint\n", blanks);
#endif
#if DEBUG1b
			printf("%sstack\n", blanks);
#endif
#if DEBUG1c
			printf("%scat\n", blanks);
#endif
#if DEBUG1d
			printf("%scd, pwd\n", blanks);
#endif
#if DEBUG1e
			printf("%sdriver\n", blanks);
#endif
#if DEBUG1f
			printf("%ssar\n", blanks);
#endif
#if DEBUG1g
			printf("%sfiles\n", blanks);
#endif
#if DEBUG1h
			printf("%smalloc, free\n", blanks);
#endif
#if DEBUG1i
			printf("%smemory\n", blanks);
#endif
			continue;
			}

		/*
		 * exit
		 */
		if (*sp == '\004')
			sp = "exit";
		if (0 == strcmp(sp,"exit"))
			return;

		/*
		 * ls
		 */
		if (0 == strcmp(sp,"ls"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				sp = ".";
			if (stat(sp,&statbuf) == -1)
				{
				perror(sp);
				continue;
				}
			if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
				printf("%s: Not a directory\n", sp);
			else
				ls(sp);
			continue;
			}

#if DEBUG1d
		/*
		 * cd
		 */
		if (0 == strcmp(sp,"cd"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				sp = "/";
			if (chdir(sp) == -1)
				perror(sp);
			continue;
			}

		/*
		 * pwd
		 */
		if (0 == strcmp(sp,"pwd"))
			{
			pwd();
			continue;
			}
#endif

		/*
		 * stat
		 */
		if (0 == strcmp(sp,"stat"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				printf("Usage: stat file\n");
			else
				{
				if (stat(sp,&statbuf) == -1)
					perror(sp);
				else
					{
					int type;
					printf("%s:\n", sp);
					printf("     st_mode = 0%o\n", statbuf.st_mode);
					printf("     st_ino  = %d\n", statbuf.st_ino);
					if ((type = statbuf.st_mode & S_IFMT) == S_IFCHR || type == S_IFBLK)
						printf("     st_rdev = %d,%d\n",
								major(statbuf.st_rdev), minor(statbuf.st_rdev));
					printf("     st_nlink= %d\n", statbuf.st_nlink);
					printf("     st_uid  = %d\n", statbuf.st_uid);
					printf("     st_gid  = %d\n", statbuf.st_gid);
					printf("     st_size = %D\n", statbuf.st_size);
					}
				}
			continue;
			}

#if DEBUG1c
		/*
		 * cat
		 */
		if (0 == strcmp(sp,"cat"))
			{
			cat(strtok((char*)NULL,"\r\n\t "));
			continue;
			}
#endif

#if DEBUG1f
		/*
		 * sar - system activity report
		 */
		if (0 == strcmp(sp,"sar"))
			{
			extern address lowater;
			extern address hiwater;

			printf("System activity\n");
			printf("    %D stat()\n", sysinfo.sysstat);
			printf("    %D open()\n", sysinfo.sysopen);
			printf("    %D read()\n", sysinfo.sysread);
			printf("    %D lseek()\n", sysinfo.sysseek);
			printf("    %D close()\n", sysinfo.sysclose);
			printf("    %D disk blocks read\n", sysinfo.bread);
			printf("    %D logical blocks read\n", sysinfo.lread);
			printf("    %D characters read\n", sysinfo.readch);
			printf("    %D hits in buffer cache\n", sysinfo.bhit);
			printf("    %D misses in buffer cache\n", sysinfo.bmiss);
			printf("    %D iget()\n", sysinfo.iget);
			printf("    %D hits in inode cache\n", sysinfo.ihit);
			printf("    %D misses in inode cache\n", sysinfo.imiss);
			printf("    %D namei()\n", sysinfo.namei);
			printf("    %D overflow directory blocks read\n", sysinfo.dirblk);
			printf("Memory allocation\n");
			printf("    high water: 0x%6lX\n", hiwater);
			printf("    low water:  0x%6lX\n", lowater);
			continue;
			}
#endif

#if DEBUG1h
		/*
		 * malloc & free tests
		 */
		if (0 == strcmp(sp,"malloc"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				printf("Usage: malloc size\n");
			else
				printf("Malloc(%s) = 0x%lX\n", sp, malloc((unsigned)strtol(sp,(char**)NULL,0)));
			continue;
			}
		if (0 == strcmp(sp,"free"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				printf("Usage: free addr\n");
			else
				free((char*) strtol(sp,(char**)NULL,0));
			continue;
			}
#endif

#if DEBUG1i
		if (0 == strcmp(sp,"memory"))
			{
			extern void mallocp();

			mallocp();
			continue;
			}
#endif

		/*
		 * symbol table tests
		 */
		if (0 == strcmp(sp,"symbol"))
			{
			if ((sp=strtok((char*)NULL,"\r\n\t ")) == NULL)
				printf("Usage: symbol [ name | number ]\n");
			else
				{
				register SYMBOL *p;
				if (isdigit(*sp))
					{
					p = Ksym_number((long)strtol(sp,(char**)NULL,0));
					printf("Ksymtab[%s] = %s 0x%8lX size=%D flag=%2X\n", sp, p->name, p->value, p->size, p->flag);
					}
				else
					{
					if ((p=Ksym_name(sp)) != NULL)
						printf("Ksym = 0x%8lX\n", p->value);
					else
						if ((p=Xsym_name(sp)) != NULL)
							printf("Xsym = 0x%8lX size=%D flag=%2X\n", p->value, p->size, p->flag);
						else
							printf("Symbol undefined\n");
					}
				}
			continue;
			}
#if DEBUG1a
		if (0 == strcmp(sp,"Xdepth"))
			{
			extern int Xdepth(), Xsym_count;

			printf("Xsymtab contains %d symbols and is %d levels deep\n", Xsym_count, Xdepth(NULL));
			continue;
			}
		if (0 == strcmp(sp,"Xprint"))
			{
			extern void Xprint();

			Xprint(NULL);
			continue;
			}
#endif

		if (0 == strcmp(sp,"time"))
			{
			printf("The time is %D\n", time());
			continue;
			}

		/*
		 * test system()
		 */
		if (0 == strcmp(sp,"system"))
			{
			if ((sp=strtok((char*)NULL,"\r\n")) == NULL || blankline(sp))
				printf("Usage: system input-line\n");
			else
				system(sp);
			continue;
			}

#if DEBUG1b
		if (0 == strcmp(sp,"stack"))
			{
			extern address STACK_TOP;

			printf("The stack reached 0x%lX\n", STACK_TOP);
			continue;
			}
#endif

#if DEBUG1e
		if (0 == strcmp(sp,"driver"))
			{
			extern void print_driver();

			print_driver();
			continue;
			}
#endif

#if DEBUG1g
		if (0 == strcmp(sp,"files"))
			{
			extern void files();

			files();
			continue;
			}
#endif

		/*
		 * panic test ...no return
		 */
		if (0 == strcmp(sp,"panic"))
			panic("your wish is my command");

		/*
		 * unknown command
		 */
		printf("%s: unknown command\n", sp);
		}
	/*NOTREACHED*/
	}
#endif

#if DEBUG1c
/*
 *	Cat(fname)
 *
 *	This routine reads the named file and writes it to the terminal
 */
 static
cat(fname)
register char *fname;
	{PROGRAM(cat)

	register int fd, rc;
	char c[2];
	boolean saveit = BreakHit;

	if (fname == NULL)
		{
		printf("Usage: cat file\n");
		return;
		}

	if ((fd = open(fname)) == -1)
		{
		perror(fname);
		return;
		}

	BreakHit = FALSE;

	for (c[1]='\0'; ! break_hit();)
		{
		if ((rc=read(fd,c,1)) != 1)
			{
			/* end of file */
			if (rc == -1)
				perror(fname);
			break;
			}

		printf("%s", c);
		}

	close(fd);

	BreakHit = saveit;
	}
#endif

#if DEBUG1d
/*
 *	Pwd
 *
 *	This routine prints the full path name of the current directory.
 */
 static
pwd()
	{PROGRAM(pwd)

	struct direct dentry;
	struct stat d;
	ino_t dot, dotdot;
	char cwd[100], tcwd[100], parent[50];
	register int fd, rc;

	strcpy(parent, "..");
	strcpy(cwd, "");

	if (stat(".",&d) == -1)
		{
		perror(".");
		return;
		}
	dot = d.st_ino;

	if (dot == ROOTINO)
		{
		printf("/\n");
		return;
		}

	while (dot != ROOTINO)
		{

		if ((fd=open(parent)) == -1)
			{
			perror(parent);
			return;
			}

		do	{
			if ((rc=read(fd,(char*)&dentry,sizeof(dentry))) != sizeof(dentry))
				{
				if (rc == -1)
					perror(parent);
				else
					printf("%s: missing entry for inode %d\n", parent, dot);
				return;
				}
			if (0 == strncmp(".",dentry.d_name,DIRSIZ))
				dotdot = dentry.d_ino;
			}
			while (dot != dentry.d_ino);

		close(fd);

		strcpy(tcwd, cwd);
		strcat(strncat(strcpy(cwd,"/"),dentry.d_name,DIRSIZ), tcwd);
		dot = dotdot;

		strcat(parent, "/..");
		}

	printf("%s\n", cwd);
	}
#endif

#if TEST
#ifdef u3b2
/*
 * Exception()
 *
 * This routine receives control from the firmware after a normal exception
 */
 static
exception()
	{PROGRAM(exception)

	printf("Normal exception:  PSW=0x%8X  PC=0x%8X\n", OLDPSW, OLDPC);

	panic("stack trace");
	}
#endif

/*
 * Panic(message)
 */
panic(message)
	char *message;
	{PROGRAM(panic)

	register address *ap, **fp, *sp, *pc, *regs;
	extern address getsp();
#if DEBUG1g
	extern void files();
#endif

	printf("PANIC: %s\n", message);

	if (errno >= 0 && errno < sys_nerr && sys_errlist[errno])
		printf("       errno=%d (%s)\n" , errno, sys_errlist[errno]);
	else
		printf("       errno=%d (Unknown error)\n" , errno);

#if DEBUG1f
		{
		extern struct user u;

		if (u.u_error >= 0 && u.u_error < sys_nerr && sys_errlist[u.u_error])
			printf("       u.u_error=%d (%s)\n" , u.u_error, sys_errlist[u.u_error]);
		else
			printf("       errno=%d (Unknown error)\n" , errno);
		}
#endif

#if DEBUG1g
	files();
#endif

	ap = (address *) &message;
	fp = (address **) ap + 10;
	sp = (address *) getsp();

	printargs(ap, (address *) (fp-9));
	regs = sp + 4;
	do	{
		printregs(ap, fp, sp, regs);
		printframe((address *) fp, sp - (address *)fp);

		sp = ap;
		regs = (address *) fp - 6;
		pc = *(fp - 9);
		ap = *(fp - 8);
		fp = (address **) *(fp - 7);

		printargs(ap, (address *) (fp-9));
		}
		while ((address) pc > MAINSTORE);

	while (TRUE)
		continue;
	}

/*
 * Printargs(ap, ret)
 *
 * Print arguments to a function
 */
 static
printargs(ap, ret)
	register address *ap, *ret;
	{PROGRAM(printargs)

	register char *name;
	register reasonable;
	extern int etext[], edata[];

	name = (char*) *(ret+9);
	if (name >= (char*)etext && name < (char*)edata)
		printf("%s(", name);
	else
		printf("called from %lX(", *ret);

	reasonable = 0;
	while (ap < ret)
		{
		printf(" %lX", *ap++);
		if (++reasonable > 10)
			{
			printf(" ...");
			break;
			}
		}
	printf(")\n");
	}

/*
 * Printregs(ap, fp, sp, r3)
 *
 * Print the registers %ap, %fp, %sp and %r3 - %r8
 */
 static
printregs(ap, fp, sp, r3)
	register address *ap, **fp, *sp, *r3;
	{PROGRAM(printregs)

	register int i;

	printf("  AP=%8lX  FP=%8lX  SP=%8lX\n", ap, fp, sp);

	for (i=3; i<9; ++i)
		printf("  R%d=%8lX", i, *r3++);
	printf("\n");
	}

/*
 * Printframe(fp, words)
 *
 * Print the automatic variables
 */
 static
printframe(fp, words)
	register address *fp;
	register int words;
	{PROGRAM(printframe)

	register int offset, count;

	printf("  Automatic Storage");

	offset = 0;
	while (words > 0)
		{
		printf("\n  %6lX  %4X  ", fp, offset);
		offset += 0x10;
		for (count=0; count<4 && words>0; ++count, --words)
			printf(" %8lX", *fp++);
		}
	printf("\n");
	}

/*
 * Timer routines:
 *
 *	time()    - return current time in seconds
 *	elapsed() - print current and elapsed time
 *	sitcmd()  - communicate with SIT (u3b5 only)
 */


#ifdef u3b5
/*
 * sitcmd(command, flag, data)
 *
 * Implement the communication protocol between the CC and the SIT
 */

#define	READSIT		0x80
#define	WRITESIT	0x40
#define	SITFLAG		0xc0

/*VARARGS2*/
sitcmd(command, flag, data)
	register command;
	register flag;
	register char *data;
	{PROGRAM(sitcmd)

	register status, delay;
	register struct sit *sit = CICSIT;
	static boolean first_time = TRUE;

	if (first_time)
		{
		first_time = FALSE;

		status = sit->data;	/* clear SIT data register */

		sitcmd(RSTSIT, 0);
		sitcmd(DTHWT | DTRTCT | DTST | DTSWIT, 0);
		sitcmd(IERTCT, WRITESIT+4, "\0\0\0\0");
		}

	for (delay=10000; sit->command & SITF0; --delay)
		if (delay < 0)
			panic("stuck SITF0");

	sit->command = command;

	switch (flag & SITFLAG)
		{
	/*
	 * write data to the SIT
	 */
	case WRITESIT:
		--data;
		for (flag&=~SITFLAG; flag>0; --flag)
			{
			for (delay=10000; sit->command & SITIBF; --delay)
				if (delay < 0)
					panic("stuck SITIBF");
			sit->data = *(data + flag);
			}
		break;
	/*
	 * read data from the SIT
	 */
	case READSIT:
		--data;
		for (flag&=~SITFLAG; flag>0; --flag)
			{
			for (delay=10000; (sit->command & SITOBF) == 0; --delay)
				if (delay < 0)
					panic("stuck SITOBF");
			*(data + flag) = sit->data;
			}
		break;
		}

	for (delay=10000; sit->command & SITIBF; --delay)
		if (delay < 0)
			panic("stuck SITIBF");

	for (delay=10000; (status = sit->command) & SITF0; --delay)
		if (delay < 0)
			panic("stuck SITF0");

	if ((status & SITERR) != ((command==RSTSIT)? SITROK : SITOK))
		printf("SIT error: command=0x%X status=0x%X\n", command, status);
	}


/*
 * time()
 *
 * Return current time in seconds
 */
 static
 long
time()
	{PROGRAM(time)

	union	{
		long	seconds;
		char	bytes[4];
		}
		t;

	sitcmd(RDRTCT, READSIT+4, t.bytes);

	return(t.seconds);
	}
#endif


#ifdef u3b2
/*
 * time()
 *
 * Return current time in seconds
 */
 static
 long
time()
	{PROGRAM(time)

	register int r, seconds;
	static boolean first_time = TRUE;

	if (first_time)
		{
		first_time = FALSE;

		SBDTOD->test = 0;
		SBDTOD->stop_star = 1;
		}

	while (TRUE)
		{
		r = SBDTOD->tenths;

		if ((r=SBDTOD->secs.units) != 15)
			{
			seconds = r;

			if ((r=SBDTOD->secs.tens) != 15)
				{
				seconds += 10 * r;

				if ((r=SBDTOD->mins.units) != 15)
					{
					seconds += 60 * r;

					if ((r=SBDTOD->mins.tens) != 15)
						{
						seconds += 600 * r;

						if ((r=SBDTOD->hours.units) != 15)
							{
							seconds += 3600 * r;

							if ((r=SBDTOD->hours.tens) != 15)
								{
								seconds += 36000 * r;

								return(seconds);
								}
							}
						}
					}
				}
			}
		}
	}
#endif


/*
 * elapsed(message)
 *
 * Print current time and elapsed time in seconds
 */
elapsed(message)
	char *message;
	{PROGRAM(elapsed)

	static long last_time = 0;
	long seconds;

	if ((seconds=time()) < last_time)
		last_time -= 24*60*60;

	printf("%s: time=%4D  elapsed=%4D\n", message, seconds, seconds-last_time);

	last_time = seconds;
	}

#if DEBUG1b
/*
 * _spmonitor(name)
 *
 * Monitor the height of the stack.  Called by PROGRAM macro at entry to each
 * function.
 */
address STACK_TOP;

 char *
_spmonitor(name)
	char *name;
	{
	extern address getsp();

	STACK_TOP = (address) umax(STACK_TOP, getsp());

	return(name);
	}
#endif

#else ! TEST
/*
 * Panic(message)
 */
panic(message)
	char *message;
	{

	printf("ERROR: %s\n\n[Hit RESET to return to firmware] ", message);

	while (TRUE)
		continue;
	}
#endif
