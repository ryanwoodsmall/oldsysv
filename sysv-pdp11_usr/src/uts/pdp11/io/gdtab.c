/* @(#)gdtab.c	1.1 */
#include <sys/types.h>
#include <sys/iobuf.h>
#include <sys/elog.h>
#include <sys/gdisk.h>

extern nulldev();
extern ghpstrategy(), ghpintr(), ghpustart();
extern gmlopen(), gmlstrategy(), gmlintr(), gmlustart();
extern struct d_sizes hp_sizes[], hm_sizes[];
extern struct d_sizes ml_sizes[];
struct gdsw gdsw[] = {
	{GDRP04, "RP04", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRP05, "RP05", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRP06, "RP06", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRM05, "RM05", 19, 32,
		hm_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDML11A, "ML11", 1, 1,
		ml_sizes, gmlopen, gmlstrategy, gmlintr, gmlustart},
	{GDML11B, "ML11", 1, 1,
		ml_sizes, gmlopen, gmlstrategy, gmlintr, gmlustart},
	{ 0 }
};
