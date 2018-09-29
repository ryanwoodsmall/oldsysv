/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/main.c	1.11"

/*
 *	UNIX debugger
 */

#include	"head.h"
#include	<fcntl.h>
#include	<sys/utsname.h>

extern INT		wtflag;
extern INT		mauflag;
extern INT		mkfault;
extern INT		executing;
extern ADDR		sigint;
extern ADDR		sigqit;
int	fpe();

main(argc, argv)
REG STRING	*argv;
REG INT		argc;
{
	FILE	*fp;
	FILE	*fopen();
	register char *p;
	register int xflag;
	register char *name;
	struct stat stbuf;
	struct utsname sysinfo;

#if u3b2
	uname(&sysinfo);
#if SWAPPING
	if (strncmp(sysinfo.release,"2.0",3)!=0)
	{
		fprintf(FPRT1, "Cannot use this sdb on a paging system.\n");
		exit(1);
	}
#else
	if (strncmp(sysinfo.release,"2.0",3)==0)
	{
		fprintf(FPRT1, "Cannot use this sdb on a swapping system.\n");
		exit(1);
	}
#endif
#endif
	/* use unbuffered output */
	setbuf(stdout, NULL);		/* EGB wants this deleted. Leave it */
	setbuf(stderr, NULL);

	name = argv[0];
	argc--, argv++;
	while (argc>0 && argv[0][0] == '-' && argv[0][1] != '\0')
	{   
		for (p = &argv[0][1]; *p; p++)
		{
			switch (*p)
			{
			case 's':
				sflag++;
				break;
			case 'x':
				xflag++;
				break;
			case 'w':
				wtflag = 2;  /* allow write to files,ISP */
				break;
			case 'd':
				debugflag++;
				break;
			case 'W':
				Wflag++;
				break;
			default:
				fprintf(FPRT1, "%s: unknown flag %c\n",
					name, *p);
				exit(4);
			}
		}
		argc--, argv++;
	}

	if ( argc > 0 ) symfil = argv[ 0 ];
	if ( argc > 1 ) corfil = argv[ 1 ];

	/* Put fwp at beginning of filework */
	fwp = filework;
	if ( argc > 2 )
	{
		strncpy( sdbpath, argv[ 2 ], sdbpsz );
		if( isfile( sdbpath ) )
		{
			if( ( fp = fopen( sdbpath, "r" ) ) != NULL )
			{
				fgets( sdbpath, sdbpsz, fp );
				sdbpath[ strlen( sdbpath ) -1 ] = 0;
				fclose( fp );
			}
			else
			{
				fprintf( "Can't open pathfile: %s\n", sdbpath );
				exit( 2 );
			}
		}
	}
	if( sdbpath[ 0 ] )
	{
		printf( "Source path: " );
		for( p = sdbpath; *p; p++ )
		{
			printchar(*p);	/* user will see if junk is in file */
		}
		printf("\n");
		argc--;
		argv++;
	}
	argcount = argc;

	if ( symfil[ 0 ] == '-' )	/* name "-" ==> ignore a.out */
	{
		fprintf( FPRT1, "Warning: `%s' does not exist\n", symfil );
	}
	else
	{
		if ( stat( symfil, &stbuf ) == -1 )
		{
			fprintf(FPRT1, "`%s' does not exist\n", symfil);
			exit(4);
		}
	}

	/* Check that core file is newer than symbol file */
	symtime = stbuf.st_mtime;
	if ( stat( corfil, &stbuf ) != -1 )
	{
		if ( symtime > stbuf.st_mtime )
		{
			fprintf(FPRT1, "Warning: `%s' newer than `%s'\n",
				symfil, corfil);
		}
	}

	/* initialize sdb data structures */
	setsym();
	readstrtbl( fsym );
	setcor();
	initfp();

	/* set up variables for user */
	ioctl( FDTTY, TCGETA, &sdbttym );   /* save initial state of terminal */
	sdbttyf = fcntl( FDIN, F_GETFL, 0 ); /* save initial status bits */

	/* if user was ignoring interrupts, fine; else trap to fault() */
	if ( (sigint = ( ADDR ) signal( SIGINT, SIG_IGN ) ) != ( ADDR ) SIG_IGN)
	{
		sigint = ( ADDR ) fault;
		signal( SIGINT, fault );
	}
	sigqit = ( ADDR ) signal( SIGQUIT, SIG_IGN ); /* ignore "quit" signal */
	signal( SIGILL, fpe );	/* fpe() handles illegal instructions */

	sdbenter(xflag);

	setjmp(env);

#if u3b2 || u3b5 || u3b15
	if (!mauflag)		/* mauflag == -1 system has no MAU */
		setmauflg();	/* mauflag ==  1 system has a MAU  */
#endif				/* mauflag ==  0 mauflag not set   */

	sdbtty();	/* save user tty modes and restore sdb tty modes */

	if (executing)
	{
		delbp();
		executing = FALSE;
	}

	/* print prompt (*), read user command, and execute it */
	for (;;) {
		mkfault = 0;
		printf("*");
		if (decode(readline(inputfile)) == 1) {
			fprintf(FPRT1, "Error; Try again\n");
			continue;
		}

		docommand();
	}
}


fault(a)
int a;
{
	signal(a,fault);	/* cancel pending signal "a" */
	mkfault++;
	if(pid)
		delbp();
	printf("\n");
	fflush(stdout);
	setbuf(stdout, NULL);	/* EGB deleted this I'm leaving it in */
	longjmp(env, 0);
}

fpe() {
	signal(SIGILL, fpe);	/* cancel pending signal SIGILL */
	error("Illegal floating constant");
	longjmp(env, 0);
}

isfile( s )
char	*s;
{
	struct stat	sbuf;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("isfile");
	}
	else if (debugflag == 2)
	{
		enter2("isfile");
		arg("s");
		printf("\"%s\"",s);
		closeparen();
	}
#endif
	if( stat( s, &sbuf ) == -1 )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("isfile");
			printf("0x%x",0);
			endofline();
		}
#endif
		return( 0 );
	}
	if( ( sbuf.st_mode&S_IFMT ) == S_IFREG )
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("isfile");
			printf("0x%x",1);
			endofline();
		}
#endif
		return( 1 );
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("isfile");
		printf("0x%x",0);
		endofline();
	}
#endif
	return( 0 );
}
