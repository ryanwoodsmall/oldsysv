/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:hash.c	1.2"
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * These algorithm's are derived, in part
 * from Knuth Volume 3
 *
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#include <assert.h>
#include <stdio.h>
#include "hash.h"

#define	TRUE		1
#define	FALSE		0

#define	COMPARE(A,B)	((thetable->compare)((A), (B)) == 0)
#define	DELETED(A)	((A)->data == NULL)
#define	DELETE(A)	(A)->data = NULL
#define	DATA(A)		(A)->data
#define	NEXT(A)		(A)->next
#define	KEY(A)		(thetable->getkey)(A)

#define	M		thetable->m
#define	TABLE		thetable->head
#define	NUMENTRIES	thetable->numentries

#define	HEAD(H)		((H)==0?default_table:H)

extern	char	*pgm_name;
H_HEAD	*default_table, *thetable;
static	H_ENTRY	*mkentry();
H_ENTRY	*calloc();


int
hcreate(table, compare, getkey, estimated_size)
H_HEAD	**table;
unsigned (*getkey)();
int	(*compare)();
int	estimated_size;
{
	double	sqrt();
	
	/* set up the header */
	thetable = (H_HEAD *)calloc(1, sizeof(*thetable));
	NUMENTRIES = 0;

	/* check for a valid compare routine */
	if (compare == NULL)
		return FALSE;
	thetable->compare = compare;

	/* check for a valid getkey routine */
	if (getkey == NULL)
		return FALSE;
	thetable->getkey = getkey;

	/* set up the reference to the proper table */
	if (table == NULL) {
		if (default_table != NULL)
			return FALSE;
		default_table = thetable;
	} else
		*table = thetable;

	/* determine the table size, and allocate it */
	M = estimated_size + sqrt((double) estimated_size);
	for (; !prime((long)M); M++)
		;
	TABLE = (H_ENTRY *) calloc(M, sizeof(*TABLE));	
	return TRUE;
}


int
hinsert(table, data)
H_HEAD	*table;
int	*data;
{
	unsigned k;
	int	h1(), h2();
	H_ENTRY	*p, *q;

	thetable = HEAD(table);
	/* first hash */
	k = KEY(data);
	p = TABLE + h1(k);

	if (!DELETED(p))
		if (!COMPARE(data, DATA(p))) {
			/* second hash */
			p -= h2(k);	
			if (p < TABLE)
				p += M;
			if (!DELETED(p))
				if (!COMPARE(data, DATA(p))) {
					/* check the chain */
					for (p = NEXT(q = p); p != NULL; p = NEXT(q = p))
						if (COMPARE(data, DATA(p)))
							/* found a <<matching>> record */
							return FALSE;
					assert(p == NULL);
					/* add the new node to the chain */
					NEXT(q) = mkentry(data);
					NUMENTRIES++;
					return TRUE;
				} else
					/* found a <<matching>> record */
					return FALSE;
			else {
				/* add the new entry in the empty slot */
				DATA(p) = data;
				NUMENTRIES++;
				return TRUE;
			}
		} else
			/* found a <<matching>> record */
			return FALSE;
	else {
		/* add the new entry in the empty slot */
		DATA(p) = data;
		NUMENTRIES++;
		return TRUE;
	}
}


int	*
hlookup(table, data)
H_HEAD	*table;
int	*data;
{
	unsigned k;
	int	h1(), h2();
	H_ENTRY	*p;

	thetable = HEAD(table);
	/* first hash */
	k = KEY(data);
	p = TABLE + h1(k);

	if (DELETED(p) || !COMPARE(data, DATA(p))) {
		/* second hash */
		p -= h2(k);	
		if (p < TABLE)
			p += M;
		if (DELETED(p) || !COMPARE(data, DATA(p))) {
			/* check the chain */
			for (p = NEXT(p); p != NULL; p = NEXT(p))
				if (COMPARE(data, DATA(p)))
					/* found a <<matching>> record */
					return DATA(p);
			assert(p == NULL);
			/* did not find a match */
			return NULL;
		} else
			/* found a <<matching>> record */
			return DATA(p);
	} else
		/* found a <<matching>> record */
		return DATA(p);
}


int	
hdelete(table, data)
H_HEAD	*table;
int	*data;
{
	unsigned k;
	int	h1(), h2();
	H_ENTRY	*p, *q;

	thetable = HEAD(table);
	/* first hash */
	k = KEY(data);
	p = TABLE + h1(k);

	if (DELETED(p) || !COMPARE(data, DATA(p))) {
		/* second hash */
		p -= h2(k);	
		if (p < TABLE)
			p += M;
		if (DELETED(p) || !COMPARE(data, DATA(p))) {
			/* check the chain */
			for (p = NEXT(q = p); p != NULL; p = NEXT(q = p))
				if (COMPARE(data, DATA(p))) {
					/* found a <<matching>> record */
					/* close up the chain */
					NEXT(q)=NEXT(p);
					free(p);
					NUMENTRIES--;
					return TRUE;
				}
			assert(p == NULL);
			/* did not find a match */
			return FALSE;
		} else {
			/* found a <<matching>> record */
			/* move the first record of the chain in(if possible) */
			if ((q = NEXT(p)) != NULL) {
				DATA(p) = DATA(q);
				NEXT(p) = NEXT(q);
				free(q);
			} else
				DELETE(p);
			NUMENTRIES--;
			return TRUE;
		}
	} else {
		/* found a <<matching>> record */
		/* move the first record of the chain in(if possible) */
		if ((q = NEXT(p)) != NULL) {
			DATA(p) = DATA(q);
			NEXT(p) = NEXT(q);
			free(q);
		} else
			DELETE(p);
		NUMENTRIES--;
		return TRUE;
	}
}


int
hscan(table, datalist, listsize)
H_HEAD	*table;
int	*datalist[];
int	listsize;
{
	H_ENTRY	*p, *q;
	int	added;
	int	**datapointer;

	thetable = HEAD(table);
	added = 0;
	datapointer = datalist;
	for (p = TABLE; p < TABLE+M; p++) {
		if (!DELETED(p) && (datapointer < datalist+listsize) ) {
			added++;	
			*(datapointer++) = DATA(p);
		}
		for (q=NEXT(p); q != NULL; q = NEXT(q))
			if (!DELETED(q) && (datapointer < datalist+listsize) ) {
				added++;	
				*(datapointer++) = DATA(q);
			}
	}
	return added;
}


int
hdestroy(table)
H_HEAD	*table;
{
	H_ENTRY	*p, *q, *t;

	thetable = HEAD(table);
	for (p = TABLE; p < TABLE+M; p++) {
		/* this could be done with a `for,` but this is cleaner */
		q = NEXT(p);
		while (q != NULL) {
			/* save the address of the block to be freed */
			t = q;
			/* get the address of the next block */
			q = NEXT(q);
			/* free the block */
			free(t);
		}
	}
}


int
hdump(table)
H_HEAD	*table;
{
	H_ENTRY	*p, *q;

	thetable = HEAD(table);
	fprintf(stderr, "numentries: %d, size: %d, head:%lx\n",
		NUMENTRIES, M, TABLE);
	fprintf(stderr, " addr\tnext\tdata\n");
	for (p = TABLE; p < TABLE+M; p++) {
		fprintf(stderr, " %d\t%d\t%d\n", p, NEXT(p), DATA(p));
		for (q=NEXT(p); q != NULL; q = NEXT(q))
			fprintf(stderr, "^%d\t%d\t%d\n", q, NEXT(q), DATA(q));
	}
}


static	int
h1(key)
unsigned key;
{
	return (key % M);
}

static	int
h2(key)
unsigned key;
{
	return (1 + (key % (M-2)));
}

static	H_ENTRY	*
mkentry(data)
int	*data;
{
	H_ENTRY	*p;
	p = calloc(1, sizeof(*p));
	DATA(p) = data;
	NEXT(p) = NULL;
	return p;
}


#define	MAXELEM	150
static	long	primes[MAXELEM]	= {	/* only first 100 are initialized! */
	  2,   3,   5,   7,  11,  13,  17,  19,  23,  29,
	 31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
	 73,  79,  83,  89,  97, 101, 103, 107, 109, 113,
	127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
	179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
	233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
	283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
	353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
	419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
	467, 479, 487, 491, 499, 503, 509, 521, 523, 541
};
prime(i) /* determine if i is prime */
long i;
{
	static	int	ep = 100;	/* 1st unitialize element of primes[] */
	int	j;
	long	root;
	double	pow(), ceil();

	root = ceil(pow((double)i,.5));
	for( j=0; primes[j]<=root; j++ )
		if( i%primes[j]==0 )
			return(0);

	if( i>primes[ep] ) /* add prime to list */
		if( ep<MAXELEM )
			primes[ep++]=i;
		else if( i>primes[ep-1]*primes[ep-1] )
			fprintf(stderr, "table size too large\n");
	return(1);
}
