static char ged__[]="@(#)ged.h	1.1";
#include "../../../../include/gsl.h"
#include "../../../../include/gpl.h"
#include "../../../../include/util.h"
#include "../../../../include/debug.h"
#include "../../../../include/errpr.h"
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
