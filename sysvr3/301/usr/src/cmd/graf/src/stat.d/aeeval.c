/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/aeeval.c	1.4"
#include "s.h"
#include "stdio.h"
#include "ae.h"
#define BINOP(op) pushd(&sp,sp+MAXS,popd(&sp,s) op popd(&sp,s))
#define PUSH(val) pushd(&sp,sp+MAXS,val)
#define POP popd(&sp,s)
#define MAXS 50

double aeeval(pfix,n,i,col) /* evalute postfix expression */
struct p_entry *pfix;
struct operand n[];
int *i, col;
{
	double s[MAXS], *sp=s-1, *sp2;
	double x, y, fmod(), pow1(), popd();

	for( ; pfix->opr!=0; pfix++ ) {
		switch(pfix->opr) {
		case OPN: PUSH(pfix->opn->val); break;
		case '+': BINOP(+); break;
		case '-': y = POP; x = POP; PUSH(x-y); break;
		case '*': BINOP(*); break;
		case '|': y = POP; x = POP;
			if( y!=0 ) PUSH(x/y);
			else if( x==0 ) PUSH(0);
			else if( x>0 ) {
				PUSH(POSINFIN);
				ERRPR1(division by zero\, %g output,POSINFIN);
			} else {
				PUSH(NEGINFIN);
				ERRPR1(division by zero\, %g output,NEGINFIN);
			}; break;
		case '%': y = POP; x = POP;
			if( y!=0 ) PUSH(fmod(x,y));
			else {	PUSH(0);
				ERRPR0(modulo zero yeilds zero);
			}; break;
		case '^': y = POP; x = POP;
			PUSH(pow1(x,y)); break;
		case '~': PUSH(-POP); break;
		case ',': PRINT(*s,(*i)++,col);
			for(sp2=s+1; sp2<=sp; sp2++)
				sp2[-1] = sp2[0];
				POP; break;
		default: ERRPR0(bad token in postfix string);
		}
	}
	return(POP);
}

pushd(sp,limit,val)
double **sp, *limit, val;
{
	if( *sp<limit-1 ) { *++*sp = val; return(1); }
	else { ERRPR0(stack overflow\, too many operands); return(0); }
}

double popd(sp,limit)
double **sp, *limit;
{
	if( *sp>=limit ) return(*(*sp)--);
	else { ERRPR0(stack underflow\, missing operand likely); return(0); }
}
