/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stincc:m32/stasg.c	1.1"

#include "mfile2.h"

/* Structure assignment and arguments.
**
** The idea here is to do a real good job of moving words
** from one place to another.  We assume that we are handed
** operands that are pointers to the left (and right, if
** assignment) side.  The shapes matched are those things we
** can add offsets to:  REG, (+ REG ICON), &STK, ICON (named).
** Other stuff gets resolved into a REG.
**
** The obvious approach is to try to get the "from" and "to"
** pointers into registers at run-time, then do a block-move.
** A slightly more elegant solution is to do specific moves
** in place of the block move when the number of words to be
** moved is small.
**
** This approach requires these steps:
**	1)  Determine whether the move is above threshold.
**	2a) For offset moving, generate the moves.
**	2b) For block move:  move operands to registers,
**		then do the move.
*/

static void dooff();
static void movaw();

/* register number corresponding to macro An */
#define	A(n)	(resc[n-1].tn.rval)
/* print-name for that register */
#define	RNAMES(n) (rnames[scratch[n]])

/* Number of words to move before block move pays off */
#define	THRESH	4


/* macro returning TRUE if node is a scratch register */

#define	istnode(p) (p->in.op == REG && istreg(p->tn.rval))


/* We need to find the scratch registers to use for from, to, and count
** in the face of sharing.  Use this array to keep track of what's what.
*/

static int scratch[NRGS];
/* These defines give indexes in "sharing". */
#define	R_TO	0	/* "to" */
#define	R_FROM	1	/* "from" */
#define	R_COUNT	2	/* word count */

/* This routine figures out the assigning of registers to scratch
** registers, accounting for sharing.
*/
static void
getscratch(node, share)
NODE * node;
int share;				/* one of LSHARE or RSHARE */
{
    NODE * sharenode;
    /* assume simple case:  no sharing occurs */

    scratch[R_TO] = A(1);
    scratch[R_FROM] = A(2);
    scratch[R_COUNT] = A(3);

    sharenode = (share == LSHARE) ? node->in.left : node->in.right;

    if (istnode(sharenode)) {		/* possibly a shared node */
	int sharereg = sharenode->tn.rval;
	int i;

	for (i = 0; i < NRGS; ++i)
	    if (scratch[i] == sharereg)
		break;
	
	/* If we are, in fact, sharing a register, make sure it's the operand
	** we want.  If not, interchange with the "wrong" one.
	*/
	if (i <= R_COUNT) {

	    if (share == LSHARE && i != R_TO) {
		scratch[i] = scratch[R_TO];
		scratch[R_TO] = sharereg;
	    }
	    else {			/* RSHARE */
		scratch[i] = scratch[R_FROM];
		scratch[R_FROM] = sharereg;
	    }
	}
    }
    return;
}


/* Generate code to do structure copy for assignment.  The node must be an
** STASG node.  The left and right sides are assumed to be recognizable
** addressing shapes as described above.  Assume that 3, so there are
** three scratch registers available.  Further assume that $> was set,
** which means the right side could be one of the scratch registers.
*/

void
stasg(node, flag, q)
NODE * node;
int flag;			/* non-zero if left side pointer retained */
OPTAB * q;
{
    NODE * left = node->in.left;
    NODE * right = node->in.right;

    int size = node->stn.stsize/SZINT;	/* number of WORDS to copy */

    /* Do the block move if the move is "too long". */

    if (size <= THRESH) {
	/* generate direct moves.  On 3b20 use movd for pairs, movw for last */
	int i;
	for (i = 0; i < size; i++ ) {
	    printf("	movw	");
	    dooff(right, i*4);		/* generate "from" offset */
	    putchar(',');
	    dooff(left, i*4);		/* "to" offset */
	    putchar('\n');
	}
    }
    else {
	int lastlab;
	getscratch(node, RSHARE);	/* assume $> set in template */

	/* Handle right side first, since it could share */
	movaw(right, R_FROM);

	/* Move the left side to a temp reg unless it's in one already and
	** we don't need the left side later.
	*/
	if ( !istnode(left))		/* Register variable*/
	    movaw(left, R_TO);
	else if (flag)			/* save the destination*/
	    save_dest(left);
	else
	    scratch[R_TO] = left->tn.rval; /* allow this scratch reg to share */
	
	/* get count into register */
	printf("	movw	&%d,%s\n", size, RNAMES(R_COUNT));
	/* now do the block move */
	lastlab = getlab();
	printf(".L%d:\tmovw\t0(%s),0(%s)\n",lastlab, RNAMES(R_FROM),
	RNAMES(R_TO));
	printf("\taddw2\t&4,%s\n\taddw2\t&4,%s\n",RNAMES(R_FROM),
	RNAMES(R_TO));
	printf("\tsubw2\t&1,%s\n\tjpos\t.L%d\n", RNAMES(R_COUNT), lastlab);

	if (flag)
	    restore_dest(left);
    }
    return;
}


/* Structure argument.  This is similar to structure assignment.  The left
** side of the implied assignment is the stack.
*/

void
starg(node, q)
NODE * node;
OPTAB * q;
{
    NODE * left = node->in.left;
    int size = node->stn.stsize/SZINT;	/* words to move */

    /* Same thing here as with assignment: if too many words, use block move. */

    if (size <= THRESH) {
	int i;

	for (i = 0; i < size; i++ ) {
	    printf("	pushw	");
	    dooff(left, i*4);
	    putchar('\n');
	}
    }
    else {
	int lastlab;
	getscratch(node, LSHARE);	/* assume $> set in template */


	/* Move the left side to a temp reg unless it's in one already and
	** we don't need the left side later.
	*/
        movaw(left, R_FROM);
	
	/*Prepare destination register*/
	printf("\tmovw\t%%sp,%s\n", RNAMES(R_TO));
	/*Leave enough room on the stack for the arg*/
	printf("\taddw2\t&%d,%%sp\n", size*4);

	/* get count into register */
	printf("	movw	&%d,%s\n", size, RNAMES(R_COUNT));
	/* now do the block move */
	lastlab = getlab();
	printf(".L%d:\tmovw\t0(%s),0(%s)\n",lastlab, RNAMES(R_FROM),
	RNAMES(R_TO));
	printf("\taddw2\t&4,%s\n\taddw2\t&4,%s\n",RNAMES(R_FROM),
	RNAMES(R_TO));
	printf("\tsubw2\t&1,%s\n\tjpos\t.L%d\n", RNAMES(R_COUNT), lastlab);

    }
    return;
}


/* print address offset by some number */

static void
dooff(node, offset)
NODE * node;
int offset;
{
    NODE dummy;				/* to call adrput() with */

    dummy.in.op = STAR;
    dummy.in.left = node;

    switch( node->in.op ) {
    case ICON:
	node->in.op = NAME;		/* make look like external temporarily */
	goto stack;
    case UNARY AND:			/* &STK */
	node = node->in.left;
stack:
	/* fake different offset from name */
	node->tn.lval += offset;
	adrput(node);
	node->tn.lval -= offset;
	if (node->in.op == NAME)
	    node->in.op = ICON;		/* fix up OP again */
	break;
    
    /* *(&(R+/-C)) ==> R +/- C */
    case MINUS:
	node->in.right->tn.lval -= offset;
	adrput(&dummy);
	node->in.right->tn.lval += offset;
	break;
    case PLUS:
	node->in.right->tn.lval += offset;
	adrput(&dummy);
	node->in.right->tn.lval -= offset;
	break;

    case REG:
	/* register offset */
	printf("%d(%s)", offset, rnames[node->tn.rval]);
	break;

    default:
	cerror("screwy dooff() case");
    }

    return;
}

/* Generate move to get operand (address) into a register.
** Since addw3 is faster than movaw, generate the former.
*/

static void
movaw(node, regno)
NODE * node;				/* "from" operand */
int regno;				/* scratch reg. #:  R_TO - R_COUNT */
{
    char * regstring = (char *) 0;
    char * offname = (char *) 0;
    int offsign = 0;			/* offset sign:  1 means negate */
    long offset = 0;

    switch( node->in.op ) {
    case MINUS:
	offsign = 1;
	/*FALLTHRU*/
    case PLUS:
	regstring = rnames[node->in.left->tn.rval];
	node = node->in.right;
	/*FALLTHRU*/
    case ICON:
	offname = node->tn.name;
	offset = node->tn.lval;
	if (offsign) offset = -offset;	/* negate offset if necessary */
	break;

    case REG:
	/* for REG, do direct move if different register number */
	if (node->tn.rval == scratch[regno])
	    return;
	regstring = rnames[node->tn.rval];
	break;

    
    case UNARY AND:			/* &STK */
	node = node->in.left;
	switch( node->tn.op ){
	case VAUTO:	regstring = "%fp"; break;
	case VPARAM:	regstring = "%ap"; break;
	default:	cerror("confused movaw()");
	}
	offset = node->tn.lval;
	break;

    default:
	cerror("confused movaw()");
    }

    /* 		4	2	1	*/
    /*	    offname  offset  regstring	*/
    switch( (offname ? 4 : 0) + (offset ? 2 : 0) + (regstring ? 1 : 0)) {
    case 0+0+0:	cerror("movaw() case 0?");
    case 0+0+1: printf("	movw	%s", regstring); break;
    case 0+2+0: cerror("movaw() case 2?");
    case 0+2+1: if (offset >= 0)
		    printf("	addw3	&%d,%s", offset, regstring);
		else
		    printf("	subw3	&%d,%s", -offset, regstring);
		break;
    case 4+0+0: printf("	movw	&%s", offname); break;
    case 4+0+1: printf("	addw3	&%s,%s", offname, regstring); break;
    case 4+2+0: printf("	movw	&%s%s%d",
		    offname, (offset > 0 ? "+" : ""), offset); break;
    case 4+2+1: printf("	addw3	&%s%s%d,%s",
		    offname, (offset > 0 ? "+" : ""), offset, regstring); break;
    }
    printf(",%s\n", RNAMES(regno));
    return;
}

save_dest(p)
NODE *p;
{
			/*Save the destination register*/
	printf("	pushw	%s\n", rnames[p->tn.rval]);
        scratch[R_TO] = p->tn.rval; /* allow this scratch reg to share */
}

restore_dest(p)
NODE *p;
{
			/*Save the destination register*/
	printf("	POPW	%s\n", rnames[p->tn.rval]);
}

