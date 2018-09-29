/*	@(#)gtty.c	1.1	*/

# include <stand.h>

gtty(fildes, arg)
struct sgttyb *arg; {
	extern struct sgttyb _ttstat;

	if (fildes < 0 || fildes > 2) {
		errno = ENOTTY;
		return (-1);
	}

	strncpy((char *) arg, (char *) &_ttstat, sizeof _ttstat);
	return (0);
}
