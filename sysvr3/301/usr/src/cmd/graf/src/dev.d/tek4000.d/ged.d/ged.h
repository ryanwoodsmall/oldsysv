/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/ged.h	1.2"
#include "gsl.h"
#include "gpl.h"
#include "util.h"
#include "debug.h"
#include "errpr.h"
#define UNSET      -1
#define STRLEN     256
#define MAXTXLINES	120
#define TEXTBUF	((char *)endbuf+10*MAXTXLINES)
#define BUFSZ      10000
#include "../include/gedstructs.h"

#define NUMOPS ('z'-'a')
#define OP(x) (clo[0][x-'a'])
#define OPF(x) (clo[1][x-'a'])

#define CATCHSIG signal(SIGINT,interr),signal(SIGQUIT,interr)
#define IGNSIG signal(SIGINT,SIG_IGN),signal(SIGQUIT,SIG_IGN)
