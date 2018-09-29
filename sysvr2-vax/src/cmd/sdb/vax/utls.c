	/*	@(#) utls.c: 1.3 5/17/83	*/

/* VAX disasembler utility routines.
 *
 * printline() - prints disassembled line, as stored is mneu[].
 */

printline()
{
	extern char mneu[];
	printf("%s", mneu);		/* to print */
}
