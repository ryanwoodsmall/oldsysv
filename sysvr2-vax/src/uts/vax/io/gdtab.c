/* @(#)gdtab.c	6.1 */
#include <sys/types.h>
#include <sys/gdisk.h>

extern nulldev();
extern ghpstrategy(), ghpintr(), ghpustart();
extern struct d_sizes hp_sizes[];
extern struct d_sizes hm_sizes[];
extern struct d_sizes hP_sizes[];
extern struct d_sizes he_sizes[];
struct gdsw gdsw[] = {
	{GDRP04, "RP04", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRP05, "RP05", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRP06, "RP06", 19, 22,
		hp_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRM05, "RM05", 19, 32,
		hm_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRM80, "RM80", 14, 31,
		he_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{GDRP07, "RP07", 32, 50,
		hP_sizes, nulldev, ghpstrategy, ghpintr, ghpustart},
	{ 0 }
};
