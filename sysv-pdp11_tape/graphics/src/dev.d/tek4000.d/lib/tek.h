static char tek__[]="@(#)tek.h	1.3";
#include "../../../../include/gsl.h"
#include "../../../../include/gpl.h"
#include "../../../../include/debug.h"
#define UNSET      -1
#define BOLDWIDTH 2.5

struct tekaddr {
	int yh,yl,xh,xl;
	int ch;
};

#include "../include/gedstructs.h"
