/*	@(#)sysfix.c	1.1	*/
#include <a.out.h>

char	buf[2048];

unsigned	ovsize[8];

int	ofd;

unsigned	bsize, dsize, tsize;

int	vflg, ovflg;

struct	exec	hdr;

main(argc, argv)
char *argv[];
{
	unsigned register	i, j;
	int		ifd;
	unsigned	npad;
	long		totovsiz = 0;
	long		ltsize;

	if( argc == 4 && strcmp(argv[1], "-v") == 0 ) {
		vflg++;
		argc--, argv++;
	}
	if( argc != 3 ) {
		printf("Usage: sysfix [-v] a.out unix\n");
		exit(1);
	}

	if((ifd = open(argv[1], 0)) == -1) {
		printf("Can't open %s\n", argv[1]);
		exit(1);
	}
	if((ofd = creat(argv[2], 0766)) == -1) {
		printf("Can't create %s\n", argv[2]);
		exit(1);
	}
	if(read(ifd, &hdr, sizeof hdr) != sizeof hdr) {
		printf("Read error on a.out header\n");
		exit(1);
	}
	if(hdr.a_magic == 0431) {
		ovflg++;
		if(read(ifd, &ovsize, sizeof ovsize) != sizeof ovsize) {
			printf("Read error on overlay segment sizes\n");
			exit(1);
		}
		for(i = 1; i < 8; i++) {
			if( ovsize[i] > 8192 ) {
				printf("Text overlay #%d > 8Kb\n", i);
				exit(1);
			}
			totovsiz += ovsize[i];
		}
	} else if(hdr.a_magic != 0411) {
		printf("%s: bad format (magic number != 0411/0437)\n", argv[1]);
		exit(1);
	}
	if( (long)hdr.a_data+hdr.a_bss > 6*8192L) {
		printf("Combined data and bss size > 48Kb\n");
		exit(1);
	}
	dsize = hdr.a_data;
	bsize = hdr.a_bss;
	tsize = hdr.a_text;
	ltsize = tsize==0? 0200000L: (long)tsize;

	hdr.a_data += hdr.a_bss;	/* sum < 48Kb */
	hdr.a_bss = 0;
	npad = (64 - (hdr.a_data % 64))%64;
	hdr.a_data += npad;
	npad += bsize;

	if( ovflg ) {
		if( ltsize > 7*8192L ) {
			printf("Root text segment > 56Kb\n");
			exit(1);
		}
		ltsize += totovsiz;
		hdr.a_hitext = ltsize >> 16;
		hdr.a_text = (unsigned)ltsize;
		hdr.a_magic = 0437;
	} else {
		hdr.a_magic = 0407;
	}

	if(write(ofd, &hdr, sizeof hdr) != sizeof hdr) {
		perror("Write error on a.out header");
		exit(1);
	}

	lseek(ifd, ltsize, 1);	/* point at data */
	for( j = dsize; j; j -= i ) {
		if( (i = read(ifd, buf, sizeof buf)) == -1) {
			perror("Read error in data segment");
			exit(1);
		}
		if(i == 0) {
			printf("Unexpected EOF in data segment\n");
			exit(1);
		}
		if( j < i )
			i = j;
		if(write(ofd, buf, i) != i) {
			perror("Write error in data segment");
			exit(1);
		}
	}

	for(i = 0; i < sizeof buf; i++ )
		buf[i] = 0;

	for( j = npad ; j; j -= i ) {
		if( j < i )
			i = j;
		if( write(ofd,buf,i) != i ) {
			if( ovflg )
				perror("Write error in bss segment");
			else
				perror("Write error on data pad");
			exit(1);
		}
	}

	lseek(ifd, (long) sizeof hdr, 0);
	if(ovflg)
		lseek(ifd, (long) sizeof ovsize, 1);

	while(ltsize) {
		if( (i = read(ifd, buf, sizeof buf)) == -1) {
			perror("Read error in text segment");
			exit(1);
		}
		if(i == 0) {
			printf("Unexpected EOF in text segment\n");
			exit(1);
		}
		if(ltsize < i)
			i = ltsize;
		ltsize -= i;
		if(write(ofd, buf, i) != i) {
			perror("Write error in text segment");
			exit(1);
		}
	}
	ltsize = tsize==0? 0200000L: (long)tsize;
	if(ovflg)
		ltsize += totovsize;

	lseek(ifd, (long) sizeof hdr, 0);
	if(ovflg)
		lseek(ifd, (long) sizeof ovsize, 1);
	lseek(ifd, ltsize, 1);
	lseek(ifd, (long) dsize, 1);

	while( (int)(i = read(ifd, buf, sizeof buf)) > 0) {
		if(write(ofd, buf, i) != i) {
			perror("Write error in symbol table");
			exit(1);
		}
	}
	if(i == -1) {
		perror("Read error in symbol table");
		exit(1);
	}

	return mmreg(argv[1]);		/* mm reg for kernal + overlays */
}

/*
 *	Calculate values for memory management registers,
 *	both for overlayed and non-overlayed kernals.
 *	Store values into output a.out.
 */

#define RO      02
#define RW      06

struct nlist nlmm[] = {
	{ "__ova" },
	{ "__ovd" },
	{ "__kisa" },
	{ "__kisd" },
	{ "__kdsa" },
	{ "__kdsd" },
	{ 0 }
};

mmreg(file)
char *file;
{
	register unsigned phys = 0;	/* physical memory pointer */
	register i;
	register unsigned siz;
	static unsigned kisa[8], kisd[8];	/* prototype kernal I-space */
	static unsigned kdsa[8], kdsd[8];	/* prototype kernal D-space */
	static unsigned ova[8], ovd[8];

	if( nlist(file, &nlmm) == -1 )		/* locate symbols */
		return(-1);

	siz = (dsize + bsize + 63)/64;	/* dataspace size in core clicks */
	for(i = 0; i < 6; i++)	/* data + bss */
		if(siz > 0200) {
			kdsd[i] = 077406;	/* max size and RW */
			kdsa[i] = phys;		/* location */
			phys += 0200;
			siz -= 0200;
		} else {
			kdsd[i] = RW | ( (siz-1)<<8 );
			kdsa[i] = phys;
			phys += siz;
			break;
		}

	siz = (tsize+63)/64;
	for(i = 0; i < 8; i++)		/* text */
		if(siz > 0200) {
			kisd[i] = 077402;	/* max size and RO */
			kisa[i] = phys;		/* location */
			phys += 0200;
			siz -= 0200;
		} else {
			kisd[i] = RO | ( (siz-1)<<8 );
			kisa[i] = phys;
			phys += siz;
			break;
		}

	if( ovflg ) {
		for(i = 1; i < 8; i++) {	/* handle overlays */
			siz = ovsize[i]/64;	/* size in core clicks */
			if(siz == 0)
				break;
			ova[i] = phys;
			ovd[i] =  RO | ((siz-1)<<8);
			phys += siz;
		}
	}

	kdsa[6] = phys;		/* user area */
	kdsd[6] = 07406;	/* USIZE + RW */
	kdsa[7] = 0177600;	/* IO page */
	kdsd[7] = 077406;	/* maxsize + RW */

	mmsym("__kdsa", kdsa, sizeof kdsa);
	mmsym("__kdsd", kdsd, sizeof kdsd);
	mmsym("__kisa", kisa, sizeof kisa);
	mmsym("__kisd", kisd, sizeof kisd);

	if( ovflg ) {
		mmsym("__ova",  ova,  sizeof ova);
		mmsym("__ovd",  ovd,  sizeof ovd);
	}

	if( !vflg )
		return 0;

	printf("\nkisd: ");
	for(i = 0; i < 8; i++)
		printf("%6o ", kisd[i]);

	printf("\nkdsd: ");
	for(i = 0; i < 8; i++)
		printf("%6o ", kdsd[i]);

	printf("\nkisa: ");
	for(i = 0; i < 8; i++)
		printf("%6o ", kisa[i]);

	printf("\nkdsa: ");
	for(i = 0; i < 8; i++)
		printf("%6o ", kdsa[i]);
	printf("\n");

	if( !ovflg )
		return 0;

	printf("\novd:  ");
	for(i=0;i<8;i++)
		printf("%6o ", ovd[i]);

	printf("\nova:  ");
	for(i = 0; i < 8; i++)
		printf("%6o ", ova[i]);
	printf("\n");

	return 0;
}

mmsym(id, value, size)
char	 *id;
unsigned *value;
unsigned size;
{
	struct	nlist	*n;

	for(n = &nlmm; n->n_name[0]; n++) {
		if(strncmp(n->n_name, id, 8) != 0)
			continue;
		if(n->n_value == 0)
			break;
		lseek(ofd, (long)(sizeof hdr)+(long)(n->n_value), 0);
		write(ofd, value, size);
		return;
	}
	printf("%s not found\n", id);
}

#define SPACE 100		/* number of symbols read at a time */

extern long lseek();
extern int open(), read(), close(), strncmp();

int
nlist(name, list)
char *name;
struct nlist *list;
{
	register struct	nlist	*p, *q;
	unsigned n, m;
	struct	nlist	space[SPACE];
	struct	exec	buf;
	int	i;
	int	fd, nlen=sizeof(space[0].n_name);
	long	sa;

	for(p = list; p->n_name[0]; p++) {
		p->n_type = 0;
		p->n_value = 0;
	}
	if((fd = open(name, 0)) < 0)
		return(-1);
	read(fd, (char *)&buf, sizeof buf);
	if( ovflg )
		read(fd, ovsize, 16);
	sa = buf.a_text;
	sa += buf.a_data;
	if(buf.a_flag != 1)
		sa += sa;
	if( ovflg ) {
		for(i = 1; i < 8; i++)
			sa += ovsize[i];
		sa += sizeof ovsize;
	}
	sa += (long) sizeof buf;
	lseek(fd, sa, 0);
	n = buf.a_syms;
	while(n) {
		m = (n < sizeof space)? n: sizeof space;
		read(fd, (char*)space, m);
		n -= m;
		for(q=space; (int)(m -= sizeof space[0]) >= 0; ++q) {
			for(p=list; p->n_name[0]; ++p) {
				if(strncmp(p->n_name, q->n_name, nlen))
					continue;
				p->n_value = q->n_value;
				p->n_type = q->n_type;
				break;
			}
		}
	}
	close(fd);
	return(0);
}
