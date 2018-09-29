/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)profil-3b5:prfpr.c	1.6"
/*
 *	prfpr - print profiler log files
 */

#include <stdio.h>
#include <time.h>
#include <a.out.h>

#define PRFMAX  2048		/* max number of text symbols */
int N_TEXT;

struct	profile	{
	long	p_date;		/* time stamp of record */
	long	p_ctr[PRFMAX];	/* profiler counter values */
} p[2];


struct syment *stbl;		/* start of symbol table */
char *strtab;			/* start of string table */
int symcnt;			/* number of symbols */
int prfmax;			/* actual number of text symbols */
int cutoff = 1;
int pc;
int t[PRFMAX];

char *namelist = "/unix";
char *logfile;

long sum, osum;


main(argc, argv)
char **argv;
{
	register int ff, log, i;
	struct syment *sp, *search();

	switch(argc) {
		case 4:
			namelist = argv[3];
		case 3:
			cutoff = atoi(argv[2]);
		case 2:
			logfile = argv[1];
			break;
		default:
			error("usage: prfpr file [ cutoff [ namelist ] ]");
	}
	if((log = open(logfile, 0)) < 0)
		error("cannot open data file");
	if(cutoff > 100 || cutoff < 0)
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
	rdstrtab();
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
			pc = 1000 * (p[ff].p_ctr[i] - p[!ff].p_ctr[i]) /
				(sum - osum);
			if(pc > 10 * cutoff)
				if(i == prfmax)
					printf("user     %d.%d\n",
					 pc/10, pc%10);
				else {
					sp = search(t[i], N_TEXT, N_TEXT);
					if(sp == 0)
						printf("unknown  %d.%d\n",
						 pc/10, pc%10);
					else {
						printname(sp);
						printf(" %d.%d\n", pc/10, pc%10);
					}
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

rdsymtab()
{
	struct filehdr filehdr;
	struct scnhdr scnptr;
	FILE *fp;
	struct syment *sp;
	int i;
	char *sbrk();

	if((fp = fopen(namelist, "r")) == NULL)
		error("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		error("read error in namelist file");
	if(filehdr.f_magic != FBOMAGIC)
		error("namelist not in a.out format");

	/*
	 * Read the section headers to find the section number
	 * for .text. First seek past the file header 
	 * and optional header, then loop through the section headers
	 * searching for the names .text.
	 */
	N_TEXT=0;
	if(fseek(fp, FILHSZ + filehdr.f_opthdr, 0) != 0)
		error("error in seeking to section headers");

	for(i=1; i<=filehdr.f_nscns; i++) {
		if(fread(&scnptr, SCNHSZ, 1, fp) != 1)
			error("read error in section headers");

		switch(scnmatch(scnptr.s_name,1,_TEXT)) {
			case 1:		/* .text */
					N_TEXT = i; break;
			default:
					break;
		}
	}
	if(N_TEXT == 0) 
		error(".text was not found in section headers");

	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*SYMESZ)) == (struct syment *)-1)
		error("cannot allocate space for namelist");
	fseek(fp, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, fp) != 1)
			error("read error in namelist file");
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

/* 
 * scnmatch() is only called by rdsymtab() to match a section
 * name in the section headers to find .text.
 * A number is returned indicating which name matched, or a zero
 * is returned if none matched.  This routine was copied out of
 * exec.c in the kernel code.
 */
scnmatch(target, count, sources)
char *target;
int count;
char *sources;
{
	register char *p, *q, **list;
	list = &sources;
	while (count-- > 0) {
		p = target;
		q = *list++;
		while(*p == *q++) {
			if (*p++ == '\0')
			      return(list - &sources);
		}
	}
	return(0);
}

rdstrtab()
{
	static long length;
	struct filehdr filehdr;
	FILE *fp;

	if((fp = fopen(namelist, "r")) == NULL)
		error("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		error("read error in namelist file");

	fseek(fp, filehdr.f_symptr + filehdr.f_nsyms * SYMESZ, 0);
	if (fread((char *)&length, sizeof(long), 1, fp) != 1
	   || (strtab = (char *)malloc((unsigned)length)) == NULL
	   || fread(strtab + sizeof(long), sizeof(char),
	      length - sizeof(long), fp) != length-sizeof(long)
	   || strtab[length - 1] != '\0' ) {
		fprintf(stderr, "error in obtaining string table");
	}
}

printname(ent)
struct syment *ent;
{
	if (ent->n_zeroes == 0L)
		printf("%s", strtab + ent->n_offset);
	else
		printf("%-8.8s", ent->n_name);
}
