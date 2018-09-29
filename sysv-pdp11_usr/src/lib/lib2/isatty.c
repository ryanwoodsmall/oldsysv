/*	@(#)isatty.c	1.1	*/

isatty(fildes) {

	if (fildes < 0 || fildes > 2)
		return (0);

	return (1);
}
