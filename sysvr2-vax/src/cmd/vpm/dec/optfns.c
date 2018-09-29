/*      optfns.c 1.2 of 12/19/82
 */
static char Sccsid[] = "@(#)optfns.c	1.2";
#include "pass0.h"
extern int previous();
extern struct stmt {
	char field[NFIELDS][MAXFSIZE];
	int nfields;
	struct label *labels;
	char prlabel[MAXFSIZE];
} stmt[];
extern int newest,oldest;
extern struct label {
	char *label;
	struct label *next;
};

/*
 * These optimization functions are named according to the number
 * of stmts they need before they are applicable.  A function returns the
 * number of stmts it deletes, or NOTRY if peephole does not have enough
 * statements for the function to be applicable.
 * Optimization functions are responsible for saving or throwing away the
 * labels of the statements they delete.
 */

opt1(new)		/* one-stmt optimization */
int new;
{
	if(have() < 1)
		return(NOTRY);	/* dont have enough stmts to try */
	return(0);		/* no 1-stmt opt */
}

opt2(new)		/* two-stmt optimization */
int new;
{
	int s1,s2;		/* s1 is the earlier stmt */

	if(have() < 2)
		return(NOTRY);	/* dont have enough stmts to try */

	s2 = new;
	s1 = previous(s2);

/* useless jump (jump to next stmt) */

	if(	EQ(stmt[s1].OPCODE,"JMP") &&
		(member(stmt[s1].OPERAND,stmt[s2].labels))
	) {
		stmt[s1].labels = (struct label *) NIL; /* flush old labels */
		copyst(s1,s2); 	/* overwrite old stmt */
		printf("/* useless JMP deleted */\n");
		return(1);
	}

/* redundant register load */
	
	if(	NOLABEL(s2) &&
		EQ(stmt[s2].OPCODE,"MOV") &&
		EQ(stmt[s2].OPERAND,stmt[s1].OPERAND) &&
		(	EQ(stmt[s1].OPCODE,"MOVM") ||
			EQ(stmt[s1].OPCODE,"INC"))
	) {
		printf("/* redundant MOV deleted */\n");
		return(1);
	}

/* unreachable stmt */

	if(	NOLABEL(s2) &&
		(EQ(stmt[s1].OPCODE,"JMP") ||
		EQ(stmt[s1].OPCODE,"RETURN") ||
		EQ(stmt[s1].OPCODE,"RETURNI") ||
		EQ(stmt[s1].OPCODE,"HALT") ||
		EQ(stmt[s1].OPCODE,"HALTI") )
	) {
		stmt[s2].labels = (struct label *) NIL; /* flush old labels */
		printf("/* unreachable code deleted */\n");
		return(1);
	}

/* unused accumulator load */

	if(	(EQ(stmt[s1].OPCODE,"MOV") ||
		 EQ(stmt[s1].OPCODE,"MOVI"))    &&
		(EQ(stmt[s2].OPCODE,"MOV") ||
		 EQ(stmt[s2].OPCODE,"MOVI"))
	) {
		copyst(s1,s2); /* overwrite first load */
		printf("/* unused MOV(I) deleted */\n");
		return(1);
	}

/* can't delete anything */

	return(NO);	/* nothing deleted */
}

opt3(new)		/* three-stmt optimization */
int new;
{
	int s1,s2,s3;

	if(have() < 3)
		return(NOTRY);	/* dont have enough stmts to try */

	s3 = new;
	s2 = previous(s3);
	s1 = previous(s2);

/*	 	JMPEQZ	label1	  	-->			JMPNEZ	label2
		JMP	label2 	  	-->		
	label1:			  	-->		label1:
		anything		-->			anything
*/

	if(	NOLABEL(s2) &&
		EQ(stmt[s1].OPCODE,"JMPEQZ") &&
		member(stmt[s1].OPERAND,stmt[s3].labels) &&
		EQ(stmt[s2].OPCODE,"JMP")
	) {
		strcpy(stmt[s1].OPCODE,"JMPNEZ");
		strcpy(stmt[s1].OPERAND,stmt[s2].OPERAND);
		copyst(s2,s3);
		printf("/* JMPEQZ - JMP changed to JMPNEZ */\n");
		return(1);
	}

	return(0);	/* nothing deleted */
}

int (*optfn[NOPTFNS])() = {		/* optimization function pointers */
	opt1,
	opt2,
	opt3
};


have()		/* how many stmts for optimization do we have? */
{
	if (oldest == NIL)
		return(0);
	else {
		int count,index;

		count = 1;
		index = oldest;
		while((index = nexst(index)) != nexst(newest))
			++count;
		return(count);
	}
}


member(label,labelist)		/* is label a member of labelist */
char *label;
struct label *labelist;
{
	while(labelist != (struct label *) NIL)
		if(EQ(label,labelist->label))
			return(YES);
		else
			labelist = labelist->next;

	return(NO);
}


copyst(dst,src)		/* copy stmt[src] to stmt[dst] - save dst labels */
int dst,src;
{
	int i,n;
	struct label *labcat();

	stmt[dst].labels = labcat(dst,src);	/* save old labels */
	strcpy(stmt[dst].prlabel,stmt[src].prlabel);
	stmt[src].labels = (struct label *) NIL;
	n = stmt[dst].nfields = stmt[src].nfields;
	for(i = 0; i < n; ++i)
		strcpy(stmt[dst].field[i],stmt[src].field[i]);
}

