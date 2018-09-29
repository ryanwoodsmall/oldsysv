/* @(#)swap.c	1.1 */
swap(longnum)
short longnum[];
{
	/* Subroutine to exchange the high order and low order words of long */
	register short temp;

	temp = longnum[0];
	longnum[0] = longnum[1];
	longnum[1] = temp;
}
