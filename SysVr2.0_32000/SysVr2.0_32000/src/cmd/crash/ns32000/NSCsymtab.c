/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)NSCsymtab.c	1.1 */
#include "crash.h"

struct syment *stbl;
extern char *namelist;
int symcnt;
char *strngtab;
int strtbsz;

rdsymtab()
{
	FILE *fp;
	struct filehdr filehdr;
	struct syment *sp;
	int i;

	if((fp = fopen(namelist, "r")) == NULL)
		fatal("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		fatal("read error in namelist file");
#ifndef	ns32000
	if(filehdr.f_magic != VAXROMAGIC)
#else
	if(filehdr.f_magic != NS32ROMAGIC)
#endif
		fatal("namelist not in a.out format");
	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*sizeof(struct syment))) == (struct syment *)-1)
		fatal("cannot allocate space for namelist");
	i = (int)filehdr.f_symptr + filehdr.f_nsyms*SYMESZ;
	fseek(fp, i, 0);
	fread(&strtbsz, 4, 1, fp);
	if (strtbsz) {
		if ((strngtab = (char *)sbrk(strtbsz)) == (char *)-1)
			fatal("cannot allocate space for string table");
		fread(strngtab+4, strtbsz-4, 1, fp);
	}
	fseek(fp, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, fp) != 1)
			fatal("read error in namelist file");
		if (sp->n_zeroes == 0) {
			if (sp->n_offset < 4 || sp->n_offset >= strtbsz)
				fatal("bad flex symbol offset");
			strncpy(sp->n_name, strngtab + sp->n_offset, 8);
		}
		if(sp->n_numaux) {
			fseek(fp, AUXESZ*sp->n_numaux, 1);
			i += sp->n_numaux;
		}
	}
	brk(sp);
	fclose(fp);
}

struct syment *
search(addr, sect1, sect2)
unsigned addr;
register sect1, sect2;
{
	register struct syment *sp;
	register struct syment *save;
	unsigned value;

	value = 0;
	save = 0;
#ifdef DEBUG
	fprintf(stderr,"addr:\t%x\tsect1:\t%d\tsect2:\t%d\n",addr,sect1,sect2);
#endif
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if (sp->n_scnum == sect1 || sp->n_scnum == sect2) { 
#ifdef DEBUG
	fprintf(stderr,"nm=%s scnum=%o sp_val=%x <=> sect1=%o sect2=%o addr=%x\n",
		sp->n_name, sp->n_scnum, sp->n_value, sect1, sect2, addr);
#endif
#ifndef	ns32000
			if(sp->n_sclass == C_EXT && sp->n_value <= addr
		  		&& sp->n_value > value) {
#else
			/* Find nonexternal syms, but not starting with '.' */
			if(sp->n_value<=addr && sp->n_value>value &&
				*sp->n_name != '.') {
#endif
#ifdef DEBUG
	fprintf(stderr,"save=%x value=%x\n", save, value);
#endif
				value = sp->n_value;
				save = sp;
				if (sp->n_value == addr)
					return(save);
			}
		}
	}
	return(save);
}

struct syment *
symsrch(s)
register char *s;
{
	register struct syment *sp;
	register struct syment *found;

	found = 0;
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
#ifndef	ns32000
		if(sp->n_sclass == C_EXT && strncmp(sp->n_name, s, 8) == 0) {
#else
		/* Find symbol even if it's not external */
		if(strncmp(sp->n_name, s, 8) == 0) {
#endif
			found = sp;
			break;
		}
	}
	return(found);
}

prnm(s)
register char *s;
{
	register char *cp;
	register struct syment *sp;
	struct syment *nmsrch();

	printf("%-8.8s ", s);
	if((sp = nmsrch(s)) == NULL) {
		printf("no match\n");
		return;
	}
	printf("%08.8lx  ", sp->n_value);
	switch(sp->n_scnum) {
	case N_TEXT:
		cp = " text";
		break;
	case N_DATA:
		cp = " data";
		break;
	case N_BSS:
		cp = " bss";
		break;
	case N_UNDEF:
		cp = " undefined";
		break;
	case N_ABS:
		cp = " absolute";
		break;
	default:
		cp = " type unknown";
	}
	printf("%s\n", cp);
}

struct syment *
nmsrch(s)
	register  char  *s;
{
	char	ct[20];
	register  struct  syment  *sp;
	struct syment *symsrch();

	if(strlen(s) > 19)
		return(0);
	if((sp = symsrch(s)) == NULL) {
		strcpy(ct, "_");
		strcat(ct, s);
		sp = symsrch(ct);
	}
	return(sp);
}

prod(addr, units, style)
	unsigned	addr;
	int	units;
	char	*style;
{
	register  int  i;
	register  struct  prmode  *pp;
	int	word;
	long	lword;
	char	ch;
	extern	struct	prmode	prm[];

	if(units == -1)
		return;
	for(pp = prm; pp->pr_sw != 0; pp++) {
		if(strcmp(pp->pr_name, style) == 0)
			break;
	}
#ifndef	ns32000
	if (addr > (unsigned)VIRT_MEM)
		addr = sysvad(addr);
#endif
	if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
		error("bad seek of addr");
	}
	switch(pp->pr_sw) {
	default:
	case NULL:
		error("invalid mode");
		break;

	case OCTAL:
	case DECIMAL:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 8 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, (int)addr + i * NBPW);
				printf(":");
			}
			if(read(mem, &word, NBPW) != NBPW) {
				printf("  read error");
				break;
			}
			printf(pp->pr_sw == OCTAL ? " %7.7o" :
				"  %5u", word);
		}
		break;

	case LOCT:
	case LDEC:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, (int)addr + i * NBPW);
				printf(":");
			}
			if(read(mem, &lword, sizeof (long)) != sizeof (long)) {
				printf("  read error");
				break;
			}
			printf(pp->pr_sw == LOCT ? " %12.12lo" :
				"  %10lu", lword);
		}
		break;

	case CHAR:
	case BYTE:
		for(i = 0; i < units; i++) {
			if(i % (pp->pr_sw == CHAR ? 16 : 8) == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, (int)addr + i * sizeof (char));
				printf(":");
			}
			if(read(mem, &ch, sizeof (char)) != sizeof (char)) {
				printf("  read error");
				break;
			}
			if(pp->pr_sw == CHAR)
				putch(ch);
			else
				printf(" %4.4o", ch & 0377);
		}
		break;
	case HEX:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, (int)addr + i * NBPW);
				printf(":");
			}
			if(read(mem, &lword, sizeof (long)) != sizeof (long)) {
				printf("  read error");
				break;
			}
			printf(" %08x", lword);
		}
		break;


	}
	putc('\n', stdout);
}
