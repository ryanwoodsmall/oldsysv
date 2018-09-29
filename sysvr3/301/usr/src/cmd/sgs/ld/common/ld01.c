/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/ld01.c	1.25"
#include "system.h"
#include <stdio.h>
#include <signal.h>
#include "structs.h"
#include "extrns.h"

#if TRVEC
#include "tv.h"
#include "ldtv.h"
#endif

#include "list.h"
#include "sgsmacros.h"
#include "instr.h"
#include "ldmacros.h"

#define STRINC 256
/*eject*/
pass2()
{

/*
 * This is the MAIN procedure for PASS 2 of ld
 *
 * PASS 2 is the execution pass:
 *	1. The control information extracted during PASS 1 is read in
 *	2. Control blocks are constructed, containing the information
 *		extracted during PASS 1
 *	3. Allocation is performed
 *	4. The output file is generated
 */

/*
 * Initialize PASS 2
 */

	initpss2();

/*
 * undefine() is now in ld.lex
 */

/*
 * Apply, if any, the default SECTIONS directives
 */

	dfltsec();

	pmemreg();		/* process memlist and reglist */
#if TRVEC 
	if (tvflag)		/* look for references to functions */
		tvreloc();	
#endif
#if COMMON
	commspec();		/* process COMMON specifications */
	if (aflag)
		pf77comm();	/* process common data for F77 */
#endif
	pboslist();		/* process BLDOUTSEC list */
	split_scns();		/* break up outscts if they are too big */
	alloc();		/* perform output section allocation */
	if (Bflag)
		add_pad();	/* add padding sections */
#if ILDOPT
	if (ildflag)		/* add extra sections */
		add_extra();
#endif
	updatsms();		/* complete section hdrs & relocate symbols */
	pexlist();		/* process expression list */


/*
 * Output a memory map if requested:
 *	1. The -m flag was set
 *	2. There were no .o or archive files in the ld run
 */

	if( mflag )
		ldmap();

	output();	/* write output object file */

#if PERF_DATA
	instrment();
#endif

	exit(0);
}



#if PERF_DATA
instrment()
{
	register FILE *instrfile;
	long times();

/*
 * Complete the collection of the timing statistics
 */

	ttime = times(&ptimes) - ttime;

/*
 * Determine if any statistics are to be output
 */

	if( (instrfile = fopen("instr.data", "r")) == NULL )
		return;
	else
		fclose(instrfile);

	if( (instrfile = fopen("instr.data", "a")) == NULL ) {
		lderror(0,0,NULL,"Can't open 'instr.data'");
		return;
		}
	
/*
 * Output the ld run statistics
 */

	fprintf( instrfile, "%07ld %07ld %07ld %07ld %07ld %07ld %07ld\n",
		noutsyms, numldsyms, numldaux, nwalks, nfwalks, maxchain, 
		ncolisns );
	fprintf( instrfile, "%07ld %07ld %07ld %07ld %07ld\n",
		ttime, ptimes );
	fprintf( instrfile, "%07ld\n", allocspc );

	fclose(instrfile);
}
#endif



initpss2()
{

	int ldexit();

/*
 * Initialize the process time counters
 */



/*
 * Set the following signals to be caught, unless ld is running in the
 * background. In this case, leave them set at "ignore"
 */

	if( signal(SIGINT, ldexit)  ==  SIG_IGN )
		signal(SIGINT, SIG_IGN);
	if( signal(SIGHUP, ldexit)  ==  SIG_IGN )
		signal(SIGHUP, SIG_IGN);
	if( signal(SIGQUIT, ldexit)  ==  SIG_IGN )
		signal(SIGQUIT, SIG_IGN);

#if TRVEC
	if (tvflag && aflag)
		tvspecdef();	/* complete definition of tvspec */
#endif

}
/*eject*/
ldexit()
{

/*
 * Clean up and exit after a fatal error
 */

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_DFL);

/*
 * Unlink the a.out file, to cause its removal
 */

	unlink( outfilnm );

	exit(13);  /* unlucky break */
}
/*eject*/
undefsm(name)
char *name;
{

/* 
 *  Create a symbol table entry for an undefined symbol
 *  ( which may get defined later).
 */

	SYMENT sym;

	zero( (char *) &sym, SYMESZ );

#if FLEXNAMES
	if (strlen(name) > 8) {
		sym.n_zeroes = 0L;
		sym.n_nptr = name;
		}
	else
#endif
		copy( sym.n_name, name, 8);
	sym.n_sclass = C_EXT;

	makesym(&sym, NULL);
}
