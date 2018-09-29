/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libPW:curdir.c	3.13"

/*	Find the current directory and stuff its name into the given string.
	Returns 0 upon success and -1 for failure.
	The string is not modified unless the function is successful.

	Note:  Unlike previous versions of this function, it does not do
	chdir() calls, thus avoiding potential problems.
*/

#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>

static	char	*dotdot,
		*dotend = 0,
		*dottoofar;

static char	*finddir();
static char	*stradd();


curdir( str )
char	*str;
{
	char	buf[ 1024 + MAXNAMLEN ];	/* room for over 300 levels! */
	struct stat	statbuf;

	dotend = dotdot = buf;
	dottoofar = &buf[ sizeof buf ];
	if( stat( ".", &statbuf ) == -1 )
		return  -1;
	return  finddir( str, &statbuf ) ? 0 : -1 ;
}


/*	finddir() is recursively called to climb towards the file system root.
	At each step, it collects the next name to be remembered.
	When it gets to the root, it returns through all the recursions,
	adding the / and the name to the string at each level.
*/
static
char *
finddir( str, sbp )
char	*str;
struct stat	*sbp;		/* The directory whose name we are to find. */
{
	DIR		*cdp;
	struct dirent	*cdirent;
	struct stat	cdirectory;	/* current directory stat */
	struct stat	centry;		/* current entry stat */
	char		name[ MAXNAMLEN ];
	int		pass;

	dotend = stradd( dotend, "../" );
	if( !(cdp = opendir( dotdot )) )
		return  0;
	if( fstat( cdp->dd_fd, &cdirectory ) == -1 )
		return  0;

	/*	When .. is identical to the directory we are looking for, we
		are at the root.  Start returning up the stack of finddir()s.
	*/
	if( identical( &cdirectory, sbp ) ) {
		stradd( str, "/" );
		return  str;
	}

	/*	Inspect each entry in the directory for one identical to the
		directory immediately below.
		The stat() inside this loop is very expensive, so we reject
		many of the directory entries on the basis of ino alone.
		However, when crossing the mount point diretories, the stat()
		is necessary, so we try a second time if we don't find a match
		relying only on the values returned by stat().
	*/
	for( pass = 1;  pass <= 2;  rewinddir( cdp ), pass++ ) {
		while( cdirent = readdir( cdp ) ) {
			if( pass == 1  &&  cdirent->d_ino != sbp->st_ino )
				continue;
			if( dotend + strlen( cdirent->d_name ) >= dottoofar )
				return  0;
			stradd( dotend, cdirent->d_name );
			if( stat( dotdot, &centry ) == -1 )
				return  0;
			if( identical( &centry, sbp ) ) {
				closedir( cdp );
				stradd( name, cdirent->d_name );
				if( str = finddir( str, &cdirectory ) ) {
					*(str++) = '/';
					return  stradd( str, name);
				}
				return  0;
			}
		}
	}
	closedir( cdp );
	return  0;
}



/*
 * Add s2 to s1.  S1's space must be large enough.
 * Return the address of the last character written, i.e. the '\0' character.
 */

static
char *
stradd(s1, s2)
register char *s1, *s2;
{
	while(*s1++ = *s2++)
		;
	return(--s1);
}


/*	Do the stat structures represent the same file?
*/
static
int
identical( s1, s2 )
struct stat	*s1, *s2;
{
	return  s1->st_ino == s2->st_ino  &&  s1->st_dev == s2->st_dev;
}
