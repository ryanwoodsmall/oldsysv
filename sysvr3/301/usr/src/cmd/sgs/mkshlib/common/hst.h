/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/hst.h	1.2"

#define TRSIZ	503	/* size of hash table trgsymtab */
#define OBSIZ	503	/* size of hash table objlst */
#define ARSIZ	503	/* size of hash table arsymtab */
#define TRUE	1
#define FALSE	0

/* Structures */

/* structure used to hold an entry in the target symbol table */
typedef struct stab {
        char		*name; 		/* name of symbol */
	long		absaddr;	/* absolute address of symbol */
        struct stab     *next;
} stab;

/* structure used to set up a list of symbols */
typedef struct symlst {
	SYMENT		*symptr;
	struct symlst 	*next;
} symlst;

/* structure used to keep all relevent section information */
typedef struct scnlst {
	SCNHDR	shead;			/* section header of section */
	char	*contents;		/* holds section contents */
	RELOC	*relinfo;	/* holds relocation info. */
	struct scnlst	*next;
} scnlst;

/* structure used to associate initialization code with an object file */
typedef struct initinfo {
	char	*initname;	/* name of object file holding .init information */
	char	*objname;	/* name of object file which will get initialization
				 * code */
	struct initinfo	*next;
} initinfo;

/* structure used to keep all relevent information about an input object file */
typedef struct obj {
        char    *objname;       /* name of object file */
        FILHDR  fhead;		/* file header of object */
	scnlst	*sects;		/* list of saved section information */
	initinfo *init;		/* .init section information */
	SYMENT	*symtab;	/* pointer to symbol table */
	char	*strtab;	/* pointer to string table */
        symlst	*chainlst;      /* list of file definition symbols of linked files */
	SYMENT	*fdefsym;	/* file definition symbol */
        struct obj	*next;
} obj;

/* structure use to hold an entry in the archive symbol table */
typedef struct arstab {
        char    *name;          /* symbol name */
        obj     *objptr;        /* pointer to the object file in objlst in which the
                                   symbol is defined */
        struct arstab      *next;
} arstab;

/* structure to describe a string table */
typedef struct strtbl {
	char	*start;		/* beginning of string table */
	char	*next;		/* pointer to the end of the string table */
	long	strsize;	/* maximum size of the string table */
} strtbl;


/* Symbols defined in extn.c */
extern char	*libdefsym,
		*moddir,
		*defname,
		*ainit;
extern initinfo	*inits;
extern stab	*trgsymtab[];
extern obj	*objlst[];
extern obj	**objects;
extern arstab	*arsymtab[];
extern char	*defsuffix;


/* User supplied functions */
extern void	archive();
extern void	checkinits();
extern void	creatobjs();
extern long	getabs();
extern char	*getdsuf();
extern void	initpr();
extern void	initsym();
extern void	init_trgsymtab();
extern symlst	*merge();
extern char	*mkdefsym();
extern void	mklibdef();
extern arstab	*newarstab();
extern initinfo	*newinitinfo();
extern obj	*newobj();
extern scnlst	*newscnlst();
extern stab	*newstab();
extern symlst	*newsymlst();
extern void	prlal();
extern void	readobj();
extern void	refchain();
extern void	writesym();
