/*	@(#)opcode.c	1.3	*/
#include "defs.h"

SCCSID(@(#)opcode.c	1.3);

/*
 * Argument access types
 */
#define ACCA (8<<3) /* address only */
#define ACCR (1<<3) /* read */
#define ACCW (2<<3) /* write */
#define ACCM (3<<3) /* modify */
#define ACCB (4<<3) /* branch displacement */
#define ACCI (5<<3) /* XFC code */

/*
 * Argument data types
 */
#define TYPB 0 /* byte */
#define TYPW 1 /* word */
#define TYPL 2 /* long */
#define TYPQ 3 /* quad */
#define TYPF 4 /* floating */
#define TYPD 5 /* double floating */

#define ARPC (&reglist[1].rname)

typedef struct optab *OPTAB;
static struct optab {
    char *iname;
    char val;
    char nargs;
    char argtype[6];
} optab[] = {
#define OP(a,b,c,d,e,f,g,h,i) {a,b,c,d,e,f,g,h,i}
#include "instrs.h"
0};

static STRING fltimm[] = {
    "0.5",    "0.5625", "0.625", "0.6875", "0.75",  "0.8125", "0.875", 
    "0.9375", "1.0",    "1.125", "1.25",   "1.375", "1.5",    "1.625", 
	"1.75",   "1.875",  "2.0",   "2.25",   "2.5",   "2.75",   "3.0",
	"3.25",   "3.5",    "3.75",  "4.0",    "4.5",   "5.0",    "5.5", 
	"6.0",    "6.5",    "7.0",   "7.5",    "8.0",   "9.0",    "10.0",  
	"11.0",   "12.0",   "13.0",  "14.0",   "15.0",  "16.0",   "18.0",   
	"20.0",   "22.0",   "24.0",  "26.0",   "28.0",  "30.0",   "32.0",
	"36.0",   "40.0",   "44.0",  "48.0",   "52.0",  "56.0",   "60.0",
    "64.0",   "72.0",   "80.0",  "88.0",   "96.0",  "104.0",  "112.0",
	"120.0"
};

static int incp;
static int ioptab[256]; /* index by opcode to optab */
static void printfloating();
static void printquad();

void mkioptab()                 /* set up ioptab */
{
    OPTAB p;

    for (p = optab; p->iname; p++)
        ioptab[p->val & LOBYTE] = p - optab;
}

void printins(idsp, ins, pflag)
	int idsp;
	int ins;
	int pflag;
{
	int   argno;              /* argument index */
	int   mode;               /* mode */
	char  **r;                /* register name */
	int   d;                  /* assembled byte, word, long or float */
	char  *ap;
	OPTAB ip;

	ins &= LOBYTE;
	ip = optab + ioptab[ins];
	if(pflag) adbpr("%s%8t", ip->iname);
	incp = 1;
	ap = ip->argtype;
	for (argno = 0; argno < ip->nargs; argno++, ap++) {
		var[argno] = 0x80000000;
		if (argno != 0) 
		{
			if(pflag) printc(',');
		}
top:
		if (*ap & ACCB)
			mode = 0xAF + ((*ap & 07) << 5);
			/* branch displacement */
		else {
			mode = bchkget(inkdot(incp), idsp);
			++incp;
		}
		if (mode & 0300) {          /* not short literal */
			r = &reglist[(MAXREGS - 1) - (mode & 0xF)].rname;
			mode >>= 4;

			switch (mode) {
				case 4:           /* [r] */
					if(pflag) adbpr("[%s]", *r);
					goto top;

				case 5:           /* r */
					if(pflag) adbpr("%s", *r);
					break;

				case 6:           /* (r) */
					if(pflag) adbpr("(%s)", *r);
					break;

				case 7:           /* -(r) */
					if(pflag) adbpr("-(%s)", *r);
					break;

				case 9:           /* *(r)+ */
					if(pflag) printc('*');
				case 8:           /* (r)+ */
					if (r == ARPC) {
						if(pflag) printc('$');
						/* PC absolute, always 4 bytes */
						if (mode == 9) {
							d = snarf(4, idsp);
							goto disp;
						}
						switch(*ap & 7) {

							case TYPB:
								d = snarf(1, idsp);
								goto disp;

							case TYPW:
								d = snarf(2, idsp);
								goto disp;

							case TYPL:
								d = snarf(4, idsp);
								goto disp;

							case TYPQ:
								d = snarf(4, idsp);
								if(pflag) printquad(d, snarf(4, idsp));
								break;

							case TYPF:
								if(pflag) printfloating(snarf(4, idsp), 0);
								break;

							case TYPD:
								d = snarf(4, idsp);
								if(pflag) printfloating(d, snarf(4, idsp));
								break;
						} /* End of type switch */
						/*
						 * Here only for TYPQ, TYPf, TYPD
						 * others went to disp
						*/
					}
					else        /* It's not PC immediate or abs */
						if(pflag) adbpr("(%s)+", *r);
					break;

				case 0xB:           /* byte displacement defferred */
					if(pflag) printc('*');
				case 0xA:           /* byte displacement */
					d = snarf(1, idsp);
					goto disp;

				case 0xD:           /* word displacement deferred */
					if(pflag) printc('*');
				case 0xC:           /* word displacement */
					d = snarf(2, idsp);
					goto disp;

				case 0xF:           /* long displacement deferred */
					if(pflag) printc('*');
				case 0xE:           /* long displacement */
					d = snarf(4, idsp);
					goto disp;
disp:
				var[argno] = d;
				if ((r == ARPC) && mode >= 0xA) {
					/* PC offset addressing */
					var[argno] += dot + incp;
					if(pflag) psymoff(var[argno], ISYM, "");
				}
				else
					if(pflag) psymoff(var[argno], DSYM, "");
				if (r != ARPC)
					if(pflag) adbpr("(%s)", *r);
				break;
			} /* end of the mode switch */
		} 
		else {   /* short literal */
			var[argno] = mode;
			if ((*ap & 07) == TYPF || (*ap & 07) == TYPD)
			{
				if(pflag) adbpr("$%s", fltimm[mode]);
			}
			else
				if(pflag) adbpr("$%r", mode);
		}
	}
	if (ins == 0xCF || ins == 0xAF || ins == 0x8F) { /* CASEx instr */
		for (argno = 0; argno <= var[2]; ++argno) {
			if(pflag) printc(EOR);
			if(pflag) adbpr("    %R:  ", argno + var[1]);
			d = get(inkdot(incp + argno + argno), idsp) & 0xFFFF;
			if (d & 0x8000) 
				d -= 0x10000;
			if(pflag) psymoff(((int) (inkdot(incp) + d)), DSYM, "");
		}
		incp += var[2] + var[2] + 2;
	}
	dotinc = incp;
}

/*
 * magic_masks check the sign bit
 * magic_compl is used to perform sign extension on bytes and
 * words when moved into a long
 */

                           /* 0   1      2       3  4 */
static long magic_masks[5] = {0, 0x80,  0x8000,  0, 0};
static long magic_compl[5] = {0, 0x100, 0x10000, 0, 0};

static int snarf(nbytes, idsp)
	int nbytes;
	int idsp;
{
	int byteindex;
	union {
		char int_bytes[4];
		int int_value;
	} d;

	d.int_value = 0; 
	for (byteindex = 0; byteindex < nbytes; byteindex++) {
		d.int_bytes[byteindex] = bchkget(inkdot(incp), idsp);
		++incp;
	}
	if (d.int_value & magic_masks[nbytes])
	d.int_value -= magic_compl[nbytes];
	return(d.int_value);
}

static void printfloating(word_first, word_last)
	int word_first;
	int word_last;
{
	union {
		struct {
			int word_first;
			int word_last;
		} composite;
		double dvalue;
	} reconstructed;

	reconstructed.composite.word_first = word_first;
	reconstructed.composite.word_last = word_last;
	adbpr ("%f", reconstructed.dvalue);
}

static void printquad(word_first, word_last)
	int word_first;
	int word_last;
{
	union {
		char quad_bytes[8];
		int  quad_int[2];
	} reconstructed;
	int leading_zero = 1;
	int byteindex;
	int nibbleindex;
	int ch;

	reconstructed.quad_int[0] = word_first;
	reconstructed.quad_int[1] = word_last;
	for (byteindex = 7; byteindex >= 0; --byteindex)
		for (nibbleindex = 4; nibbleindex >= 0; nibbleindex -= 4) {
			ch = (reconstructed.quad_bytes[byteindex] >> 
				  nibbleindex) & 0x0F;
			if ( ! (leading_zero &= (ch == 0))) {
				if (ch <= 0x09)
					printc(ch + '0');
				else
					printc(ch - 0x0A + 'a');
			}
		}
}
