/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/subr.c	10.3"

#include <sys/types.h>
#include <a.out.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <ctype.h>
#include "error.h"

#ifdef u3b5
#include <sys/cc.h>
#endif

#ifdef u3b2
#include <sys/sbd.h>
#endif

#ifdef unix	/* get rid of special pre-processor name */
#undef unix
#endif


/*
 * Static function declarations for this file
 */
static char    *do_boot();
static char    *do_device();
static char    *do_exclude();
static char    *do_include();
static char    *do_swapdevice();
static char    *interpret();
static boolean  numeric();
static int      parse();
static void     prompt();

/*
 * Findsystem()
 *
 * This routine will ask for the name of the /etc/system file if the autoboot
 * switch is not set.  If a directory name is given, then the contents will be
 * listed as though "ls -p | pr -t -5 -w80" was issued.
 *
 * If the name is a regular file, then it is read to determine if it is ascii
 * text or an object file.  If an object file, then it is assumed to be an
 * absolute bootable file (if not, then loadunix() will call system() to get
 * the system devices).  If the name is not an object file, then it must be
 * the /etc/system file, and its name is saved in `etcsystem'.
 *
 * Therefore, when this routine returns:
 *
 *		user file name		bootprogram.path[0]	etcsystem[0]
 *		--------------------	-------------------	------------
 *		object file name	           SET		    '\0'
 *		ascii text file name	          '\0' 		    SET
 *
 * This routine also supports the "magic mode" undocumented feature.  If
 * "magic mode" is typed, MagicMode is set TRUE.  An optional third word
 * will be used as the name of the /boot directory.
 */
 void
findsystem()
	{PROGRAM(findsystem)

	extern boolean	DebugMode;
	register char *sp;
	register int fd;
	char path[81];
	struct stat stat_unix, stat_system;
	static int state;
	static boolean first_time = TRUE;
	static char unix[]={"/unix"}, system[]={"/etc/system"};
	extern boolean MagicMode;

	/*
	 * stat /unix and /etc/system; if the file does not exist, then set
	 * st_ino to zero in the stat structure (no valid inode number is zero)
	 */
	if (stat(unix,&stat_unix) == -1)
		stat_unix.st_ino = 0;
	if (stat(system,&stat_system) == -1)
		stat_system.st_ino = 0;

	/*
	 * we don't leave this routine until we get either a valid system
	 * file name or a valid boot program name
	 */
	while (TRUE)
		{
		/*
		 * Set state:	0 (u3b2 only) if not ISAUTO and
		 *		  first time here
		 * 		1 if ISAUTO and first time here
		 *		2 if ISAUTO and second time here
		 *	      >=3 if not ISAUTO or if third (or later)
		 *		  time here
		 */
		if (ISAUTO)
			{
			/* assume /unix, then /etc/system, then prompt */
			if (first_time)
				/* assume /unix */
				state = 1;
			else
				/* assume /etc/system once, then prompt */

				state += 1;
			}
		else
			{
#ifdef u3b2
			if (first_time)
				/* DEMANDBOOT or AUTOBOOT; use name provided by firmware */
				state = 0;
			else
				if (P_CMDQ->b_type == AUTOBOOT) {
					/* go back to firmware if AUTOBOOT failed */
					restart();
				} else {
					/* this is a DEMANDBOOT that failed */
#endif
					/* prompt for system file name */
					state = 3;
				}
			}

		first_time = FALSE;

		switch (state)
			{
		/*
		 * use /unix if there has been no change to /etc/system
		 */

		case 1:
			if (stat_unix.st_ino == 0)
				{
				/* <unix> does not exist */
				error(ER51, unix);
				continue;
				}
			else
				/* at least /unix is present */
				{
				if (stat_system.st_ino == 0)
					/* /etc/system does not exist */
					{
					sp = unix;
					break;
					}
				if (stat_system.st_mtime > stat_unix.st_mtime)
					/* /etc/system is newer than /unix */
					sp = system;
				else
					/* /unix is newer than /etc/system */
					sp = unix;
				}
			break;
		/*
		 * either /unix was out of date, or an error occurred; use
		 * /etc/system to do a full self-config boot if possible
		 */
		case 2:
			if (0 == strcmp(system,etcsystem))
				/* whoops, /etc/system has already been tried */
				continue;
			if (stat_system.st_ino == 0)
				{
				/* <system> does not exist */
				error(ER51, system);
				continue;
				}
			sp = system;
			break;
#ifdef u3b2
		/*
		 * 3B2 auto or demand boot; use the name provided
		 * by the firmware
		 */
		case 0:
			if (*(sp=strcpy(path,P_CMDQ->b_name)) != '\0')
				break;

			/*
			 * no name so drop through and prompt
			 */
#endif
		/*
		 * last resort or manual boot; prompt for system file name
		 */
		default:
			while (TRUE)
				{
#ifdef u3b5
				printf("\nEnter path name of system file: ");
#endif
#ifdef u3b2
				printf("\nEnter path name: ");
#endif

				/*
				 * read input from console
				 */
				if ((fd=scanf(path)) != SCANF_OK)
					if (fd == -1) {
						/*
						 * break key hit; return to IAU
						 */
						restart();
					} else {
						/*
						 * line too long
						 */
						continue;
					}

				if ((sp=strtok(path,"\r\n\t ")) == NULL)
					{
					ls("/");
					continue;
					}

				/*
				 * check for secret "magic mode" feature
				 */
				if (0 == strcmp(sp,"magic")) {
					char *mp;

					if ((mp=strtok((char*)NULL,"\r\n\t ")) == NULL || 0 != strcmp(mp,"mode"))
						break;

					MagicMode = TRUE;
					printf("\nPOOF!");

					if ((mp=strtok((char*)NULL,"\r\n\t ")) != NULL)
						{
						if (*mp == '/')
							++mp;

						strcat(strcpy(slash_boot,"/"), mp);

						printf("   A hollow voice says \"%s\".", slash_boot);
						}

					printf("\n");
					continue;
				}

				/*
				 * check for secret "debug mode" feature
				 */

				if (0 == strcmp(sp,"debug")) {
					char *mp;

					if ((mp=strtok((char*)NULL,"\r\n\t ")) == NULL || 0 != strcmp(mp,"mode"))
						break;

					mp = strtok((char *)NULL, "\r\n\t ");

					if (!strcmp(mp, "on"))
						DebugMode = TRUE;
					 else if (!strcmp(mp, "off"))
						DebugMode = FALSE;

					printf("\ndebug mode is %s.\n",
						(DebugMode ? "on" : "off"));

					continue;
				}

				break;
				}
			}

		/*
		 * we've got a path name now; it must be examined to determine
		 * what kind of file it is
		 */
		if (stat(sp,&stat_unix) == -1)
			{
			/* <sp>: perror() message */
			error(ER7, sp);
			continue;
			}

		if ((stat_unix.st_mode & S_IFMT) == S_IFDIR)
			{
			ls(sp);
			continue;
			}

		if ((fd=open(sp)) == -1)
			{
			/* <sp>: perror() message */
			error(ER7, sp);
			continue;
			}

		ON(IOERROR)
			goto badfile;

		if (stat_unix.st_mode & S_IFREG)
			{
			unsigned short magic;

			if (stat_unix.st_size <= sizeof(magic))
				/*
				 * assume /etc/system file since its obviously not an object file;
				 * it will probably cause a syntax error later, but that's their
				 * problem
				 */
				{
				strcpy(etcsystem, sp);
				bootprogram.path[0] = '\0';
				break;
				}

			read_and_check(fd, (char*)&magic, sizeof(magic));

			if (magic == FBOMAGIC)
				/*
				 * object file; loadunix() will prompt for system file info
				 */
				{
				strcpy(bootprogram.path, sp);
				etcsystem[0] = '\0';
				break;
				}

			if (isascii(((char*)&magic)[0]) &&
			    (isprint(((char*)&magic)[0]) || isspace(((char*)&magic)[0])))
				if (isascii(((char*)&magic)[1]) &&
				    (isprint(((char*)&magic)[1]) || isspace(((char*)&magic)[1])))
					/*
					 * assume /etc/system file
					 */
					{
					strcpy(etcsystem, sp);
					bootprogram.path[0] = '\0';
					break;
					}
			}

		/* <sp>: not object file and not ascii text file */
		error(ER52, sp);

		/*
		 * bad file, try again
		 */
	badfile:
		close(fd);
		}

	close(fd);
	}

/*
 * Fsystem(stream)
 * System(string)
 *
 * Read the /etc/system file, parse and extract all the necessary information.
 * The stream is either an actual file, or it is a memory image.
 */
 void
system(string)
	char *string;
	{PROGRAM(system)

	FILE strbuf;

	strbuf._flag = _IOREAD;
	strbuf._ptr = strbuf._base = (unsigned char *) string;
	strbuf._cnt = strlen((char*)strbuf._ptr);
	strbuf._file = -1;

	fsystem(&strbuf);
	}



 void
fsystem(stream)
	register FILE *stream;
	{PROGRAM(fsystem)

	char line[256];
	register char *msg;
	register lineno = 1;

	while ((int) fgets(line,sizeof(line),stream) != NULL)
		{

		if (strlen(line) == sizeof(line)-1)
			{
			/* System: line too long */
			error((stream->_flag&_IOMYBUF)? ER68 : ER69, lineno);
			continue;
			}

		if (ferror(stream))
			{
			/* system: perror() message */
			error(ER7, "system");
			break;
			}

		if (line[0] != '*' && ! blankline(line))
			if ((msg=interpret(line)) != NULL)
				{
				if (stream->_flag & _IOMYBUF)
					/* System: line <lineno>; <msg> */
					error(ER53, lineno, msg);
				else
					/* System: <msg> */
					error(ER54, msg);
				}

		++lineno;
		}
	}

/*
 * Interpret(line)
 *
 * Interpret the line from the /etc/system file
 */


struct	syntax
	{
	char	*keyword;
	char	*(*process)();
	char	*argument;
	};

static struct syntax syntax[] ={
			{ "BOOT", do_boot, 0 },
			{ "EXCLUDE", do_exclude, 0 },
			{ "INCLUDE", do_include, 0 },
#ifdef u3b5
			{ "DUMPDEV", do_device, (char*) &dumpdev },
#endif
			{ "ROOTDEV", do_device, (char*) &rootdev },
			{ "SWAPDEV", do_swapdevice, 0 },
			{ "PIPEDEV", do_device, (char*) &pipedev },
				0 };

 static
 char *
interpret(line)
	char *line;
	{PROGRAM(interpret)

	register int argc;
	char *argv[50];
	register struct syntax *p;

	if ((argc=parse(argv,sizeof(argv)/sizeof(argv[0]),line)) > sizeof(argv)/sizeof(argv[0]))
		return("line too long");

	if (argc == 0)
		return(NULL);

	if (argc == 1 || *argv[1] != ':')
		return("syntax error");

	for (p=syntax; p->keyword; ++p)
		{
		if (0 == strcmp(*argv,p->keyword))
			{
			if (argc == 2)
				return(NULL);
			else
				return((*(p->process))(argc-2, &argv[2], p->argument));
			}
		}

	return("syntax error");
	}

/*
 * Parse(argv, sizeof(argv), line)
 *
 * Parse a line from the /etc/system file; argc is returned, and argv is
 * initialized with pointers to the elements of the line.  The contents
 * of the line are destroyed.
 */
 static
 int
parse(argv, sizeof_argv, line)
	char **argv;
	unsigned sizeof_argv;
	register char *line;
	{PROGRAM(parse)

	register char **argvp = argv;
	register char **maxvp = argv + sizeof_argv;
	register c;

	while (c = *line)
		{
		switch (c)
			{
		/*
		 * white space
		 */
		case ' ':
		case '\t':
		case '\r':
			*line++ = '\0';
			line += strspn(line," \t\r");
			continue;
		/*
		 * special characters
		 */
		case ':':
			*line = '\0';
			*argvp++ = ":";
			++line;
			break;
		case ',':
			*line = '\0';
			*argvp++ = ",";
			++line;
			break;
		case '(':
			*line = '\0';
			*argvp++ = "(";
			++line;
			break;
		case ')':
			*line = '\0';
			*argvp++ = ")";
			++line;
			break;
		case '?':
			*line = '\0';
			*argvp++ = "?";
			++line;
			break;
		case '=':
			*line = '\0';
			*argvp++ = "=";
			++line;
			break;
		/*
		 * end of line
		 */
		case '\n':
			*line = '\0';
			*argvp = NULL;
			return(argvp - argv);
		/*
		 * words and numbers
		 */
		default:
			*argvp++ = line;
			line += strcspn(line,":,()?= \t\r\n");
			break;
			}

		/*
		 * don't overflow the argv array
		 */
		if (argvp >= maxvp)
			return(sizeof_argv + 1);
		}

	*argvp = NULL;
	return(argvp - argv);
	}

/*
 * prompt(message)
 */
 static
 void
prompt(message)
	char *message;
	{PROGRAM(prompt)

	char input[256];
	register char *sp;
	register rc;

	while (TRUE)
		{
		printf("%s ? ", strcpy(input,message));

		if ((rc=scanf(sp=input+strlen(input))) != SCANF_OK) {
			if (rc == -1) {
				/*
				 * break key hit; return to IAU
				 */
				restart();
			}
		} else {
			if (blankline(sp))
				/* empty line */
				break;

			if ((sp=interpret(input)) == NULL)
				break;
		}

		printf("%s; re-enter ", sp);
		}
	}

/*
 * Blankline(line)
 *
 * Return TRUE if the line is entirely blank or null; return FALSE otherwise.
 */
 boolean
blankline(line)
	register char *line;
	{PROGRAM(blankline)

	return(strspn(line," \t\r\n") == strlen(line));
	}

/*
 * Numeric(assign, string)
 *
 * If the string is a valid numeric string, then set *assign to its numeric
 * value and return TRUE; otherwise return FALSE.
 */
 static
 boolean
numeric(assign, string)
	register int *assign;
	char *string;
	{PROGRAM(numeric)

	register long value;
	char *next;

	value = strtol(string, &next, 0);

	if (*next)
		/*
		 * bad number
		 */
		return(FALSE);

	*assign = value;

	return(TRUE);
	}

/*
 * Do_??????(argc, argv, optional)
 *
 * Handle the processing for each type of line in /etc/system
 */


/*
 * BOOT: program
 */
 static
 char *
do_boot(argc, argv)
	int argc;
	register char **argv;
	{PROGRAM(do_boot)

	struct stat statbuf;
	register type;

	if (argc > 1)
		return("syntax error");

	if (*argv[0] == '?')
		{
		prompt("BOOT:");
		return(NULL);
		}

	if (stat(*argv,&statbuf) == -1)
		return("no such file");

	if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
		return("cannot boot directory");

	if ((type=statbuf.st_mode&S_IFMT) == S_IFCHR || type == S_IFBLK)
		return("cannot boot special device");

	if (type == S_IFIFO)
		return("cannot boot special file");

	if (strlen(*argv) >= sizeof(bootprogram.path))
		return("path too long");

	strcpy(bootprogram.path, *argv);

	return(NULL);
	}


/*
 * EXCLUDE: driver ...
 */
 static
 char *
do_exclude(argc, argv)
	register int argc;
	register char **argv;
	{PROGRAM(do_exclude)

	while (argc-- > 0)
		{
		if (*argv[0] != ',')
			/* not comma */
			if (*argv[0] == '?')
				prompt("EXCLUDE:");
			else
				exclude(*argv);

		++argv;
		}

	return(NULL);
	}


/*
 * INCLUDE: driver(optional-number) ...
 */
 static
 char *
do_include(argc, argv)
	register int argc;
	register char **argv;
	{PROGRAM(do_include)

	register char *p;
	int n;

	while (argc > 0)
		{

		if (*argv[0] == ',')
			{
			--argc;
			++argv;
			continue;
			}

		if (*argv[0] == '?')
			{
			prompt("INCLUDE:");
			--argc;
			++argv;
			continue;
			}

		p = *argv;
		n = 1;

		if (argc >= 4 && *argv[1] == '(')
			{
			if (*argv[3] != ')')
				return("syntax error");

			if (! numeric(&n, argv[2]))
				return("count must be numeric");

			argc -= 3;
			argv += 3;
			}
		else
			if (*p == '(' || *p == ')')
				return("syntax error");

		include(p, n);

		--argc;
		++argv;
		}

	return(NULL);
	}


/*
 * DUMPDEV: { path | DEV(number,number) }
 * ROOTDEV: { path | DEV(number,number) }
 * PIPEDEV: { path | DEV(number,number) }
 */
 static
 char *
do_device(argc, argv, device)
	register int argc;
	register char **argv;
	register dev_t *device;
	{PROGRAM(do_device)

	struct stat statbuf;
	register type;
	int M, m;

	if (argc == 1)
		/*
		 * path
		 */
		{
		if (*argv[0] == '?')
			{
			char what[10];

			/*
			 * we cheat here; we know that interpret() calls this
			 * routine with argv+2; therefore, argv[-2] is the original
			 * statement type
			 */
			strcat(strcpy(what,argv[-2]), ":");

			prompt(what);
			return(NULL);
			}

		if (stat(*argv,&statbuf) == -1)
			return("no such file");
		else
			if ((type=statbuf.st_mode&S_IFMT) != S_IFCHR && type != S_IFBLK)
				return("file not BLOCK or CHAR special");

		*device = statbuf.st_rdev;

#if DEBUG1
		if (prt[_SUBR])
			printf("%s  major=0x%X  minor=0%o\n", argv[-2], major(*device), minor(*device));
#endif

		return(NULL);
		}

	if (argc != 6 || 0 != strcmp("DEV",argv[0]) || *argv[1] != '(' || *argv[3] != ',' || *argv[5] != ')')
		return("syntax error");
	
	/*
	 * DEV(number,number)
	 */

	if (! numeric(&M,argv[2]) || ! numeric(&m,argv[4]))
		return("major/minor must be numeric");

	*device = makedev(M, m);

#if DEBUG1
	if (prt[_SUBR])
		printf("%s  major=0x%X  minor=0%o\n", argv[-2], major(*device), minor(*device));
#endif

	return(NULL);
	}


/*
 * SWAPDEV: { path | DEV(number,number) }  swplo  nswap
 */
 static
 char *
do_swapdevice(argc, argv)
	register int argc;
	register char **argv;
	{PROGRAM(do_swapdevice)

	register char *p;

	if (argc == 3)
		/*
		 * path swplo nswap
		 *
		 * swapdev = path
		 * swplo = number
		 * nswap = number
		 */
		{
		if (*argv[1] == '=')
			/*
			 * internal prompt response
			 */
			{
			if (0 == strcmp("swapdev",argv[0]))
				return(do_device(1,&argv[2],&swapdev));

			if (0 == strcmp("swplo",argv[0]))
				{
				int temp;

				if (! numeric(&temp,argv[2]))
					return("must be numeric");

				swplo = temp;
				}

			if (0 == strcmp("nswap",argv[0]))
				if (! numeric(&nswap,argv[2]))
					return("must be numeric");

			return(NULL);
			}

		if (*argv[0] == '?')
			prompt("SWAPDEV: swapdev=");
		else
			if ((p=do_device(1,argv,&swapdev)) != NULL)
				return(p);

		--argc;
		++argv;
		}
	else
		if (argc == 8)
			/*
			 * DEV(number,number) swplo nswap
			 *
			 * swapdev = DEV(number,number)
			 */
			{
			if (*argv[1] == '=')
				/*
				 * internal prompt response
				 */
				{
				if (0 == strcmp("swapdev",argv[0]))
					return(do_device(6,&argv[2],&swapdev));
				}

			if ((p=do_device(6,argv,&swapdev)) != NULL)
				return(p);

			argc -= 6;
			argv += 6;
			}
		else
			return("syntax error");

	if (*argv[0] == '?')
		prompt("SWAPDEV: swplo=");
	else
		{
		int temp;

		if (! numeric(&temp,argv[0]))
			return("must be numeric");

		swplo = temp;
		}

	if (*argv[1] == '?')
		prompt("SWAPDEV: nswap=");
	else
		if (! numeric(&nswap,argv[1]))
			return("must be numeric");

	return(NULL);
	}

/*
 * Loadmap generation and verification routines.  These routines will create
 * and maintain the loadmap for two reasons:
 *
 *	1. to print the load map table if the auto-boot switch is OFF
 *	2. to enable checking for overlaps of loaded sections
 *
 * alloc_loadmap():	allocate the loadmap[] array
 * make_loadmap():	fill the loadmap[] array and catch overlaps
 * print_loadmap():	print the load map table
 */

struct	loadmap
	{
	char	*file;
	char	section[9];
	address	paddr;
	long	length;
	address	vaddr;
	};

static struct loadmap *loadmap = NULL,
                      *nextmap;



/*
 * Alloc_loadmap(number)
 *
 * Allocate the loadmap[] array; its size is 'number' of sections.
 */
 void
alloc_loadmap(number)
	unsigned number;
	{PROGRAM(alloc_loadmap)

	/*
	 * allocate load map array
	 */
	if ((loadmap=(struct loadmap *)malloc(number*sizeof(*loadmap))) == NULL)
		panic("No memory for loadmap");

	nextmap = loadmap;
	}


/*
 * Make_loadmap(file, section, paddr, vaddr, length)
 *
 * Build the loadmap[] array.  Check for overlap with other sections.
 * Sections can be identified by paddr/vaddr values:
 *
 *	addr,-1    : a physical section
 *	-1,addr    : a virtual bss section
 *	addr,addr  : a virtual section
 */
 void
make_loadmap(file, section, paddr, vaddr, length)
	char *file;
	char *section;
	register address paddr;
	address vaddr;
	register long length;
	{PROGRAM(make_loadmap)

	register struct loadmap *lp, *tp;
	extern address lowater, maxmem, topboot;


	/*
	 * fill in the next loadmap[] entry
	 */
	lp = nextmap++;

	lp->file = file;
	strncat(strcpy(lp->section,""), section, sizeof(lp->section)-1);
	lp->paddr = paddr;
	lp->length = length;
	lp->vaddr = vaddr;

	if (paddr == -1)
		/*
		 * a bss section, for which we do not yet know its origin
		 */
		return;

	if (paddr < MAINSTORE)
		/* Section <file>(<section>) loaded below MAINSTORE address */
		error(ER55, file, section);

	if (paddr+length > maxmem)
		/* Section <file>(<section>) loaded beyond end of MAINSTORE */
		error(ER56, file, section);

	if (paddr < topboot && paddr+length > lowater)
		/* Section <lp->file>(<lp->section>) overlaps boot program */
		error(ER57, lp->file, lp->section);

	/*
	 * check for overlapping other sections
	 */
	for (tp=loadmap; tp<lp; ++tp)
		{
		if (tp->paddr == -1)
			continue;

		if (paddr < tp->paddr+tp->length && paddr+length > tp->paddr)
			/* Section <lp->file>(<lp->section>) overlaps <tp->file>(<tp->section>) */
			error(ER46, lp->file, lp->section, tp->file, tp->section);
		}
	}


/*
 * Print_loadmap()
 *
 * Print the load map unless QuietMode is TRUE.
 */
 void
print_loadmap()
	{PROGRAM(print_loadmap)

	register struct loadmap *lp;
	extern boolean QuietMode;


	if (QuietMode || (QuietMode = break_hit()))
		return;

	/*
	 * print the loadmap
	 */
	printf("\nLOAD MAP\n========\n     --------section--------   --------address-------");
	/*                           \n     DIRSIZ__LENGTH(.section)  p XXXXXX   v XXXXXX */      

	for (lp=loadmap; lp<nextmap; ++lp)
		{
		char buffer[DIRSIZ+1+8+1+1];	/* NAME(section) */

		if (QuietMode = break_hit())
			{
			printf("\n");
			return;
			}

		strcpy(buffer, lp->file);
		strcat(buffer, "(");
		strcat(buffer, lp->section);
		strncat(buffer, ")                        ", (int)sizeof(buffer)-1-strlen(buffer));

		if (lp->paddr == -1)
			/*
			 * bss section
			 */
			lp->paddr = REAL(lp->vaddr, bss_locctr);

		printf("\n     %s  p %6lX", buffer, lp->paddr);

		if (lp->vaddr != -1)
			/*
			 * virtual section; print virtual origin
			 */
			printf("   v %6lX", lp->vaddr);
		}

	printf("\n     END                       p %6lX\n\n", Xsym_name("END")->value);
	}

/*
 *	Ls(dirname)
 *
 *	This routine prints the names of all allocated files in the directory
 *	named by dirname.  Names are printed in alphabetic order.
 */

#define	SIZELINE	80
#define	SIZENAME	(DIRSIZ+2)

struct	list
	{
	char	name[DIRSIZ+1+1];
	};

 void
ls(dir)
	char *dir;
	{PROGRAM(ls)

	register struct direct *d;
	register struct list *list, *lp;
	register count;
	struct stat statbuf;
	char fullname[100+DIRSIZ];
	char line[SIZELINE+1];

	printf("Files in %s are:\n", dir);

	if (strlen(dir)+1+DIRSIZ >= 100)
		{
		printf("%s: pathname too long\n", dir);
		return;
		}

	if (stat(dir,&statbuf) == -1)
		{
		/* <dir>: perror() message */
		error(ER7, dir);
		return;
		}

	if ((list=lp=(struct list *)malloc(((unsigned)statbuf.st_size/sizeof(struct direct))*sizeof(struct list))) == NULL)
		{
		printf("No memory for directory list\n");
		return;
		}

	if ((d=directory(dir)))
		{
		count = 0;

		do	{
			/*
			 * skip directory items '.', '..' 
			 */
			if (d->d_name[0]=='.')
				{
				if (d->d_name[1]=='\0')
					continue;
				if (d->d_name[1]=='.' && d->d_name[2]=='\0')
					continue;
				}

			strcat(strcat(strcpy(fullname,dir),"/"), strncat(strcpy(lp->name,""),d->d_name,DIRSIZ));

			if (stat(fullname, &statbuf) == -1)
				{
				/* <fullname>: perror() message */
				error(ER7, fullname);
				continue;
				}

			if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
				strcat(lp->name, "/");

			++lp;
			++count;
			}
			while ((d=directory((char*)NULL)) != NULL);

		/*
		 * sort the names
		 */
			{
			register i, m;

			for (m=count/2; m>0; m/=2)
				{
				for (i=0; i<count-m; ++i)
					{
					lp = &list[i];

					if (strcmp(lp[0].name,lp[m].name) > 0)
						{
						struct list temp;

						temp = lp[m];

						do	{
							lp[m] = lp[0];
							}
							while ((lp-=m) >= list && strcmp(lp[0].name,temp.name) > 0);

						lp[m] = temp;
						}
					}
				}
			}

		/*
		 * produce the display
		 */
		strcpy(line, "");

		for (lp=list; lp < &list[count]; ++lp)
			{
			strcat(line, lp->name);
			strncat(line, "              ", SIZENAME-strlen(lp->name));

			if (strlen(line) >= SIZELINE-SIZENAME)
				{
				printf("%s\n", line);
				strcpy(line, "");
				}
			}

		if (strlen(line) > 0)
			printf("%s\n", line);
		}

	free(list);
	}

/*
 * Directory(dirname)
 *
 * Read a directory and return a pointer to a valid directory entry.  The
 * first time this routine is called, it must be given a directory path
 * name.  This directory is then opened.  Each subsequent time the routine
 * is called, name must be NULL.  A pointer to a static directory entry
 * is returned until the end-of-file is encountered.  At end-of-file,
 * NULL is returned, and the directory is closed.
 */
 struct direct *
directory(dirname)
	register char *dirname;
	{PROGRAM(directory)

	static struct direct dentry;
	static int fd = -1;
	register int rc;

	if (dirname != NULL)
		{
		if (fd != -1)
			close(fd);
		if ((fd=open(dirname)) == -1)
			{
			/* <dirname>: perror() message */
			error(ER7, dirname);
			return(NULL);
			}
		}
	else
		{
		if (fd == -1)
			return(NULL);
		}

	/*
	 * read the directory
	 */
	while (TRUE)
		{

		if ((rc=read(fd,(char*)&dentry,sizeof(dentry))) == -1)
			{
			/* <filename>: perror() message */
			error(ER7, filename(fd));
			goto exit;
			}

		if (rc != sizeof(dentry))
			{
			if (rc != 0)
				/* <filename>: truncated read */
				error(ER49, filename(fd));
			goto exit;
			}

		if (dentry.d_ino != 0)
			return(&dentry);
		}

exit:	close(fd);
	fd = -1;
	return(NULL);
	}

/*
 * Coff_section(fname)
 *
 * Read the section headers of an object file.  The first time this routine
 * is called, it must be given a path name for the object file.  Each
 * subsequent time the routine is called, fname must be NULL.  A pointer
 * to a static section header is returned.  When there are no more
 * section headers in the object file, NULL is returned and the file is closed.
 *
 * Any section marked NOLOAD is ignored.
 */
 SCNHDR *
coff_section(fname)
	register char *fname;
	{PROGRAM(coff_section)

	static SCNHDR shdr;
	static int fd = -1;
	static unsigned short nscns;
	FILHDR fhdr;

	ON(IOERROR)
		goto exit;

	if (fname != NULL)
		{
		if (fd != -1)
			close(fd);
		if ((fd=open(fname)) == -1)
			{
			/* <fname>: perror() message */
			error(ER7, fname);
			return(NULL);
			}
		read_and_check(fd, (char*)&fhdr, FILHSZ);
		if (fhdr.f_magic != FBOMAGIC || (nscns=fhdr.f_nscns) == 0)
			{
			/* <fname>: invalid object file */
			error(ER58, fname);
			goto exit;
			}
		seek_and_check(fd, (long)fhdr.f_opthdr, 1);
		}
	else
		{
		if (fd == -1)
			return(NULL);
		}

	/*
	 * read the section headers
	 */
	while (nscns-- != 0)
		{

		read_and_check(fd, (char*)&shdr, SCNHSZ);

		if (! (shdr.s_flags & (STYP_NOLOAD|STYP_DSECT)))
			return(&shdr);
		}

exit:	close(fd);
	fd = -1;
	return(NULL);
	}

/*
 * Coff_symbol(fname, number)
 *
 * Read the symbol table of an object file.  The first time this routine
 * is called, it must be given a path name for the object file.  Each
 * subsequent time the routine is called, fname must be NULL.  A pointer
 * to a static symbol table entry is returned.  When there are no more
 * symbols in the object file, NULL is returned and the file is closed.
 *
 * Only symbols whose storage class is C_EXT or C_STAT are returned.  An
 * optional second parameter is a pointer to an long; if not NULL
 * then the index number of the symbol is returned.
 *
 * If the symbol is exactly SYMNMLEN characters long, then it will be
 * converted to a flexname in order to make it null-terminated.  If the
 * symbol name is a flexname, then the string table will be read.
 * The offset (n_offset) is modified to be relative to the symbol table
 * entry n_name[].  Thus, the symbol name is accessed by:
 *
 *		s = coff_symbol(...)
 *		if (s->n_zeroes)
 *			/* s->n_name[SYMNMLEN] is the null-terminated symbol
 *		else
 *			/* (s->n_name+s->n_offset) -> the symbol
 */
 SYMENT *
coff_symbol(fname, number)
	register char *fname;
	register long *number;
	{PROGRAM(coff_symbol)

	extern boolean	DebugMode;
	FILHDR fhdr;
	static FILE *stream = NULL, *string;
	static long nsyms, sequence, stringorigin;
	union auxent	auxentry;
	static struct
		{
		SYMENT syment;
		char flexname[256];
		}
		sentry;

	if (fname != NULL)
		{
		register fd1, fd2;

		if (stream != NULL)
			{
			close(fdclose(stream));
			close(fdclose(string));
			stream = NULL;
			}
		if ((fd1=open(fname)) == -1 || (fd2=open(fname)) == -1)
			{
			if (fd1 != -1)
				close(fd1);
			/* <fname>: perror() message */
			error(ER7, fname);
			return(NULL);
			}

		ON(IOERROR)
			{
exit2:			close(fd1);
			close(fd2);
			return(NULL);
			}

		read_and_check(fd1, (char*)&fhdr, FILHSZ);

		if (fhdr.f_magic != FBOMAGIC)
			{
			/* <fname>: invalid object file */
			error(ER58, fname);
			goto exit2;
			}
		if ((nsyms=fhdr.f_nsyms) == 0)
			goto exit2;

		sequence = -1;

		seek_and_check(fd1, fhdr.f_symptr, 0);
		stream = fdopen(fd1);

		stringorigin = fhdr.f_symptr + nsyms * SYMESZ;
		string = fdopen(fd2);
		}
	else
		{
		if (stream == NULL)
			return(NULL);
		}

	/*
	 * read the symbol table
	 */

	while (++sequence < nsyms) {

		if (fread((char*)&sentry.syment,SYMESZ,1,stream) == 0)
			{
			/* <filename>: perror() message */
			/* <filename>: truncated read */
			error(ferror(stream)? ER7 : ER49, filename(fileno(stream)));
			goto exit1;
			}

		if (number != NULL)
			*number = sequence;
		
		if (DebugMode  &&  sentry.syment.n_numaux > 0  &&
		   sentry.syment.n_sclass == C_TPDEF) {
			if (fread(&auxentry, AUXESZ, 1, stream) == 0) {
				/* <filename>: perror() message */
				/* <filename>: truncated read */

				error(ferror(stream)? ER7 : ER49,
				      filename(fileno(stream)));
				goto exit1;
			}
			sequence += 1;
			sentry.syment.n_numaux--;
			sentry.syment.n_value =
				auxentry.x_sym.x_misc.x_lnsz.x_size;
		}

		if (sentry.syment.n_numaux > 0) {
			if (fseek(stream,
			   (long)sentry.syment.n_numaux*AUXESZ,1) == -1)
				goto exit1;
			sequence += sentry.syment.n_numaux;
		}

		if (sentry.syment.n_sclass == C_EXT   ||
		   sentry.syment.n_sclass == C_STAT  ||
		   (DebugMode  &&  (sentry.syment.n_sclass == C_TPDEF  ||
				    sentry.syment.n_sclass == C_MOS    ||
				    sentry.syment.n_sclass == C_MOU)))
		{
			if (sentry.syment.n_zeroes) {
				/*
				 * not a flexname, but if its exactly SYMNMLEN
				 * characters long it must be converted to a
				 * flexname so that it is null terminated
				 */

				if (sentry.syment.n_name[SYMNMLEN-1]) {
					/* convert to a flexname */

					strncat(strcpy(sentry.flexname,""),
						sentry.syment.n_name,
						SYMNMLEN);
					sentry.syment.n_zeroes = 0;
					sentry.syment.n_offset =
						sentry.flexname -
						sentry.syment.n_name;
				}
			} else {
				/*
				 * flexname
				 */

				register int c, n = sizeof(sentry.flexname);
				register char *p = sentry.flexname;

				if (fseek(string,stringorigin+sentry.syment.n_offset,0) == -1)
					goto exit1;

				while (--n >= 0 && (c = getc(string)) != EOF)
					{
					if ((*p++ = c) == '\0')
						break;
					}

				if (n < 0)
					/* flexname too long */
					error(ER5);

				if (c == EOF)
					{
					/* <filename>: perror() message */
					/* <filename>: truncated string table*/
					error(ferror(string)? ER7 : ER59, filename(fileno(string)));
					goto exit1;
					}

				sentry.syment.n_offset = sentry.flexname - sentry.syment.n_name;
			}

			return(&sentry.syment);
		}
	}

exit1:	close(fdclose(stream));
	close(fdclose(string));

	stream = NULL;

	return(NULL);
	}

/*
 * Edtscan(base, lba, function)
 *
 * Search the EDT at *base, calling function() for each EDT entry.  If the
 * entry is a LBE, then recursively scan its EDT.
 *
 * Function() is called with three parameters: &edt_entry, lba, elb.
 *
 *			 lba	 elb
 *			----	----
 *	non-LBE device	  0	3-15
 *	LBE device	3-15	1-15
 */
 void
edtscan(base, lba, function)
#ifdef u3b5
	register struct edts *base;
#endif
#ifdef u3b2
	register struct edt *base;
#endif
	register int lba;
	register int (*function)();
	{PROGRAM(edtscan)

	register struct edt *edtp;
	register i;
	int elb;

#if u3b2
	for (i=0, edtp=base; i < NUM_EDT; ++i)
		{
		elb = edtp->opt_slot;

		(*function)(edtp++, lba, elb);
		}
#endif

#if u3b5
	edtp = base->edtx;

	for (i=0; i<base->number; ++i, ++edtp)
		{
		elb = edtp->bd_code & 0x7F;

		(*function)(edtp, lba, elb);

		/*
		 * if this is a LBE, then scan its EDT now
		 */
		if (0 == strcmp("LBE",edtp->dev_name))
			{
			if (lba != 0)
				panic("LBE found on ELB");

			edtscan((struct edts *)(edtp->lb_baddr | (long)EDT_START), elb, function);
			}
		}
#endif
	}

/*
 * Read and seek routines with error checking.  Condition IOERROR is signalled
 * if an I/O error occurs.
 */

CONDITION IOERROR ={ "IOERROR" };


/*
 * Read with error checking; if an I/O error occurs, condition IOERROR is
 * signalled.
 */
 void
read_and_check(fildes, buf, nbyte)
	int fildes;
	char *buf;
	register unsigned nbyte;
	{PROGRAM(read_and_check)

	register int rbyte;

	if ((rbyte = read(fildes,buf,nbyte)) == -1)
		{
		/* <filename>: perror() message */
		error(ER7, filename(fildes));
		SIGNAL(IOERROR);
		}
	else
		if (rbyte != nbyte)
			{
			/* <filename>: truncated read */
			error(ER49, filename(fildes));
			SIGNAL(IOERROR);
			}
	}




/*
 * Lseek with error checking; if an I/O error occurs, condition IOERROR is
 * signalled.
 */
 void
seek_and_check(fildes, foffset, whence)
	int fildes;
	long foffset;
	int whence;
	{PROGRAM(seek_and_check)

	if (lseek(fildes,foffset,whence) == -1L)
		{
		/* <filename>: perror() message */
		error(ER7, filename(fildes));
		SIGNAL(IOERROR);
		}
	}

#ifdef u3b2
/*
 * Bzero(addr, size)
 *
 * Set `size' bytes of memory beginning at `addr' to zeros
 */
 void
bzero(addr, size)
	register caddr_t addr;
	register size;
	{PROGRAM(bzero)

	while (size-- > 0)
		*addr++ = 0;
	}
#endif

/*
 * Sreset()
 *
 * Reset the values of all variables in this file.  This is necessary to
 * clean-up after a failed boot attempt.
 */
 void
sreset()
	{PROGRAM(sreset)

	if (loadmap)
		{
		free((char*)loadmap);
		loadmap = NULL;
		}
	}
