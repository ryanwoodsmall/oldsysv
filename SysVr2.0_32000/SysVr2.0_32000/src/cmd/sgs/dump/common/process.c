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
#include	<stdio.h>
#include	<ar.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"filedefs.h"
#include	"flagdefs.h"
#include	"process.h"
#include	"sgs.h"

char	*origname;

process(listhead)

FILELIST	*listhead;

{
    extern int		fprintf( ),
			sprintf( ),
			cfree( ),
			ldaclose( ),
			ldclose( ),
			ldahread( );

    extern LDFILE	*ldaopen( );

    extern FLAG		flag[ ];
    extern int		vflag;
    extern int		pflag;
#if PORTAR
    extern int		gflag;
#endif

    int			i,
			status;
    FILELIST		*listptr,
			*saveptr,
			*lastptr;
    LDFILE		*newptr;
    ARCHDR		arhead;
    char		name[513];

#if PORTAR
	if (gflag) {
		if (!pflag)
			printf( gtitle );
		lastptr = NULL;
		for (listptr = listhead; listptr != NULL; listptr = saveptr) {
			saveptr = listptr->nextitem;
			status = ar_sym_read( listptr->ldptr, listptr->file );
			if (status != SUCCESS) {
				ldaclose(listptr->ldptr);
				if (lastptr == NULL)
					listhead = saveptr;
				else
					lastptr->nextitem = saveptr;
				cfree( listptr );
			} else
				lastptr = listptr;
		}
	}
#endif

    for (i = 0; i < NUMFLAGS; ++i) {
	if (flag[i].setting == ON) {
	    if (!pflag)
	    	printf(vflag?prvtitle[i]:prtitle[i]);
	    lastptr = NULL;

	    for (listptr = listhead; listptr != NULL; listptr = saveptr) {
		saveptr = listptr->nextitem;
		origname = listptr->file;

		if (TYPE(listptr->ldptr) == ARTYPE) {
		    if ((newptr=ldaopen(listptr->file, listptr->ldptr))==NULL) {
			fprintf(stderr, 
			        "%sdump:  cannot ldaopen %s\n", SGS, listptr->file);
			status = FAILURE;
		    } else {
			do {
			    if ((status=ldahread(newptr, &arhead)) == SUCCESS) {
				sprintf(name, "%s[%.14s]", listptr->file,
					arhead.ar_name);
				if ((status = (*(flag[i].ldfcn))(newptr, name))
				    != SUCCESS) {
				    ldaclose(newptr);
				}
			    } else {
				ldaclose(newptr);
			    }
			} while(ldclose(newptr) == FAILURE);
		    }
		} else {
		    status = (*(flag[i].ldfcn))(listptr->ldptr, listptr->file);
		}

		if (status != SUCCESS) {
		    ldaclose(listptr->ldptr);
		    if (lastptr == NULL) {
			listhead = saveptr;
		    } else {
			lastptr->nextitem = saveptr;
		    }
		    cfree(listptr);
		} else {
		    lastptr = listptr;
		}
	    }
	}
    }
    return;
}

/*
 *	@(#) process.c: 1.6 3/31/83
 */
