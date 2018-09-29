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
/*
 *	sash1 -- common code for sash program
 */

# include "stand.h"
# include "stdio.h"
# include "a.out.h"

# ifndef ROOT
# define ROOT	dsk
# endif

# ifndef USR
# define USR	dsk
# endif

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

char rootdsk[] = "/dev/dsk/0s0";
char usrdsk[] = "/dev/dsk/0s0";
char mt[] = "/dev/mt/0ln";

# define MAXDEVS 15
char devname [MAXDEVS][13] = {
 "/dev/dsk/1s0", "/dev/dsk/1s1", "/dev/dsk/1s2", "/dev/dsk/1s3", "/dev/dsk/1s4",
 "/dev/dsk/0s0", "/dev/dsk/0s1", "/dev/dsk/0s2", "/dev/dsk/0s3", "/dev/dsk/0s4",
 "/dev/dsk/2s0", "/dev/dsk/2s1", "/dev/dsk/2s2", "/dev/dsk/2s3", "/dev/dsk/2s4"
};
int unitnum [MAXDEVS] = { 8,9,10,11,12, 0,1,2,3,4, 16,17,18,19,20 };

# define PAGE	0x1ff
# define KPAGE	0x3ff
# define HBSS	0x6f800


main () {
	extern char edata, end;
	extern gd_boot;	/* unit # of boot dev (8*drv+slice) */
	int i;		/* loop counter */

	clrseg (&edata, (unsigned) (&end - &edata));

/* usage: MKNOD(*name,devsw entry#,unit#,blk offset)  */

	for (i=0; i<MAXDEVS; ++i)	/* make entries for slices */
	   if (MKNOD(devname[i], 0, unitnum[i], (daddr_t) 0) < 0)
		perror (devname[i]);
	rootdsk[9] = (gd_boot >> 3) + '0';
	rootdsk[11] = (gd_boot & 07) + '0';
	if (mount (rootdsk, "") < 0)	/* initially mount boot dev. */
		perror (rootdsk);
	usrdsk[9] = (gd_boot >> 3) + '0';
	usrdsk[11] = (gd_boot & 07) + '0';
	if (mount (usrdsk, "/usr") < 0)
		perror (usrdsk);
	mt[10] = 'n';	/* using /dev/mt/0ln winds tape to end of file */
	if (MKNOD(mt, 1, 4, (daddr_t) 0) < 0)
		perror (mt);
	mt[10] = '\0';	/* using /dev/mt/0l rewinds to beginning of tape */
	if (MKNOD(mt, 1, 0, (daddr_t) 0) < 0)
		perror (mt);
	_prs ("\n");
}

xcom () {
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
	if (strcmp (argv [0], "mt") == 0) {
		dotape ();
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
	if (read(in, (char *)op, sizeof(struct exec)) != sizeof (struct exec)
	    || (op->a_magic != AOUT1MAGIC
	    && op->a_magic != NS32WRMAGIC
	    && op->a_magic != NS32ROMAGIC
	    && op->a_magic != AOUT2MAGIC
	    && op->a_magic != AOUT3MAGIC)) {
		movc3 (sizeof (struct exec), (char *) op, (char *) 0);
		if ((corep = (char *) sread (in, (char *) sizeof (struct exec),
			0xffff)) < 0) {
				_prs (argv [0]);
				_prs (": read error\n");
				return (-1);
		}
		corep += sizeof (struct exec);
		op->a_bss = HBSS - (unsigned) corep;
		entry = (int (*)()) 32;
		_prs ("Using default entry address 32\n");
		clrseg(corep, op->a_bss);
	} else {
		switch (op->a_magic) {
		case AOUT1MAGIC:
		case AOUT2MAGIC:
		case AOUT3MAGIC:
			count = op->a_text + op->a_data;
			if (sread (in, (char *) 0, count) != count) {
				_prs (argv [0]);
				_prs (": read error\n");
				return (-1);
			}
			corep = (char *) op->a_text;
			if ((op->a_magic == AOUT2MAGIC ||
			     op->a_magic == AOUT3MAGIC) && op->a_data) {
				if (op->a_magic == AOUT3MAGIC)
					corep = (char *) (((int) corep + KPAGE)
						& ~KPAGE);
				else corep = (char *) (((int) corep + PAGE)
						& ~PAGE);
				movc3up (op->a_data, (char *) op->a_text,
					corep);
			}
			corep += op->a_data;
			entry = (int (*)()) (op->a_entry & 0x3fffffff);
			clrseg(corep, op->a_bss);
			break;

#if (NS32WRMAGIC != NS32ROMAGIC)
		case NS32WRMAGIC:
#endif
		case NS32ROMAGIC:
			lseek(in, (off_t) 0, 0);
			read(in, (char *)&hdrs.sgshdr, sizeof(struct sgshdr));
			np = &hdrs.sgshdr.naout;
			count = np->tsize + np->dsize;
			if (sread(in, (char *) 0, count) != count) {
				_prs (argv [0]);
				_prs (": read error\n");
				return (-1);
			}
			corep = (char *) np->tsize;
			if ((np->magic == AOUT2MAGIC ||
			     np->magic == AOUT3MAGIC) && np->dsize) {
				if (np->magic == AOUT3MAGIC)
					corep = (char *) (((int) corep + KPAGE)
						& ~KPAGE);
				else corep = (char *) (((int) corep + PAGE)
						& ~PAGE);
				movc3up(np->dsize, (char *) np->tsize, corep);
			}
			corep += np->dsize;
			entry = (int (*)())(np->entry & 0x3fffffff);
			clrseg(corep, np->bsize);
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

set () {
	register struct mtab *mp;
	register struct dtab *dp;
	char mname [NAMSIZ];
	int i, drive, slice, unit;

	if (argc == 1) {
		for (mp = &_mtab [0]; mp < &_mtab [NMOUNT]; mp++) {
			if (mp->mt_name == 0)
				continue;
			ppath (mp->mt_name, "\t");
			_prs (mp->mt_dp->dt_name);
			_prs ("\n");
		}
		return;
	}

	if (argc != 3) {
		_prs ("Usage:	set [ {/|/usr} /dev/dsk/<drive#>s<slice#> ]\n");
		_prs ("Example:	set /usr /dev/dsk/1s3\n");
		return;
	}

	for (dp = &_dtab[0]; dp < &_dtab[MAXDEVS]; dp++) {
		if (strcmp (argv [2], dp->dt_name) == 0)
			break;
	}
	if (dp == &_dtab[MAXDEVS]) {
		_prs ("set: invalid device specification\n");
		return;
	}
	_cond (argv [1], mname);	/* convert mount name to abs path */
	for (mp = &_mtab [0]; mp < &_mtab [NMOUNT]; mp++) {
		if (mp->mt_name == 0)
			continue;
		if (strcmp (mname, mp->mt_name) == 0) {
			mp->mt_dp = dp;
			return;
		}
	}
	_prs ("set: bad fs name\n");
}

dotape ()	{
	int count, tdesc;

	switch (argc) {
	case 2:
		if (strcmp (argv[1], "rew") == 0) {
			mt[10] = '\0';	/* use /dev/mt/0l to rewind to beg. */
			if ((tdesc = open(mt, 0)) < 0) {
				_prs ("mt rew: cannot open tape\n");
				return (-1);
			}
			close(tdesc);
			return (-1);
		}
	case 3:
		if (strcmp (argv[1], "fsf") == 0) {
			count = atoi(argv[2]);
			if ((count < 1) || (count > 100)) {
				_prs ("mt fsf: number is < 1 or > 100\n");
				return (-1);
			}
			mt[10] = 'n';	/* /dev/mt/0ln goes to end of files */
			for ( ; count > 0; count--) {
				if ((tdesc = open(mt, 0)) < 0) {
					_prs("mt fsf: error during skip\n");
					return (-1);
				}
				close(tdesc);
			}
			return (-1);
		}
	default:
		_prs ("Usage: mt { rew | fsf <count> }\n");
		return (-1);
	}
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
		movc3 ((unsigned) nread, buf, addr);
		nleft -= (unsigned) nread;
		if (!cnt)
			return (count - nleft);
		addr += nread;
	} while (nleft != 0);
	return (count);
}
