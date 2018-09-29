/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/hashfcns.c	1.8"


#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"
#include	"syms.h"
#include	"structs.h"

/* true if symbol is of type structure */
#define	ISS(x)  (((x) & N_BTMASK)==T_STRUCT)
/* true if symbol is of type union  */
#define	ISU(x)  (((x) & N_BTMASK)==T_UNION)
/* true if symbol is of derived type pointer */
#define	ISAPTR(x)  ((((x) & N_TMASK) == (DT_PTR << N_BTSHFT)) || \
	(((x) & 0xc0)==0x40) || (((x) & 0x300)==0x100) || (((x) & 0xc00)==0x400) || \
	(((x) & 0x3000)==0x1000) || (((x) & 0xc000)==0x4000))


pessymrec	eshash[ESHTSIZ]; /* hash table for all external and static symbols which
				are not unions or arrays */
puarec		ualist;	/* list of all external and static unions and arrays */
puarec		pua; /* pointer to the end of ualist */

plocsymrec	lochash[LOCHTSIZ]; /* hash table containing all local symbols for the
				current functions which are not unions or arrays */
plocuarec	locualist; /* list of all unions and arrays defined in the current 
			      function */
plocuarec	plocua; /* pointer to the end of plocua */


/* initializes the external and static hash table (eshash[]) and the external and
   static union-array list (ualist). */
init_eshash()
{
	extern char	*ldgetname();
	extern char	*namealloc();

	extern FILHDR	filhdr;
	extern LDFILE	*symb;

	long 	i;
	SYMENT	symbol;
	AUXENT	auxent;
	char	*tname;

	/* set up external-static hash table */
	for (i=0;i<ESHTSIZ;i++) {
		if ((eshash[i]= (pessymrec)malloc(sizeof(essymrec))) == NULL)
			fatal("Out of space. Do not use -s option.");
		eshash[i]->next = NULL;
	}

	/* set up external-static union-array list */
	if ((ualist= (puarec)malloc(sizeof(uarec))) == NULL)
		fatal("Out of space. Do not use -s option.");
	ualist->next = NULL;
	pua=ualist;

	/* loop through symbol table */
	for (i=0; i<filhdr.f_nsyms; i++) {
		if (ldtbread(symb, i, &symbol) == FAILURE)
			fatal("cannot read symbol table");
		i+=symbol.n_numaux;
		if (symbol.n_numaux == 1) {
			if (ldtbread(symb, i, &auxent)==FAILURE)
				fatal("cannot read symbol table");
			/* skip over structure and union tags */
			if (ISTAG(symbol.n_type)) {
				i=auxent.x_sym.x_fcnary.x_fcn.x_endndx-1;
				continue;
			}
		}

		/* Make sure symbol is a defined external or static; 
		   undefined symbols (including external symbols in .o files)
		   will have a section number of zero. */
		if (((symbol.n_sclass != C_EXT) && 
			(symbol.n_sclass != C_STAT)) || (symbol.n_scnum == 0)) {
				continue;
		}
		if ((tname=ldgetname(symb,&symbol))==NULL)
			fatal("ldgetname returned NULL");
#ifdef VAX
		if (tname[0] == '_') tname++;
#endif

		/* skip over section symbols (e.g., .text,.data and .bss) since
		   it is the first symbol declared in these sections
		   which is of interest and not the address of the
		   section in itself */
		if ((strncmp(tname, ".text", 5) == 0) ||
		    (strncmp(tname, ".init", 5) == 0) ||
		    (strncmp(tname, ".data", 5) == 0) ||
		    (strncmp(tname, ".bss", 4) == 0) ||
		    (strncmp(tname, ".comment", 8) == 0) ||
		    (strncmp(tname, ".lib", 4) == 0))
				continue;

		tname= namealloc(tname);
		symprocess(&tname,symbol.n_value,symbol.n_type,symbol.n_sclass,&auxent);
	}
}


/*
   setup_lochash() sets up the local symbol hash table and union-array list.
*/
setup_lochash()
{
	extern plocsymrec	lochash[];
	extern plocuarec	locualist;

	int	i;

	for (i=0;i<LOCHTSIZ;i++) {
		if ((lochash[i]= (plocsymrec)malloc(sizeof(locsymrec))) == NULL)
			fatal("Out of space");
	}

	if ((locualist= (plocuarec)malloc(sizeof(locuarec))) == NULL)
		fatal("Out of space");
}

/* initializes the local hash table (lochash[]) and the local
   union-array list (locualist) for the current function. */
init_lochash()
{
	extern char		*ldgetname();
	extern char		*namealloc();

	extern plocsymrec	lochash[];
	extern plocuarec	locualist;

	extern FUNCLIST	*currfunc;
	extern SCNHDR	scnhdr;
	extern FILHDR	filhdr;
	extern LDFILE	*symb;

	long	i;
	SYMENT	symbol;
	AUXENT	auxent;
	char	*tname;
	char	class;

	for (i=0;i<LOCHTSIZ;i++) 
		lochash[i]->next = NULL;

	locualist->next = NULL;
	plocua= locualist;

	if ((i= currfunc->fcnindex+ currfunc->fnumaux + 1) >= filhdr.f_nsyms)
		return;
	if (ldtbread(symb, i,&symbol) == FAILURE)
		fatal("cannot read symbol table");
	if ((tname= ldgetname(symb, &symbol)) == NULL)
		fatal("cannot get symbol name");
#ifdef VAX
	if (tname[0] == '_') tname++;
#endif

	/* check to see that we're at the beginning of the function */
	if (strncmp(tname,".bf",3) != 0)
		return;

	/* the loop will be broken when the first .ef (end of function) 
	   is encountered.  Otherwise, it will stop at the end of the
	   symbol table. */
	for (i+=2; i<filhdr.f_nsyms; i++) {
		if (ldtbread(symb, i,&symbol) == FAILURE)
			fatal("cannot read symbol table");
		if ((tname= ldgetname(symb, &symbol)) == NULL)
			fatal("cannot get symbol name");
#ifdef VAX
		if (tname[0] == '_') tname++;
#endif

		if (strncmp(tname,".ef",3) == 0)
			break;

		i+= symbol.n_numaux;
		if (symbol.n_numaux == 1) {
			if (ldtbread(symb, i, &auxent) == FAILURE)
				fatal("cannot read symbol table");
			/* skip over structure and union tags */
			if (ISTAG(symbol.n_type)) {
				i= auxent.x_sym.x_fcnary.x_fcn.x_endndx - 1;
				continue;
			}
		}
	
		class= symbol.n_sclass;
		/* make sure we have a local symbol */
		if ((class != C_AUTO) && (class != C_ARG) && (class != C_REG))
			continue;

		tname=namealloc(tname);
		symprocess(&tname,symbol.n_value,symbol.n_type,class,&auxent);
	}
}


/* 
   strucmem() creates a hash table or union-array list entry for each member of
   a structure. strucmem() is called recursively to handle nested structures.
*/
strucmem(bname,val,tag,class)
char *bname; /* name of structure symbol */
long val;  /* offset from the beginning of the structure */
long tag;  /* index of the structure tag in the symbol table */
char  class; /* storage class of topmost structure */
{
	SYMENT	symbol;
	AUXENT	auxent;
	long	tagend;  /* index of first entry after structure in 
			    the symbol table */
	long	j;
	char	*namearr;
	char	*tname;
	long	base;	/* value of current symbol */
	
	if (ldtbread(symb,tag+1,&auxent) == FAILURE)
		fatal("cannot read symbol table");
	tagend=auxent.x_sym.x_fcnary.x_fcn.x_endndx - 2;

	/* loop through structure tag */
	for (j=tag+2;j<tagend;j++) {
		if (ldtbread(symb,j,&symbol)==FAILURE)
			fatal("cannot read symbol table");
		j+=symbol.n_numaux;
		if (symbol.n_numaux == 1) {
			if (ldtbread(symb, j, &auxent)==FAILURE)
				fatal("cannot read symbol table");
		}

		if ((tname=ldgetname(symb,&symbol))==NULL)
			fatal("ldgetname returned NULL");
#ifdef VAX
		if (tname[0] == '_') tname++;
#endif
		/* namearr will hold the full structure member name */
		if ((namearr= (char *)malloc(strlen(bname)+strlen(tname)+2))==NULL)
			fatal("Out of space");
		sprintf(namearr,"%s.%s",bname,tname);

		base= symbol.n_value + val;
		symprocess(&namearr,base,symbol.n_type,class,&auxent);
	}
}

/*
   symprocess() enters a symbol in the appropriate hash table or union-array list.
   If the symbol is a structure, the strucmem() is called to resolve each member.
*/
symprocess(symname,val,typ,class,pauxent)
char	**symname;	/* name of symbol */
long	val;		/* value of symbol */
unsigned short typ;	/* type of symbol */
char	class;		/* storage class of symbol */
AUXENT	*pauxent;	/* pointer to the aux. entry for the symbol (if any) */
{
	extern 	char		*namealloc();
	extern	pessymrec	hashes();
	extern	plocsymrec	hashloc();

	extern puarec		pua;
	extern plocuarec	plocua;

	pessymrec	prec; /* pointer to external-static hash table entry */
	plocsymrec	plocrec; /* pointer to local hash table entry */
	char		*tname;

	if (ISS(typ) && (!ISAPTR(typ)) && (!ISFCN(typ))) {
		/* Symbol is a structure. Resolve each member of the structure */
		if (ISARY(typ)) {
			/* The addres of an element in the i'th structure
			   in an array of structures is often determined by
			   first calculating the offset of the i'th 
			   structure (i * (structure size)) and adding this
			   value to structure[0].element */
			if ((tname= (char *)malloc(strlen(*symname)+3))==NULL)
				fatal("Out of space");
			sprintf(tname,"%s[]",*symname);
			*symname= tname;
		}
		/* strucmem hashes every member of a struct. */
		strucmem(*symname,val,pauxent->x_sym.x_tagndx,class);
	} else if ((ISU(typ) || ISARY(typ)) && (!ISAPTR(typ)) && (!ISFCN(typ))) {
		/* Symbol is a union or array. Add it to the appropriate 
		   union-array list */
		if ((class == C_EXT) || (class == C_STAT)) {
			/* add to external-static union-array list */
			if ((pua->next= (puarec)malloc(sizeof(uarec)))==NULL)
				fatal("Out of space");
			pua= pua->next;
			pua->name= namealloc(*symname);
			pua->symval= val;
			pua->type= typ;
			pua->range=pauxent->x_sym.x_misc.x_lnsz.x_size;
			pua->next= NULL;
		} else {
			/* add to local union-array list */
			if ((plocua->next=(plocuarec)malloc(sizeof(locuarec)))==NULL)
				fatal("Out of space");
			plocua= plocua->next;
			plocua->name= namealloc(*symname);
			plocua->symval= val;
			plocua->type= typ;
			plocua->sclass= class;
			plocua->range=pauxent->x_sym.x_misc.x_lnsz.x_size;
			plocua->next= NULL;
		}
	} else {
		/* Add symbol to the appropriate hash table */
		if ((class == C_EXT) || (class == C_STAT)) {
			/* add to external or static hash table */
			prec= hashes(val);
			prec->name= namealloc(*symname);
			prec->symval= val;
			prec->next= NULL;
		} else {
			/* add to local hash table */
			plocrec= hashloc(val);
			plocrec->name= namealloc(*symname);
			plocrec->symval= val;
			plocrec->sclass= class;
			plocrec->next= NULL;
		}
	}
}

/*
   hashes() adds a new node to the external-static hash table (eshash[]) and returns
   a pointer to this node. The node is added to the end of the list pointed to
   by the hash value of val.
*/
pessymrec
hashes(val) 
long val;
{
	extern pessymrec	eshash[];

	pessymrec	prec;
	int		idx;	/* index of hash table */

	idx= comphash(val,ESHTSIZ);
	prec=eshash[idx];
	while (prec->next != NULL)
		prec= prec->next;
	if ((prec->next= (pessymrec)malloc(sizeof(essymrec))) == NULL)
		fatal("Out of hash table space");
	return(prec->next);
}

	

/*
   hashloc() adds a new node to the local hash table (lochash[]) and returns
   a pointer to this node. The node is added to the end of the list pointed to
   by the hash value of val.
*/
plocsymrec
hashloc(val) 
long val;
{
	extern plocsymrec	lochash[];

	plocsymrec	plocrec;
	int	idx;	/* index of hash table */

	idx= comphash(val,LOCHTSIZ);
	plocrec=lochash[idx];
	while (plocrec->next != NULL)
		plocrec= plocrec->next;
	if ((plocrec->next= (plocsymrec)malloc(sizeof(locsymrec))) == NULL)
		fatal("Bad Luck: Ran out of hash table space");
	return(plocrec->next);
}

/*
   comphash() computes the hash table value  of val in a table of size tabsiz.
   It returns the computed hash value.
*/
comphash(val,tabsiz)
long	val;
int	tabsiz;
{
	int idx;
	idx=((unsigned long)val >> 2) % tabsiz;
	return(idx);
}

/*
   namealloc() allocates enough space to hold the string pointed to by name and
   copies the string into the allocated space. The return value is a pointer to the
   allocated space.
*/
char *
namealloc(name)
char	*name;
{
	char	*nameptr;

	if ((nameptr= (char *)malloc(strlen(name)+1)) == NULL)
		fatal("Out of space");
	strcpy(nameptr,name);
	return(nameptr);
}
