static char incl_dev__[]="@(#)dev.h	1.1";
#include "../../../include/debug.h"
#include "../../../include/errpr.h"
#include "../../../include/util.h"
#include "../../../include/gsl.h"
#include "../../../include/gpl.h"
struct device{
	struct area v;
	int htext;
	int harc;
	int hfill;
	int hbuff;
};
struct window{
	double utodrat;
	struct area w;
};
