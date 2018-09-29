/*	@(#)chmod.c	1.1	*/

# include <stand.h>

chmod() {

	errno = EPERM;
	return (-1);
}
