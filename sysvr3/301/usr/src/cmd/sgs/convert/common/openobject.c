/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)convert:common/openobject.c	1.6"

#include	<stdio.h>
#include	<ar.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"filehdr.h"
#include	"conv.h"
#include	"ldfcn.h"

char	outfile[15];
FILE	*outstrm;

int			swab,
			fromto;
extern int		preswab;
extern unsigned short	target,
			origin;

LDFILE	*
openobject(infile)

char	*infile;

{
	extern	char	*sname( );

	extern LDFILE	*ldopen( );
	extern int	ldaclose( );

	LDFILE		*objptr;
	FILE		*tmpfd;
	struct stat	statbuf;

	if ( (tmpfd = fopen(infile, "r")) == NULL ) {
		converr(1, infile, "cannot open");
		return(NULL);
	} else {
		fclose(tmpfd);
		}

	if ( (objptr = ldopen(infile, NULL)) == NULL ) {
		converr(1,infile,"trouble reading (not an object file?)");
		return(NULL);
		} 

	if ( origin == target ) {
		converr(0,infile,"needs no conversion");
		if ( creatfil(infile) != SUCCESS )
			return(NULL);
		rewind(IOPTR(objptr));
		fstat( fileno(IOPTR(objptr)), &statbuf);
		copystrms(objptr, outstrm, statbuf.st_size);
		fclose(outstrm);
		ldaclose(objptr);
		return(NULL);
		}

	if ( origin != F_CURARCH && target != F_CURARCH ) {
		converr(1,infile,"neither origin nor destination are host");
		ldaclose(objptr);
		return(NULL);
		}


	if ( ! preswab && ((ISDEC(origin) && ISNONDEC(target))
		|| (ISNONDEC(origin) && ISDEC(target))) &&
			(  IS3B(HEADER(objptr).f_magic)
			 || ISMC68(HEADER(objptr).f_magic)
			 )) {
		/* for 3B or MC68 SGS, we want to swab
		 * everything, including .text,
		 * .data, etc when going from
		 * DEC to non-DEC
		 * or non-DEC to DEC
		 */

		swab++;
		}

	if ( preswab && ( ! IS3B(HEADER(objptr).f_magic) || ! ISDEC(origin)
		|| ! ISNONDEC(target) ) ) {
		/* user intends to swab-dump 3B object file from DEC to 3B
		 * but it is inappropriate in this context
		 */

		converr(0,"*command line*", "-s invalid in this context...ignored");
		preswab = 0;
		}

	if ( preswab && ( FLAGS(objptr) & F_SWABD ) ) {
		converr(0,"*command line*", "-s invalid, file already swabbed");
		preswab = 0;
		}

	/*
	 * set up variable for ease of decision in
	 * shortconv and longconv (not needed elsewhere)
	 *
	 * fromto is to the real conversion process what
	 *	cnvtyp is to the preliminary process
	 */

	if ( ISDEC(origin) && ISDEC(target) )
		fromto = PDPVAX;
	else
		if ( origin == F_AR32WR || target == F_AR32WR )
			fromto = VAXIBM;
		else
			fromto = PDPIBM;

	/*
	 * create new file
	 */

	if ( creatfil(infile) != SUCCESS )
		return(NULL);
	else
		return(objptr);
}

/*
	Returns pointer to "simple" name of path name; that is,
	pointer to first character after last "/".  If no slashes,
	returns pointer to first char of arg.
	If the string ends in a slash, returns a pointer to the first
	character after the preceeding slash, or the first character.
*/

char	*
sname(s)

char *s;
{
	register char *p;
	register int n;
	register int j;

	n = strlen(s);
	--n;
	if (s[n] == '/') {
		for (j=n; j >= 0; --j)
			if (s[j] != '/') {
				s[++j] = '\0';
				break;
			}
	}

	for(p=s; *p; p++) if(*p == '/') s = p + 1;
	return(s);
}

/*
 * create output file
 */

creatfil( infil )

char	*infil;
{
	int	index;

	strncpy(outfile,sname(infil), 14 );
	index = (strlen(outfile) <= 12 ? strlen(outfile) : 12);
	strcpy(outfile+index,".v");
	if (strlen( outfile ) == 14) 
		if ((outstrm = fopen( outfile, "r")) != NULL) {
			converr( 0, outfile, "file name may be non-unique");
			fclose( outstrm );
		}

	if ( (outstrm = fopen(outfile,"w")) == NULL ) {
		converr(1,outfile,"cannot create");
		return(FAILURE);
		}
	return(SUCCESS);
}
