/*	@(#)malloc.c	1.2	*/
/*LINTLIBRARY*/
#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
botch(s)
char *s;
{
	printf("assertion botched: %s\n",s);
	abort();
}
#else
#define ASSERT(p)
#endif

/*	avoid break bug */
#ifdef pdp11
#define GRANULE 64
#else
#define GRANULE 0
#endif
/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT int
#define ALIGN int
#define NALIGN 1
#define WORD sizeof(union store)
#if u370
#define BLOCK 4096	/* a multiple of WORD*/
#else
#define BLOCK 1024	/* a multiple of WORD*/
#endif
#define BUSY 1
#define NULL 0
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store {
	union store *ptr;
	ALIGN dummy[NALIGN];
	int calloc;		/*calloc clears an array of integers*/
};

extern char *sbrk();

static union store allocs[2];	/*initial arena*/
static union store *allocp;	/*search ptr*/
static union store *alloct;	/*arena top*/
static union store *allocx;	/*for benefit of realloc*/

char *
malloc(nbytes)
unsigned nbytes;
{
	register union store *p, *q;
	register int nw;
	static int temp;	/*coroutines assume no auto*/

	if(allocs[0].ptr == 0) {	/*first time*/
		allocs[0].ptr = setbusy(&allocs[1]);
		allocs[1].ptr = setbusy(&allocs[0]);
		alloct = &allocs[1];
		allocp = &allocs[0];
	}
	nw = (nbytes+WORD+WORD-1)/WORD;
	ASSERT(allocp >= allocs && allocp <= alloct);
	ASSERT(allock());
	for(p=allocp; ; ) {
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q > p && q < alloct);
					allocp = p;
					p->ptr = q->ptr;
				}
				if(q >= p+nw && p+nw >= p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p > q)
				ASSERT(p <= alloct);
			else if(q != alloct || p != allocs) {
				ASSERT(q == alloct && p == allocs);
				return(NULL);
			} else if(++temp > 1)
				break;
		}
		temp = ((nw+BLOCK/WORD)/(BLOCK/WORD))*(BLOCK/WORD);
		q = (union store *)sbrk(0);
		if(q+temp+GRANULE < q) {
			return(NULL);
		}
		q = (union store *)sbrk(temp*WORD);
		if((INT)q == -1) {
			return(NULL);
		}
		ASSERT(q > alloct);
		alloct->ptr = q;
		if(q != alloct+1)
			alloct->ptr = setbusy(alloct->ptr);
		alloct = q->ptr = q+temp-1;
		alloct->ptr = setbusy(allocs);
	}
found:
	allocp = p + nw;
	ASSERT(allocp <= alloct);
	if(q > allocp) {
		allocx = allocp->ptr;
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	return((char*)(p+1));
}

/*	freeing strategy tuned for LIFO allocation */

void
free(ap)
register char *ap;
{
	register union store *p = (union store *)ap;

	ASSERT(p > clearbusy(allocs[1].ptr) && p <= alloct);
	ASSERT(allock());
	allocp = --p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > allocp && p->ptr <= alloct);
}

/*
 *	realloc(p, nbytes) reallocates a block obtained from malloc()
 *	and freed since last call of malloc()
 *	to have new size nbytes, and old content
 *	returns new location, or 0 on failure
 */

char *
realloc(p, nbytes)
char	*p;
unsigned nbytes;
{
	register union store *q, *ap;
	union store *s, *t;
	register unsigned nw;
	unsigned onw;

	ap = (union store *)p;
	if(testbusy(ap[-1].ptr))
		free(p);
	onw = ap[-1].ptr - ap;
	q = (union store *)malloc(nbytes);
	if(q == NULL || q == ap)
		return((char *)q);
	s = ap;
	t = q;
	nw = (nbytes+WORD-1)/WORD;
	if(nw < onw)
		onw = nw;
	while(onw-- != 0)
		*t++ = *s++;
	if(q < ap && q+nw >= ap)
		(q+(q+nw-ap))->ptr = allocx;
	return((char *)q);
}

#ifdef debug
allock()
{
#ifdef longdebug
	register union store *p;
	int x;
	x = 0;
	for(p= &allocs[0]; clearbusy(p->ptr) > p; p=clearbusy(p->ptr)) {
		if(p == allocp)
			x++;
	}
	ASSERT(p == alloct);
	return(x == 1 || p == allocp);
#else
	return(1);
#endif
}
#endif
