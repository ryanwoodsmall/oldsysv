/*	@(#)sash1.c	1.7	*/
/*
 *	sash1 -- common code for sash program
 */

# include "stand.h"
# include "stdio.h"
# include "a.out.h"

# ifndef ROOT
# define ROOT	hp
# endif

# ifndef USR
# define USR	hp
# endif

# define KERNEL	0
# define USER	1
#define	VAXWRMAGIC	0570
#define	VAXROMAGIC	0575
#define	AOUT1MAGIC	0407
#define	AOUT2MAGIC	0410
#define AOUT3MAGIC	0413

extern	argc;
char	*argv [32];
char	argbuf [256];
int	_nullsys ();
unsigned sread ();

int	(*entry) ();
int	_gdstrategy ();

union hdrs
{
	struct sgshdr {
		struct	filehdr  filhdr;
		struct	aouthdr  naout;
		struct	scnhdr  scnhdr[3];
	} sgshdr;

	struct exec {
		short		a_magic,
				a_stamp;
		unsigned	a_text,
				a_data,
				a_bss,
				a_syms,
				a_entry,
				a_trsize,
				a_drsize;
	} oldhdr;
};

extern int _devcnt;

char	hp[] = "/dev/rp0";

char	devname [][5] = {
	"rp06",
};

# define PAGE	0x1ff
# define HBSS	0x4f800
# define SET2	"       set [disk {/|/usr} rp06]\n"


main () {
	extern char edata, end;
	extern gd_boot;	/* unit # of bootload device */

	clrseg (&edata, (unsigned) (&end - &edata), USER);


	/*if (gd_boot == 0)
		hp[8] = '\0';
	else
		hp[7] = gd_boot + '0';*/

/* usage: MKNOD(*name,devsw entry#,unit#,blk offset)  */
	if (MKNOD (hp, 0, gd_boot, (daddr_t) 0) < 0)
		perror (hp);
	if (mount (ROOT, "") < 0)
		perror (hp);

	/*if (gd_boot > 0)
		hp[8] = '1';
	else*/
		hp[7] = '1';
#if FsTYPE == 1
	if (MKNOD (hp, 0, gd_boot, (daddr_t) 18392) < 0)
#else
	if (MKNOD (hp, 0, gd_boot, (daddr_t) 9196) < 0)
#endif
		perror (hp);
	if (mount (USR, "/usr") < 0)
		perror (hp);
	_prs ("\n");
}

xcom () {
	register ac;

	while (load () < 0);
}

load () {
	register unsigned count;
	register char *corep;
	register in;
	union hdrs hdrs; /* combine 4.2 & 5.0 a.out header structures */
	struct aouthdr *np;
	struct exec  *op;
	extern char *_cdir, *strcpy ();

	getargv ();
	if (argc == 0)
		return (-1);
	if (strcmp (argv [0], "set") == 0) {
		set ();
		return (-1);
	}
	if (strcmp (argv [0], "cd") == 0) {
		switch (argc) {
		case 1:
			ppath (_cdir, "\n");
			return (-1);
		case 2:
			chdir (argv [1]);
			return (-1);
		}
		_prs ("Usage: cd [directory]\n");
		return (-1);
	}
	if ((in = open (argv [0], 0)) < 0) {
		_prs (argv [0]);
		_prs (": not found\n");
		return (-1);
	}
	if ((_iobuf[in-3].i_ino.i_mode & IFMT) != IFREG) {
		_prs (argv[0]);
		_prs (": not executable\n");
		return (-1);
	}
	op = &hdrs.oldhdr;
	if (read (in, (char *) op, sizeof (struct exec)) != sizeof (struct exec)
	    || (op->a_magic != AOUT1MAGIC
	    && op->a_magic != VAXWRMAGIC
	    && op->a_magic != VAXROMAGIC
	    && op->a_magic != AOUT2MAGIC
	    && op->a_magic != AOUT3MAGIC)) {
		movc3 (sizeof (struct exec), (char *) op, (char *) 0, USER);
		if ((corep = (char *) sread (in, (char *) sizeof (struct exec),
			0xffff)) < 0) {
		error:
			_prs (argv [0]);
			_prs (": read error\n");
			return (-1);
		}
		corep += sizeof (struct exec);
		op->a_bss = HBSS - (unsigned) corep;
		entry = 0;
		clrseg(corep, op->a_bss, KERNEL);
	} else {
		switch (op->a_magic) {
		case AOUT1MAGIC:
		case AOUT2MAGIC:
		case AOUT3MAGIC:
			count = op->a_text + op->a_data;
			if (sread (in, (char *) 0, count) != count)
				goto error;
			corep = (char *) op->a_text;
			if ((op->a_magic == AOUT2MAGIC || op->a_magic == AOUT3MAGIC) && op->a_data) {
				corep = (char *) (((int) corep + PAGE)
					& ~PAGE);
				movc3 (op->a_data, (char *) op->a_text,
					corep, KERNEL);
			}
			corep += op->a_data;
			entry = (int (*)()) (op->a_entry & 0x3fffffff);
			clrseg(corep, op->a_bss, KERNEL);
			break;

		case VAXWRMAGIC:
		case VAXROMAGIC:
			lseek(in, (off_t) 0, 0);
			read(in, (char *)&hdrs.sgshdr, sizeof(struct sgshdr));
			np = &hdrs.sgshdr.naout;
			count = np->tsize + np->dsize;
			if (sread(in, (char *) 0, count) != count)
				goto error;
			corep = (char *) np->tsize;
			if ((np->magic == AOUT2MAGIC || np->magic == AOUT3MAGIC) && np->dsize) {
				corep = (char *) (((int) corep + PAGE)
					& ~PAGE);
				movc3(np->dsize, (char *) np->tsize, corep,
					KERNEL);
			}
			corep += np->dsize;
			entry = (int (*)())(np->entry & 0x3fffffff);
			clrseg(corep, np->bsize, KERNEL);
			break;
		}
	}
	close (in);
	return (0);
}

# define SCAN 0
# define FILL 1

getargv () {
	register unsigned count = 0;
	register char *quote, *ptr;
	int pass = SCAN;
	int shift = 0;
	extern char *strncpy ();
	extern char *strchr ();

	_prs ("$$ ");

	ptr = argbuf;
	fgets (argbuf, sizeof (argbuf) - 1, stdin);
	argbuf [sizeof (argbuf) - 1] = '\n';
	*strchr (argbuf, '\n') = '\0';

	for (;;) {
		while ((ptr-argbuf) < sizeof (argbuf) - 1) {
			while (*ptr) {
				if (*ptr != ' ' && *ptr != '\t')
					break;
				++ptr;
			}
			if (*ptr == '\0')
				break;
			if (pass == FILL)
				argv [count] = ptr;
			++count;
			while (*ptr) {
				if (*ptr == ' ' || *ptr == '\t')
					break;
				if (*ptr == '"' || *ptr == '\'') {
					quote = strchr (&ptr [1], *ptr);
					if (pass == FILL) {
						strncpy (ptr, &ptr [1], 
							quote - ptr - 1);
						shift += 2;
					}
					ptr = quote;
				} else
					ptr [-shift] = *ptr;
				++ptr;
			}
			if (pass == FILL) {
				ptr [-shift] = '\0';
				shift = 0;
				++ptr;
			}
		}
		if (pass == FILL) {
			argc = count;
			return;
		}
		ptr = argbuf;
		count = 0;
		pass = FILL;
	}
}

# define UNIT	0
# define DISK	1

set () {
	register struct mtab *mp;
	register devno, stype;
	char unit, mname [NAMSIZ];

	if (argc == 1) {
		for (mp = &_mtab [0]; mp < &_mtab [NMOUNT]; mp++) {
			if (mp->mt_name == 0)
				continue;
			ppath (mp->mt_name, "\t");
			_prs (devname [mp->mt_dp->dt_devp - &_devsw [0]]);
			_prs (" ");
			unit = mp->mt_dp->dt_unit + '0';
			write (1, &unit, 1);
			_prs ("\n");
		}
		return;
	}

	if (argc != 4) {
		_prs ("Usage: set [unit {/|/usr} {0|1|...|7}]\n");
		_prs (SET2);
		return;
	}

	_cond (argv [2], mname);

	if (strcmp (argv [1], "unit") == 0) {
		stype = UNIT;

		unit = argv [3][0];

		if (unit < '0' || unit > '7' || argv [3][1] != '\0') {
			_prs ("set: bad unit number\n");
			return;
		} else
			unit -= '0';
	}
	if (strcmp (argv [1], "disk") == 0) {
		stype = DISK;

		for (devno = 0; devno < _devcnt; devno++)
			if (strcmp (argv [3], devname [devno]) == 0)
				break;
		if (devno == _devcnt) {
			_prs ("set: bad dev name\n");
			return;
		}
	}

	for (mp = &_mtab [0]; mp < &_mtab [NMOUNT]; mp++) {
		if (mp->mt_name == 0)
			continue;
		if (strcmp (mname, mp->mt_name) == 0) {
			switch (stype) {
			case UNIT:
				mp->mt_dp->dt_unit = (int) unit;
				return;
			
			case DISK:
				unit = (char) mp->mt_dp->dt_unit;
				mp->mt_dp = &_dtab [devno*2 
					+ (&_mtab[NMOUNT-1] - mp)];
				mp->mt_dp->dt_unit = (int) unit;
				return;

			default:
				_prs ("set: bad type\n");
				return;
			}
		}
	}
	_prs ("set: bad fs name\n");
}

ppath (str, tc)
char *str, *tc; {

	if (str == NULL || *str == '\0')
		_prs ("/");
	else
		_prs (str);
	_prs (tc);
}

unsigned
sread (filep, addr, count)
register char *addr;
unsigned count; {
	register unsigned nleft;
	register cnt, nread;
	char buf[BUFSIZ];

	if (count == 0)
		return (0);
	nleft = count;
	do {
		cnt = (nleft > BLKSIZ) ? BLKSIZ : (int) nleft;
		if ((nread = read (filep, buf, cnt)) != cnt) {
			if (nread < 0)
				return (-1);
			cnt = 0;
		} else
			cnt = 1;
		movc3 ((unsigned) nread, buf, addr, USER);
		nleft -= (unsigned) nread;
		if (!cnt)
			return (count - nleft);
		addr += nread;
	} while (nleft != 0);
	return (count);
}
