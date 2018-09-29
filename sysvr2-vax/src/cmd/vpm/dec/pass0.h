/*	pass0.h 1.2 of 12/16/82	*/
/*	@(#)pass0.h	1.2	*/
#define NIL (-1)
#define YES 1
#define NO 0
#define NOTRY -1
#define END -1
#define LABEL -2
#define STMT 1
#define NFIELDS 3
#define MAXFSIZE 9
#define MAXLINLEN 128
#define NSTMTS 3
#define NOPTFNS 3
#define white(C) ((C == ' ') || (C == '\t'))
#define COLON ':'
#define NULLCH '\0'

#define EQ(x,y) !strcmp(x,y)
#define nexst(I) ((newest==NIL) ? 0 : ((I+1) % NSTMTS))
#define ROOM ((newest==NIL) ? YES : (nexst(newest) != oldest))
#define NOLABEL(x) (stmt[x].prlabel[0] == (char) NIL)
#define NONZERO(x) strcmp(x,"0")
#define OPCODE field[0]
#define OPERAND field[1]

extern char *calloc();
