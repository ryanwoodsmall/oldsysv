/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/t_mkbt.c	1.5.1.6"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "syms.h"
#include "scnhdr.h"
#include "storclass.h"
#include "shlib.h"
#include "trg.h"


/* mkbt() creates an object file containing the branch table specification 
 * information and the .shlib section for a target shared library. It also modifies
 * the symbol table of the intermediate partially loaded target shared library,
 * tpltnam, so that each symbol
 * which has a branch table entry has its name changed to the destination label
 * of its corresponding jump in the branch table and puts this modified file 
 * in pltname.
 */
mkbt()
{
	patchsym();	/* modify the symbol table of the partially loaded object,
			 * tpltnam, so that each symbol which has a branch
			 * table entry has its name changed to the dest. label
			 * of its corresponding jump in the branch table and
			 * put the modified file in pltname */

	mkassem();	/* create assembly listing for branch table and .shlib
			 * section and put the result in  assemnam */

	/* assemble the assembly listing, assemnam; this will create btname */
	if (execute(asname,asname,"-dl","-o",btname,assemnam,(char *)0))
		fatal("Internal %s invocation failed",asname);
}


/* Make new undefined symbol structure
	Initially, create undefined symbol list by inputting name of
	symbol and the fact that the symbol has not yet been found
	in another shared library
								*/
usdef *newusdef(name, next)
char	*name;
usdef	*next;
{
	
	usdef	*tnode;

	if ((tnode = (usdef *)calloc(1,sizeof(usdef))) == NULL)
		fatal("Out of space for undefined symbol hash table");

	tnode->name = name;
	tnode->found = FALSE;
	tnode->nobjdefs = 0;
	tnode->next = next;
	return(tnode);
}

/* This routine modifies the symbol table of the partially loaded object (tpltnam)
 * so that each symbol which has a branch table entry has its name changed to
 * the dest. label of its corresponding jump in the branch table. The modified file
 * is place in pltname.
 */
void
patchsym()
{
	int common_used = FALSE;
	int badbranches = FALSE; /* Set if we find #branch references */
	char	*strtab;	/* points to the beginning of the string table */
	long	strsize;	/* size of string table */
	char	*strnext;	/* pointer into string table */
	long	slength,	/* holds offsets into string table */
		len;		/* length of entry in string table */
	SYMENT	symbol;		/* pointer to current symbol */
	char	*cursym;	/* holds name of current symbol */
	long	nsyms;		/* number of symbols in tpltnam */
	long	hval;		/* holds hash value */
	btrec	*tptr;		/* pointer into btlst */
	short	txtnum= NONE;	/* section number of the .text section- 
				   initially set to an illegal section
					   number value */
	SCNHDR	secthead;	/* holds a section header */
	unsigned short	sect;	/* counter to loop through sections */
	LDFILE	*pltrg;		/* pointer to the partially loaded target */
	FILE	*modpltrg;	/* pointer to the modified part. loaded target */
	int	i,j;
	usdef	*puslst;	/* Pointer into undefined symbol hash table */

	extern usdef *lookus();	/* External hash lookup function for undefined syms */

	/* open tpltnam for reading */
	if ((pltrg=ldopen(tpltnam,(LDFILE *)NULL)) == NULL)
		fatal("Cannot open temp file");

	/* open pltname for writing */
	if ((modpltrg= fopen(pltname,"w")) == NULL)
		fatal("Cannot open temp file");


	/* allocate space for modified string table */
	if ((strtab = malloc(BUFSIZ)) == NULL)
		fatal("Insufficient memory for string table");

	/* set up string table */
	strsize= BUFSIZ;
	slength=0; /* initial length of regenerated string table */
	strnext=strtab;


	(void)FSEEK(pltrg,0L,BEGINNING);
	/* copy everything up to the symbol table */
	if (copy(pltrg,modpltrg,HEADER(pltrg).f_symptr) == FAILURE)
		fatal("Cannot copy temp file");

	/* find the .text section number */
	for (sect=1; sect <= HEADER(pltrg).f_nscns; sect++) {
		if (ldshread(pltrg, sect, &secthead) != SUCCESS)
			fatal("Cannot read section headers of temp file");

		if (strncmp(secthead.s_name,".text",sizeof(secthead.s_name)) == 0) {
			txtnum= sect;
			break;
		}
	}
	if (txtnum == NONE)
		warn("target shared library has no .text section");

	/* Scan through the symbol table of pltrg and write the modified symbol
	 * table to modpltrg.
	 * Check each external symbol in the symbol table to see if it has
	 * branch table entry.  If a branch table entry exists, mark the branch
	 * table entry as found and change the symbol name in the symbol table to
	 * the jump label.  If a .data symbol has a branch table entry,
	 * print an appropriate warning. If a .text symbol does not
	 * have a branch table entry, print
	 * another appropriate warning message (this is necessary since external
	 * .text symbols may be visible function names and users of the 
	 * shared library shouldn't access any functions in the shared lib. except
	 * through the branch table).
	 */

	if (ldtbseek(pltrg) == FAILURE)
		fatal("Cannot find symbol table in temp file");

	nsyms= HEADER(pltrg).f_nsyms;
	for ( i=0; i<nsyms; i++) {
		if (FREAD(&symbol,SYMESZ,1,pltrg) != 1)
			fatal("Cannot read symbol table of temp file");
		cursym= ldgetname(pltrg, &symbol);

		if (symbol.n_sclass == C_EXT) {
			/* Undefined symbols put in uslst hash table */
			/* Other shared libraries will be searched for symbols, */
			/* and if not found, (or not absolute) an error will result */
			if (symbol.n_scnum == N_UNDEF) {
				if (symbol.n_value == 0)
				{
					/* Undefined symbol, add to uslst */
					if (numnold == 0)
						fatal ("Found undefined symbol %s and no '#object noload' specified",cursym);	

					usflag = TRUE;	/* At least one undefined symbol */
					/* Put symbol name in undefined */
					/* symbol hash table uslst */
					puslst = lookus (cursym);

					if (puslst == NULL){
					/* Name not found in undefined symbol */
					/* table - allocate newuslst */
						hval = hash(cursym, USSIZ);
						puslst = newusdef (stralloc(cursym), uslst[hval]);
						uslst[hval] = puslst;
					}
				}
				else {
					/* Undefined common symbols not allowed */
					(void)fprintf(stderr,"Warning: common symbol, %s, found in target\n",cursym);
					common_used = TRUE;
					}
			} else {

				/* check to see if current symbol (cursym) is in
				   branch table (btlst) */
				hval=hash(cursym, BTSIZ);
				tptr=btlst[hval];
				while (tptr != NULL) {
					if (strcmp(cursym, tptr->name) == 0)
						break;
					tptr= tptr->next;
				}
		
				if (tptr != NULL) {
					/* symbol is in branch table */
					tptr->found= TRUE;
		
					if (symbol.n_scnum != txtnum) 
						warn("symbol \"%s\" in branch table not defined in the .text section\n",cursym);
		
					/* change symbol name to branch table jump label */
					cursym= makelab(tptr->pos);
				} else {
					if (symbol.n_scnum == txtnum)
						warn("visible .text symbol, %s, not included in the branch table\n",cursym);
				}
			}
		}

		if ((len=strlen(cursym)) <= SYMNMLEN) {
			(void)strncpy(symbol.n_name, cursym, SYMNMLEN);
		} else {
			/* put name in string table */
			len++;	/* add room for \0 */
			slength += len;

			if (slength >= strsize) {
				strsize+= BUFSIZ * ((slength-strsize)/BUFSIZ + 1);
				if ((strtab=realloc(strtab,(unsigned)strsize))==NULL)
					fatal("Insufficient memory for string table");
				strnext= (strtab + slength) - len;
			}

			(void)strcpy(strnext, cursym);
			symbol.n_offset = (strnext - strtab) + sizeof(long);
			strnext += len;
		}

		/* write out symbol */
		if (fwrite((char *)&symbol, SYMESZ, 1, modpltrg) != 1)
			fatal("Cannot write symbol to temp file");

		for (i+=symbol.n_numaux, j=symbol.n_numaux; j; j--) {
			if (FREAD(&symbol,AUXESZ,1,pltrg) != 1)
				fatal("Cannot read symbol table of temp file");

			if (fwrite((char *)&symbol, AUXESZ, 1, modpltrg) != 1)
				fatal("Cannot write aux to temp file");
		}
	}
	
	if (common_used) fatal(
"There are no provisions for common symbols: provide initializations\n\
for all data items.");

	/* Scan btorder[] to see if any branch table entries have not yet been found.
	 * If any such entries exist, print an appropriate warning message.
	 */
	for (i=0; i<tabsiz; i++) 
	{
		if (btorder[i]->found != TRUE)
		{
			(void)fprintf(stderr,"Branch table entry for %s was not resolved\n",btorder[i]->name);
			badbranches = TRUE;
		}
	}
	if (badbranches)
		fatal("Unresolved branch table references.");

	/* copy string table */
	strsize= strnext - strtab;
	if (strsize) {
		strsize += sizeof(long);
		if ((fwrite((char *)&strsize, sizeof(long), 1, modpltrg) != 1) ||
				(fwrite(strtab,(int)strsize-sizeof(long),1,modpltrg) != 1))
			fatal("failed to write string table to temp file");
	}
	free(strtab);
	free(btorder);

	(void)ldclose(pltrg);
	(void)fclose(modpltrg);
	(void)unlink(tpltnam);
}


/* This routine creates an assembly listing for the branch table and .lib
 * section in the file whose name is assemnam.
 */
void
mkassem()
{
	FILE	*assembt;	/* pointer to assembly file representing branch
				   table */
	char	*label;		/* jump label of branch table entry */
	int	i;

	/* open assembt for writing */
	if ((assembt=fopen(assemnam,"w")) == NULL) 
		fatal("Cannot open temp file");

	/* print leading lines of assembly code for branch table */
	prlbtassem(assembt);	

	/* Generate branch table. */
	for (i=0; i<tabsiz; i++) {
		if (i == btorder[i]->pos - 1)
			labelassem(btorder[i]->name, assembt);
		label=makelab(btorder[i]->pos);
		genbtassem(label, assembt);
	}

	/* Create the .lib section. */
	genlibassem(assembt);

	/* close assembt */
	(void)fclose(assembt);
}


/* Create a new node for the hash table (i.e. btlst) */
btrec *
newbtrec(name, pos, next)
char	*name;
long	pos;
btrec	*next;
{
	btrec	*tnode;

	if ((tnode= (btrec *)malloc(sizeof(btrec))) == NULL)
		fatal("Out of space for hash table");

	tnode->name= name;
	tnode->pos= pos;
	tnode->found= FALSE;
	tnode->next=next;
	return(tnode);
}


/* Given a pointer to a branch table entry, create the corresponding
 * label- .bt<pos>
 */
char *
makelab(pos)
long	pos;
{
	static char label[20];	/* label must be large enough to hold .bt<pos>
				 * where <pos> is the decimal representation of pos.
				 * If pos is 4 bytes long the largest decimal number
				 * will be 11 characters so an array of 20 should
				 * be plenty big. */

	(void)sprintf(label,".bt%d",pos);
	return(label);
}
