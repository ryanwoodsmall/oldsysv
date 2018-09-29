/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/plot.h	1.2"
#include "gpl.h"
#include "gsl.h"

#define SCALE 200
#define STYLES 10
#define STLTYPES 5
#define XLTRIM (3000)
#define XRTRIM (1000)
#define YUTRIM (2000)
#define YLTRIM (3000)
#define LTITLE (-1000)
#define UTITLE (YSIZE+1000)
#define TSCALE (int)(SCALE*1.5)
#define XSIZE (XDIM-XLTRIM-XRTRIM)
#define YSIZE (YDIM-YUTRIM-YLTRIM)
#define CWIDTH (SCALE*11.2/14)
#define CHEIGHT (14./8*CWIDTH)

#define DFLTGP stdout,-(XDIM/2)+XLTRIM,-(XDIM/2)+YLTRIM,0.,0.,1.,1.,0.,0.
