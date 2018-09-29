/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)sym.c	1.3	*/
/* 
 *  Symbol table and file handling service routines
 */

#include "defs.h"

SCCSID(@(#)sym.c	1.3);

#define	ERROR	(-1)		/* Signifies something is wrong */
static int     symrqd = TRUE;
static SYMTAB  symbuf;     	/* Room for 1 symbol */
static	long curoffs = (ERROR -1);
static	char	*strtab;	/* String for string table. */
struct	sh_name{
	char	name[ 9 ];
};
static	struct	sh_name	*shstrtab;	/* Space for all names contained */
					/* in symbol table. */
static	long	strtablen;


void valpr(v, idsp)
    int v;
    int idsp;
{
    unsigned d = findsym(v, idsp);

    if (d < maxoff) {
		adbpr("%s", symbol.n_nptr);
    		if (d)
			adbpr(OFFMODE, d);
    }
}

/*ARGSUSED*/
void localsym(cframe,newest)
	int cframe;
	int newest;
{
	int word;

	if(!newest)
	{
		locadr = (get(cframe,DSP)-4);
	}
	else
	{
		locadr = cframe - 4;
	}
}

void psymoff(v, symtype, s) /* Prints symbol, offset, and suffix */
	int  v;
	int  symtype;
	char *s;                       /* Suffix for print line */
{
    unsigned w;

    if (v != 0) 
		w = findsym(v, symtype);
    if (v == 0 || w >= maxoff)     /* || (v>=0 && v<maxoff && w!=0) */
		adbpr(LPRMODE, v);
    else { 
		adbpr("%s", symbol.n_nptr);
    		if (w)
			adbpr(OFFMODE, w);
    }
    adbpr(s);
}

/* added long rdsym( offset, symep )
 *	Stores symbol in location pointed to by symep.
 *	Returns offset for next syment
 *	Seeks done here if necessary; removed from other symt.c code
 *	for COFF, always just 0 or 1 auxiliary entries
 */

long
rdsym( offset, symep )
long offset;
register SYMENT *symep;
{
	register int numaux;

	if(offset <= 0 || offset >= symbas + symnum * SYMESZ)
		return(ERROR);
	lseek( fsym, offset, 0 );
	curoffs = offset;
	if ( read( fsym, (char *)symep, SYMESZ) < SYMESZ)
	{
		curoffs = ERROR -1;
		return(ERROR);
	}
#ifdef FLEXNAMES
	if( symep->n_zeroes == 0 )
	{
		if (symep->n_offset < 4 || symep->n_offset > strtablen)
		{
			printf("Bad string table offset @ 0x%lx\n",
				curoffs - SYMESZ);
			return (ERROR);
		}
		symep->n_nptr = &strtab[ symep->n_offset ];
	}
	else
#endif
	{
		long	i = offset - symbas;
		i /= SYMESZ;
		strncpy( shstrtab[ i ].name, symep->n_name, SYMNMLEN );
		shstrtab[ i ].name[ 8 ] = '\0';
		symep->n_zeroes = 0;
		symep->n_offset = 0;
		symep->n_nptr = shstrtab[ i ].name;
	}
/*
**	printf("Found name `%s'\n", symep->n_nptr);
*/
	curoffs += SYMESZ;
	return(curoffs);
}

/*	skip_aux - skip auxilliary entries

	input - symp = symbol just read
*/
long
skip_aux( symp )
register SYMENT *symp;
{
	register int numaux = symp->n_numaux;

	curoffs += numaux * AUXESZ;
	return(curoffs);
}

unsigned findsym(value, symtype) /* Linear search through slave array */
    int value;
    int symtype;
{
/* 
 *  Linear search through slave array; finds the symbol closest
 *  above value
 */
    int      diff;
    int      symval;
    int      offset;
    int      slvtype;
    SYMSLAVE *symptr;
    SYMSLAVE *symsav;

    diff = 0x7fffffffL;
    symsav = 0;
    if (symtype != NSYM && (symptr = symvec)) { 
	while (diff && (slvtype = symptr->typslave) != ENDSYMS) {  
		if (((slvtype == symtype) || ((symtype == IDSYM) &&
				((slvtype == DSYM) || (slvtype == ISYM)))) &&
				(slvtype != EXTRASYM)) {
			symval = symptr->valslave;
               		if (value - symval < diff && value >= symval) {
    				diff = value - symval;
      				symsav = symptr;
               		}
		}
		symptr++;
        }
        if (symsav) {      /* Best entry */
	    int ret;
            offset = symsav - symvec;
	    ret = rdsym((long)(symbas + offset * SYMBOLSIZE), &symbol);
	    if (ret == ERROR)
                perror("findsym");
        }
    }
    return(diff);
}

/* sequential search through file */
void symset()
{
	if (-1L == lseek(fsym, (long) symbas, 0))
		perror("symset");
	curoffs = (long) symbas;
	(void) symread();
	curoffs = (long) symbas;
} 

SYMPTR symget()
{
	if (curoffs >= symbas + symnum * SYMESZ)
		return ((SYMPTR)NULL);
	if (symread())
		return (&symbuf);
	else
	{
		errflg = BADFIL;
		return ((SYMPTR)NULL);
	}
}
/*
	Note to future maintainers: this code used to be
	buffered in symbuf, but no longer is.  There is
	much remaining extraneous code.
*/

static int symread()
{
    if (rdsym(curoffs, &symbuf) == ERROR)
    {
	perror("symread");
	return (FALSE);
    }
    else
        return (TRUE);
}
int
readstrtbl( fd, filehdr)
int  fd;
FILHDR filehdr;
{
	extern char *sbrk();
	long home;
	register int i;

	home = lseek( fd, 0L, 1 );
	if ( lseek( fd, symbas, 0 ) == -1 )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	i = (((filehdr.f_nsyms + 1) * sizeof(struct sh_name)) + 3) & ~3;
	if ((shstrtab = (struct sh_name *)sbrk(i)) == (struct sh_name *)(-1))
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if ( lseek( fd, filehdr.f_nsyms * SYMESZ + filehdr.f_symptr, 0 ) == -1 )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if( read( fd, (char *) &strtablen, sizeof( long ) ) != sizeof( long ) )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if ((strtab = sbrk((strtablen + 3 ) & ~3)) == (char *)(-1))
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if ( read( fd, &strtab[ sizeof( long ) ], strtablen - sizeof( long ) )
		!= strtablen - sizeof( long ) )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	if ( strtab[ strtablen - 1 ] != '\0' )
	{
		(void) lseek( fd, home, 0 );
		return;
	}
	lseek( fd, home, 0 );
	/*
	**	Used for debugging the reading in of string table for FLEXNAMES.
	**	printf("STRING TABLE\n");
	**	for (i = 4; i < strtablen; i++)
	**	{
	**		if (strtab[i] == '\0')
	**			printf("^@\n");
	**		else if (strtab[i] < ' ')
	**			printf("^%c", strtab[i]|0100);
	**		else if (strtab[i] == '\177')
	**			printf("^?");
	**		else if (strtab[i] > '~')
	**			printf("M-%c", strtab[i]&0177);
	**		else
	**			printf("%c", strtab[i]);
	**	}
	*/
	return;
}
