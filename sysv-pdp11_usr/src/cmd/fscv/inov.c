/* @(#)inov.c	1.1 */
inov(c)
char c[13][3];

{
	/*
		 *  This subroutine accepts the three character inode block address
		 * formatted for a PDP 11/70 machine and formats it for a VAX.
		 */

	register char temp;
	int i;

	/*
		 * On a VAX machine the bytes are stored in increasing order (012) and
		 * on an 11/70 they are stored 201.
		 */


	/* Permute the characters */
	for (i=0; i < 13; i++) {
		temp = c[i][0];
		c[i][0] = c[i][1];
		c[i][1] = c[i][2];
		c[i][2] = temp;
	};
}
