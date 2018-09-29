	/*	@(#) tbls.c: 1.3 5/17/83	*/

#include "dis.h"

struct optab optab[] = {
#define OP(a,b,c,d,e,f,g,h,i) {a,b,c,d,e,f,g,h,i}
{0},	/* 0'th entry is used as invalid opcode */
#include "instrs.h"
{0}	/* last entry is used as sentinel (termination marker) */
};
