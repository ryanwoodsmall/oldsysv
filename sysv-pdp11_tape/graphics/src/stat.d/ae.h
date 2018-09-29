static char ae__[]="@(#)ae.h	1.1";
#define OPR 0177
#define OPN ~0377
#define IDENT 0400
#define CONST 01400
#define UNKNOWN 0200
struct operand {
	FILE *fd;
	char name[14];
	double val;
};
struct p_entry {
	int opr;
	struct operand *opn;
};
