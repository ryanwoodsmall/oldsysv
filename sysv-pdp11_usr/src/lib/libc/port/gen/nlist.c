/*	@(#)nlist.c	1.5	*/
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
/*	@(#)nlist.c	1.5	*/
#if vax
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)VAXROMAGIC) || \
			  (((unsigned short)x)==(unsigned short)VAXWRMAGIC))
#endif
#if u3b
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)N3BMAGIC) || \
			  (((unsigned short)x)==(unsigned short)NTVMAGIC))
#endif
#if u3b || vax
#define BADMAG(x)	(!ISMAGIC(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default : readonly sharable text segment
 *		AOUT2MAGIC:	     : writable text segment
 */

#define AOUT1MAGIC 0410
#define AOUT2MAGIC 0407

#define	SGSNAME	""
#define SGS ""
#define RELEASE "Release 5.0 6/1/82"
#include <a.out.h>
#if !(u3b || vax)
#define SPACE 100		/* number of symbols read at a time */
#endif

extern long lseek();
extern int open(), read(), close(), strncmp();

int
nlist(name, list)
char *name;
#if u3b || vax
struct syment *list;
#else
struct nlist *list;
#endif
{
#if u3b || vax
	struct	filehdr	buf;
	struct	syment	sym;
	struct	auxent	aux;
	long n;
	int	nlen=SYMNMLEN, bufsiz=FILHSZ;
	register struct syment *p, *q;
#else
	struct nlist space[SPACE];
	struct exec buf;
	int	nlen=sizeof(space[0].n_name), bufsiz=(sizeof(buf));
	unsigned n, m; 
	register struct nlist *p, *q;
#endif
	long	sa;
	int	fd;

	for(p = list; p->n_name[0]; p++) {
		p->n_type = 0;
		p->n_value = 0;
#if u3b || vax
		p->n_value = 0L;
		p->n_scnum = 0;
		p->n_sclass = 0;
#endif
	}
	
	if((fd = open(name, 0)) < 0)
		return(-1);
	(void) read(fd, (char *)&buf, bufsiz);
#if u3b || vax
	if(BADMAG(buf.f_magic)) {
#else
	if(BADMAG(buf)) {
#endif
		(void) close(fd);
		return(-1);
	}
#if u3b || vax
	sa = buf.f_symptr;	/* direct pointer to sym tab */
	lseek(fd, sa, 0);
	q = &sym;
	n = buf.f_nsyms;	/* num. of sym tab entries */
#else
	sa = buf.a_text;
	sa += buf.a_data;
#if u370
	sa += (long)(buf.a_trsize + buf.a_drsize);
#endif
#if pdp11
	if(buf.a_flag != 1)
		sa += sa;
	else if( buf.a_magic == A_MAGIC5 )
		sa += (long)buf.a_hitext << 16; /* remainder of text size for system overlay a.out */
#endif
	sa += (long)sizeof(buf);
	(void) lseek(fd, sa, 0);
	n = buf.a_syms;
#endif

	while(n) {
#if u3b || vax
		read(fd, (char *)&sym, SYMESZ);
		n -= (q->n_numaux + 1L);
			/* read past aux ent , if there is one */
		if (q->n_numaux != 0)	read(fd, (char *) &aux, AUXESZ);
#else
		m = (n < sizeof(space))? n: sizeof(space);
		(void) read(fd, (char*)space, m);
		n -= m;
		for(q=space; (int)(m -= sizeof(space[0])) >= 0; ++q) {
#endif
			for(p=list; p->n_name[0]; ++p) {
				if(strncmp(p->n_name, q->n_name, nlen))
					continue;
				p->n_value = q->n_value;
				p->n_type = q->n_type;
#if u3b || vax
				p->n_scnum = q->n_scnum;
				p->n_sclass = q->n_sclass;
#endif
				break;
			}
#if !(u3b || vax)
		}
#endif
	}
	(void) close(fd);
	return(0);
}
