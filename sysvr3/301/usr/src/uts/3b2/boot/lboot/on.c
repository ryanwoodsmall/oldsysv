/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/on.c	10.2"

/*
 * These routines implement a PL/I style ON condition handling.  All conditions
 * must be declared by:
 *
 *		CONDITION name ={ "name" };
 *
 * A condition is enabled by the use of the ON() macro.  Usage is:
 *
 *		ON(condition) { ... }
 *
 * A condition is disabled by the use of the REVERT() macro.  Usage is:
 *
 *		REVERT(condition);
 *
 * A condition is signalled by the use of the SIGNAL() macro.  Usage is:
 *
 *		SIGNAL(condition);
 *
 * A routine creates a condition handler for a specific condition by using the
 * ON() macro.  The condition handler remains active until either it is entered
 * as a result of a SIGNAL() macro, or until the routine exits to its caller.
 * When invoked, the condition handler is executing with the environment which
 * existed at the time the ON() macro was issued; these routines use the setjmp()
 * and longjmp() routines, therefore all of the same considerations apply (in
 * particular, register variables will have the values that existed at the time
 * the ON() macro was used -- not the values that exist when the condition is
 * signalled).  Thus, unless the condition handler re-enables itself, a further
 * signal of that condition will result in a lower level handler being invoked.
 * Examples of re-enabling and not re-enabling follow:
 *
 *      RE-ENABLE                               NO RE-ENABLE
 *      ---------                               ------------
 *
 *      CONDITION X ={ "X" };                   CONDITION X ={ "X" };
 *      rtn() {                               rtn() {
 *              ON(X) goto recover;                     ON(X) {
 *      restart: ...                                            ...
 *              return;                                         return;
 *      recover: ...                                            }
 *              ON(X) goto recover;                     ...
 *              goto restart;                           }
 *              }
 *
 * When a condition is signalled, a search is made for the most recent routine
 * (searching backwards down the stack frames) which has issued the ON() macro.
 * If no ON() macro was issued for the condition signalled, then the condition
 * ERROR is used instead (if no ON(ERROR) was ever issued, panic() is called).
 * When the condition handler is found, control is passed to the handler routine.
 * There can be no return from a condition handler back to the point at which it
 * was signalled.
 *

 *
 * The data structures used to implement PL/I style ON-CONDITIONS are organized
 * in the following fashion:
 *
 *      +---+
 *      |   |
 *      | E |       on_block_anchor ---+
 *      |   |                          |
 *      +---+            +-------------+
 *      |   |            V                       +-----------+
 *      |   | /--> +----------+ ---------------> | on_enable | ----------------------------+
 *      | D |/     | on_block |                  +-----------+                             |
 *      |   |      +----------+                                                            |
 *      |   |          A  |                                                                |
 *      +---+          |  |                                                                |
 *      |   |          |  |                                                                |
 *      | C |          |  |                                                                |
 *      +---+          |  |                                                                |
 *      |   |          |  V                      +-----------+                             |
 *      |   | /--> +----------+ ---------------> | on_enable | ---+                        |
 *      | B |/     | on_block |                  +-----------+    |                        |
 *      |   |      +----------+                                   |                        |
 *      +---+          A  |                                       |                        |
 *      |   |          |  V                      +-----------+    |     +-----------+      |
 *      |   | /--> +----------+ ---------------> | on_enable | ---|---> | on_enable |      |
 *      | A |/     | on_block |                  +-----------+    |     +-----------+      |
 *      |   |      +----------+                        |          |           |            |
 *      +---+                                          |          |           |            |
 *  STACK FRAMES                                       |          |           |            V
 *                                                     |          |           |    +--------------+
 *                                                     |          |           +--> | on_condition |
 *                                                     |          |                +--------------+
 *                                                     |          |
 *                                                     |          |                +--------------+
 *                                                     +----------+--------------> | on_condition |
 *                                                                                 +--------------+
 *
 *
 * On_blocks and on_enable blocks are built by using the ON(condition) macro.
 * Once an on_block has been created for a given routine, it will remain until
 * the routine exits to its caller.  Each CONDITION enabled is represented by
 * an on_enable block chained to the on_block.  Each on_enable block points to
 * the CONDITION which it represents.
 *
 * On_blocks are pointed to by the return address on the stack frame.  When the
 * routine exits, the RET instruction will cause a branch to the on_block.  A
 * "JSB _on_exit" instruction is the first item in the on_block.  The on_exit
 * routine will remove the current on_block and on_enable blocks before returning
 * to the original caller of the routine.
 *
 * When the SIGNAL(condition) macro is used, a search is made for the most recent
 * on_block which points to an on_enable block for the given condition.  If none
 * is found, then the ERROR condition is substituted.  When found, all intervening
 * on_blocks and their associated on_enable blocks are removed before branching
 * to the block for the ON(condition) which is being signalled.
 */

#include <sys/types.h>
#include <sys/dir.h>
#include "lboot.h"

/*
 * ON enable block
 */
struct	on_enable
	{
	struct on_enable	*next;
	boolean			active;		/* set when the condition is enabled */
	CONDITION		*condition;	/* the CONDITION which is enabled */
	label_t			jmpbuf;		/* status when ON(cond) was issued */
	};

/*
 * ON block
 */
struct	on_block
	{
	union	on_id				/* first element in on block */
		{
		long	magic;			/*	identifies this as an on_block */
		char	jsb[1+1+4];		/*	"JSB _on_exit" instruction */
		}
				on_id;
	struct on_block		*next;
	struct on_block		*prior;
	struct on_enable	*enable;
	paddr_t			pc;		/* original return address */
	};

extern union on_id JSBONEXIT;		/* actual "JSB _on_exit" instruction */

/*
 * Anchor for on_block chain; always points to most recent on_block
 */
static struct on_block *on_block_anchor = NULL;


/*
 * Global ERROR condition
 */
CONDITION ERROR ={ "ERROR" };


/*
 * Indexes relative to the current frame pointer for stored registers on
 * the stack frame
 */
#define	R8	-1
#define	R7	-2
#define	R6	-3
#define	R5	-4
#define	R4	-5
#define	R3	-6
#define	FP	-7
#define	AP	-8
#define	PC	-9	/* return address */

/*
 * On(condition)
 *
 * Enable the specified ON condition.  Allocate the on_block (if not alreacy
 * done so) and create/overlay the appropriate on_enable block.  The address
 * of the label_t buffer is returned for use by the setjmp() routine.
 */
 int *
on(condition)
	CONDITION *condition;
	{PROGRAM(on)

	register struct on_block *on_b, *p;
	register struct on_enable *on_e;
	register paddr_t **frame;

	/*
	 * MACHINE DEPENDENT: since we know we were called with one argument,
	 * then the address of the argument + 10 is the current frame pointer
	 * for this routine.  Therefore, the saved registers can be accessed
	 * via this address.
	 */
	frame = ((paddr_t **) &condition) + 10;

	/*
	 * if this is the first time on() was called for this routine, then an
	 * on_block must be allocated
	 */

	on_b = (struct on_block *)(frame[FP])[PC];

	if ((long)on_b & 0x3 || on_b->on_id.magic != JSBONEXIT.magic)
		/*
		 * MACHINE DEPENDENT: we assume that there will never be a
		 * "JSB _on_exit" instruction following a CALL to any routine
		 * which calls on() -- therefore the return address in the
		 * callers stack frame can be replaced with the address of
		 * the on_block, and the original return address saved in
		 * the on_block.
		 *
		 * When the caller finally returns, then the "JSB _on_exit"
		 * instruction in the beginning of the on_block structure
		 * will be executed and the "_on_exit" assembler routine will
		 * receive control.  The on_block and any associated on_enable
		 * blocks can be removed.  Finally, return is made to the
		 * original return address.
		 */
		{

		if ((p=(struct on_block*)((frame[FP])[PC]=(paddr_t)malloc(sizeof(*p)))) == NULL)
			panic("no memory for on_block");

		p->on_id = JSBONEXIT;
		p->pc = (paddr_t) on_b;		/* save original return address */
		p->enable = NULL;
		on_b = p;

		/*
		 * chain the new on_block at the head of the on_block chain
		 */
		if ((p=on_block_anchor) != NULL)
			p->prior = on_b;

		on_b->next = p;
		on_b->prior = NULL;

		on_block_anchor = on_b;
		}

	/*
	 * if there already is an on_enable block for this condition, then it
	 * will be overlayed; otherwise, a new on_enable block is obtained
	 */

	for (on_e=on_b->enable; on_e != NULL; on_e=on_e->next)
		if (on_e->condition == condition)
			break;

	if (on_e == NULL)
		{
		if ((on_e=(struct on_enable*)malloc(sizeof(*on_e))) == NULL)
			panic("no memory for on_enable");

		on_e->condition = condition;
		on_e->next = on_b->enable;
		on_b->enable = on_e;
		}

	on_e->active = TRUE;

	return(on_e->jmpbuf);
	}

/*
 * Revert(condition)
 *
 * Disable the specified ON condition.  Just mark the on_enable block as
 * inactive.  It will be freed when the routine finally exits.
 */
 void
revert(condition)
	CONDITION *condition;
	{PROGRAM(off)

	register struct on_block *on_b;
	register struct on_enable *on_e;
	register paddr_t **frame;

	/*
	 * MACHINE DEPENDENT: since we know we were called with one argument,
	 * then the address of the argument + 10 is the current frame pointer
	 * for this routine.  Therefore, the saved registers can be accessed
	 * via this address.
	 */
	frame = ((paddr_t **) &condition) + 10;

	/*
	 * if there is no on_block for this routine, then there is no on_enable
	 * to disable
	 */

	on_b = (struct on_block *)(frame[FP])[PC];

	if ((long)on_b & 0x3 || on_b->on_id.magic != JSBONEXIT.magic)
		return;

	/*
	 * if there is no existing on_enable block for this condition, then
	 * there is nothing to disable
	 */

	for (on_e=on_b->enable; on_e != NULL; on_e=on_e->next)
		if (on_e->condition == condition)
			break;

	if (on_e != NULL)
		on_e->active = FALSE;
	}

/*
 * Signal(condition)
 *
 * The "condition" has occurred.  Find the most recent on_block which contains
 * an enabled on_enable block for the condition.  Return to the condition handler.
 */
 void
signal(condition)
	CONDITION *condition;
	{PROGRAM(signal)

	register struct on_block *on_b, *b, *tb;
	register struct on_enable *on_e, *e, *te;
	CONDITION *p;
#if DEBUG1
	paddr_t **frame;
	extern int etext[], edata[];

	/*
	 * MACHINE DEPENDENT: since we know we were called with one argument,
	 * then the address of the argument + 10 is the current frame pointer
	 * for this routine.  Therefore, the saved registers can be accessed
	 * via this address.
	 */
	frame = ((paddr_t **) &condition) + 10;

	if (prt[_ON])
		{
		char *name;

		name = (char*) (frame[FP])[0];

		if (name >= (char*)etext && name < (char*)edata)
			printf("'%s' condition raised at location 0x%lX in routine %s\n", condition->name, frame[PC], name);
		else
			printf("'%s' condition raised at location 0x%lX in unknown routine\n", condition->name, frame[PC]);
		}
#endif

	/*
	 * find the most recent on_block with an on_enable block for this
	 * condition; if none is found, then switch to the ERROR condition
	 */
	for (p=condition; ; p=(&ERROR))
		{
		for (on_b=on_block_anchor; on_b; on_b=on_b->next)
			for (te=(struct on_enable*)&on_b->enable, on_e=te->next; on_e; te=on_e, on_e=te->next)
				if (on_e->condition == p && on_e->active)
					goto out;

		if (p == &ERROR)
			panic("ON condition not enabled");
		}

out:	on_block_anchor = on_b;

	/*
	 * free all on_blocks and associated on_enable blocks more recent than
	 * this one
	 */

	b = on_b->prior;

	while ((tb=b) != NULL)
		{
		e = b->enable;

		while ((te=e) != NULL)
			{
			e = e->next;
			free((char*)te);
			}

		b = b->prior;
		free((char*)tb);
		}

	on_b->prior = NULL;

	/*
	 * disable the current on_enable block
	 */
	on_e->active = FALSE;

	/*
	 * return to the condition handler
	 */

#if DEBUG1
	if (prt[_ON])
		{
		char *name;

		name = ((char**)on_e->jmpbuf[9])[0];

		if (name >= (char*)etext && name < (char*)edata)
			printf("'%s' condition handler entered in routine %s\n", on_e->condition->name, name);
		else
			printf("'%s' condition handler entered at location 0x%lX\n", on_e->condition->name, on_e->jmpbuf[7]);
		}
#endif

	longjmp(on_e->jmpbuf);
	}

/*
 * On_exit()
 *
 * A routine which had condition handlers activated has just attempted to
 * return.  The return was intercepted by the "JSB _on_exit" instruction in
 * the on_block of the routine.  This on_block and its associated on_enable
 * blocks must be freed before return is made to the original return address.
 */
 paddr_t
on_exit()
	{PROGRAM(on_exit)

	register struct on_block *on_b;
	register struct on_enable *e, *te;
	paddr_t pc;

	on_b = on_block_anchor;

	/*
	 * free all on_enable blocks for this on_block
	 */
	e = on_b->enable;

	while ((te=e) != NULL)
		{
		e = e->next;

		free((char*)te);
		}

	/*
	 * reset the anchor for the on_block chain
	 */
	if ((on_block_anchor=on_b->next) != NULL)
		on_block_anchor->prior = NULL;

	/*
	 * free the on_block and return the original return address
	 */
	pc = on_b->pc;

	free((char*)on_b);

	return(pc);
	}
