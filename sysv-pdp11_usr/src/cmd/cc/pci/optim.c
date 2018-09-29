/*	@(#)optim.c	2.2	*/

/*	machine independent improvement routines */

#include "optim.h"

/* unit of allocatable space (in char *'s) */
#ifndef NSPACE
#define NSPACE	1024
#endif

/* maximum number of labels referenced in a function */
#ifndef NUMLBLS
#define NUMLBLS	513
#endif
#define H_INCR	5
#define N_LBLS	(NUMLBLS / H_INCR * H_INCR + 1)

/* what to do if no input file specified */
#ifndef NOFILE
#define NOFILE()	/* by default, use stdin */
#endif

/* what to report if file-opening fails */
#ifndef FFILER
#define FFILER(S)	"can't open %s\n"
#endif

/* block of text */

typedef struct block {
	struct block *next;	/* pointer to textually next block */
	struct block *nextl;	/* pointer to next executed block if no br */
	struct block *nextr;	/* pointer to next executed block if br */
	struct block *ltest;	/* for loop termination tests */
	NODE *firstn;		/* first text node of block */
	NODE *lastn;		/* last text node of block */
	short index;		/* block index for debugging purposes */
	short length;		/* number of instructions in block */
	short indeg;		/* number of text references */
	short marked;		/* marker for various things */
#ifdef LIVEDEAD
	unsigned
	    blive:LIVEDEAD,	/* live registers */
	    bdead:LIVEDEAD;	/* dead registers */
#endif
} BLOCK;

/* symbol table entry */

typedef struct {
	char *cp;		/* the symbol */
	BLOCK *bl;		/* the block it is defined in */
} LBL;

/* data structures */

NODE n0;			/* header for text list */
NODE ntail = { NULL, NULL, TAIL };	/* trailer for text list */
REF r0;				/* header for non-text reference list */
REF *lastref;			/* pointer to last label reference */
static BLOCK b0;		/* header for block list */
static LBL *Lbltbl;		/* pointer to hash table of labels */
static int Numlbls;		/* count of labels in hash table */
static BLOCK *Prevb;		/* pointer to previous block during
				   traversal of list */
int fnum = 0;			/* function counter */
int npass;			/* pass-through-this-function counter */
static int idx;			/* block index (for debugging) */

/* space allocation control */

static struct space_t {	/* to manage space allocation */
	struct space_t *next;
	char *space[NSPACE - 1];
} *s0 = NULL, **Space;
static char *Lasta, *Lastx;	/* pointers into allocatable space */
static long Maxu = 0, Maxm = 0, Maxa = 0;

/* statistic counters */

int ndisc = 0;			/* instructions discarded */
int ninst = 0;			/* total instructions */
static int nunr = 0;		/* unreachable instructions */
static int nmerge = 0;		/* redundant instructions */
static int nsave = 0;		/* branches saved */
static int nrot = 0;		/* rotated loops */
static int noptim = 0;		/* calls to optim */
#define PCASES 13
static int Pcase[PCASES + 1];	/* block types during reconstruction of text */
static struct added {		/* to keep statistics on branches added */
	struct added *next;
	short fnum,
	n_added;
} a0, *lastadd = &a0;

/* debugging flags */

static int bflag = 0;		/* blocks after initial construction */
static int eflag = 0;		/* execution trace */
static int lflag = 0;		/* labels deleted */
static int mflag = 0;		/* common suffixes found */
static int pflag = 0;		/* reconstruction trace */
int sflag = 0;			/* statistics (on stderr) */
static int uflag = 0;		/* unreachable code deleted */
static int wflag = 0;		/* peephole trace */

/* for readability */

#define FATAL(S)	fatal(S, (char *)NULL)
#define FLAG(L, LFLAG)	case 'L': LFLAG = -1; continue;
#define ALLB(b, s)	b = s; b != NULL; b = b->next
#define PRCASE(N)	if (pflag) { prcase(N, b); Pcase[N]++; }
#define TRACE(F)	if (eflag) PRINTF("%cStarting F[%d, %d]\n", \
			    CC, fnum, npass)
#define MPRINTF		if (mflag) PRINTF
#define PRINDEX(P)	(b->P == NULL ? 0 : b->P->index)
#define PSTAT(S, N)	if ((N) > 0) FPRINTF(stderr, S, (N))
#define FINDLBL(l)	lblhash(l, (BLOCK *)NULL)
#define ADDLBL(l, b)	(void) lblhash(l, b)
#define ISUNCBL(b)	((b)->length == 1 && isuncbr((b)->lastn))
#define ISREMBR(p)	(isbr(p) && !ishb(p))
#define TARGET(b)	while (b->nextl != NULL && ISUNCBL(b->nextl)) \
				b = b->nextl

/* function declarations */

extern char *label_left();
extern BLOCK *lblhash();
extern void bldgr(), putbr(), rmunrch();

/************************************************************************/

	int
main(argc, argv) /* initialize, process parameters,  control processing, etc. */
	int argc; register char *argv[]; {

	extern void mustopen();
	char usrflag[10];
	int ufl = 0, i, fileseen = 0;

	/* process parameters on command line */

	while (--argc > 0) {
		if (**++argv != '-') { /* alternate file(s) */
			switch (fileseen) {
			case 0: /* none yet, open input file */
				mustopen(*argv, "r", stdin);
				fileseen++;
				continue;
			case 1: /* input seen, open output file */
				mustopen(*argv, "w", stdout);
				fileseen++;
				continue;
			}
			FATAL("too many filenames\n");
		}
		while ((i = *++*argv) != '\0') { /* debugging flags */
			switch (tolower(i)) {
			case 'i': /* alternate input file */
				if (--argc <= 0)
					FATAL("no argument for -I option\n");
				mustopen(*++argv, "r", stdin);
				break;
			case 'o': /* alternate output file */
				if (--argc <= 0)
					FATAL("no argument for -O option\n");
				mustopen(*++argv, "w", stdout);
				break;
			FLAG(b, bflag);
			FLAG(c, nmerge); /* kill comtail */
			FLAG(e, eflag);
			FLAG(l, lflag);
			FLAG(m, mflag);
			FLAG(n, noptim); /* kill optimization */
			FLAG(p, pflag);
			FLAG(r, nrot); /* kill loop rotations */
			FLAG(s, sflag);
			FLAG(u, uflag);
			FLAG(w, wflag);
			default:
				usrflag[ufl++] = i;
				continue;
			}
			break;
		}
	}
	if (bflag | eflag | lflag | mflag | pflag | uflag | wflag)
		setbuf(stdout, (char *)NULL); /* for easier debugging */
	NOFILE(); /* if no input file specified */

	/* initialize everything */

	Lbltbl = (LBL *) xalloc(N_LBLS * sizeof(LBL));
	init();
	usrflag[ufl] = '\0';
	yyinit(usrflag);

	/* transfer to the machine dependent part */

	(void) yylex();
	wrapup();

	/* print statistics if asked for */

	if (sflag) {
		register struct added *a;

		for (a = a0.next; a != NULL; a = a->next)
			FPRINTF(stderr, "%d branch(es) added to function %d\n",
			    a->n_added, a->fnum);
		dstats();	/* machine dependent statistics */
		PSTAT("%d unreachable instruction(s) deleted\n", nunr);
		PSTAT("%d branch(es) saved\n", nsave);
		PSTAT("%d instruction(s) merged\n", nmerge);
		FPRINTF(stderr, "%d of %d total instructions discarded\n",
		    ndisc, ninst);
		PSTAT("%d loop(s) rotated\n", nrot);
		FPRINTF(stderr,
		"%ld bytes used, %ld allocated\n%d function(s), %d optim(s)\n",
		    Maxm, Maxa, fnum - 1, noptim > 0 ? noptim : 0);
		if (pflag && noptim > 0)
			for (FPRINTF(stderr, "case\tnumber\n"),
			    i = 0; i <= PCASES; i++)
				FPRINTF(stderr, "%2d\t%3d\n", i, Pcase[i]);
	}
	return (0);
}

	static void
mustopen(name, dir, file) char *name, *dir; FILE *file; {

	if (freopen(name, dir, file) == NULL)
		fatal(FFILER(name), name);
}

	void
init() { /* reset pointers, counters, etc for next function */

	register struct space_t *p, *pp;
	long maxa = N_LBLS * sizeof(LBL);

	for (p = s0; p != NULL; p = pp) {
		maxa += sizeof(struct space_t);
		pp = p->next;
		free((char *) p);
	}
	if ((Maxu += Numlbls * sizeof(LBL)) > Maxm)
		Maxm = Maxu;
	if (maxa > Maxa)
		Maxa = maxa;
	Maxu = 0;
	Space = &s0;
	s0 = NULL;
	Lasta = Lastx = NULL;
	n0.forw = &ntail;
	ntail.back = &n0;
	b0.firstn = b0.lastn = &n0;
	r0.nextref = NULL;
	lastref = &r0;
	fnum++;
	npass = 0;
}

	void
optim() { /* control improvement sequencing */

	if (noptim < 0)
		return;
	noptim++;
	if (n0.forw != &ntail) {

		extern void rmlbls(), mrgbrs(), comtail(), reord(), rmbrs();
		int onsave = nsave;

		rmlbls();	/* remove useless labels */
		bldgr(1);	/* build flow graph and call bboptim */
		mrgbrs();	/* merge branches to branches */
		rmunrch();	/* remove unreachable code */
		if (nmerge >= 0)
			comtail(); /* remove common tails */
		reord();	/* reorder code */
#ifdef LIVEDEAD
		{
		extern void ld_anal();
		ld_anal();	/* perform live/dead analysis */
		}
#endif
		rmbrs();	/* remove redundant branches */
		rmlbls();	/* remove useless labels */
		if (sflag && onsave > nsave) {
			register struct added *a = lastadd = lastadd->next =
			    (struct added *) xalloc(sizeof(struct added));

			a->next = NULL;
			a->fnum = fnum;
			a->n_added = onsave - nsave;
		}
	}
	npass++;
}

	static void
rmlbls() { /* remove unreferenced labels */

	register REF *r;
	register NODE *p;
	register char *s;

	TRACE(rmlbls);

	clrltbl();

	/* add references from data section */

	for (r = r0.nextref; r != NULL; r = r->nextref)
		ADDLBL(r->lab, &b0);

	/* add references from branches in text section */

	for (ALLN(p))
		if (isbr(p) && (s = getp(p)) != NULL)
			ADDLBL(s, &b0);

	/* delete non-hard labels that are not now in the label table */

	for (ALLN(p))
		if (islabel(p) && !ishl(p) && FINDLBL(p->ops[0]) == NULL) {
			if (lflag)
				PRINTF("%clabel %s removed\n", CC, p->ops[0]);
			DELNODE(p);
		}

}

	static void
bldgr(opt) int opt; { /* build flow graph of procedure */

	register BLOCK *b = &b0;
	register NODE *p;

	TRACE(bldgr);

	idx = 0;
	for (p = n0.forw; p != &ntail; ) {

		register NODE *prevn = p->back;
		BLOCK *prevb = b;

		b = prevb->next = GETSTR(BLOCK);
		b->index = ++idx;
		b->length = b->indeg = 0;

		/* a block starts with 0 or more labels */

		while (islabel(p))
			p = p->forw;

		/* followed by 0 or more non-branch instructions
		   terminated with a branch or before another label */

		for ( ; p != &ntail && !islabel(p); p = p->forw) {
			b->length++;
			if (isbr(p)) {
				p = p->forw;
				break;
			}
		}
		if (opt) { /* do dependent basic-block optimization */

			int omit = bboptim(prevn->forw, p->back);
			if (omit > b->length)
				omit = b->length;
			b->length -= omit;
			ndisc += omit;
		}
		b->lastn = p->back;
		if ((b->firstn = prevn->forw) == p) /* if no nodes left */
			b = prevb; /* wipe out this block */
	}
	b->next = NULL;

	mkltbl(); /* make label table with only definitions */

	/* set branch pointers */

	for (ALLB(b, b0.next)) {
		char *s;

		p = b->lastn;
		b->nextl = b->next;
		b->nextr = isbr(p) && (s = getp(p)) != NULL ?
		    FINDLBL(s) : NULL;
		if (isuncbr(p)) {
			b->nextl = b->nextr;
			b->nextr = NULL;
		}
		if (bflag) {
			PRINTF(
		"%c\n%cblock %d (left: %d, right: %d, length: %d)\n%cfirst:\t",
			    CC, CC, b->index, PRINDEX(nextl), PRINDEX(nextr),
			    b->length, CC);
			prinst(b->firstn);
			PRINTF("%clast:\t", CC);
			prinst(p);
		}
	}
}

	static void
mrgbrs() { /* merge branches to unconditional branches */

	register BLOCK *b, *bb;

	TRACE(mrgbrs);

	/*
	 * It is assumed that "ret" is an unconditional branch;
	 * that getp on a "ret" returns NULL; that this can be
	 * placed on an unconditional branch ("jbr");
	 * that prinst() will convert "jbr NULL" back to "ret";
	 * but that the NULL
	 * cannot be placed on a conditional branch ("jne").
	 */

	for (ALLB(b, b0.next)) {

		char *t;
		register NODE *p = b->lastn;

		if (!isbr(p))
			continue;
		if (isuncbr(p))
			while ((bb = b->nextl) != NULL &&
			    bb->nextl != b->nextl &&
			    ISUNCBL(bb) && !ishb(bb->lastn)) {
				t = getp(bb->lastn);
				putp(p, t);
				b->nextl = bb->nextl;
			}
		else
			while ((bb = b->nextr) != NULL &&
			    bb->nextl != b->nextr &&
			    ISUNCBL(bb) && !ishb(bb->lastn) &&
			    (t = getp(bb->lastn)) != NULL) {
				putp(p, t);
				b->nextr = bb->nextl;
			}
	}
}

	static void
comtail() { /* merge common tails from code blocks */

	boolean changed;

	TRACE(comtail);

	do {
		extern boolean chktarget(), chktail();
		extern void mrglbl();
		register BLOCK *bi, *bj;

		changed = false;
		for (ALLB(bi, b0.next))
			for (ALLB(bj, bi->next))
				if (bi->nextr == bj->nextr &&
				    chktarget(bi, bj) == true &&
				    chktail(bi, bj) == true)
					 changed = true;
		mrglbl(); /* merge blocks consisting only of labels */
	} while (changed == true);
}

	static boolean
chktarget(bi, bj) register BLOCK *bi, *bj; { /* check lefts of bi and bj */

	/* if both blocks end in conditional branches, look ahead */

	if (isbr(bi->lastn) && !isuncbr(bi->lastn) ||
	    isbr(bj->lastn) && !isuncbr(bj->lastn)) {
		if (bi->lastn->op != bj->lastn->op)
			return (false);
		if (bi->nextr == NULL && !same(bi->lastn, bj->lastn))
			return (false);
		TARGET(bi);
		TARGET(bj);
	}

	/* both blocks must fall through to the same place */

	if (bi->nextl != bj->nextl)
		return (false);

	/* dead-end branches must be identical in text */

	return (bi->nextl != NULL || same(bi->lastn, bj->lastn) ?
		true : false);
}

	static boolean
chktail(bi, bj) register BLOCK *bi, *bj; { /* merge tails of bi-> and bj-> */

	extern void rmtail(), modrefs();
	register BLOCK *bn;
	NODE *pi = bi->lastn, *pj = bj->lastn, *firstn, *lastn, *pb = NULL;
	int length = 0, isbri = 0, isbrj = 0;

	/* pi and pj scan backwards through blocks bi and bj 
	   until difference or no more code */

	lastn = pj;
	if (isbr(pi)) { /* trailing branches have already been matched */
		pb = pi;
		pi = (pi == bi->firstn) ? NULL : pi->back;
		isbri++;
	}
	if (isbr(pj)) {
		pb = pj;
		pj = (pj == bj->firstn) ? NULL : pj->back;
		isbrj++;
	}
	if (isbri && pi != NULL && isbrj && pj != NULL &&
	    (islabel(pi) || islabel(pj))) {
		length++;
		isbri = isbrj = 0;
		pb = NULL;
	}
	else
		lastn = pj;
	firstn = lastn;
	for ( ; pi != NULL && pj != NULL &&
	   !islabel(pi) && !islabel(pj) && same(pi, pj) == true; length++) {
		firstn = pj;
		pi = (pi == bi->firstn) ? NULL : pi->back;
		pj = (pj == bj->firstn) ? NULL : pj->back;
	}
	if (length == 0)
		return (false);
	MPRINTF("%c%d instruction(s) common to blocks %d and %d\n",
		CC, length, bi->index, bj->index);

	/* if blocks identical, change references to one to the other */

	if (pi == NULL && pj == NULL) {
		isbri = 0;
		modrefs(bj, bn = bi);
		MPRINTF("%cblock %d merged into block %d and deleted\n",
			CC, bj->index, bi->index);
	}

	/* if one block is a tail of the other, remove the tail from the
	   larger block and make it reference the smaller */

	else if (pi == NULL) {
		isbri = 0;
		bj->lastn = pj;
		bj->length -= length + isbrj;
		bj->nextl = bn = bi;
		rmtail(bj);
	}
	else if (pj == NULL) {
		isbrj = 0;
		bi->lastn = pi;
		bi->length -= length + isbri;
		bi->nextl = bn = bj;
		rmtail(bi);
	}

	/* otherwise make a new block, remove tails from common blocks and
	   make them reference the new block */

	else {
		bi->lastn = pi;
		bj->lastn = pj;
		bi->length -= length + isbri;
		bj->length -= length + isbrj;
		bn = GETSTR(BLOCK);
		*bn = *bj;
		bn->firstn = firstn;
		bn->lastn = lastn;
		bn->length = length;
		bn->index = ++idx;
		bi->nextl = bj->nextl = bj->next = bn;
		bi->nextr = bj->nextr = NULL;
		MPRINTF("%ctails of %d and %d merged into new block %d\n",
		    CC, bi->index, bj->index, idx);
	}
	if (pb != NULL && !isbr(bn->lastn)) { /* save final branch */
		ndisc--;
		nsave--;
		bn->length++;
		pb->back = bn->lastn;
		bn->lastn = bn->lastn->forw = pb;
	}

#ifdef IDVAL
	for (pb = bn->firstn; pb != NULL; pb = pb->forw) {
		pb->uniqid = IDVAL;
		if (pb == bn->lastn)
			break;
	}
#endif

	ndisc += length + isbri + isbrj;
	nmerge += length;
	nsave += isbri + isbrj; /* don't blame resequence for added branch */
	return (true);
}

	static void
rmtail(b) register BLOCK *b; { /* remove tail of b */

	b->nextr = NULL;
	MPRINTF("%ctail of block %d deleted\n", CC, b->index);
}

	static void
mrglbl() { /* merge all-label blocks into their destination blocks */

	register BLOCK *b, *bl;

	for (ALLB(b, b0.next)) {
		if ((bl = b->nextl) == NULL || bl == b)
			continue;
		if (ISUNCBL(b) && islabel(b->firstn)) {
			ndisc++;
			nsave++;
			b->lastn = b->lastn->back;
		}
		if (islabel(b->lastn)) {
			bl->firstn->back = b->lastn;
			b->lastn->forw = bl->firstn;
			bl->firstn = b->firstn;
			modrefs(b, bl);
			MPRINTF("%cblock %d prepended to block %d\n",
			    CC, b->index, bl->index);
		}
	}
}

	static void
modrefs(bi, bj) BLOCK *bi, *bj; { /* change all refs from bi to bj */

	register BLOCK *b;

	for (ALLB(b, &b0)) {
		if (b->next == bi)
			b->next = bi->next;
		if (b->nextl == bi)
			b->nextl = bj;
		if (b->nextr == bi)
			b->nextr = bj;
	}
}

	static void
reord() { /* reorder code */

	extern BLOCK *reord1();
	extern void findlt();
	register BLOCK *b, *bb;

	TRACE(reord);

	for (ALLB(b, b0.next)) { /* compute indegree */
		if ((bb = b->nextl) != NULL)
			bb->indeg++;
		if ((bb = b->nextr) != NULL)
			bb->indeg++;
		b->ltest = NULL;
	}

	if (nrot >= 0)
		findlt(); /* find rotatable loops */

	/* tie blocks back together */

	if (pflag)
		PRINTF("%cblock\tleft\tright\tcase\tlabels\n", CC);
	for (ALLB(b, b0.next)) /* mark all blocks as unprocessed */
		b->marked = 0;
	Prevb = &b0;
	for (b = b0.next; b != NULL; )
		b = reord1(b);
	putbr(Prevb);
	Prevb->lastn->forw = &ntail;
	ntail.back = Prevb->lastn; /* tack on tail node to text list */

	mkltbl(); /* make label table with only definitions */
	rmunrch(); /* remove unreachable code */
}

	static void
findlt() { /* find rotatable loops */

#define B_ABOVE	1
#define B_LOOP	2
#define B_EXIT	4
#define B_BEEN	8

	/*
	 * To identify the top of a rotatable loop:
	 * If there is only one reference, then it isn't the start of a loop.
	 * Mark all blocks down the fall-through path
	 * of the block being tested.
	 * If this reaches the block being tested, then there is a loop.
	 * Then go down the fall-through path derived from the
	 * conditionals of each of the branches in the loop.
	 * If these paths reach the loop being tested,
	 * its branch is a simple conditional-and-rejoin, not a loop.
	 * If there is more than one exit from the loop,
	 * rotate at the exit nearest to the top.
	 * Interior loops down all of these paths must be
	 * detected and exited, of course. This is done by next_left().
	 */

	extern BLOCK *next_left();
	register BLOCK *b, *bb, *bl;

	if (bflag)
		PRINTF("%cltests are:", CC);
	for (b = b0.next; b != NULL; b->marked = B_ABOVE, b = b->next) {
		if (b->indeg < 2) /* must have at least 2 paths in */
			continue;
		for (ALLB(bb, b)) /* remove old marks below b */
			bb->marked = 0;
		/* find next unmarked block, marking with B_LOOP as we go */
		for (bb = b->nextl;
		    bb != NULL && bb != b && !(bb->marked & ~B_BEEN); )
			bb = next_left(bb, B_LOOP);
		if (bb != b) /* not a loop */
			continue;
		b->marked = B_LOOP;
		for (ALLB(bl, b)) { /* look for loop termination test block */
			if (!(bl->marked & B_LOOP) || bl->nextr == NULL ||
			    !isrev(bl->lastn))
				continue; /* not a branch, or a dead-end */
			for (ALLB(bb, bl))
				bb->marked &= ~B_EXIT;
			for (bb = bl->nextr;
			    bb != NULL && !(bb->marked & (B_ABOVE | B_LOOP)); ) {
				register BLOCK *prevb = bb;
				if ((bb = next_left(bb, B_EXIT)) == prevb)
					break;
			}
			if (bb == NULL || (bb->marked & B_ABOVE)) {
				b->ltest = bl;
				if (bflag)
					PRINTF(" %d/%d", b->index, bl->index);
				break;
			}
		}
	}
	if (bflag)
		PUTCHAR('\n');
}

	static BLOCK *
next_left(b, mark) register BLOCK *b; register int mark; {

	/* find left successor of b, exiting from inner loops */

	register BLOCK *bb;

	for (ALLB(bb, b0.next)) /* clear our mark */
		bb->marked &= ~B_BEEN;
	b->marked = mark | B_BEEN; /* leave our mark */
	for (bb = b = b->nextl; b != NULL && (b->marked & mark); b = b->nextr) {
		if (b->nextr == b) /* inner loop */
			return (NULL);
		if (b->marked & B_BEEN) /* been here before */
			return (bb);
		b->marked |= B_BEEN; /* leave our mark */
	}
	return (b);
}

	static BLOCK *
reord1(b) register BLOCK *b; {

	extern BLOCK *nextbr();
	extern void prcase();
	register BLOCK *bl, *br, *blt;

	/* top of rotatable loop */
	/* don't rotate unless there already must be a branch to the entry */

	if (b->ltest != NULL && b != Prevb->nextl &&
	    (bl = b->ltest->nextl)->ltest == NULL && !bl->marked) {
		b->ltest = NULL;
		nrot++;
		return (bl);
	}

	/* mark block as processed and tie it in */

	b->marked++;
	if (b != Prevb->nextl)
		putbr(Prevb);
	Prevb->lastn->forw = b->firstn;
	b->firstn->back = Prevb->lastn;
	Prevb = b;

	/* dead-end block */

	if ((bl = b->nextl) == NULL) {
		PRCASE(0);
		return (nextbr(b));
	}

	bl->indeg--;
	if ((br = b->nextr) != NULL)
		br->indeg--;

	/* top of rotatable loop */

	if ((blt = bl->ltest) != NULL && blt->nextl->ltest == NULL &&
	    !blt->nextl->marked && !blt->nextr->marked && outdeg(blt) <= 1) {
		PRCASE(1);
		b = blt->nextl;
		bl->ltest = NULL;
		nrot++;
		return (b);
	}

	if (br == NULL) { /* unconditional branch or conditional to dead-end */

		if (!bl->marked) { /* to unprocessed block */

			if (bl->indeg <= 0) { /* with indeg 1 */
				PRCASE(2);
				return (bl);
			}

			/* branch to block with indeg > 1
			   that originally followed this one */

			if (bl == b->next) {
				PRCASE(3);
				return (bl);
			}

			/* branch to dead-end block */

			if (bl->nextl == NULL) {
				PRCASE(4);
				return (bl);
			}

		}

		/* all other unconditional branches */

		PRCASE(5);
		return (nextbr(b));
	}

	/* conditional branch to processed block */

	if (br->marked && !bl->marked) {

		/* fall through to unprocessed block with indeg = 1 */

		if (bl->indeg <= 0) {
			PRCASE(6);
			return (bl);
		}

		/* fall through to unprocessed block with indeg > 1
		   that originally followed this one */

		if (bl == b->next) {
			PRCASE(7);
			return (bl);
		}
	}

	/* reversible conditional branch to unprocessed block,
	   fall through to processed block */

	if (bl->marked && !br->marked && isrev(b->lastn)) {
		revbr(b->lastn);
		putp(b->lastn, label_left(b));
		b->nextr = b->nextl;
		b->nextl = br;
		PRCASE(8);
		return (br->indeg <= 0 ? br : nextbr(b));
	}

	/* all other conditional branches that have one leg or the
	   other going to processed blocks */

	if (bl->marked || br->marked) {
		PRCASE(9);
		return (nextbr(b));
	}

	/* fall through to block with indeg = 1
	   but not if it is an unlabeled unconditional transfer */

	if (bl->indeg <= 0 && !(isuncbr(bl->firstn) && isrev(b->lastn))) {
		PRCASE(10);
		return (bl);
	}

	/* reversible branch to block with indeg = 1 */

	if (br->indeg <= 0 && isrev(b->lastn)) {
		revbr(b->lastn);
		putp(b->lastn, label_left(b));
		b->nextr = b->nextl;
		b->nextl = br;
		PRCASE(11);
		return (br);
	}

	/* fall through to block with indeg > 1 that
	   originally followed this block */

	if (bl == b->next) {
		PRCASE(12);
		return (bl);
	}

	/* everything else */

	PRCASE(13);
	return (nextbr(b));
}

	static int
outdeg(b) register BLOCK *b; { /* compute indeg of loop exit target */

	register BLOCK *bb, *br = b->nextr;
	int indegree = br->indeg;

	for (ALLB(bb, b0.next))
		bb->marked &= ~B_EXIT;
	for (bb = b->nextl; bb != b && !(bb->marked & B_EXIT); ) {
		bb->marked |= B_EXIT;
		if (bb->nextr == br)
			indegree--; /* this is a loop exit */
		if ((bb = bb->nextl) == NULL)
			break;
		/* exit from inner loops */
		while ((bb->marked & B_EXIT) && (bb = bb->nextr) != NULL)
			if (bb->nextr == br)
				indegree--; /* this is a loop exit */
		if (bb == NULL)
			break;
	}
	for (ALLB(bb, b0.next))
		bb->marked &= ~B_EXIT;
	return (indegree);
}

	static void
prcase(n, b) int n; register BLOCK *b; { /* print information during reord */

	register NODE *p;

	PRINTF("%c%d\t%d\t%d\t%d", CC, b->index,
	    PRINDEX(nextl), PRINDEX(nextr), n);
	for (p = b->firstn; islabel(p); p = p->forw)
		PRINTF("\t%s", p->ops[0]);
	PRINTF("\n");
}

	static BLOCK *
nextbr(b) register BLOCK *b; { /* select next block to process */

	register BLOCK *bb;

	/* first look for orphan blocks (no more references) from the top */

	for (ALLB(bb, b0.next))
		if (!bb->marked && bb->indeg <= 0)
			return (bb);

	/* now look for unmarked block with live consequent (circularly) */

	for (bb = b->next; bb != b; bb = bb->next)
		if (bb == NULL) /* circular scan for next block */
			bb = &b0;
		else if (!bb->marked &&
		    bb->nextl != NULL && !bb->nextl->marked)
			return (bb);

	/* now look for any unmarked block (circularly) */

	for (bb = b->next; bb != b; bb = bb->next)
		if (bb == NULL) /* circular scan for next block */
			bb = &b0;
		else if (!bb->marked)
			return (bb);

	return (NULL); /* no more blocks to process */
}

	static void
putbr(b) register BLOCK *b; { /* append a branch to b->nextl onto b */

	register NODE *p, *pl = b->lastn;
	char *s;

	if (b == &b0 || isuncbr(pl))
		return;
	ndisc--;
	nsave--;
	b->length++;
	p = Saveop(0, (char *)NULL, 0, GHOST); /* make node but don't link */
	b->lastn = pl->forw = p; /* link at end of this block */
	p->back = pl;
	s = label_left(b); /* get destination label, in 2 steps */
	setbr(p, s); /* in case setbr is a macro which double-evaluates */
}

	static char *
label_left(b) register BLOCK *b; { /* get label of b->nextl */

	register NODE *pf, *p;
	register BLOCK *bl;

	if ((bl = b->nextl) == NULL)
		FATAL("label of nonexistent block requested\n");
	for ( ; ISUNCBL(bl); bl = bl->nextl)
		if (bl->nextl == NULL) {

			char *s = getp(bl->lastn);

			if (s == NULL) /* no target */
				break;
			b->nextl = NULL; /* dead-end */
			return (s);
		}
	b->nextl = bl; /* re-aim b at final target */
	pf = bl->firstn;
	if (islabel(pf) && !ishl(pf))
		return (pf->ops[0]);
	p = Saveop(0, newlab(), 0, GHOST); /* make node but don't link */
	p->forw = pf; /* link at beginning of this block */
	p->back = pf->back;
	if (bl->marked) /* this block already processed by reord */
		pf->back->forw = p;
	bl->firstn = pf->back = p;
	setlab(p);
	return (p->ops[0]);
}

	static void
rmunrch() { /* remove unreachable code */

	extern void reach();
	register REF *r;
	register BLOCK *b, *prevb;

	TRACE(rmunrch);

	if (b0.next == NULL)
		return;
	for (ALLB(b, b0.next))
		b->marked = 0;
	reach(b0.next); /* mark all blocks reachable from initial block */

	/* mark all blocks reachable from hard-label blocks */

	for (ALLB(b, b0.next))
		if (!b->marked && ishlp(b->firstn))
			reach(b);

	/* mark all blocks reachable from non-text references */

	for (r = r0.nextref; r != NULL; r = r->nextref)
		if ((b = FINDLBL(r->lab)) != NULL && !b->marked)
			reach(b);

	for (ALLB(b, b0.next)) /* remove unmarked blocks */
		if (b->marked)
			prevb = b;
		else {
			ndisc += b->length;
			if (ISUNCBL(b) && islabel(b->firstn))
				nsave++;
			else {
				if (uflag)
					PRINTF("%cunreachable block %d removed\n",
					    CC, b->index);
				nunr += b->length;
			}
			b->firstn->back->forw = b->lastn->forw;
			b->lastn->forw->back = b->firstn->back;
			prevb->next = b->next;
		}
}

	static void
reach(b) register BLOCK *b; { /* recursively mark reachable blocks */

	register BLOCK *bb;

	b->marked++;

	/* can't reach second of successive identical branches */

    	while ((bb = b->nextl) != NULL && !bb->marked && bb->length == 1 &&
	    bb->lastn->op == b->lastn->op && ISREMBR(bb->lastn))
		b->nextl = bb->nextl;
	if (bb != NULL && !bb->marked)
		reach(bb);
	if ((bb = b->nextr) != NULL && !bb->marked)
		reach(bb);
}

	static void
rmbrs() { /* remove redundant branches */

	extern void rmbr();
	register BLOCK *b, *bl;
	register NODE *p;

	TRACE(rmbrs);

	for (ALLB(b, b0.next))
		if ((bl = b->nextl) != NULL &&
		    (p = b->lastn)->forw == bl->firstn && ISREMBR(p)) {

			/* delete unconditional branch ahead of target */

			if (isuncbr(p)) {
				rmbr(p);
				continue;
			}

			/* delete conditional branch ahead of target
			   or ahead of unconditional branch to same target */

			do {
				if (b->nextr == bl || b->nextr == NULL &&
				    bl->nextl == NULL && ISUNCBL(bl) &&
				    sameaddr(p, bl->lastn)) {
					rmbr(p);
					break;
				}
			} while (ISUNCBL(bl) &&
				    (bl = bl->nextl) != NULL);
		}
}

	static void
rmbr(p) register NODE *p; { /* remove redundant branch */

	ndisc++;
	nsave++;
	DELNODE(p);
}

#ifdef LIVEDEAD
	void
ldanal() { /* perform live-dead register analysis (user-callable) */

	bldgr(0); /* update block structure but don't call bboptim */
	ld_anal(); /* do the analysis */
}

	static void
ld_anal() { /* perform live/dead register analysis */

	register BLOCK *b, *bb;
	register NODE *p;
	int live, dead;
	boolean changed;

	TRACE(ldanal);

	/* determine live/dead status for each block */

	for (ALLB(b, b0.next)) {
		b->blive = b->bdead = 0;
		for (p = b->lastn; !islabel(p); p = p->back) {
			p->nlive = (uses(p) | LIVEREGS) & REGS;
			p->ndead = (sets(p) & ~p->nlive) & REGS;
			b->blive = p->nlive | (b->blive & ~p->ndead);
			b->bdead = p->ndead | (b->bdead & ~p->nlive);
			if (p == b->firstn)
				break;
		}
	}

	/* propagate live/dead status throughout flow graph */

	do {
		changed = false;
		for (ALLB(b, b0.next)) {
			if ((b->blive | b->bdead) == REGS)
				continue;
			dead = 0;
			if (isret(b->lastn) && isuncbr(b->lastn))
				live = RETREG; /* unconditional return */
			else if ((bb = b->nextl) == NULL || /* dead-end */
			    /* switch-table transfer or case transfer */
			    b->nextr == NULL && isbr(b->lastn) &&
			    !isuncbr(b->lastn) && !isret(b->lastn))
				live = REGS; /* unknown, everything live */
			else { /* fall-through or branch or cond. return */
				live = bb->blive;
				dead = bb->bdead;
				if ((bb = b->nextr) != NULL) {
					live |= bb->blive;
					dead &= bb->bdead;
				}
				else if (isret(b->lastn)) {
					live |= RETREG;
					dead &= ~RETREG;
				}
			}
			live = (b->blive | (live & ~b->bdead)) & REGS;
			dead = (b->bdead | dead) & ~live;
			if (b->blive != live || b->bdead != dead) {
				b->blive = live;
				b->bdead = dead;
				changed = true;
			}
		}
	} while (changed == true);

	/* propagate live/dead status through each block */
	/* note that now anything not live is assumed dead */

	for (ALLB(b, b0.next)) {
		live = 0;
		if ((bb = b->nextl) != NULL) {
			live = bb->blive;
			if ((bb = b->nextr) != NULL)
				live |= bb->blive;
		}
		for (p = b->lastn; !islabel(p); p = p->back) {
			int newlive = p->nlive | (live & ~p->ndead);
			p->nlive = live;
			live = newlive;
			if (p == b->firstn)
				break;
		}
	}
}
#endif

#ifdef PEEPHOLE
static NODE *pf;	/* pointer to first window node */
static NODE *opf;	/* pointer to predecessor of first window node */
static int wsize;	/* window size for peephole trace */

window(size, func) register int size; boolean (*func)(); { /* peephole scan */

	extern NODE *initw();
	register NODE *pl;
	register int i;

	TRACE(window);

	/* find first window */

	wsize = size;
	if ((pl = initw(n0.forw)) == NULL)
		return;

	/* move window through code */

	for (opf = pf->back; ; opf = pf->back) {
		if ((*func)(pf, pl) == true) {
			if (wflag)
				if (opf->forw == pl->forw)
					PRINTF("%cdeleted\n", CC);
				else {
					PRINTF("%cchanged to:\n", CC);
					prwindow(opf->forw, size);
				}
			if (size > 1) {

				/* move window back in case
				   there is an overlapping improvement */

				for (i = 2; i <= size; i++)
					if ((opf = opf->back) == &n0) {
						opf = n0.forw;
						break;
					}
				if ((pl = initw(opf)) == NULL)
					return;
				continue;
			}
		}

		/* move window ahead */

		if ((pl = pl->forw) == &ntail)
			return;
		pf = pf->forw;
		if (islabel(pl) && (pl = initw(pl->forw)) == NULL)
			return;
	}
}

	static NODE *
initw(p) register NODE *p; { /* find first available window */

	register int i;

	if ((pf = p) == NULL)
		return (NULL);

	/* move p down until window is large enough */

	for (i = 1; i <= wsize; i++) {
		if (p == &ntail) /* no more windows */
			return (NULL);
		if (islabel(p)) { /* restart scan */
			pf = p->forw;
			i = 0;
		}
		p = p->forw;
	}
	return (p->back);
}

wchange() { /* print window before change */

	if (wflag) {
		PRINTF("%cwindow:\n", CC);
		prwindow(opf->forw, wsize);
	}
}

	static
prwindow(p, size) /* print "size" instructions starting at p */
	register NODE *p; register int size; {

	for ( ; --size >= 0 && p != &ntail && !islabel(p); p = p->forw) {
		PUTCHAR(CC);
#ifdef LIVEDEAD
		PRINTF("(live: 0x%X)", p->nlive);
#endif
		prinst(p);
	}
}
#endif

	static
mkltbl() { /* make label table with only definitions */

	register BLOCK *b;
	register NODE *p;

	clrltbl();

	/* add definitions from labels in text section */

	for (ALLB(b, b0.next))
		for (p = b->firstn; islabel(p); p = p->forw)
			ADDLBL(p->ops[0], b);
}

	static
clrltbl() { /* clear label table */

	(void) strncpy((char *)Lbltbl, "", N_LBLS * sizeof(LBL));
	Numlbls = 0;
}

	static BLOCK *
lblhash(l, b) char *l; BLOCK *b; { /* add or find label in label table */

	register LBL *p;
	register char *ll = l;
	register int lh = 0, c;

	while ((c = *ll++) != '\0')
		lh += c;
	while ((p = Lbltbl + lh % N_LBLS)->cp != NULL && strcmp(p->cp, l))
		lh += H_INCR;
	if (b != NULL) {
		if (++Numlbls >= N_LBLS)
			FATAL("too many labels\n");
		p->cp = l;
		p->bl = b;
	}
	return (p->bl);
}

	char *
getspace(n) register unsigned n; { /* return a pointer to "n" bytes */

	register char *p = Lasta;

	/* round up so pointers are always word-aligned */
	/* int conversions are to avoid call to software remaindering */

	n += sizeof(char *) - ((int) n % (int) sizeof(char *));
	Maxu += n;
	while ((Lasta += n) >= Lastx) {
		*Space = (struct space_t *) xalloc(sizeof(struct space_t));
		p = Lasta = (char *) &(*Space)->space[0];
		Lastx = (char *) &(*Space)->space[NSPACE - 1];
		(*Space)->next = NULL;
		Space = &(*Space)->next;
	}
	return (p);
}
