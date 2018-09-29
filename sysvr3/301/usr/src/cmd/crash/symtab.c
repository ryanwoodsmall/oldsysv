/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:symtab.c	1.11"
/*
 * This file contains code for the crash functions: nm, ds, and ts, as well
 * as the initialization routine rdsymtab.
 */

#include "a.out.h"
#include "stdio.h"
#include "string.h"
#include "sys/sbd.h"
#include "crash.h"

extern	int	nmlst_tstamp ;		/* namelist timestamp */
extern char *namelist;
extern short N_TEXT,N_DATA,N_BSS;	/* used in symbol search */
struct syment *stbl;			/* symbol table */
int symcnt;				/* symbol count */
char *strtbl;				/* pointer to string table */
extern char *malloc();

/* symbol table initialization function */

int
rdsymtab()
{
	FILE *np;
	struct filehdr filehdr;
	struct syment	*sp,
			*ts_symb ;
	struct scnhdr	scnptr ,
			boot_scnhdr ;
	int	i ,
		N_BOOTD ;
	char *str;
	long *str2;
	long strtblsize;

	/*
	 * Open the namelist and associate a stream with it. Read the file into a buffer.
	 * Determine if the file is in the correct format via a magic number check.
	 * An invalid format results in a return to main(). Otherwise, dynamically 
	 * allocate enough space for the namelist. 
	 */

	if(!(np = fopen(namelist, "r")))
		fatal("cannot open namelist file\n");
	if(fread((char *)&filehdr, FILHSZ, 1, np) != 1)
		fatal("read error in namelist file\n");
	if(filehdr.f_magic != FBOMAGIC)
		fatal("namelist not in a.out format\n");
	/*
	 * Read the section headers to find the section numbers
	 * for .text, .data, and .bss.  First seek past the file header 
	 * and optional header, then loop through the section headers
	 * searching for the names .text, .data, and .bss.
	 */
	N_TEXT=0;
	N_DATA=0;
	N_BSS=0;
	N_BOOTD=0 ;
	if(fseek(np, (long)(FILHSZ + filehdr.f_opthdr), 0) != 0
	  && fread((char *)&filehdr, FILHSZ, 1, np) != 1)
		fatal("read error in section headers\n");

	for(i=1; i<=filehdr.f_nscns; i++)
	{
		if(fread(&scnptr, SCNHSZ, 1, np) != 1)
			fatal("read error in section headers\n");

		if(strcmp(scnptr.s_name,_TEXT) == 0)
			N_TEXT = i ;
		else if(strcmp(scnptr.s_name,_DATA) == 0)
			N_DATA = i ;
		else if(strcmp(scnptr.s_name,_BSS) == 0)
			N_BSS = i ;
		else if(strcmp(scnptr.s_name,"boot") == 0)
		{
			/* save data section for later processing */
			N_BOOTD = 1 ;
			boot_scnhdr = scnptr ;
		}

	}
	if(N_TEXT == 0 || N_DATA == 0 || N_BSS == 0) 
		fatal(".text, .data, or .bss was not found in section headers\n");

	if(!(stbl=(struct syment *)malloc((unsigned)(filehdr.f_nsyms*SYMESZ))))
		fatal("cannot allocate space for namelist\n");

	/*
	 * Find the beginning of the namelist and read in the contents of the list.
	 *
	 * Additionally, locate all auxiliary entries in the namelist and ignore.
	 */

	fseek(np, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, np) != 1)
			fatal("read error in namelist file\n");
		if(sp->n_numaux) {
			fseek(np,(long)AUXESZ*sp->n_numaux,1);
			i += sp->n_numaux;
		}
	}
	/*
	 * Now find the string table (if one exists) and
	 * read it in.
	 */
	if(fseek(np,filehdr.f_symptr + filehdr.f_nsyms * SYMESZ,0) != 0)
		fatal("error in seeking to string table\n");
	
	if(fread((char *)&strtblsize,sizeof(int),1,np) != 1)
		fatal("read error for string table size\n");
	
	if(strtblsize)
	{
		if(!(strtbl = (char *)malloc((unsigned)strtblsize)))
			fatal("cannot allocate space for string table\n");

		str2 = (long *)strtbl;
		*str2 = strtblsize;

		for(i = 0,str = (char *)((int)strtbl + (int)sizeof(long)); i < strtblsize - sizeof(long); i++, str++)
			if(fread(str, sizeof(char), 1, np) != 1)
				fatal("read error in string table\n");
	}
	else
		str = 0;

	/* save timestamp from data space of namelist file */
		
	if(!(ts_symb = symsrch("crash_sync")) || !N_BOOTD)
		nmlst_tstamp = 0 ;
	else
	{
		if(fseek(np,(long)(boot_scnhdr.s_scnptr + (ts_symb -> n_value - boot_scnhdr.s_paddr)),0) != 0)
			fatal("could not seek to namelist timestamp\n") ;
		if(fread((char *)&nmlst_tstamp,sizeof(int),1,np) != 1)
			fatal("could not read namelist timestamp\n") ;
	}
	fclose(np);
}


/* find symbol */
struct syment *
findsym(addr)
unsigned long addr;
{
	struct syment *sp;
	struct syment *save;
	unsigned long value;

	value = MAINSTORE;
	save = NULL;

	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if((sp->n_sclass == C_EXT) && ((unsigned long)sp->n_value <= addr)
		  && ((unsigned long)sp->n_value > value)) {
			value = (unsigned long)sp->n_value;
			save = sp;
		}
	}
	return(save);
}

/* get arguments for ds and ts functions */
int
getsymbol()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do {prsymbol(args[optind++]);
		}while(args[optind]);
	}
	else longjmp(syn,0);
}

/* print result of ds and ts functions */
int
prsymbol(string)
char *string;
{
	struct syment *spd,*spt,*sp;
	long addr;
	static char temp[SYMNMLEN+1];

	spd = spt = sp = NULL;
	if((addr = strcon(string,'h')) == -1)
		error("\n");
	if(!(spd = findsym((unsigned long)addr)) &&
		!(spt = findsym((unsigned long)addr))) {
			prerrmes("%s does not match\n",string);
			return;
	}
	if(!spd) 
		sp = spt;
	else if(!spt) 
		sp = spd;
	else {
		if((addr - spt->n_value) < (addr - spd->n_value)) 
			sp = spt;
		else sp = spd;
	}
	if(sp->n_zeroes) {
		strncpy(temp,sp->n_name,SYMNMLEN);
		fprintf(fp,"%-8s",temp);
	}
	else fprintf(fp,"%s",strtbl+sp->n_offset);		
	fprintf(fp," + %x\n",addr - (long)sp->n_value);
}


/* search symbol table */
struct syment *
symsrch(s)
char *s;
{
	struct syment *sp;
	struct syment *found;
	static char tname[SYMNMLEN + 1];
	char *name;

	found = 0;


	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if((sp->n_sclass == C_EXT) &&
			((unsigned long)sp->n_value >= MAINSTORE)) {
			if(sp->n_zeroes)
				{
				strncpy(tname,sp->n_name,SYMNMLEN);
				name = tname;
			}
			else name = strtbl + sp->n_offset;
			if(!strcmp(name,s)) {
				found = sp;
				break;
			}
		}
	}
	return(found);
}

/* get arguments for nm function */
int
getnm()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		do { prnm(args[optind++]);
		}while(args[optind]);
	else longjmp(syn,0);
}


/* print result of nm function */
int
prnm(string)
char *string;
{
	char *cp;
	struct syment *sp;

	if(!(sp = symsrch(string))) {
		prerrmes("%s does not match in symbol table\n",string);
		return;
	}
	fprintf(fp,"%s   %08.8lx  ",string,sp->n_value);


	if      (sp -> n_scnum == N_TEXT)
		cp = " text";
	else if (sp -> n_scnum == N_DATA)
		cp = " data";
	else if (sp -> n_scnum == N_BSS)
		cp = " bss";
	else if (sp -> n_scnum == N_UNDEF)
		cp = " undefined";
	else if (sp -> n_scnum == N_ABS)
		cp = " absolute";
	else
		cp = " type unknown";

	fprintf(fp,"%s\n", cp);
}
