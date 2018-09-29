/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/extn.c	1.4"

#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "scnhdr.h"
#include "storclass.h"
#include "ldfcn.h"
#include "reloc.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"

/* common global variables */
char	*specname=NULL;	/* holds name of shared lib. specification file */
char	*trgname=NULL;	/* holds name of target shared library */
char	*hstname=NULL;	/* holds name of host shared library */
char	*idstr=NULL;	/* holds #ident string of spec. file if one exists */
char	*progname=NULL;	/* holds name of program */
char	*prefix=NULL;	/* holds prefix of mkbt (E.g., 3b2mkbt) */
char	*ldname=NULL;	/* name of ld command (e.g., 3b2ld) */
char	*asname=NULL;	/* name of as command (e.g., 3b2as) */

char    **trgobjects;   /* list of input object files in the proper order
			 * for the target */
long    numobjs;	/* number of input object files */
int	maketarg=FALSE,	/* TRUE if the target is to be regenerated */
	makehost=FALSE,	/* TRUE if the host is to be built */
	qflag=FALSE;	/* TRUE if reporting of Warnings is quieted */


/* global variables used by the target only */
char    *pltname,       /* name of partially loaded object */
	*ifil1name,	/* name of ifile containing list of objects in target */
	*tpltnam,	/* name of intermediate partially loaded object */
	*btname,        /* name of object hold branch table */
	*ifil2name,     /* name of ifile cotaining start addresses of
			 * all loaded sections */
	*assemnam;	/* holds name of assembly file to hold branch table */
char	*trgpath;	/* string holds target pathname */

btrec *btlst[BTSIZ];	/* btlst is a hash table containing the branch table
			 * symbols; entries are hashed on a function of the
			 * symbol name */

btrec  **btorder;	/* array of ptrs to entries in the branch table; this
			 * array keeps the order of the branch table (i.e.,
			 * btorder[0] will point to the 1st entry in the 
			 * branch table and so on) */
long tabsiz=0;		/* holds the actual number of entries in the branch table */



/* global variables used by the host only */
char	*moddir=NULL,	/* name of directory which will hold modified objects. */
	*defname=NULL,	/* holds name of the library definition file */
	*ainit=NULL;	/* holds name of assembly file used to create .init
			 * sections */

initinfo	*inits;	/* list of files containing initialization code */

char	*libdefsym=NULL;/* library definition symbol. This symbol will be defined in
			 * the library definition file and every archive member will
			 * get an unresolved reference to it. */
char	*defsuffix;	/* holds suffix of file definition symbols */

stab	*trgsymtab[TRSIZ];	/* trgsymtab is a hash table containing all the
				 * external symbols in the target's symbol table;
				 * entries are hashed on a function of the symbol
				 * name.  */

obj	*objlst[OBSIZ];	/* objlst is a hash table containing information
			 * about the constituent object files. Entries are 
			 * hashed on a function of the object file name.
			 * This info. is stored in a hash table so that
			 * files may be easily located when processing the
			 * initialization code.  */ 
obj	**objects;	/* holds list of constituent .o files for easy file by
			 * file processing.  This list contains pointers
			 * into objlst.  */


arstab	*arsymtab[ARSIZ];	/* arsymtab is a hash table containing the archive
				 * symbol table. Entries are hashed on a function
				 * of the symbol name. This table facilitates the
				 * resolving of unresolved references.  */
