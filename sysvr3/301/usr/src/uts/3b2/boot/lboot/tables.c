/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/tables.c	10.5"

/*
 * Symbol table processing routines
 *
 *
 * UNIX KERNEL SYMBOL TABLE
 * ------------------------
 *
 * The UNIX kernel symbol table is arranged to enable two types of access:
 * alphabetically, and by number.  The symbol table is numbered in
 * the object file, and relocation entries refer to symbols by number.
 * For symbol lookup, however, alphabetical searching is required.
 *
 * The Ksymtab[] array is the sorted symbol table containing the symbols
 * from the UNIX object file.  No auxilary entries or file names are
 * included.  Only symbols with storage class of C_STAT or C_EXT are
 * included.  In addition, only defined symbols are included.
 *
 * The K_ascii[] array is indexed by the first letter (ascii character)
 * of the symbol.  This provides the index of the first symbol in the
 * sorted symbol table beginning with that letter.
 *
 * The K_index[] array is indexed by the original symbol number.  This
 * provides a pointer to the symbol.  This pointer will be either a pointer
 * to the entry in Ksymtab[] or to the entry in *Xsymtab.
 *
 *
 * EXTERNAL SYMBOL TABLE
 * ---------------------
 *
 * All external symbols (those symbols which will be assigned values by
 * the boot program) are contained in the *Xsymtab symbol table.  This
 * symbol table is arranged as a sorted binary tree.
 *
 *
 * NOTES
 * -----
 * Symbols having a class of C_EXT or C_STAT are the only symbols included in
 * either symbol table.  Those symbols from the /unix object file having a
 * section number which is NOT zero are assumed to have CORRECT values; they
 * are entered into the Ksymtab[] array as DEFINED, and NO relocation is
 * performed for locations referencing these symbols.  Those symbols from the
 * /unix object file having a section number of zero are not defined; they are
 * entered into the *Xsymtab symbol table as EXTERN.  Relocation IS performed
 * for locations referencing these symbols.  In summary, relocation is performed
 * for symbols flagged EXTERN, and is not performed otherwise.
 *
 * All C_EXT or C_STAT symbols from a driver object file are entered into the
 * *Xsymtab symbol table, unless the symbol is already in the Ksymtab[] array.
 * Relocation is always performed for driver object files.
 */

#include <sys/types.h>
#include <a.out.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/psw.h>
#include <sys/pcb.h>
#include <sys/boothdr.h>
#include "error.h"

static ushort		K_ascii[128+1];

static SYMBOL        	**K_index;

static SYMBOL        	*Ksymtab;

int			Ksym_count;


/*
 * Struct Xsymbol must match SYMBOL
 */
struct	Xsymbol
	{
	char			*name;			/* symbol name */
	union	{
		address	value;				/* if DEFINED: value of symbol */
		struct Xreloc *reloc;			/*       else: ==> relocation entry chain */
		}
				x;
	unsigned long		size;			/* sizeof symbol */
	unsigned char		flag;			/* always EXTERN; DEFINED? */
	char			balance;		/* balance factor; '<', 0 or '>' */
	struct Xsymbol		*lnext;			/* ==> left child */
	struct Xsymbol		*rnext;			/* ==> right child */
	};



static struct Xsymbol	*Xsymtab = NULL;

int			Xsym_count;		/* running total of symbols in *Xsymtab */
int			Xsym_size;		/* running total of size of symbol names in *Xsymtab */


struct	Xreloc
	{
	struct Xreloc	*next;			/* ==> next relocation or NULL */
	address		r_paddr;		/* real address of reference */
	unsigned short	r_type;			/* relocation type */
	};

/*
 * Static function declarations for this file
 */
static void	Krelocate();
static void	relocate();
static void	Xsym_free();

/*
 * Ksymread(nsyms)
 *
 * Read and organize the UNIX symbol table from the boot program; a side effect
 * is to allocate the kernel BSS
 */
 void
Ksymread(nsyms)
	long nsyms;
	{PROGRAM(Ksymread)

	register i;
	register SYMENT *s;
	register SYMBOL *sp;
	register long *np, *n_shadow;
	char *p;
	int count;
	long size;

	/*
	 * count the number of symbol table entries that will be needed;
	 * only static and extern symbols are counted
	 *
 	 * at the same time, determine how much space is needed for the
	 * symbol names in Ksymtab[]
	 */
	if ((s=coff_symbol(bootprogram.path,(long*)NULL)) == NULL)
		{
		/* <bootprogram.path>: no symbols */
		error(ER60, bootprogram.path);
		goto exit;
		}

	count = size = 0;
	do	{
		if (s->n_scnum != 0  || s->n_sclass == C_TPDEF  ||
		   s->n_sclass == C_MOS  || s->n_sclass == C_MOU) {
			++count;

			if (s->n_zeroes)
				size += strlen(s->n_name) + 1;
			else
				size += strlen(s->n_name+s->n_offset) + 1;
			}
		}
		while ((s=coff_symbol((char*)NULL,(long*)NULL)) != NULL);

	Ksym_count = count;

#if TEST
	elapsed("UNIX symbol table counted");
#endif

	/*
	 * get the memory for the UNIX kernel symbol table
	 */
	if ((Ksymtab=sp=(SYMBOL *)malloc((unsigned)(count*sizeof(SYMBOL)+nsyms*sizeof(*K_index)+size))) == NULL)
		goto nomem;

	K_index = (SYMBOL **) (Ksymtab + count);

	p = (char*) (K_index + nsyms);

	/* pad n_shadow[] with one extra slot */
	if ((n_shadow=np=(long *)malloc(((unsigned)count+1)*sizeof(*n_shadow))) == NULL)
		goto nomem;

	/*
	 * build the UNIX symbol table and the n_shadow[] array; the n_shadow[]
	 * array will contain the original symbol table index for each symbol --
	 * that is, symbol[i] occured at location n_shadow[i] in the object file
	 *
	 * the rules used to determine the type of the symbol are:
	 *
	 *	n_scnum != 0	- the symbol is defined and has the value n_value
	 *	n_scnum == 0
	 *		n_value != 0	- the symbol is an undefined .bss symbol
	 *				  having a length of n_value
	 *		n_value == 0	- the symbol is an undefined external reference
	 *
	 * if the symbol is not defined, then add it to the *Xsymtab symbol
	 * table, and set the pointer in K_index now
	 */
	if ((s=coff_symbol(bootprogram.path,np)) == NULL)
		goto exit;

	do {
		if (s->n_scnum != 0  ||  s->n_sclass == C_TPDEF  ||
		   s->n_sclass == C_MOS  ||  s->n_sclass == C_MOU) {
			if (s->n_zeroes)
				strcpy(p, s->n_name);
			else
				strcpy(p, s->n_name+s->n_offset);

			sp->name = p;
			sp->value = s->n_value;
			sp->flag = DEFINED;

			if (s->n_sclass == C_STAT) {
				sp->flag |= STATIC;

				/* make sure Ksym_name() won't find statics */
				sp->name = "";
			} else if (s->n_sclass != C_EXT) {
				sp->flag |= PASSTHRU;
			}

			p += strlen(p) + 1;
			++sp;
			++np;
			}
		else
			{
			register SYMBOL *xsp;

			if (s->n_zeroes)
				xsp = Xsym_name(s->n_name);
			else
				xsp = Xsym_name(s->n_name+s->n_offset);

			K_index[ *np ] = xsp;

			if (s->n_value != 0)
				/*
				 * kernel BSS
				 */
				{
				xsp->flag |= BSS;
				xsp->size = s->n_value;
				allocate(&bss_locctr.v_locctr, xsp->name, (long)s->n_value);
				}
			}
		}
		while ((s=coff_symbol((char*)NULL,np)) != NULL);

#if TEST
	elapsed("UNIX symbol table read");
#endif

	/*
	 * sort the symbol table alphabetically, moving the corresponding
	 * n_shadow[] element
	 */
		{
		register m;

		for (m=count/2; m>0; m/=2)
			{
			for (i=0; i<count-m; ++i)
				{
				sp = & Ksymtab[i];
				np = & n_shadow[i];

				if (strcmp(sp[0].name,sp[m].name) > 0)
					{
					SYMBOL t;
					long t_shadow;

					t = sp[m];
					t_shadow = np[m];

					do	{
						sp[m] = sp[0];
						np[m] = np[0];
						np -= m;
						}
						while ((sp-=m)>=Ksymtab && strcmp(sp[0].name,t.name)>0);

					sp[m] = t;
					np[m] = t_shadow;
					}
				}
			}
		}

#if TEST
	elapsed("UNIX symbol table sorted");
#endif

	/*
	 * now finish the K_index[] array; K_index[i] will be a pointer to
	 * the symbol whose original index was i
	 */
	for (i=0; i<count; ++i)
		K_index[ n_shadow[i] ] = Ksymtab + i;

	free((char*)n_shadow);

	/*
	 * now build the K_ascii[] array; K_ascii['c'] will be the subscript
	 * of the first symbol whose name begins with the character 'c'
	 */
	i = 0;
	for (sp=Ksymtab; sp<&Ksymtab[count]; ++sp)
		{
		while (i <= sp->name[0])
			K_ascii[i++] = sp - Ksymtab;
		}

	while (i <= 0x80)
		K_ascii[i++] = count;

#if TEST
	elapsed("UNIX symbol table indexes built");

#if DEBUG1
	if (prt[_TABLES])
		shell();
#endif
#endif

	return;


	/*
	 * error exit
	 */
nomem:
	printf("No memory for symbol table\n");

exit:
	SIGNAL(ABORT);
	}

/*
 * Ksym_name(name)
 *
 * Locate a symbol by name in the UNIX kernel symbol table; NULL is returned
 * if not found
 * name must be a NULL terminated string
 */
 SYMBOL *
Ksym_name(name)
	register char *name;
	{PROGRAM(Ksym_name)

	register SYMBOL *p;
	register SYMBOL *q;
	register result, low, high, middle;

	low = K_ascii[ name[0] ];
	high = K_ascii[ name[0] + 1 ];

	do {
		p = Ksymtab + (middle = (low+high)>>1);

		if ((result=strcmp(name,p->name)) == 0)
			if (p->flag & STATIC  ||  p->flag & PASSTHRU)
				break;
			else
				return(p);

		if (result < 0)
			high = middle-1;
		else
			low = middle+1;
	} while (low <= high);

	if (!(p->flag & PASSTHRU))
		return(NULL);
	
	q = p - 1;
	while (!strcmp(name, q->name)) {
		if (!(q->flag & PASSTHRU))
			return(q);
		q--;
	}
	
	q = p + 1;
	while (!strcmp(name, q->name)) {
		if (!(q->flag & PASSTHRU))
			return(q);
		q++;
	}

	return(NULL);
}

/*
 * Ksym_number(number)
 *
 * Locate a symbol by number in the UNIX kernel symbol table
 */
 SYMBOL *
Ksym_number(number)
	register long number;
	{PROGRAM(Ksym_number)

	return(K_index[number]);
	}

/*	Ksym_copyall()
**
**	This routine is invoked only if DebugMode is set.
**	It copies all defined entries in Ksymtab to
**	Xsymtab.  This ensures that they get copied into
**	the sys3b symbol table where they can be accessed
**	by demon.
*/

Ksym_copyall()
{
	register SYMBOL	*sp;
	register SYMBOL	*splim;

	sp = Ksymtab;
	splim = &sp[Ksym_count];

	while (sp < splim) {
		if (sp->flag & DEFINED)
			ucxdefine(sp->name, sp->value);
		sp++;
	}
}

/*
 * Kload(fd, shdr, paddr)
 *
 * Load a section from the bootprogram.path file.
 */
 boolean
Kload(fd, shdr, paddr)
	register int fd;
	register SCNHDR *shdr;
	register address paddr;
	{PROGRAM(Kload)

	register FILE *stream;
	RELOC reloc;

	seek_and_check(fd, (long)shdr->s_scnptr, 0);

	read_and_check(fd, (char*)paddr, (unsigned)shdr->s_size);

	/*
	 * do any required relocation
	 */
	if (shdr->s_nreloc != 0)
		{

		seek_and_check(fd, (long)shdr->s_relptr, 0);

		stream = fdopen(fd);

		while (shdr->s_nreloc-- != 0)
			{
			if (fread((char*)&reloc,RELSZ,1,stream) == 0)
				{
				/* <filename>: perror() message */
				/* <filename>: truncated read */
				error(ferror(stream)? ER7 : ER49, filename(fileno(stream)));
				(void) fdclose(stream);
				return(FALSE);
				}

			Krelocate(&reloc, paddr, (address)shdr->s_vaddr);
			}

		(void) fdclose(stream);
		}

	return(TRUE);
	}

/*
 * Xsym_name(name)
 *
 * Locate a symbol by name in the *Xsymtab binary tree.  If not found, then
 * the symbol is inserted as EXTERN but not DEFINED.  This routine implements
 * a balanced tree search algorithm, reference Knuth (6.2.3) algorithm A.
 *
 * Note that STATIC symbols will never be found.
 */

#define	INVERSE(balance)	(balance ^ ('<'^'>'))

 SYMBOL *
Xsym_name(name)
	char *name;
	{PROGRAM(Xsym_name)

	register struct Xsymbol	*p;		/* Pointer moving down the tree */
	register struct Xsymbol	*q;		/* New node if key not found */
	register struct Xsymbol	*r;		/* Re-balance pointer */
	register struct Xsymbol	*s;		/* Re-balance point */
	register struct Xsymbol	*t = NULL;	/* Father of S */
	register int result;			/* Result of compare/balance factor */

	if ((p = Xsymtab) == NULL)				/* A1 */
		/*
		 * tree is empty
		 */
		{
		if ((p = Xsymtab = (struct Xsymbol*) malloc(sizeof(*p)+strlen(name)+1)) == NULL)
			panic("No memory for Xsymbol");

		Xsym_count = 1;
		Xsym_size = (strlen(name)+1+sizeof(long) + sizeof(long)-1) & ~(sizeof(long)-1);

		p->name = strcpy((char*)(p+1), name);
		p->size = 0;
		p->x.reloc = NULL;
		p->flag = EXTERN;
		p->balance = 0;
		p->lnext = p->rnext = NULL;

		return((SYMBOL *) p);
		}

	for (s=p; ; p=q)					/* A1; ; A3,A4 */
		{

		if ((result=strcmp(name,p->name)) == 0) /* A2: compare */
			if (p->flag & STATIC)
				result = 1;
			else
				return((SYMBOL *) p);

		if (result < 0)				/* A3: move left */
			{
			if ((q=p->lnext) == NULL)
				/*
				 * create new node
				 */
				{
				if ((q=p->lnext=(struct Xsymbol *)malloc(sizeof(*q)+strlen(name)+1)) == NULL)
					panic("No memory for Xsymbol");
				break;
				}
			}
		else						/* A4: move right */
			{
			if ((q=p->rnext) == NULL)
				/*
				 * create new node
				 */
				{
				if ((q=p->rnext=(struct Xsymbol *)malloc(sizeof(*q)+strlen(name)+1)) == NULL)
					panic("No memory for Xsymbol");
				break;
				}
			}

		if (q->balance != 0)
			{
			t = p;
			s = q;
			}
		}

	++Xsym_count;
	Xsym_size += (strlen(name)+1+sizeof(long) + sizeof(long)-1) & ~(sizeof(long)-1);

	q->name = strcpy((char*)(q+1), name);			/* A5: initialize new node */
	q->size = 0;
	q->x.reloc = NULL;
	q->flag = EXTERN;
	q->balance = 0;
	q->lnext = q->rnext = NULL;

	if ((result=strcmp(name,s->name)) < 0)		/* A6: adjust balance factors */
		r = p = s->lnext;
	else
		r = p = s->rnext;

	while (p != q)
		{
		if (strcmp(name,p->name) < 0)
			{
			p->balance = '<';
			p = p->lnext;
			}
		else
			{
			p->balance = '>';
			p = p->rnext;
			}
		}

	result = (result<0)? '<' : '>';				/* A7: balancing act */

	if (s->balance == 0)
		/*
		 * the tree has grown higher
		 */
		{
		s->balance = result;
		return((SYMBOL *) q);
		}

	if (s->balance == INVERSE(result))
		/*
		 * the tree has gotten more balanced
		 */
		{
		s->balance = 0;
		return((SYMBOL *) q);
		}

	/* s->balance == result
	 *
	 * the tree has gotten out of balance
	 */
	if (r->balance == result)
		/*
		 * single rotation				   A8
		 */
		{
		p = r;
		if (result == '<')
			{
			s->lnext = r->rnext;
			r->rnext = s;
			}
		else
			{
			s->rnext = r->lnext;
			r->lnext = s;
			}
		s->balance = r->balance = 0;
		}
	else
		/*
		 * double rotation				   A9
		 */
		{
		if (result == '<')
			{
			p = r->rnext;
			r->rnext = p->lnext;
			p->lnext = r;
			s->lnext = p->rnext;
			p->rnext = s;
			}
		else
			{
			p = r->lnext;
			r->lnext = p->rnext;
			p->rnext = r;
			s->rnext = p->lnext;
			p->lnext = s;
			}

		if (p->balance == 0)
			s->balance = r->balance = 0;
		else
			{
			if (p->balance == result)
				{
				s->balance = INVERSE(result);
				r->balance = 0;
				}
			else
			  /* p->balance == INVERSE(result) */
				{
				s->balance = 0;
				r->balance = result;
				}
			p->balance = 0;
			}
		}

	/*
	 * re-balancing transformation is now complete, with p pointing to the
	 * new root and t pointing to the father of the old root
	 */
	if (t == NULL)					/* A10: finishing touch */
		Xsymtab = p;
	else
		if (s == t->rnext)
			t->rnext = p;
		else
			t->lnext = p;

	return((SYMBOL *) q);
	}

/*
 * Xsym_walk(root, function, argument)
 *
 * Perform an in-order walk of the *Xsymtab symbol table.  At each node
 * call (*function) with arguments (sp, level) where "sp" is a pointer
 * to the node, and "level" is the current depth of the tree.
 */
 /*VARARGS2*/
 void
Xsym_walk(root, function, argument)
	register struct Xsymbol *root;
	register int (*function)();
	int argument;
	{PROGRAM(Xsym_walk)

	register int level;
	register struct Xsymbol *r;

	if (root == NULL)
		/*
		 * this is the first time; initiate an inorder traversal
		 */
		{
		if ((root = Xsymtab) == NULL)
			return;

		level = 1;
		}
	else
		level = argument;

	if (root->lnext)
		Xsym_walk(root->lnext, function, level+1);

	r = root->rnext;	/* in case *root is freed by (*function)() */

	(*function)(root, level);

	if (r)
		Xsym_walk(r, function, level+1);
	}

/*
 * Xsym_free(root)
 *
 * Free the Xsymtab entry *root and any chained relocation entries
 */
 static
 void
Xsym_free(root)
	register struct Xsymbol *root;
	{PROGRAM(Xsym_free)

	register struct Xreloc *rp, *tp;


	if (!(root->flag & DEFINED))
		{
		rp = root->x.reloc;

		while ((tp=rp) != NULL)
			{
			rp = rp->next;
			free((char*)tp);
			}
		}

	free((char*)root);
	}

#if DEBUG1a
/*
 * Xdepth(root, level)
 *
 * Compute the depth of the Xsymtab binary tree
 */
 /*VARARGS1*/
 int
Xdepth(root, level)
	register SYMBOL *root;
	register level;
	{PROGRAM(Xdepth)

	static long maxlevel;

	if (root == NULL)
		/*
		 * first time called; walk the tree
		 */
		{
		maxlevel = 0;

		Xsym_walk(Xsymtab, Xdepth, 1);

		return(maxlevel);
		}

	maxlevel = max(maxlevel, (long)level);
	}

/*
 * Xprint(root, level)
 *
 * Print the contents of the *Xsymtab in alphabetical order.
 */
 /*VARARGS1*/
 void
Xprint(root, level)
	register SYMBOL *root;
	register level;
	{PROGRAM(Xprint)

	if (root == NULL)
		/*
		 * first time called; walk the tree
		 */
		Xsym_walk(Xsymtab, Xprint, 1);
	else
		{

		printf("   [%2d]  size=%5D  flag=%2X  0x%8lX  %s\n", level, root->size, root->flag, root->value, root->name);
		}
	}
#endif

/*
 * Xsym_resolve(sp)
 *
 * Make sure that every symbol in *Xsymtab is resolved.  If a symbol is
 * not yet DEFINED (and it is not marked DEFER) then it may be a reference
 * to a routine name in an unloaded driver.  If so, call routine() to define
 * the symbol.  Otherwise, the symbol is an unresolved external reference.
 */
 void
Xsym_resolve(sp)
	register struct Xsymbol *sp;
	{PROGRAM(Xsym_resolve)

	if (!(sp->flag & DEFINED))
		/*
		 * symbol is still undefined
		 */
		{

		if (sp->flag & DEFER)
			/*
			 * symbol will be defined later (ie. we know about this one)
			 */
			return;

		if (!routine(sp->name))
			{
			/* External symbol <sp->name> is undefined; set to zero */
			error(ER61, sp->name);
			define(sp->name, 0L);
			}
		}
	}

/*
 * Xsym_copy(root, level, origin)
 *
 * Copy the *Xsymtab symbol table alphabetically to the s3bsym structure for
 * the sys3b(2) system call.
 */

 void
Xsym_copy(root, level, origin)
	register SYMBOL *root;
	int level;
	struct s3bsym *origin;
	{PROGRAM(Xsym_copy)

	register length;
	static char *s3bname;

	if (root == NULL)
		/*
		 * first time called; initiate the traversal of *Xsymtab
		 */
		{
		s3bname = origin->symbol;

		Xsym_walk(Xsymtab, Xsym_copy, 0);

		return;
		}

	if (!(root->flag & DEFINED))
		panic(root->name);

	length = (strlen(root->name)+1 + sizeof(long)-1) & ~(sizeof(long)-1);

	strncpy(s3bname, root->name, length);
	* (long*) (s3bname + length) = root->value;

	/*
	 * bump the floating pointer
	 */
	s3bname += length + sizeof(long);
	}

/*
 * Alloc_string(string)
 *
 * Allocate a character string in the data section and return its virtual
 * address. The data location counter is updated to the next mod sizeof(int)
 * boundry.
 */
 address
alloc_string(string)
	register char *string;
	{PROGRAM(alloc_string)

	register char *to;
	register address save;
	extern address hiwater;	/* highest data location in memory used so far */

	to = (char*) REAL(save=data_locctr.v_locctr, data_locctr);

	strcpy(to, string);

	data_locctr.v_locctr += (strlen(string) + 1 + sizeof(int) - 1) & ~(sizeof(int) - 1);

	hiwater = REAL(data_locctr.v_locctr, data_locctr);

	return(save);
	}

/*
 * Allocate(&locctr, name, size)
 *
 * Allocate `size' storage for symbol `name' at *locctr.  The location counter
 * is updated to the next mod sizeof(int) boundry.  The symbol `name'
 * must not have been previously defined.
 */
 address
allocate(locctr, name, size)
	register address *locctr;
	register char *name;
	long size;
	{PROGRAM(allocate)

	register struct Xsymbol *sp;
	register SYMBOL *up;
	register struct Xreloc *rp, *tp;

	if ((up=Ksym_name(name)) != NULL)
		{
		/* <name>: already allocated */
		error(ER62, name);
		return(up->value);
		}

	sp = (struct Xsymbol *) Xsym_name(name);

	if (sp->flag & DEFINED)
		{
		/* <name>: previously allocated */
		error(ER48, name);
		return(sp->x.value);
		}

	sp->flag |= DEFINED;

	/*
	 * If relocation must be performed, do it
	 */
	rp = sp->x.reloc;
	while ((tp=rp) != NULL)
		{
		relocate(rp->r_paddr, rp->r_type, *locctr);
		rp = rp->next;

		free((char*)tp);
		}

	sp->x.value = *locctr;

	*locctr += (size + sizeof(int) - 1) & ~(sizeof(int) - 1);

	return(sp->x.value);
	}

/*
 * Define(name, value)
 *
 * Define symbol `name' as value `value'.  The symbol `name' must not have
 * been previously defined.  If any relocation was queued for this symbol,
 * then take care of it.
 */
 SYMBOL *
define(name, value)
	register char *name;
	address value;
	{PROGRAM(define)

	register struct Xsymbol *sp;
	register SYMBOL *up;
	register struct Xreloc *rp, *tp;

	if ((up=Ksym_name(name)) != NULL)
		{
		/* <name>: already defined */
		error(ER63, name);
		return(up);
		}

	sp = (struct Xsymbol *) Xsym_name(name);

	if (sp->flag & DEFINED)
		{
		/* <name>: previously defined */
		error(ER64, name);
		return((SYMBOL *) sp);
		}

	sp->flag |= DEFINED;

	/*
	 * If relocation must be performed, do it
	 */
	rp = sp->x.reloc;
	while ((tp=rp) != NULL)
		{
		relocate(rp->r_paddr, rp->r_type, value);
		rp = rp->next;

		free((char*)tp);
		}

	sp->x.value = value;

	return((SYMBOL *) sp);
	}

/*	ucxdefine(name, value)
**
**	Unconditionally define a symbol.  This rouine is basically
**	the same as "define(name, value)" above except that here
**	it is not an error for the symbol to already be defined
**	and no relocation is done.  This routine is used by
**	Ksym_copyall() to copy all defined symbols from Ksymtab
**	to Xsymtab just before the sys3b symbol table is written.
**	This ensures that all defined symbols are known to demon.
*/

ucxdefine(name, value)
register char		*name;
register address	value;
{
	register struct Xsymbol	*sp;

	sp = (struct Xsymbol *)Xsym_name(name);
	if (sp->flag & DEFINED)
		return(sp);
	sp->flag |= DEFINED;
	sp->x.value = value;
	return(sp);
}

/*
 * Krelocate(reloc, porigin, vorigin)
 *
 * This routine will either perform the relocation, or queue a Xreloc entry
 * for a symbol in the /unix object file.  Input is a relocation entry from
 * the /unix object file, the real address of the origin of the section
 * applicable to the relocation entry, and the virtual origin of the section.
 *
 * If the symbol refered to is not EXTERN, then it is assumed that the location
 * needs no relocation (since all defined symbols must already be bound to the
 * correct address in the /unix object file).  If the symbol is EXTERN, then it
 * must be either DEFINED or not.  If DEFINED, then the relocation is performed:
 *
 *	location porigin+reloc->r_vaddr-vorigin += symbol->x.value
 *
 * If not DEFINED, then a Xreloc entry is added to the chain at symbol->x.reloc.
 */
 static
 void
Krelocate(reloc, porigin, vorigin)
	register RELOC *reloc;
	register address porigin;
	register address vorigin;
	{PROGRAM(Krelocate)

	register struct Xsymbol *sp;
	register struct Xreloc *rp;

	if (((sp=(struct Xsymbol *)Ksym_number((long)reloc->r_symndx))->flag & EXTERN) == 0)
		return;

	porigin += reloc->r_vaddr - vorigin;

	if (sp->flag & DEFINED)
		{
		if (sp->flag & BSS)
			/*
			 * size of symbol must be backed out of reference
			 */
			relocate(porigin, reloc->r_type, -sp->size);

		relocate(porigin, reloc->r_type, sp->x.value);
		}
	else
		{
		if ((rp=(struct Xreloc *)malloc(sizeof(*rp))) == NULL)
			panic("No memory for Xreloc");

		rp->next = sp->x.reloc;
		rp->r_paddr = porigin;
		rp->r_type = reloc->r_type;

		sp->x.reloc = rp;
		}
	}

/*
 * Xrelocate(paddr, type, name)
 *
 * This routine will either perform the relocation, or queue a Xreloc entry
 * for a symbol.  Input is the real address to be relocated, the type of
 * relocation and the name of the symbol to which `paddr' is to be relocated.
 */
 void
Xrelocate(paddr, type, name)
	register address paddr;
	register unsigned short type;
	register char *name;
	{PROGRAM(Xrelocate)

	register struct Xsymbol *sp;
	register struct Xreloc *rp;

	if ((sp=(struct Xsymbol *)Ksym_name(name)) == NULL)
		sp = (struct Xsymbol *)Xsym_name(name);

	if (sp->flag & DEFINED)
		relocate(paddr, type, sp->x.value);
	else
		{
		if ((rp=(struct Xreloc *)malloc(sizeof(*rp))) == NULL)
			panic("No memory for Xreloc");

		rp->next = sp->x.reloc;
		rp->r_paddr = paddr;
		rp->r_type = type;

		sp->x.reloc = rp;
		}
	}

/*
 * Drelocate(paddr, type, symbol, adjustment)
 *
 * This routine will either perform the relocation, or queue a Xreloc entry
 * for a symbol refered-to by a driver.  Input is the real address to be
 * relocated, the type of relocation, the pointer to the symbol to which
 * `paddr' is to be relocated, and an adjustment value which must be added
 * to the location being relocated.
 *
 * The adjustment value is necessary to undo the partial relocation already
 * done for a symbol defined within the driver.  The location within the
 * driver containing the reference to the symbol defined within the driver
 * already contains the value of the symbol.  Since the value of the symbol
 * is now different, the difference between the original section origin and
 * the new section origin must be added to the location being relocated.
 */
 void
Drelocate(paddr, type, sp, adjustment)
	register address paddr;
	register unsigned short type;
	SYMBOL *sp;
	register unsigned long adjustment;
	{PROGRAM(Drelocate)

	register struct Xsymbol *xp = (struct Xsymbol *)sp;
	register struct Xreloc *rp;

	/*
	 * if the value to be relocated must be adjusted first, then do it
	 */
	if (adjustment)
		relocate(paddr, type, adjustment);

	/*
	 * now, either do the actual relocation or queue an Xreloc entry
	 */
	if (xp->flag & DEFINED)
		relocate(paddr, type, xp->x.value);
	else
		{
		if ((rp=(struct Xreloc *)malloc(sizeof(*rp))) == NULL)
			panic("No memory for Xreloc");

		rp->next = xp->x.reloc;
		rp->r_paddr = paddr;
		rp->r_type = type;

		xp->x.reloc = rp;
		}
	}

/*
 * Relocate(paddr, type, value)
 *
 * Relocate the word at location `paddr' to point to `value'.  The relocation
 * is specified by `type'. 
 */
 static
 void
relocate(paddr, type, value)
	register address paddr;
	unsigned short type;
	register unsigned long value;
	{PROGRAM(relocate)

	union	word
		{
		char		byte[4];
		unsigned long	value;
		};

	register union word *p;
	union word temp;

	p = (union word*) paddr;

	switch (type)
		{

	/*
	 * 32 bit direct reference
	 */
	case R_DIR32:
		p->value += value;
		break;

	/*
	 * 32 bit direct reference with bytes swapped
	 */
	case R_DIR32S:
		temp.byte[0] = p->byte[3];
		temp.byte[1] = p->byte[2];
		temp.byte[2] = p->byte[1];
		temp.byte[3] = p->byte[0];

		temp.value += value;

		p->byte[0] = temp.byte[3];
		p->byte[1] = temp.byte[2];
		p->byte[2] = temp.byte[1];
		p->byte[3] = temp.byte[0];
		break;

	/*
	 * unknown relocation
	 */
	default:
		/* Unsupported relocation type */
		error(ER6);
		}
	}

/*
 * Generate(what, args...)
 *
 * Allocate the memory and define the symbol for a generated data structure
 *
 *
 *		      WHAT        #ARGS
 *		----------------  -----  ------------ARGUMENTS-------------
 *	G_TEXT	            text     3   char* name, long size, char* init
 *	G_DATA	initialized data     3   char* name, long size, char* init
 *	G_UDATA	    uninit. data     2   char* name, long size
 *	G_BSS	             bss     2   char* name, long size
 *	G_PCB	             pcb     4   char* name, char* entry, ipl, vector#
 *	G_IRTN	interupt routine     3   char* name, char* entry, minor
 *	G_IOSYS	   I/O subsystem     0
 */

/*
 * Template for interrupt routine interface
 */
static	char	template[] ={   
				0xa0, 0x5f, 0, 0,			/* PUSHW &0 */
				0x2c, 0xcc, 0xfc, 0x7f, 0, 0, 0, 0,	/* CALL  -4(%sp),0 */
				0x24, 0x7f, 0, 0, 0, 0 };		/* JMP   int_ret */

int		IRTNSIZE ={ (sizeof(template) + sizeof(int) -1) & ~(sizeof(int)-1) };

#define	PUSHW	0		/* offset in template[] to PUSHW instruction */
#define	CALL	4		/* offset in template[] to CALL instruction */
#define	JMP	12		/* offset in template[] to JMP instruction */



/*VARARGS1*/
 void
generate(what, args)
	int what;
	int args;
	{PROGRAM(generate)

	register union	what
			{
			struct	{		/* G_TEXT, G_DATA, G_UDATA, G_BSS */
				char	*name;
				long	size;
				char	*init;
				}
				db;

			struct	{		/* G_PCB, G_IRTN */
				char	*name;
				char	*entry;
				int	value;
				int	vec;
				}
				pi;
			}
			*arg = (union what*) &args;

	char name[9];
	extern address hiwater;	/* highest data location in memory used so far */

	switch (what)
		{
	/*
	 * TEXT
	 */
	case G_TEXT:
			{
			register char *to;

			to = (char*) REAL(allocate(&text_locctr.v_locctr,arg->db.name,arg->db.size), text_locctr);
			bcopy(arg->db.init, to, (unsigned)arg->db.size);
			}
		break;
	/*
	 * Unitialized DATA
	 */
	case G_UDATA:
			{
			register address to;

			to = (address) REAL(allocate(&data_locctr.v_locctr,arg->db.name,arg->db.size), data_locctr);

			bzero(to, arg->db.size);

			hiwater = REAL(data_locctr.v_locctr, data_locctr);
			}
		break;
	/*
	 * DATA
	 */
	case G_DATA:
			{
			register char *to;

			to = (char*) REAL(allocate(&data_locctr.v_locctr,arg->db.name,arg->db.size), data_locctr);

			bcopy(arg->db.init, to, (unsigned)arg->db.size);

			hiwater = REAL(data_locctr.v_locctr, data_locctr);
			}
		break;
	/*
	 * BSS
	 */
	case G_BSS:
		allocate(&bss_locctr.v_locctr, arg->db.name, arg->db.size);
		break;
	/*
	 * I/O subSYStem: [cb]devcnt, [cb]devsw, MAJOR, MINOR
	 */
	case G_IOSYS:
			{
			register i, x, j;

			generate(G_DATA, "cdevcnt", sizeof(int), &cdevcnt);
			generate(G_UDATA, "cdevsw", cdevcnt*sizeof(struct cdevsw));
			cdevswp = (struct cdevsw *) REAL(Xsym_name("cdevsw")->value, data_locctr);
			for (i=0; i<cdevcnt; ++i)
				{
				register struct cdevsw *p = &cdevswp[i];

				p->d_open = p->d_close = p->d_read = p->d_write = p->d_ioctl = (int(*)()) (rtname[RNODEV].symbol->value);
				p->d_ttys = NULL;
				}

			generate(G_DATA, "bdevcnt", sizeof(int), &bdevcnt);
			generate(G_UDATA, "bdevsw", bdevcnt*sizeof(struct bdevsw));
			bdevswp = (struct bdevsw *) REAL(Xsym_name("bdevsw")->value, data_locctr);
			for (i=0; i<bdevcnt; ++i)
				{
				register struct bdevsw *p = &bdevswp[i];

				p->d_open = p->d_close = p->d_strategy = p->d_print = (int(*)()) (rtname[RNODEV].symbol->value);
				}

			generate(G_UDATA, "MAJOR", 128);
			MAJOR = (char*) REAL(Xsym_name("MAJOR")->value, data_locctr);
			x = max((long)bdevcnt, (long)cdevcnt);
			for (i=0; i<128; ++i)
				MAJOR[i] = x;

			generate(G_UDATA, "MINOR", 128);
			MINOR = (char*) REAL(Xsym_name("MINOR")->value, data_locctr);

			generate(G_DATA, "fmodcnt", sizeof(int), &fmodcnt);
			generate(G_UDATA, "fmodsw",
				 fmodcnt * sizeof(struct fmodsw));
			fmodswp = (struct fmodsw *)REAL(Xsym_name("fmodsw")->value,
					data_locctr);
			generate(G_DATA,"fstypcnt",sizeof(int),&fstypcnt);
			generate(G_DATA,"nfstyp",sizeof(short),&nfstyp);
			generate(G_UDATA,"fstypsw",fstypcnt*sizeof(struct fstypsw) );
			fstypswp = (struct fstypsw *) REAL(Xsym_name("fstypsw")->value, data_locctr);
			for (i=0; i<fstypcnt; ++i)
				{
				register struct fstypsw *fs = &fstypswp[i];
				fs->fs_init = fs->fs_iput =  
				fs->fs_filler = fs->fs_iupdat = fs->fs_readi = 
				fs->fs_writei = fs->fs_itrunc = fs->fs_statf =
				fs->fs_namei = fs->fs_mount = fs->fs_umount =
				fs->fs_openi = fs->fs_closei =
				fs->fs_update = fs->fs_statfs = fs->fs_access = 
				fs->fs_getdents = fs->fs_allocmap = 
				fs->fs_readmap = fs->fs_setattr = 
				fs->fs_notify = fs->fs_fcntl = fs->fs_ioctl =
				fs->fs_fsinfo =
				  NULL;
				/*
				(int(*)()) (rtname[RFSNULL].symbol->
								value);
								*/
				fs->fs_iread = fs->fs_getinode = 
				(struct inode *(*)()) (rtname[RFSNULL].symbol->value);
				fs->fs_freemap = (int *(*)()) (rtname[RFSNULL].symbol->value);
				for (j=0;j<5;j++)
					fs->fs_fill[j] = (int(*)()) (rtname
						[RFSNULL].symbol->value);
			}
			generate(G_DATA,"fsincnt",sizeof(int),&fsincnt);

			generate(G_UDATA,"fsinfo",fsincnt*sizeof(struct fsinfo) );
			fsinfop = (struct fsinfo *) REAL(Xsym_name("fsinfo")->value, data_locctr);
			for (i=0; i<fsincnt; i++)
				{
				register struct fsinfo *fo = &fsinfop[i];
				fo->fs_flags = 0;
				/*
				fo->fs_name = (char(*)()) (rtname
						[RFSNULL].symbol->value);
				fo->fs_pipe = (struct mount(*)()) (rtname
						[RFSNULL].symbol->value);
				*/
				fo->fs_name = '\0';
				fo->fs_pipe = NULL;
				fo->fs_notify = 0;
			}
		}
		break;
	/*
	 * Interrupt RouTiNe
	 */
	case G_IRTN:
			{
			register char *text;

			generate(G_TEXT, arg->pi.name, IRTNSIZE, template);
			text = (char*) REAL(Xsym_name(arg->pi.name)->value, text_locctr);

			text[PUSHW+2] = arg->pi.value;
			
			Xrelocate((address)&text[CALL+4], R_DIR32S, arg->pi.entry);
			Xrelocate((address)&text[JMP+2], R_DIR32S, "int_ret");
			}
		break;
	/*
	 * Process Control Block
	 */
	case G_PCB:
			{
			static psw_t psw = KPSW0;
			register struct kpcb *pcbp, **Ivect;
			register SYMBOL *xp;

			generate(G_UDATA, arg->pi.name, sizeof(struct kpcb));
			pcbp = (struct kpcb *) REAL(Xsym_name(arg->pi.name)->value, data_locctr);

			pcbp->ipcb.psw = psw;
			pcbp->ipcb.psw.IPL = arg->pi.value;
			Xrelocate((address) &(pcbp->ipcb.pc), R_DIR32, arg->pi.entry);

			strcpy(name, "kstk?");
			name[4] = "0123456789ABCDEF"[arg->pi.value];
			if ((xp=Ksym_name(name)) == NULL)
				{
				xp = Xsym_name(name);
				if (xp->flag & DEFINED)
					pcbp->ipcb.sp = (int*) (xp->value);
				else
					pcbp->ipcb.sp = (int*) allocate(&bss_locctr.v_locctr,name,(long)(ISTKSZ*sizeof(int)));
				}
			else
				pcbp->ipcb.sp = (int*)xp->value;

			pcbp->slb = pcbp->ipcb.sp;
			pcbp->sub = pcbp->slb + ISTKSZ;
			pcbp->movesize = 0;

			if (virtual0 != -1)
				{
				Ivect = (struct kpcb **) (virtual0 + 0x8C);

				Ivect[arg->pi.vec] = NULL;
				Xrelocate((address) &Ivect[arg->pi.vec], R_DIR32, arg->pi.name);
				}
			}
		break;
		}
	}

/*
 * Treset()
 *
 * Reset the values of all variables in this file.  This is necessary to
 * clean-up after a failed boot attempt.
 */
 void
treset()
	{PROGRAM(treset)

	if (Ksymtab)
		{
		free((char*)Ksymtab);
		Ksymtab = NULL;
		}

	if (Xsymtab)
		Xsym_walk(Xsymtab, Xsym_free);

	Xsymtab = NULL;
	Xsym_count = 0;
	}
