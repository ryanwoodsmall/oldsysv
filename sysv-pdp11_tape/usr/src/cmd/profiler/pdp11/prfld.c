/* @(#)prfld.c	1.1 */
#include <a.out.h>
#define	OVMAG	0437
struct	symbol	{
	char	s_name[8];
	char	s_type;
	char	s_ovno;
	unsigned	s_value;
};

char	*namelist = "/unix";
struct	symbol	symbol;
struct	exec	aout;
unsigned	symcnt;
long	symloc;
int	taddr[1024];

main(argc, argv)
	int	argc;
	char	**argv;
{
	register int *ip, *tp;
	register nm, prf;
	int	compar();

	if(argc == 2)
		namelist = argv[1];
	else if(argc != 1)
		error("usage: prfld [/unix]\n");
	if((nm = open(namelist, 0)) < 0)
		error("cannot open namelist file\n");
	if((prf = open("/dev/prf", 1)) < 0)
		error("cannot open /dev/prf\n");
	if(read(nm, &aout, sizeof aout) != sizeof aout)
		error("cannot read namelist file\n");
	if(aout.a_syms == 0)
		error("no namelist\n");

	symcnt = aout.a_syms / sizeof symbol;
	if(aout.a_flag)
		symloc = 020 + (long)aout.a_text + (long)aout.a_data;
	else
		symloc = 020 + 2 * ((long)aout.a_text + (long)aout.a_data);
	if (aout.a_magic == OVMAG)
		symloc += aout.a_bss + ((long)aout.a_hitext<<16);
	lseek(nm, symloc, 0);
	ip = taddr;
	*ip++ = 0;
	while(symcnt--) {
		if(ip == &taddr[1024])
			error("too many text symbols\n");
		if(read(nm, &symbol, sizeof symbol) != sizeof symbol)
			error("cannot read namelist\n");
		if(symbol.s_type == 042) {
			if (symbol.s_ovno)
				*ip = 0100000 | (symbol.s_ovno<<12) |
					((symbol.s_value-0160000)>>1);
			else
				*ip = (symbol.s_value>>1)&077777;
		}
		for (tp = taddr; tp < ip; tp++)
			if (*tp == *ip)
				break;
		if (tp == ip)
			ip++;
	}
	qsort(taddr, ip - taddr, sizeof(int), compar);
	if(write(prf, taddr, (unsigned)ip - (unsigned)taddr) !=
	    (unsigned)ip - (unsigned)taddr)
		error("cannot load profile addresses\n");
}

compar(x, y)
	register  unsigned  *x, *y;
{
	if(*x > *y)
		return(1);
	else if(*x == *y)
		return(0);
	return(-1);
}

error(s)
	register  char  *s;
{
	write(2, "prfld: ", 7);
	write(2, s, strlen(s));
	exit(1);
}
