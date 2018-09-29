/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)convert:common/util.c	1.9"

#include	<stdio.h>
#include	<ar.h>
#include	"conv.h"
#include	"aouthdr.h"
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"scnhdr.h"

int			cnvtyp;		/* foreign conversion type */
long		nsyms;		/* number of symbols in an archive symbol table */
int			ar_org;		/* origin of archive */
unsigned short		ar_origin;	/* old or new format */

extern unsigned short	target,
			origin;
extern char		curfile[];
extern char		outfile[];
extern int		errlvl,
			fromto,		/* conversion type */
			preswab,	/* preswab chars */
			swab;		/* swab everything*/

charconv( old, num )

char	*old;
int	num;
{
	if ( ! preswab )
		return(SUCCESS);

	return( swabchar(old, num) );
}

shortconv( old )

short	*old;
{
	GEN	new, tmp;

	if ( preswab )
		return(SUCCESS);

	new.s[0] = *old;

	if ( fromto == PDPIBM || fromto == VAXIBM ) {
		tmp.c[0] = new.c[0];
		new.c[0] = new.c[1];
		new.c[1] = tmp.c[0];
		}

	*old = new.s[0];

	return(SUCCESS);
}

ushortconv( old )

unsigned short	*old;
{

	return( shortconv((short *)old) );
}

longconv( old )

long	*old;
{
	GEN	new, tmp;

	if ( preswab )
		return(SUCCESS);

	new.l = *old;

	if ( fromto == PDPVAX ) {
		tmp.us[0] = new.us[0];
		new.us[0] = new.us[1];
		new.us[1] = tmp.us[0];
	} else {
		if ( fromto == VAXIBM ) {
			tmp.us[0] = new.us[0];
			new.us[0] = new.us[1];
			new.us[1] = tmp.us[0];
			}
		tmp.c[0] = new.c[0];
		tmp.c[2] = new.c[2];
		new.c[0] = new.c[1];
		new.c[2] = new.c[3];
		new.c[1] = tmp.c[0];
		new.c[3] = tmp.c[2];
		}

	*old = new.l;

	return(SUCCESS);
}


swabchar( old, num)

char	*old;
int	num;
{
	GEN	tmp;

	while ( num >= 2 ) {
		tmp.c[0] = *old;
		*old = *(old+1);
		*(old+1) = tmp.c[0];
		old += 2;
		num -= 2;
		}

	return(SUCCESS);
}

copystrms ( instrm, outstrm, nbytes )

LDFILE	*instrm;
FILE	*outstrm;
long	nbytes;
{
	/*
	 * given instrm positioned to read and outstrm
	 * positioned to write, copy nbytes from instrm to outstrm
	 */

	char	buf[BUFSIZ];

	while (nbytes >= BUFSIZ) {
		if ( FREAD(buf, BUFSIZ, 1, instrm) != SUCCESS ) {
			converr(1,curfile,"unexpected EOF");
			return(FAILURE);
			}

		if ( swab )
			swabchar(buf, BUFSIZ);

		if ( fwrite(buf, BUFSIZ, 1, outstrm) != SUCCESS ) {
			converr(1,outfile,"cannot fwrite()");
			return(FAILURE);
			}

		nbytes -= BUFSIZ;

		} /* while nbytes >= BUFSIZ */

	if ( nbytes > 0L ) {
		if ( FREAD((char *)buf, (unsigned)nbytes, 1, instrm) != SUCCESS ) {
			converr(1,curfile,"unexpected EOF");
			return(FAILURE);
			}

		if ( swab )
			swabchar(buf, (unsigned)nbytes);

		if ( fwrite(buf, (unsigned)nbytes, 1, outstrm) != SUCCESS ) {
			converr(1,outfile,"cannot fwrite()");
			return(FAILURE);
			}
		} /* nbytes > 0 */

	return(SUCCESS);

} /* copystrms */

aoutconv( sptr )

char	*sptr;
{
	AOUTHDR	ahdr;
	char	*aptr;
	int	i;

	aptr = (char *) &ahdr;

	for ( i=0; i < AOUTSIZE ; ++i)
		*aptr++ = *sptr++;

	shortconv(&ahdr.magic);
	shortconv(&ahdr.vstamp);
	longconv(&ahdr.tsize);
	longconv(&ahdr.dsize);
	longconv(&ahdr.bsize);
	longconv(&ahdr.entry);
	longconv(&ahdr.text_start);
	longconv(&ahdr.data_start);

	for ( i=0; i < AOUTSIZE; ++i)
		*--sptr = *--aptr;

	return(SUCCESS);
}


converr( lvl, curfil, strng, a1, a2, a3, a4, a5, a6, a7, a8)

int lvl;	/* severity [0=warning, 1=fatal] */
char	*curfil;	/* current input file */
char	*strng;		/* format of message */
int	a1,a2,a3,a4,
	a5,a6,a7,a8;	/* format arguments */

{
	errlvl += lvl;

	fprintf(stderr,"sgsconv: %s : ", curfil);

	switch( lvl ) {
	case 0:
		fprintf(stderr,"warning : ");
		break;
	case 1:
		fprintf(stderr,"fatal : ");
		break;
	default:
		break;
	} /* switch */

	fprintf(stderr, strng, a1, a2, a3, a4, a5, a6, a7, a8);
	fprintf(stderr, "\n");
	fflush(stderr);

	return(SUCCESS);
}

/*
 * Routines allowing conversion of object files from other machines
 */
int
detorigin( ldptr )

LDFILE	*ldptr;
{
	unsigned short	dmagic,
			dflags;

	origin = 0;
	cnvtyp = -1;

	if ( vldldptr(ldptr) == FAILURE )
		return(FAILURE);

	dmagic = HEADER(ldptr).f_magic;
	dflags = FLAGS(ldptr);

	if ( ! ANYMAGIC(dmagic) ) {
		swabchar(&dmagic, sizeof(dmagic));
		swabchar(&dflags, sizeof(dflags));
		}

	if ( ! ANYMAGIC(dmagic) )
		return(FAILURE);

	if ( F_AR16WR & dflags )
		origin = F_AR16WR;
	else
		if ( F_AR32WR & dflags )
			origin = F_AR32WR;
		else
			if ( F_AR32W & dflags )
				origin = F_AR32W;
			else
				return(FAILURE);

	if ( origin == F_CURARCH )
		cnvtyp = 0;
	else
		if ( ISDEC(origin) && ISDEC(F_CURARCH) )
			cnvtyp = PDPVAX;
		else
			if ( origin == F_AR32WR || F_CURARCH == F_AR32WR )
				cnvtyp = VAXIBM;
			else
				cnvtyp = PDPIBM;
	return(SUCCESS);
}


int
ldcfhdr( fhdr )

FILHDR	*fhdr;
{
	if (cnvtyp==0) return(SUCCESS); else if (cnvtyp<0) return(FAILURE);

		ldshortconv(&fhdr->f_magic);
		ldushortconv(&fhdr->f_nscns);
		ldlongconv(&fhdr->f_timdat);
		ldlongconv(&fhdr->f_symptr);
		ldlongconv(&fhdr->f_nsyms);
		ldushortconv(&fhdr->f_opthdr);
		ldushortconv(&fhdr->f_flags);

	return(SUCCESS);
}

ldshortconv( old )

short	*old;
{
	GEN	new, tmp;

	if ( cnvtyp == 0 )
		return(SUCCESS);

	new.s[0] = *old;

	if ( cnvtyp == PDPIBM || cnvtyp == VAXIBM ) {
		tmp.c[0] = new.c[0];
		new.c[0] = new.c[1];
		new.c[1] = tmp.c[0];
		}

	*old = new.s[0];

	return(SUCCESS);
}

ldushortconv( old )

unsigned short	*old;
{
		return( ldshortconv((short *)old) );

}

ldlongconv( old )

long	*old;
{
	GEN	new, tmp;

	if ( cnvtyp == 0 )
		return(SUCCESS);

	new.l = *old;

	if ( cnvtyp == PDPVAX ) {
		tmp.us[0] = new.us[0];
		new.us[0] = new.us[1];
		new.us[1] = tmp.us[0];
	} else {
		if ( cnvtyp == VAXIBM ) {
			tmp.us[0] = new.us[0];
			new.us[0] = new.us[1];
			new.us[1] = tmp.us[0];
			}
		tmp.c[0] = new.c[0];
		tmp.c[2] = new.c[2];
		new.c[0] = new.c[1];
		new.c[2] = new.c[3];
		new.c[1] = tmp.c[0];
		new.c[3] = tmp.c[2];
		}

	*old = new.l;

	return(SUCCESS);
}

ldintconv( old )

int	*old;
{

	GEN	new, tmp;

	if ( cnvtyp == 0 )
		return(SUCCESS);

	if ( cnvtyp == PDPVAX )
		return(SUCCESS);

	/*
	 * we have DEC<->IBM
	 */

	new.i = *old;

	tmp.c[0] = new.c[0];
	tmp.c[2] = new.c[2];
	new.c[0] = new.c[1];
	new.c[2] = new.c[3];
	new.c[1] = tmp.c[0];
	new.c[3] = tmp.c[2];

	*old = new.i;

	return(SUCCESS);
}


/*
 *
 *	CONVERT VERSIONS OF LIBLD FUNCTIONS
 */
LDFILE *
ldopen(filename, ldptr)

char	*filename;
LDFILE	*ldptr; 

{
	/* functions called */
	extern FILE	*fopen( );
	extern		fclose( );
	extern int	fseek( );
	extern int	fread( );

	/* libld functions called */
	extern int	vldldptr( );
	extern LDFILE	*allocldptr( );
	extern int	freeldptr( );
	extern long	sgetl( );

	/* convert functions called */
	extern		detorigin( );
	extern int	ldcfhdr( );

	/* local variables */
	FILE	 	*ioptr;
	unsigned short	type;
	RANDARCH	arbuf;
	unsigned short	atype;
	char		magicbuf[ AMAGSIZE ];

	type = atype = 0;
	if (vldldptr(ldptr) == FAILURE)
	{
		if ((ioptr=fopen(filename, "r")) == NULL)
	    		return(NULL);

		if (fread( &arbuf, sizeof( RANDARCH ), 1, ioptr) != 1)
			arbuf.ar_magic[0] = '\0';
		fseek( ioptr, 0L, 0 );
		if (fread( magicbuf,  AMAGSIZE, 1, ioptr) != 1)
			magicbuf[0] = '\0';
		fseek( ioptr, 0L, 0 );
	
		if (fread(&type, sizeof(type), 1, ioptr) != 1)
		{
	    		fclose(ioptr);
	    		return(NULL);
		}

		/*
	 	 * If int is 4 bytes or we are looking at a foreign file
	 	 */
		if ( (sizeof(ARTYPE) != sizeof(type)) || (target == F_CURARCH) )
		{
			if ( fread(&atype, sizeof(atype), 1, ioptr) != 1)
				atype = type;
		} else {
			atype = type;
		}
	
		if ((ldptr = allocldptr( )) == NULL)
		{
	    		fclose(ioptr);
	    		return(NULL);
		}
	
		/*
	 	 * is it an archive in the 5.0 format
	 	 */
		if (strncmp( arbuf.ar_magic, RAND_MAGIC, sizeof(arbuf.ar_magic)) == 0)
		{
			ar_origin = RANDOM;
			nsyms = sgetl( arbuf.ar_syms );
			TYPE(ldptr) = USH_ARTYPE;
			OFFSET(ldptr) = sizeof( RANDARF ) + sizeof( RANDARCH ) +
					nsyms * 12;
	
		/*
	 	 * or is it an archive in the 6.0 format.
	 	 */
		}
		else if ( strncmp( magicbuf, ASCII_MAGIC, AMAGSIZE ) == 0)
		{
			long		arsize;
			PORT_HEAD	porthead;
	
			ar_origin = ASCIIAR;
			TYPE(ldptr) = USH_ARTYPE;
			if ((fseek(ioptr, (long)AMAGSIZE, BEGINNING) == OKFSEEK)
				    && (fread( &porthead, sizeof( PORT_HEAD ), 1, ioptr ) == 1)
				    && (strncmp( porthead.ar_fmag, ARFMAG, ARFMAGSIZE ) == 0)
				    && (porthead.ar_name[0] == '/')
				    && (sscanf( porthead.ar_size, "%ld", &arsize ) == 1))
				OFFSET(ldptr) = AMAGSIZE + (2 * sizeof( PORT_HEAD )) + ((arsize + 01) & ~01);
			else
				OFFSET(ldptr) = AMAGSIZE + sizeof( PORT_HEAD );
	
		/*
	 	 * or is it an archive in the old format, created on the host machine
	 	 */
		}
		else if ( (atype == USH_ARTYPE) 
			|| (type == USH_ARTYPE) )
		{
			ar_origin = OLD;
			TYPE(ldptr) = USH_ARTYPE;
			if ( atype == 0 || type == 0 )
			{
				OFFSET(ldptr) = 32 + 4; /* 32 bit machine */
#if AR16WR || AR32WR
				ar_org = F_AR32WR;
#else
				ar_org = F_AR32W;
#endif
			} else {
				OFFSET(ldptr) = 26 + 2; /* pdp */
				ar_org = F_AR16WR;
			}

		/*
	 	 * or is it an archive in the old format, created on another machine
	 	 */
		} else {
			swabchar(&type,sizeof(type));
			swabchar(&atype, sizeof(atype));
			if ( (atype == USH_ARTYPE) ||
	                  	(type == USH_ARTYPE) )
			{
				ar_origin = OLD;
				TYPE(ldptr) = USH_ARTYPE;
				/*
			 	 * we had to swab to find out it was
			 	 * an archive so we know it is foreign
			 	 */
				if ( atype == 0 || type == 0 )
				{
					 /* 32 bit machine */
					OFFSET(ldptr) = 32 + 4 ;
#if AR32W
					ar_org = F_AR32WR;
#else
					ar_org = F_AR32W;
#endif
				} else {
					OFFSET(ldptr) = 26 + 2;
					ar_org = F_AR16WR;
				}

			/*
		 	 * if it isn't an archive, it must be a normal object file
		 	 */
			} else {
				TYPE(ldptr) = type;
				OFFSET(ldptr) = 0L;
			}
		}
	
		IOPTR(ldptr) = ioptr;
	
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
		{
	    		if (FREAD(&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1)
			{
				detorigin(ldptr);
				if ( ldcfhdr( &(HEADER(ldptr)) ) == SUCCESS )
		    			return(ldptr);
	    		}
		}
	} else {
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
	    		return(ldptr);
	}

	fclose(IOPTR(ldptr));
	freeldptr(ldptr);
	return(NULL);
}
	
int
ldsseek(ldptr, sectnum)

LDFILE		*ldptr;
unsigned short	sectnum; 

{
	extern	int	ldshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS) {
		ldlongconv(&shdr.s_scnptr);
		if (shdr.s_scnptr != 0) {
		    if (FSEEK(ldptr, shdr.s_scnptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}

int
ldrseek(ldptr, sectnum)

LDFILE		*ldptr;
unsigned short	sectnum; 

{
	extern	int	ldshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS) {
		ldushortconv(&shdr.s_nreloc);
		if (shdr.s_nreloc != 0) {
		    ldlongconv(&shdr.s_relptr);
		    if (FSEEK(ldptr, shdr.s_relptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}

int
ldlseek(ldptr, sectnum)

LDFILE		*ldptr;
unsigned short	sectnum; 

{
	extern	int	ldshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS) {
		ldushortconv(&shdr.s_nlnno);
		ldlongconv(&shdr.s_lnnoptr);
		if (shdr.s_nlnno != 0) {
		    if (FSEEK(ldptr, shdr.s_lnnoptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}

int
ldahread(ldptr, arhead)

LDFILE    *ldptr;
NEWARHDR  *arhead; 

{
	/* local variables */
	long		dummy;
	/*
	 * NOTE: the def of ahdrbuf must be proceeded by a long
	 * to insure alignment on 3B
	 */
	char		ahdrbuf[BUFSIZ];
	int		tmpint,
			asize;	/* size of ARCHDR on origin machine */
	short		tmpshort;
	char		tmpc;

	/* functions called */
	extern int	fseek( );
	extern int	fread( );

	/* libld functions called */
	extern int	vldldptr( );
	extern long	sgetl( );

	if ((vldldptr(ldptr) != SUCCESS) || (TYPE(ldptr) != ARTYPE))
		return( FAILURE );

	if (ar_origin == RANDOM)
	{
		RANDARF	arfbuf;

		if ((FSEEK( ldptr, -((long) sizeof(RANDARF)), BEGINNING) == OKFSEEK) &&
	  		(FREAD( &arfbuf, sizeof( RANDARF ), 1, ldptr) == 1))
		{
			strncpy( arhead->ar_name, arfbuf.arf_name, sizeof(arhead->ar_name));
			arhead->ar_date = sgetl( arfbuf.arf_date );
			arhead->ar_uid = sgetl( arfbuf.arf_uid );
			arhead->ar_gid = sgetl( arfbuf.arf_gid );
			arhead->ar_mode = sgetl( arfbuf.arf_mode );
			arhead->ar_size = sgetl( arfbuf.arf_size );
			return( SUCCESS );
		}

	}
	else if (ar_origin == ASCIIAR)
	{
		PORT_HEAD	porthead;
		int		i;

		if ((FSEEK( ldptr, -((long) sizeof( PORT_HEAD )), BEGINNING) == OKFSEEK)
			&& (FREAD( &porthead, sizeof(PORT_HEAD), 1, ldptr) == 1)
			&& (strncmp(porthead.ar_fmag, ARFMAG, ARFMAGSIZE) == 0))
		{
			strncpy( arhead->ar_name, porthead.ar_name, 16 );
			for ( i = 15; arhead->ar_name[i] != '/'; i-- )
				arhead->ar_name[i] = '\0';
			arhead->ar_name[i] = '\0';

			sscanf( porthead.ar_date, "%ld", &(arhead->ar_date) );
			sscanf( porthead.ar_uid, "%d", &(arhead->ar_uid) );
			sscanf( porthead.ar_gid, "%d", &(arhead->ar_gid) );
			sscanf( porthead.ar_mode, "%o", &(arhead->ar_mode) );
			sscanf( porthead.ar_size, "%ld", &(arhead->ar_size) );
			return( SUCCESS );
		}
			
	} else {
		if ( ar_org == F_AR16WR )
			asize = 26;
		else
			asize = 32;

		if ((FSEEK(ldptr, -((long)asize), BEGINNING) != OKFSEEK)
			    || (FREAD(ahdrbuf, asize, 1, ldptr) != 1))
			return( FAILURE );

		mvbytes(arhead->ar_name, ahdrbuf, 14);
		arhead->ar_name[15] = arhead->ar_name[16] = '\0';
		if ( asize == 32 )
		{
			/* compress 32 to 26 bytes */
			arhead->ar_date = *((long *)&ahdrbuf[16]);
			arhead->ar_uid = (int) *((char *)&ahdrbuf[20]);
			arhead->ar_gid = (int) *((char *)&ahdrbuf[21]);
			if ( ar_org == F_AR32WR )
				arhead->ar_mode = *((int *)&ahdrbuf[24]);
			else
				/* mvbytes needed because of 3B
		 		 * allignment problems
		 		 */
				mvbytes((char *)&arhead->ar_mode, (char *)&ahdrbuf[24], sizeof(int));
			if (ar_org != F_CURARCH)
				longconv( &arhead->ar_mode );
			arhead->ar_size = *((long *)&ahdrbuf[28]);
		} else {
			/* extend 26 to 32 bytes */
			mvbytes( (char *)&arhead->ar_date, (char *)&ahdrbuf[14], sizeof(long));
			tmpc = *((char *)&ahdrbuf[18]);
			arhead->ar_uid = (short) tmpc;
			tmpc = *((char *)&ahdrbuf[19]);
			arhead->ar_gid = (short) tmpc;
			tmpshort = *((short *)&ahdrbuf[20]);
			ldshortconv(&tmpshort);
			tmpint = (int) tmpshort;
			tmpint &= 0x0000ffff; /* rub out sign extension */
			ldintconv(&tmpint);
			arhead->ar_mode = tmpint;
			if (ar_org != F_CURARCH)
				longconv( &arhead->ar_mode );

			/* mvbytes needed because of 3B allignment problems */
			mvbytes((char *)&arhead->ar_size, (char *)&ahdrbuf[22], sizeof(long));
		}

		if ( ar_org != F_CURARCH )
		{
			charconv( arhead->ar_name, 16 );
			longconv( &arhead->ar_date );
			longconv( &arhead->ar_size );
		}
		return(SUCCESS);
	}

    	return(FAILURE);
}

int
ldclose(ldptr)

LDFILE    *ldptr; 

{
	/* functions called */
	extern int	fseek( );
	extern int	fread( );
	extern 		fclose( );

	/* libld functions called */
	extern int	vldldptr( );
	extern int	freeldptr( );

	/* local variables */
	int		asize;
	NEWARHDR 	arhdr;

	if (vldldptr(ldptr) == SUCCESS)
	{
		if (TYPE(ldptr) == ARTYPE)
		{
			if ( ldahread(ldptr, &arhdr) == SUCCESS )
			{
				if ( ar_origin == RANDOM )
					asize = 36;
				else if ( ar_origin == ASCIIAR )
					asize = 60;
				else if ( ar_org == F_AR16WR )
					asize = 26;
				else
					asize = 32;
				OFFSET(ldptr) = OFFSET(ldptr) + arhdr.ar_size
						+ asize + (arhdr.ar_size & 01);
						/* (to make OFFSET even) */
				if ((FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
					&& (FREAD(&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1))
				{
					ldcfhdr(&(HEADER(ldptr)));
					return(FAILURE);
				}
			}
		}


		fclose(IOPTR(ldptr));
		freeldptr(ldptr);
	}

	return(SUCCESS);

}

mvbytes(s1, s2, n)
register char *s1, *s2;
int	n;
{
	register i;

	for(i = 0; i < n; i++)
		(*s1++ = *s2++);
	return(n);
}
