#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"sgs.h"
#include	"filedefs.h"


FILELIST *
openobject(filename)

char		*filename;

{
    extern		fprintf( ),
			exit( );
    extern char		*calloc( );

    extern LDFILE	*ldopen( );
    extern int		ldaclose( );

    LDFILE		*objptr;
    FILELIST		*listptr;


    if ((objptr = ldopen(filename, NULL)) == NULL) {
	fprintf(stderr, "%sdump:  cannot open %s\n", SGS, filename);
	return(NULL);
    } 

    if (!ISMAGIC(HEADER(objptr).f_magic)) {
	fprintf(stderr, "%sdump:  bad magic %s\n", SGS, filename);
	ldaclose(objptr);
	return(NULL);
    }

    if ((listptr = (FILELIST *) calloc(1, LISTSZ)) == NULL) {
	fprintf(stderr, "%sdump:  cannot allocate memory", SGS);
	exit(0);
    }

    listptr->file = filename;
    listptr->ldptr = objptr;
    listptr->nextitem = NULL;

    return(listptr);
}

/*
*     static char	ID[ ] = "@(#) openobject.c: 1.2 2/26/82";
*/
