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
/*	@(#)search.c	1.2	*/
#include	"crash.h"

struct	nlist	*stbl;
unsigned  symcnt;
extern	char	*namelist;
extern	int nmfd;

#ifdef	pdp11
extern	ovly;
int	in_ovly = 0;
extern	struct exec abuf;
#endif

struct	nlist	*
search(addr, m1, m2)
	unsigned  addr;
#ifdef	pdp11
	register int	m1, m2;
#else
	register char	m1, m2;
#endif
{
	register  struct  nlist  *sp, *save;
	unsigned  value = 0;
#ifdef pdp11
	struct xinterface xi;
	long off;
#endif

	save = 0;

#ifdef	DEBUG
printf("looking for %o\n",addr);
#endif
	for(sp = stbl; sp != &stbl[symcnt]; sp++) {
		if(sp->n_type == m1 || sp->n_type == m2) {
#ifdef	DEBUG
printf("nm=%s typ=%o sp_val=%o <=> m1=%o m2=%o addr=%o\n",sp->n_name,sp->n_type,sp->n_value,m1,m2,addr);
#endif
			if((sp->n_value <= addr) && (sp->n_value > value)) {
				value = sp->n_value;
				save = sp;
#ifdef	DEBUG
printf("save=%o value=%o\n",save,value);
#endif
				if(sp->n_value == addr) break;
			}
		}
	}
#ifdef pdp11
	if (ovly && save->n_value != addr) {
		off = (unsigned) addr;
		lseek(nmfd, off + TXT, 0);
		read(nmfd, &xi, sizeof(xi));
		addr = (unsigned)xi.i_func - 4;
		for(sp = stbl; sp != &stbl[symcnt]; sp++) {
			if((sp->n_type&0377) == m1 || (sp->n_type&0377) == m2) {
#ifdef	DEBUG
printf("OV:nm=%s typ=%o sp_val=%o <=> m1=%o m2=%o addr=%o\n",sp->n_name,sp->n_type,sp->n_value,m1,m2,addr);
#endif
				if((sp->n_value <= addr) && (sp->n_value > value)) {
					value = sp->n_value;
					save = sp;
#ifdef	DEBUG
printf("OV:save=%o value=%o\n",save,value);
#endif
					if(sp->n_value == addr) break;
				}
			}
		}
	}
#endif
	return(save);
}

struct	nlist	*
symsrch(s)
	register  char  *s;
{
	register  struct  nlist  *sp;
	register  struct  nlist  *found;

	found = NULL;
	for(sp = stbl; sp != &stbl[symcnt]; sp++) {
		if(strncmp(sp->n_name,s,8) == 0) {
			found = sp;
			break;
		}
	}
	return(found);
}

struct	nlist	*
nmsrch(s)
	register  char  *s;
{
	char	ct[20];
	register  struct  nlist  *sp;

	if(strlen(s) > 19)
		return(0);
	if((sp = symsrch(s)) == NULL) {
		strcpy(ct, "_");
		strcat(ct, s);
		sp = symsrch(ct);
	}
	return(sp);
}

prnm(s)
	register  char  *s;
{
	register  char  *cp;
	register  struct  nlist  *sp;

	printf("%-10.10s ", s);
	if(strlen(s) > 8) {
		printf("bad name\n");
		return;
	}
	if((sp = nmsrch(s)) == NULL) {
		printf("no match\n");
		return;
	}
#ifdef	pdp11
	printf("%06.6o  ", sp->n_value);
#endif
#if vax || ns32000
	printf("%08.8x  ", sp->n_value);
#endif
	switch(sp->n_type & 0377) {
	case N_UNDF | N_EXT:
	case N_UNDF:
		cp = "undefined"; break;
	case N_ABS | N_EXT:
	case N_ABS:
		cp = "absolute"; break;
	case N_TEXT | N_EXT:
	case N_TEXT:
		cp = "text"; break;
	case N_DATA | N_EXT:
	case N_DATA:
		cp = "data"; break;
	case N_BSS | N_EXT:
	case N_BSS:
		cp = "bss"; break;
	case N_FN:
		cp = "ld file name"; break;
	default:  cp = "unknown?"; break;
	}
	printf("%-10.10s", cp);
	if((sp->n_type >> 8) & 0377)
		printf("ov# %o", ((sp->n_type>>8) & 0377));
	printf("\n");
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
