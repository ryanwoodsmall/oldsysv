/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/srciofcns.c	1.9"
/* UNIX HEADER */
#include	<stdio.h>
#include	<ctype.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"linenum.h"
#include	"ldfcn.h"
#if DMERT
#include	"sgsmacros.h"
#endif

/* LISTER HEADER */
#include	"defs.h"

#if DMERT
/* SGS SPECIFIC HEADERS */
#include	"sgs.h"
#include	"paths.h"
#endif

/* EXTERNAL VARIABLES DEFINED */
char			buffer[MAXLINE] = "";
int			bufndx = 0;
int			nif = 0;	/* inside ifdef */
int			nfunc = 0;	/* inside function */
unsigned short		linecount = 0;
short			inif = 0;

/* STATIC VARIABLE USED */
static char		linenum[8] = "";

    /*  srciofcns.c contains a pair of functions that read the source
     *  and write out the source listing
     *
     *  linenum is the character string containing the source line number
     *  it is set by printfcn( ) just before the line in question in printed,
     *  and it is always reset to the null string by getline( )
     *
     *  getline( )
     *
     *  reads and writes a line of source in buffer
     *  increments linecount to keep track of the lines it has read
     *
     *  resets bufndx to 0 each time it reads a new line
     *
     *  returns !EOF when it reads a new source line into buffer
     *  returns EOF at end-of-file
     *
     *  although it increments linecount for preprocessor statements (lines
     *  beginning with a '#')  it does not return to the calling function
     *  until it reads a regular old line
     */


int
getline( )

{
    /* UNIX FUNCTIONS CALLED */
    extern			printf( );
    extern char			*fgets( );
   extern char 			*strpbrk();

    /* EXTERNAL VARIABLES USED */
    extern FILE			*srcptr;
    extern char			*objname;

    extern char			buffer[ ];
    extern int			bufndx;
    extern unsigned short	linecount;
    extern int			fflag;
	 char			*ptr1;

    	extern int		ok_to_print;

#if DMERT
	extern	char		Oname[];
	FILE			*fd;
	extern FILE		*vfopen( );
	extern	char		*dname();
	char			temp[BUFSIZ/2];
	extern	char	 	*simnam();
	extern	char		srcname[];
	FILE			*fd2;
	char			c;
	int			found;
	char			Sname[BUFSIZ/2];
	char			pname[BUFSIZ/2];
	char			sysbuf[BUFSIZ/2];
	int			i;
	char			filnam[20];
#endif

	static int		prev_ok = 0;
	static char		prev_buf[BUFSIZ];
	static int		prev_ndx = 0;

	int			tmpndx,
				tmpndx2;

	if ( fflag == 0 )
		prev_ok = 1;

	if ( ok_to_print && !prev_ok ) {
	    /*
	     * we are now allowed to print but it is too late;
	     * we must bactrack to the declaration of the function
	     * and print from there
	     */
	    if ( prev_ndx == 0 ) {
		printf("\n");
	    } else {
		prev_buf[prev_ndx] = '\0';
		while ( --prev_ndx )
		    if ( prev_buf[prev_ndx] == FUNCEND || prev_buf[prev_ndx] == '}' )
			break;
		printf("%s",&prev_buf[prev_ndx+1]);
		prev_ndx = 0;
	    }
	}

	if ( bufndx == 0 )
	{
		if ( ok_to_print )
		{
			prev_ok = 1;
			printf("\n");
		}
		else if ( prev_ok )
		{
			printf("\n");
			prev_ok = 0;
		}
		else
		{
			if ( prev_ndx >= BUFSIZ )
			{
				tmpndx = 0;
				tmpndx2 = 10;
				for ( ; tmpndx2 < prev_ndx; ++tmpndx,++tmpndx2)
					prev_buf[tmpndx] = prev_buf[tmpndx2];
				prev_ndx = tmpndx;
				prev_buf[tmpndx] = '\0';
			}
			prev_buf[prev_ndx++] = '\n';
		}
	}
	else
	{
		if ( ok_to_print )
		{
			prev_ok = 1;
			printf("%7.7s\t%s", linenum, buffer);
		}
		else if ( prev_ok )
		{
			prev_ok = 0;
			printf("%7.7s\t%s", linenum, buffer);
		}
		else
		{
			if ( prev_ndx + 8 + strlen(buffer) >= BUFSIZ )
			{
				tmpndx = 0;
				tmpndx2 = strlen(buffer) + 10;
				for ( ; tmpndx2 < prev_ndx; ++tmpndx,++tmpndx2)
					prev_buf[tmpndx] = prev_buf[tmpndx2];
				prev_ndx = tmpndx;
				prev_buf[tmpndx] = '\0';
			}
			sprintf(&prev_buf[prev_ndx],"%7.7s\t%s", linenum, buffer);
			prev_ndx += 8 + strlen(buffer);
		}
		linenum[0] = '\0';
		bufndx = 0;
	}

    while(fgets(buffer, MAXLINE, srcptr) != NULL) {
	++linecount;
	if (buffer[0] == '#') {
	    for(;;) {
		switch (buffer[bufndx]) {
		    case '\n':
			if ( ok_to_print )
			    printf("       \t%s",buffer);
			break;
		    case '/':
			if (buffer[bufndx + 1] == '*') {
			    bufndx += 2;
			    if (endcomment() == EOF) {
				return(EOF);
			    }
			    if ( ok_to_print )
				printf("\t%s",buffer);
			    break;
			}

 
		    case 'e':
			if (strncmp( &buffer[bufndx], "endif", 5 ) == 0) {
				inif--;
				bufndx += 5;
				continue;
			}

		    case 'i':
			if ( strncmp( &buffer[bufndx], "if", 2 ) == 0) {
				inif++;
				bufndx += 5;
				continue;
			}
#if DMERT
/*begin fix*/
			if (strncmp(&buffer[bufndx], "include", 7) == 0) {
			    	if ( ok_to_print )
					printf("\t%s",buffer);
				if ( (i=index(buffer,"<")) != -1 ) {
					buffer[i+1+index(buffer+i+1,">")]=NULL;
				} else {
					i = index(buffer,"\"");
					buffer[i+1+index(buffer+i+1,"\"")]=NULL;
				}

			    	sprintf(filnam, "%s", buffer+i+1);
			    	sprintf(Sname,"%s.S",Oname);
			    	found = 0;
			    	if ( (fd = vfopen(Sname,"r")) != NULL ) {
			        	while (fscanf(fd,"%s\n",pname) == 1) {
			                	if ( strcmp(simnam(pname),filnam)==0)  {
							found = 1;
							break;
						}
					}
					fclose(fd);
				}
				if ( found != 1 && index(buffer,".c") == -1)
					break;
				if ( found != 1 ) {
					sprintf(temp,"%s",Oname);
					sprintf(pname,"./%s/%s",dname(temp),
							filnam);
				}
				if ( (fd=vfopen(pname,"r")) == NULL ) {
		                	printf("\\@@SOURCEFILE: %.14s not found\n", filnam);
				} else {
			        	printf("\n\\@@SOURCEFILE: %s\n\n", pname);
					sprintf(sysbuf, "%s -h -t%s -O%s %s %s",
						LIST, simnam( srcname ), Oname, pname, objname);
					fflush(stdout);
					system(sysbuf);
			        	printf("\n\\@@SOURCEFILE: %s\n\n", srcname);
				}
				break;
			}
/*end fix*/
#endif

		    default:
			++bufndx;
			continue;
		}
		bufndx = 0;
		break;
	    }
	} else {
	    return(!EOF);
	}
    }

    return(EOF);
}




    /*  printfcn(filename, fcndx)
     *
     *  determines when to print a source line number
     *
     *  resets linecount to 1 at the curly brace ({) opening a function
     *  sets linenum to the line number just before the appropriate line
     *  is printed
     *
     *  calls
     *    - countbrace(lnno, bracecount)
     *        to keep track of curly braces
     *        countbrace returns when linecount == lnno or when bracecount == 0
     *
     *  returns SUCCEED when it has gone through all the line numbers in a
     *  function and ended at the closing curly brace
     *
     *  calls error(file, string) if the closing curly brace does not
     *  coincide with the last line number (and returns FAIL)
     */

int
printfcn(filename, fcndx)

char		*filename;
long		fcndx;

{
    /* UNIX FUNCTIONS CALLED */
    extern			sprintf( );

    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern int			ldlinit( ),
				ldlitem( );

    /* LISTER FUNCTIONS CALLED */
    extern int			countbrace( );
    extern			error( );

    /* EXTERNAL VARIABLES USED */
    extern LDFILE		*ldptr;
#if DMERT
    extern char			*objname;
#endif
    extern unsigned short	linecount;

    /* LOCAL VARIABLES */
    int				bracecount;
    unsigned short		lnno;
    LINENO			line;
    int				seenzero;


    if (ldlinit(ldptr, fcndx) != SUCCESS) {
	if (inif)
		return( NOTFOUND );
	else
		fatal( "missing or inappropriate line numbers" );
    }

    lnno = 0;
    linecount = 1;
    bracecount = 1;

    seenzero = 0;

    while (ldlitem(ldptr, ++lnno, &line) == SUCCESS) {

	lnno = line.l_lnno;

	if ((bracecount = countbrace(lnno, bracecount)) == EOF) {
	    error(filename, "unexpected end-of-file" );
	    return(FAIL);
	}

	sprintf(linenum, "[%d]", lnno);

	if (bracecount == 0) {
	/* CHECK TO MAKE SURE THAT THE LINE NUMBERS ARE IN SYNC:
	 * first:  linecount (at the closing curly) has to equal
	 *	   the last line number read (lnno)
	 * second:  the largest line number for the function should 
         *	    have been read
	 */
	    seenzero = 1;		/* countbrace has hit 0 now */
	    if ( linecount < lnno ) {
/*begin fix*/
		while ( linecount < lnno  && getline() != EOF);
		continue;
	    }
/*end fix*/
	}
    }

/*add logic*/
	/*
	 * we may have seen bracecount <= 0 before and ignored it
	 * due to ifdefs; therefore see if we have;  if so, then
	 * assume we are at end of function even though bracecount != 0
	 */
	if ( seenzero == 1 ) {
		bracecount = 0;
		return(SUCCEED);
		}
    /*	at this point the last line number entry has been read
     *	and braceoount != 0
     */

	/* see if ifdefs are screwing us up again
	 */

/*begin fix*/
	if ( index(buffer, "}") != -1 ) {
		bracecount = 0;
		return(SUCCEED);
		}
/*end fix*/
    if ((bracecount = countbrace(lnno + 1, bracecount)) != 0) {
/*begin fix*/
	if ( endfunc(bracecount) != SUCCEED ) {
		error(filename, " inappropriate line numbers" );
		return(FAIL);
		}
    }

    return(SUCCEED);
}

endfunc( bracecnt )

int	bracecnt;
{

	int	oldndx;

	while ( bracecnt > 0 ) {
		if ( getnonblank() == EOF)
			return(FAIL);

		oldndx = bufndx;
		++bufndx;

		if ( isalnum((int)buffer[oldndx])) {
			for ( ; isalnum((int)buffer[bufndx]); ++bufndx);
		} else {
			switch (buffer[oldndx]) {
				case '\'':
				case '"':
					if (endquote(buffer[oldndx]) == EOF) {
						return(FAIL);
					}
					break;
				case '{':
					++bracecnt;
					break;
				case '}':
					--bracecnt;
					if ( bracecnt == 0 )
						return(SUCCEED);
					break;
				default:
					break;
				}
			}
		}
	return(FAIL);
}

/*
	If `s2' is a substring of `s1' return the offset of the first
	occurrence of `s2' in `s1',
	else return -1.
*/

index(as1,as2)
char *as1,*as2;
{
	register char *s1,*s2,c;
	int offset;

	s1 = as1;
	s2 = as2;
	c = *s2;

	while (*s1)
		if (*s1++ == c) {
			offset = s1 - as1 - 1;
			s2++;
			while ((c = *s2++) == *s1++ && c) ;
			if (c == 0)
				return(offset);
			s1 = offset + as1 + 1;
			s2 = as2;
			c = *s2;
		}
	 return(-1);
}

#if DMERT
/*
	Returns directory name containing a file
	(by modifying its argument).
	Returns "." if current
	directory; handles root correctly.
	Returns its argument.
	Bugs: doesn't handle null strings correctly.
*/

char *dname(p)
char *p;
{
	register char *c;
	register int s;

	s = size(p);
	for(c = p+s-2; c > p; c--)
		if(*c == '/') {
			*c = '\0';
			return(p);
		}
	if (p[0] != '/')
		p[0] = '.';
	p[1] = 0;
	return(p);
}
#endif
/*
*/
