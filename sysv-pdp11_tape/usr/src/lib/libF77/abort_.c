/*	@(#)abort_.c	1.2	*/
#include <stdio.h>

abort_()
{
fprintf(stderr, "Fortran abort routine called\n");
_cleanup();
abort();
}
