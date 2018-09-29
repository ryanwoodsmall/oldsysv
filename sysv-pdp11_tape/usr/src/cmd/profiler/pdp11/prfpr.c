/* @(#)prfpr.c	1.1 */
#include <time.h>
#include <a.out.h>
#define	OVMAG	0437

#define	CMAX	750
#define	NULL	0

struct	profile	{
	long	p_date;
	long	p_ctr[CMAX];
} p[2];

struct	symbol	{
	char	s_name[8];
	char	s_type;
	char	s_ovno;
	unsigned  s_value;
} *stbl, *sp, *search();

unsigned  symcnt;
int	prfmax;

int	t[CMAX];

char	*namelist = "/unix";
char	*datafile;

long	sum, osum;

double	pc;
double	cutoff = 1e-2;

main(argc, argv)
	int	argc;
	char	**argv;
{
	register  int  ff, log, i;
	double	atof();

	switch(argc) {
		case 4:
			namelist = argv[3];
		case 3:
			cutoff = atof(argv[2]) / 1e2;
		case 2:
			datafile = argv[1];
			break;
		default:
			error("usage: prfpr file [ cutoff [ namelist ] ]");
	}
	if((log = open(datafile, 0)) < 0)
		error("cannot open data file");
	if(cutoff >= 1e0 || cutoff < 0e0)
		error("invalid cutoff percentage");
	if(read(log, &prfmax, sizeof prfmax) != sizeof prfmax || prfmax == 0)
		error("bad data file");
	if(read(log, t, prfmax * 2) != prfmax * 2)
		error("cannot read profile addresses");
	osum = sum = ff = 0;
	init();

	read(log, &p[!ff], prfmax * 4 + 8);
	for(i = 0; i <= prfmax; i++)
		osum += p[!ff].p_ctr[i];

	for(;;) {
		sum = 0;
		if(read(log, &p[ff], prfmax * 4 + 8) != prfmax * 4 + 8)
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
						1e2 * pc);
				else {
					sp = search(t[i], 042, 042);
					if(sp == NULL)
						printf("unknown  %5.2f\n",
							pc * 1e2);
					else if(sp->s_name[0] == '_')
						printf("%-7.7s  %5.2f\n",
							&sp->s_name[1],
							pc * 1e2);
					else
						printf("%-8.8s %5.2f\n",
							sp->s_name,
							pc * 1e2);
				}
		}
		ff = !ff;
		osum = sum;
		printf("\n");
	}
}

error(s)
	char	*s;
{
	printf("error: %s\n", s);
	exit(1);
}

struct	symbol	*
search(addr, m1, m2)
	unsigned  addr;
	register  int  m1, m2;
{
	register  struct  symbol  *sp;
	int	movno;

	if (addr&0100000) {
		movno = (addr&070000)>>12;
		addr = 0160000 | ((addr&07777)<<1);
	} else {
		movno = 0;
		addr <<= 1;
	}
	for(sp = stbl; sp != &stbl[symcnt]; sp++)
		if((sp->s_type == m1 || sp->s_type == m2) &&
			sp->s_value == addr && sp->s_ovno == movno) {
			return(sp);
		}
	return 0;
}

init()
{
	register  int  nmfd;
	long	symloc;
	struct exec abuf;

	if((nmfd = open(namelist, 0)) < 0)
		error("cannot open namelist");
	if(read(nmfd, &abuf, sizeof abuf) != sizeof abuf)
		error("cannot read namelist");
	if(abuf.a_flag)
		symloc = 020 + (long)abuf.a_text + (long)abuf.a_data;
	else
		symloc = 020 + 2 * ((long)abuf.a_text + (long)abuf.a_data);
	if (abuf.a_magic == OVMAG)
		symloc += abuf.a_bss + ((long)abuf.a_hitext<<16);
	symcnt = abuf.a_syms / sizeof (struct symbol);
	if((stbl = (struct symbol *) sbrk(abuf.a_syms)) == (struct symbol *)-1)
		error("cannot allocate space for namelist");
	lseek(nmfd, symloc, 0);
	if(read(nmfd, stbl, abuf.a_syms) != abuf.a_syms)
		error("cannot read symbol table");
	close(nmfd);
}

shtime(l)
	register  long  *l;
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
