	/*	@(#) extn.c: 1.3 10/4/82	*/

/*	for VAX sdb
 *	External variables used by VAX disassembler.
 *	(Initialized arrays are in tbls.c)
 */

#include "dis.h"

long loc;		/* byte location in section being disassembled	*/
			/* IMPORTANT: loc is incremented only by the	*/
			/* disassembler dis_dot()			*/

char mneu[NLINE];	/* array to store mneumonic code for output	*/

int caseflg;		/* true if disassembled instruction is CASEx	*/
int argument [VARNO];	/* arguments of current instruction		*/
