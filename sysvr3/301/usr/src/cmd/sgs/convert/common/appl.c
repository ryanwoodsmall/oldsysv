/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)convert:common/appl.c	1.7"


#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"conv.h"
#include	"aouthdr.h"

extern unsigned short target;
extern unsigned short origin;
extern int preswab;

/*
 *	This file is for use by special applications who have unique
 *	needs. Although some changes may have to be made in the other
 *	files, the bulk of them should be maintained here, away from
 *	the general purpose code. A -D flag should be added to the
 *	makefile and the special code should be "ifdef"ed out.
 */

#if DMERT
#include	"ncps.h"
#include	"dtype.h"
#include	"bool.h"
#include	"pev.h"
#include	"ldpdefs.h"
#include	"utilid.h"
#include	"phdr.h"

lhdrconv( sptr )

char	*sptr;
{
	int	i,j;
	char	*tmp;

	aoutconv(sptr);			/* convert unix a.out header */
	tmp = sptr + AOUTSIZE;

	for ( i=0; i < 4; ++i ) {	/* lversion...l_vects */
		longconv(tmp);
		tmp += sizeof(long);
		}

	for ( i=0; i < NLIBSEGS ; ++i ) { 	/* convert l_seg */
		for ( j=0; j < sizeof(struct hsegmnt); j+=sizeof(long)) {
			longconv(tmp+(i*sizeof(struct hsegmnt))+j);
			}
		}

	tmp += (NLIBSEGS * sizeof(struct hsegmnt));

	for ( i=0; i < 3; ++i ) {	/* hnseg, hnoterm, hncom */
		shortconv(tmp);
		tmp += sizeof(short);
		}

	/* the library pathnames are pre-swabbed by ldp on
	 * DEC machines but not on AR32W machines; therefore
	 * when sending directly to 3B20 (not using TUS) or
	 * when sending from DEC/non-DEC host to non-DEC/DEC host,
	 * we need to swab the pathname
	 */
	if ( ! preswab && ((ISDEC(target) && ISNONDEC(origin)) ||
			(ISNONDEC(target) && ISDEC(origin))))
		swabchar(tmp, LHDRPATHSZ);

	return(SUCCESS);
}

phdrconv( sptr )

char	*sptr;
{

	int	i,j;
	char	*tmp;

	aoutconv(sptr);			/* unix a.out header */
	tmp = sptr + AOUTSIZE;

	for ( i = 0; i < 12; ++i ) {	/* hprior....htype */
		shortconv(tmp);
		tmp += sizeof(short);
		}

	for ( i = 0; i < 8; ++i ) {	/* h_class...hswap */
		longconv(tmp);
		tmp += sizeof(long);
		}

	for ( i = 0; i < NSHAR; ++i ) {	/* hshrs	*/
		longconv(tmp);
		/* the segment names are pre-swabbed by ldp on
		 * DEC machines but not on AR32W machines; therefore
		 * when sending directly to 3B20 (not using TUS) or
		 * when sending from DEC/non-DEC host to non-DEC/DEC host,
		 * we need to swab the pathname
		 */
		if ( ! preswab && ((ISDEC(target) && ISNONDEC(origin) ) ||
				( ISNONDEC(target) && ISDEC(origin))))
			swabchar(tmp+sizeof(long), NCPS);
		tmp += (sizeof(long) + (NCPS * sizeof(char)));
		}

	for ( i = 0; i < MAXENTS; ++i ) {	/* h_ent */
		longconv(tmp);
		longconv(tmp += sizeof(long));
		tmp += sizeof(long);
		}

	for ( i = 0; i < H_NSEG; ++i) {		/* h_seg */
		for ( j = 0; j < (sizeof(struct hsegmnt)/sizeof(long)); ++j) {
			longconv(tmp + (i * sizeof(struct hsegmnt)) + (j * sizeof(long)));
			}
		}

	tmp += (H_NSEG * sizeof(struct hsegmnt));

	/* the library pathnames are pre-swabbed by ldp on
	 * DEC machines but not on AR32W machines; therefore
	 * when sending directly to 3B20 (not using TUS) or
	 * when sending from DEC/non-DEC host to non-DEC/DEC host,
	 * we need to swab the pathname
	 */
	if ( ! preswab && ((ISDEC(target) && ISNONDEC(origin) ) ||
				( ISNONDEC(target) && ISDEC(origin))))
		swabchar(tmp, LPATHSZ);		/* hpathnam */

	return(SUCCESS);
}
#endif
