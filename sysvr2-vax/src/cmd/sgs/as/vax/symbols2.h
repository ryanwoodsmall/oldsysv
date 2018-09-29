/*	@(#)symbols2.h	2.2	*/

/*
 *	NINSTRS	= number of legal assembler instructions (from ops.out)
 *	NHASH	= the size of the hash symbol table
 *	NSYMS	= the number of symbols allowed in a user program
 */

#define	NINSTRS	333

#define NHASH	4001

#define NSYMS	(NHASH - NINSTRS)
