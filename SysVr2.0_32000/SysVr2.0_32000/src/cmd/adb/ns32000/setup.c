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
/*	@(#)setup.c	1.2	*/
#include "defs.h"

SCCSID(@(#)setup.c	1.2);

#define TXTRNDSIZ 512


void setsym()
{
	SYMSLAVE    *symptr;
	char numaux;
	SYMPTR      symp;
	AOUTHDR aouthdr;
	FILHDR filehdr;
	SCNHDR scnhdr;

	qstmap.ufd = fsym = getfile(symfil, 1);
	if((read(fsym,(char *) &filehdr,FILHSZ) == FILHSZ) && filehdr.f_opthdr)
	{
		read(fsym, (char *) &aouthdr, filehdr.f_opthdr);
		magic = aouthdr.magic;
		txtsiz = (aouthdr.tsize & ~0x80000000) ;
		datsiz = (aouthdr.dsize & ~0x80000000) ;
		entrypt = aouthdr.entry;
	}
	symbas = filehdr.f_symptr;
	symnum = filehdr.f_nsyms;
#ifdef DEBUG
	printf("symnum=%x\n",symnum);
#endif
	read(fsym, (char *) &scnhdr, SCNHSZ);
/* comments contain original 5.2 code */
/*	qstmap.b1 = 0;*/
        qstmap.b1 = (magic == 0411 ? 0x200 : scnhdr.s_vaddr);;
/*	qstmap.e1 = txtsiz;*/
        qstmap.e1 = qstmap.b1 + (magic == 0407 ? symbas : 
				 magic == 0411 ? txtsiz+((int)sizeof(filehdr))
						       +((int)sizeof(aouthdr)) :
						 txtsiz);
/*	qstmap.f1 = scnhdr.s_scnptr;*/
        qstmap.f1 = (magic == 0411 ? 0x200 : scnhdr.s_scnptr);
	read(fsym, (char *) &scnhdr, SCNHSZ);
/*	qstmap.b2 = (magic == 0410 ? round(txtsiz, TXTRNDSIZ) : txtsiz);*/
	qstmap.b2 = scnhdr.s_vaddr;
/*	qstmap.e2 = qstmap.b2 + datsiz;*/
        qstmap.e2 = qstmap.b2 + (magic == 0407 ? symbas : datsiz);
/*	qstmap.f2 = scnhdr.s_scnptr;*/
	qstmap.f2 = (magic == 0411 ? qstmap.e1 : scnhdr.s_scnptr);
#ifdef DEBUG
	printf("symbas=%x\n", symbas);
#endif

	symvec = (SYMSLAVE *) sbrk((1 + symnum) * sizeof (SYMSLAVE));
	if (((int) (symptr = symvec)) == -1) 
	{
		(void) fprintf(stderr, "%s\n", BADNAM);
		symptr = symvec = (SYMSLAVE *) sbrk(sizeof (SYMSLAVE));
		if (-1 == (int) symvec)
	            perror("setsym");
	}
	else 
	{
		readstrtbl( fsym, filehdr );
	        symset();
	        while ((symp = symget()) && errflg == NULL) 
		{
			if(symp->n_sclass<C_NULL||symp->n_sclass>=C_BLOCK||
				eqstr(symp->n_nptr, ".text")||
				eqstr(symp->n_nptr, ".data")||
				eqstr(symp->n_nptr, ".bss"))
			{
				symptr->typslave = N_DEBUG;
				symptr->valslave = 0;
				symptr++;
			}
			else
			{
				symptr->typslave = typesym( symp->n_scnum);
				symptr->valslave = symp->n_value;
				symptr++;
		        }
			/* get rid of auxilliary entries */
			skip_aux(symp);
			numaux = symp->n_numaux;
			while(numaux)
			{
				symptr->typslave = N_DEBUG;
				symptr->valslave = 0;
				symptr++;
				numaux--;
			}
		}
	    }
	symptr->typslave = ENDSYMS;
	if (magic == 0)
	qstmap.e1 = maxfile;
}

void setcor()
{
    slshmap.ufd = fcor = getfile(corfil, 2);
    if (read(fcor, (char *) &u, ctob(USIZE)) == ctob(USIZE)) {

	/* u.u_ar0 is an address relative to the incore UBLOCK
	   currently 0xFF0208.  It must be transformed to be
	   relative to u, a variable in adb's data space.   */

	u.u_ar0 = (int *) (((char *) u.u_ar0) - ADDR_U);
	u.u_ar0 = (int *) (((char *) u.u_ar0) + (int) &u);

	txtsiz = ctob(u.u_tsize);
	datsiz = ctob(u.u_dsize);
	stksiz = ctob(u.u_ssize);
	slshmap.b1 = ((magic == 0410 || magic == 0411) ? txtsiz : 0);
	slshmap.e1 = (magic == 0407 ? txtsiz : slshmap.b1) + datsiz;
	slshmap.f1 = ctob(USIZE);
	slshmap.b2 = maxstor - stksiz;
	slshmap.e2 = maxstor;
	slshmap.f2 = ctob(USIZE) + ((magic==0410 || magic==0411) ?
			datsiz : slshmap.e1);
	signo = u.u_arg[0] & 017;
	if (magic && magic != u.u_exdata.ux_mag)
	    (void) fprintf(stderr, "%s\n", BADMAGIC);
    } else
	u.u_ar0 = (int *) (((int) &u) + ADRREG);
}

static int create(f)
    STRING f;
{
    int fd;

    if ((fd = creat(f, 0644)) != -1) { 
	if (-1 == close(fd))
	    perror("create");
	return(open(f, wtflag));
    }
    else 
	return(-1);
}

static int getfile(filnam, cnt)
    STRING filnam;
{
    int symfid;

    if ( ! eqstr("-", filnam)) { 
	symfid = open(filnam, wtflag);
	if (symfid == -1 && argcount > cnt) { 
	    if (wtflag == O_RDWR)
	        symfid = create(filnam);
	    if (symfid == -1)
	        (void) fprintf(stderr, "Cannot open `%s'\n", filnam);
	}
    }
    else
	symfid = -1;
    return(symfid);
}

static int typesym(sflag) /* Returns the space to look in for value */
    short sflag;
{
	if(sflag == N_DEBUG)
		return(EXTRASYM);
    switch (sflag) {
	
	case SECT1:
	    return(ISYM);

	case SECT2:
	case SECT3:
	    return(DSYM);

	default:
	    return(NSYM);
    }
}
