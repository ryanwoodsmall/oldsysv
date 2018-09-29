/*	@(#)perror.c	1.2 of 3/28/83	*/

extern int errno;
main(ac,av)
int ac;
char **av;
{
	errno = atoi(av[1]);

	perror("");
}
