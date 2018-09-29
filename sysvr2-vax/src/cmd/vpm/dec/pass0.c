/*      pass0.c 1.3 of 12/4/82
 */
static char Sccsid[] = "@(#)pass0.c	1.3";
/*
 *	pass0.c - vpmc optimizer
 *
 *	This program takes as input the output of pl, the main
 *	pass of the vpm compiler.  pass0.c writes to standard output
 * 	the input minus whatever stmts it can delete.
 *	pass0.c takes as a command-line argument the name of the
 * 	file to which it is to write C-preprocessor #define statements
 * 	which redefine some labels.
 */

#include <stdio.h>

#include "pass0.h"

extern int (*optfn[])(); /* pointers to optimization functions.  */

struct stmt {	/* each stmt struct holds info about one stmt */
	char field[NFIELDS][MAXFSIZE]; /* contents of fields in this stmt */
	int nfields;		/* no. fields actually in this stmt */
	struct label *labels;	/* labels of this stmt */
	char prlabel[MAXFSIZE];	/* single label to be printed */
} stmt[NSTMTS];
int 	newest = NIL,		/* index of latest stmt read */
	oldest = NIL;		/* index of earliest stmt read */
int	total = 0;		/* total no. stmts in original program */

struct label {
	char *label;
	struct label *next;
};


FILE	*fopen(),*fp;

main(argc,argv)
int argc;
char *argv[];
{
	int n;

	if((fp = fopen(argv[1],"w")) == NULL) {
		fprintf(stderr,"pass0: Can't open intermediate file.  Goodbye!\n");
		exit(2);
	}
	for(n = 0; n < NSTMTS; ++n)
		stmt[n].labels = (struct label *) NIL;
	n = opt();
	flush();
	printf("/* %d statements, %d deleted */\n",total,n);
	fclose(fp);
}

flush()		/* cant get rid of anything else - flush what's left */
{
	if(oldest != NIL) {
		do {
			prstmt(oldest);
			oldest = nexst(oldest);
		} while (oldest != nexst(newest));
	}
	newest = oldest = NIL;
}


readstmt()		/* read a stmt - make room if necessary */
{
	char line[MAXLINLEN], *labelof();
	int index;

	if(!ROOM)
		outold();	/* make room */
	index = nexst(newest);

	for(;;) {		/* look for an instruction */
		if(gets(line) == NULL)
			return(END);
		if(*line == '\0')
			continue;

		/* got an input line - parse it */
	
		if(parseline(index,line) == LABEL) { /* save label */
			struct label *lp, *nlp;
			char *savestring();

			lp = stmt[index].labels;
			nlp = ((struct label *) calloc(1,sizeof(struct label)));
			nlp->label = savestring(stmt[index].field[0]);
			stmt[index].labels = nlp;
			nlp->next = lp;
		} else
			break;		/* got instr */
	}
	
	/* got an instruction */
	++total;
	newest = index;
	if(oldest == NIL)
		oldest = index;
/*
printf("readstmt: stmt %d: \n",index);
prstmt(index);
*/
	labelof(index);
	return(YES);
}

parseline(index,line)		/* break up line into stmt[index] */
int index;
char *line;
{
	char *lp;
	int fields,len,type;

	lp = line;
	fields = 0;
	while(*lp != NULLCH) {
		while(white(*lp))
			++lp;	/* skip white space */

		len = 0;	/* length of this token */
		while((*lp != NULLCH) && !white(*lp))
			stmt[index].field[fields][len++] = *lp++;
		if(len > 0) {	/* got a new token */
			if(stmt[index].field[fields][len-1] == COLON) {
				type = LABEL;
				--len;
			} else 
				type = STMT;
			stmt[index].field[fields][len] = NULLCH;
			++fields;
		}
	}
	stmt[index].nfields = fields;
	return(type);
}


opt()			/* main optimization loop */
{
	int n;		/* how many stmts thrown away */

	n = 0;
	while(1) {
		int i;

		for(i = 0; i < NOPTFNS; ++i) {
			int r,j;

			if((r = (*optfn[i])(newest)) >= YES) {
				n += r;
				for(j = 0; j < r; ++j)
					newest = previous(newest);
				break; /* restart main opt loop */
			} else if(r == NOTRY) {
				if(readstmt() == END)
					return(n);
				break;
			} else {	/* r == NO */
				if(i == (NOPTFNS-1)) { /* nothing worked */
					outold();
					if(readstmt() == END)
						return(n);
				}
			}
		}
	}
}

/*
 * Return index of previous active stmt - NIL if none.
 * (If NIL, save labels of most recent stmt in stmt[0].)
 */
previous(oldindex)
int oldindex;
{
	if(newest==oldest) {	/* nothing before this */
		stmt[0].labels = stmt[oldindex].labels;
		return(NIL);
	} else if (oldindex > 0)
		return(oldindex-1);
	else 	/* zero - wrap around to end of array */
		return(NSTMTS-1);
}


prstmt(num)	/* print statement number num */
int num;
{
	int f;

	if(stmt[num].prlabel[0] != (char) NIL) /* print label if there is one */
		printf("%s:\n",stmt[num].prlabel);
/* debug: print all labels:
{ struct label *tp;
tp = stmt[num].labels;
while(tp != (struct label *) NIL){
	printf("%s:\n",tp->label);
	tp = tp->next;
}
}*/

	if(stmt[num].nfields != 3)
		putchar('\t');	/* dont tab for BSS lines */
	printf("%s",stmt[num].field[0]);

	for(f=1; f < stmt[num].nfields; ++f)
		printf("\t%s",stmt[num].field[f]);

	putchar('\n');
}

/*
 * Fill in the one label stmt[index] is to have - NIL if none.
 * (JMP stmt gets no label.)
 * Write appropriate #defines to sas_tempb:
 * JMP dstlab --> #define all labels to dstlab, stmt gets no label
 * (Watch out for jump to self.)
 * other stmts --> #define all labels to first label, which labels the stmt
 * Old labels are kept around for future optimization.
 */

char *
labelof(index)
int index;
{
	char 	*defnlab;	/* label to be used in #defines */
	struct label *lp;

	if((lp = stmt[index].labels) == (struct label *) NIL) {
		stmt[index].prlabel[0] = (char) NIL;
		return((char *) NIL);
	}

	if(EQ(stmt[index].OPCODE,"JMP")) {
		defnlab = stmt[index].OPERAND;
		if(member(stmt[index].OPERAND,stmt[index].labels)) /* jump to self */
			strcpy(stmt[index].prlabel,stmt[index].OPERAND);
		else
			stmt[index].prlabel[0] = (char) NIL;
	} else {
		defnlab = lp->label;
		strcpy(stmt[index].prlabel,lp->label);
		lp = lp->next;
	}

	while(lp != (struct label *) NIL) {	/* write #defines */
		if(!EQ(lp->label,defnlab)) /* write only if not jump to self */
			fprintf(fp,"#define %s %s \n",lp->label,defnlab);
		lp = lp->next;
	}

	return(stmt[index].prlabel);
}


/*
 *	Catenate labels of stmt[src] to labels of stmt[dst].
 *	Return pointer to list of dst labels.
 */
struct label *
labcat(dst,src)
int dst,src;
{
	struct label *lp, **lpp;

	for(lpp = &(stmt[dst].labels); (lp = *lpp) != (struct label *) NIL; lpp = &(lp->next))
		;	/* move lpp to end of list of dst labels */

	*lpp = stmt[src].labels;
	return(stmt[dst].labels);
}

outold()		/* output oldest stmt */
{
	if(oldest != NIL) {
		prstmt(oldest);
		stmt[oldest].labels = (struct label *) NIL;
		oldest = nexst(oldest);
	}
}

char *
savestring(string)		/* save string, return ptr to it */
char *string;
{
	char *loc;

	loc = ((char *) calloc(1,strlen(string)+1));
	strcpy(loc,string);
	return(loc);
}
