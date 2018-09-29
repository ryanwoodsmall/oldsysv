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
/* @(#)prfpr.c	1.3 */
/*
 *	prfpr - print profiler log files
 */

#include "stdio.h"
#include "time.h"
#include "filehdr.h"
#include "syms.h"
#include "storclass.h"

#define	N_TEXT	1
#define PRFMAX  1024		/* max number of text symbols */

struct	profile	{
	long	p_date;		/* time stamp of record */
	long	p_ctr[PRFMAX+2];	/* profiler counter values */
} p[2];

struct syment *stbl;		/* start of symbol table */
int	symcnt;			/* number of symbols */
int	prfmax;			/* actual number of text symbols */

int	t[PRFMAX+2];

char	*namelist = "/unix";
char	*logfile;

long	sum, osum;

double	pc;
double	cutoff = 1e-2;
main(argc, argv)
char **argv;
{
	register int ff, log, i;
	double	atof();
	struct syment *sp, *search();

	switch(argc) {
		case 4:
			namelist = argv[3];
		case 3:
			cutoff = atof(argv[2]) / 1e2;
		case 2:
			logfile = argv[1];
			break;
		default:
			error("usage: prfpr file [ cutoff [ namelist ] ]");
	}
	if((log = open(logfile, 0)) < 0)
		error("cannot open data file");
	if(cutoff >= 1e0 || cutoff < 0e0)
		error("invalid cutoff percentage");
	if(read(log, &prfmax, sizeof prfmax) != sizeof prfmax || prfmax == 0)
		error("bad data file");
	if(read(log, t, prfmax * sizeof (int)) != prfmax * sizeof (int))
		error("cannot read profile addresses");
	osum = sum = ff = 0;

	read(log, &p[!ff], (prfmax + 2) * sizeof (int));
	for(i = 0; i <= prfmax; i++)
		osum += p[!ff].p_ctr[i];

	rdsymtab();
	for(;;) {
		sum = 0;
		if(read(log, &p[ff], (prfmax + 2) * sizeof (int)) !=
		    (prfmax + 2) * sizeof (int))
			exit(0);
		shtime(&p[!ff].p_date);
		shtime(&p[ff].p_date);
		printf("\n");
		for(i = 0; i <= prfmax; i++)
			sum += p[ff].p_ctr[i];
		if(sum == osum)
			printf("no samples\n\n");
		else for(i = 0; i <= prfmax; i++) {
			pc = (double) (p[ff].p_ctr[i] - p[!ff].p_ctr[i]) /
				(double) (sum - osum);
			if(pc > cutoff)
				if(i == prfmax)
					printf("user     %5.2f\n",
					 pc * 1e2);
				else {
					sp = search(t[i], N_TEXT, N_TEXT);
					if(sp == 0)
						printf("unknown  %5.2f\n",
							pc * 1e2);
					else if(sp->n_name[0] == '_')
						printf("%-7.7s  %5.2f\n",
						 &sp->n_name[1], pc * 1e2);
					else
						printf("%-8.8s %5.2f\n",
						 sp->n_name, pc * 1e2);
				}
		}
		ff = !ff;
		osum = sum;
		printf("\n");
	}
}

error(s)
char *s;
{
	printf("error: %s\n", s);
	exit(1);
}


struct syment *
search(addr, sect1, sect2)
unsigned addr;
{
	register struct syment *sp;
	register struct syment *save;
	unsigned value;

	value = 0;
	save = 0;
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if(sp->n_sclass == C_EXT && (sp->n_scnum == sect1
		  || sp->n_scnum == sect2) && sp->n_value <= addr
		  && sp->n_value > value) {
			value = sp->n_value;
			save = sp;
		}
	}
	return(save);
}

rdsymtab()
{
	struct filehdr filehdr;
	FILE *fp;
	struct syment *sp;
	int i;
	char *sbrk();
	char *strngtab;
	int strtbsz;

	if((fp = fopen(namelist, "r")) == NULL)
		error("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		error("read error in namelist file");
	if(filehdr.f_magic != VAXROMAGIC)
		error("namelist not in a.out format");
	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*sizeof(struct syment))) == (struct syment *)-1)
		error("cannot allocate space for namelist");
	
	i = (int)filehdr.f_symptr + filehdr.f_nsyms*SYMESZ;
	fseek(fp, i, 0);
	fread(&strtbsz, 4, 1, fp);
	if (strtbsz) {
		if ((strngtab = (char *)sbrk(strtbsz)) == (char *)-1)
			printf("cannot allocate space for string table\n");
		fread(strngtab+4, strtbsz-4, 1, fp);
	}

	fseek(fp, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, fp) != 1)
			error("read error in namelist file");

		if (sp->n_zeroes == 0) {
			if (sp->n_offset < 4 || sp->n_offset >= strtbsz)
				printf("bad flex symbol offset\n");
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

shtime(l)
register long *l;
{
	register  struct  tm  *t;
	struct  tm  *localtime();

	if(*l == (long) 0) {
		printf("initialization\n");
		return;
	}
	t = localtime(l);
	printf("%02.2d/%02.2d/%02.2d %02.2d:%02.2d\n", t->tm_mon + 1,
		t->tm_mday, t->tm_year, t->tm_hour, t->tm_min);
}
