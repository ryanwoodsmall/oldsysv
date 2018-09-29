static char Sccsid[] "@(#)move.c	3.1";
/*
	Copies `n' characters from string `a' to string `b'.
*/

struct { int *ip; };

char *move(a,b,n)
char *a,*b;
unsigned n;
{
	register char *x, *y;
	register int m;

	/*
		Test for non-zero number of characters to move
	*/
	if (m=n) {
		x = a;
		y = b;
		/*
			Compare the low order bits of the two pointers
			If both are equal (both even or both odd) then
			move words instead of bytes
		*/
		if (((x^y)&(char *)1) == 0) {
			/*
				If both odd move one byte to make both even
			*/
			if (x&(char *)1) {
				*y++ = *x++;
				n--;
			}
			/*
				Add one to n/2 so that we can use --m instead
				of m--; --m is more efficient
			*/
			for (m=n/2+1; --m; )
				*(y.ip)++ = *(x.ip)++;
			m = n&1;
		}
		/*
			Add one to m so that we can use --m instead of m--;
			--m is more eficient.
		*/
		for (++m; --m; ) *y++ = *x++;
	}
}
