/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:incount.c	1.1"
/*
	subroutine to increment a number stored in a countfn, and return it.
 */
#include	<stdio.h>
#include	<signal.h>
#define	Maxcount	(20)

static char	*lockfile;
static FILE	*lockfp	= NULL;
static int	(*sighup)(), (*sigint)(), (*sigterm)();

int
incount(countfn, lockfn, modulus)
char    *countfn, *lockfn;
int	modulus;
{
	unsigned	sleep();
	unsigned	count;
	FILE	*fopen(), *countfp;
	int	catch();

	/* assign lockfn to the global lockfile for use by "catch()" */
	lockfile = lockfn;

	/* catch and handle interupts */
	sighup = signal(SIGHUP, catch);
	sigint = signal(SIGINT, catch);
	sigterm = signal(SIGTERM, catch);

	/* create a lock file for this process  */
	umask(0777);
	for( count = 0;
		count <= Maxcount  &&  (lockfp = fopen(lockfn,"w")) == NULL;
		sleep(++count) )
		;
	if (count > Maxcount) {
		catch(0);
		return  -1;
	}
	fclose(lockfp);

	/* read the old number from the file, increment, and return it */
	if ((countfp = fopen(countfn, "r")) == NULL) { 
		catch(0);
		return  -2;
	}
	fscanf(countfp, "%d", &count);
	fclose(countfp);
	if ((countfp = fopen(countfn, "w")) == NULL) { 
		catch(0);
		return  -3;
	}
	count = (count + 1 ) % modulus;
	fprintf(countfp, "%d\n", count);

	fclose(countfp);
	catch(0);
	return  count;
}


static
catch(sig)
int	sig;
{
	/* remove the lockfile */
	if ( lockfp )
		unlink(lockfile);

	/* restore signals */
	signal(SIGHUP, sighup);
	signal(SIGINT, sigint);
	signal(SIGTERM, sigterm);

	/* if interupted, reissue the signal so the the original signal()
	can catch it */
	if ( sig )
		kill( getpid(), sig );

	return;
}
