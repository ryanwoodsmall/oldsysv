/*	@(#)creat.c	1.1	*/

creat(str)
char *str; {

	return (open(str, 1));
}
