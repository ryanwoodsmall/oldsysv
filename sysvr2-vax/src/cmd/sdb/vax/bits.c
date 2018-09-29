	/*	@(#) bits.c: 1.5 10/4/82	*/

/* 	Dissambler for VAX sdb
 *	Took from /usr/src/cmd/sdb/opset.c (UNIX 4.2), originally from
 *	/usr/src/cmd/adb/vax/opcode.c.
 *	printins(fmt, idsp, ins) low level code rewritten as
 *		dis_dot(ldot, idsp, fmt), paralleling code for 3B sdb.
 */

#include	<stdio.h>
#include	<sys/reg.h>
#include	"dis.h"
#define SGS	"VAX"

int errlev;	/* to keep track of errors encountered during	*/
		/* the disassembly, probably due to being out	*/
		/* of sync.					*/

/* the following arrays are contained in tbls.c */
extern struct optab optab[];	/* opcode mneumonics, values, etc. */
static int ioptab[256];		/* index by opcode to optab */

/* mkioptab initializes ioptab to index into optab - the table of opcodes.
 * By virtue of using this indirect indexing method, optab need not be in
 * numeric order (and can use the same table information as the assembler).
 */
mkioptab()
{
	struct optab *p;
	for (p = &optab[1]; p->iname; p++)
		ioptab[p->val & LOBYTE] = p - optab;
}

/* dis_dot attempts to disassemble an instruction, and returns the location
 *	of the next instruction.
 */

dis_dot(ldot, idsp, fmt)
long ldot;
int idsp;
char fmt;	/* not used */
{
	/* the following are declared in extn.c */
	extern long loc;	/* byte location being disassembled */
	extern char mneu[];	/* line to be printed (if desired) */
	extern int argument[];	/* array of operands (used by prassym in
				 * opset.c to disassemble case statements
				 */
	extern int caseflg;	/* set if case statement to be disassembled */

	extern int debug;	/* declared in udef.c (used by all of sdb) */
	extern int debugflag;	/* declared in udef.c (used by all of sdb) */

	register struct optab * ip;	/* pointer to opcode table entry */
	register unsigned char * ap;	/* pointer to arg type of current
					   operand */
	char *curpos;		/* current position in output string */
	short numargs;		/* number of operands (arguments) for opcode */
	short i;		/* usual loop counter */
	int ins;		/* saves opcode of current instruction */
	unsigned char oprndbyte; /* first byte of current operand */
	short mode;		/* addressing mode used by opcode */

	loc = ldot;
	errlev = 0;

	/* get opcode of current instruction */
	ins = chkget(loc, idsp) & 0xff;
	loc += 1;

#ifdef DEBUG
	if (debug)
		printf("(dis): opcode = %#x\n", ins);
#endif
	ip = optab + ioptab[ins];
	/* if invalid opcode, set errlev and return location */
	if (ip == optab) {
		errlev++;
		return(loc);
	}
	ap = ip->argtype;
	numargs = ip->nargs;

	/* print the mneumonic */
	sprintf(mneu, "%-8s", ip -> iname);
	curpos = mneu + 8;

	for (i=0; i < numargs; i++, ap++) {
		if (i != 0) sprintf(curpos++, ",");

		/* branch displacement address mode is made to
		 * appear as Program Counter address mode A (byte displacement)
		 * or C (short word displacement) by setting the operand byte
		 * to 0xAF + arg type <<  5 .  0xF = PC = Program Counter
		 * register, and the mode (high nibble) is 0xA or 0xC, since
		 * arg type is 0 (byte) or 1 (short word).
		 */
top:
		if (*ap & ACCB) {
			oprndbyte = 0xAF + ((*ap & 07) << 5);
		}
		else {
			oprndbyte = chkget(loc++, idsp) & 0xff;
		}

#if DEBUG>1
		if (debugflag > 2) {
			printf("(dis): oprndbyte = %#x", oprndbyte);
			if (*ap & ACCB) {
				printf(" - it was constructed (not read)\n");
			}
			else {
				printf(", loc (of oprndbyte) = %#x\n", loc-1);
			}
		}
#endif

		/* Literal Mode (constant operand) has mode nibble of 0-3.
		 * All non-literal operands are handled here.
		 */
		if (oprndbyte & 0xc0) {
			int regno;
			regno = oprndbyte & 0xf;  /* register is low nibble */
			mode = oprndbyte >> 4;    /* mode is high nibble */

#if DEBUG > 1
			if (debugflag > 2) {
			    printf("(dis): regno = %#x (%s), ",
				regno, regname[regno]);
			    printf("oprndbyte = %#x\n", oprndbyte);
			}
#endif

			switch ((int)mode) {
				case 4: /* [r] */
					curpos += sprintf(curpos, "[%s]",
							regname[regno]);
					goto top;  /* more of same operand */

				case 5: /* r */
					curpos += sprintf(curpos, "%s",
							regname[regno]);
					continue;

				case 7: /* -(r) */
					sprintf(curpos++, "-");
					/* fall through to case 6 */

				case 6: /* (r) */
					curpos += sprintf(curpos, "(%s)",
							regname[regno]);
					continue;

				case 9: /* *(r)+ */
					/* regno == PCNO means PC absolute
					 * addressing
					 */
					if (regno == PCNO) {
						curpos += sprintf(curpos,
							"*$0x%x",
							chkget(loc, idsp));
						loc += 4;
						continue;
					}
					curpos += sprintf(curpos, "*(%s)",
							regname[regno]);
					continue;

				case 8: /* (r)+ */
					/* regno == PCNO means PC immediate
					 * addressing
					 */
					if (regno != PCNO) {
						curpos += sprintf(curpos,
							"(%s)+",
							regname[regno]);
						continue;
					}
					/* for PC immediate addressing,
					 * switch on data type
					 */
					sprintf(curpos++, "$");
					switch (*ap & 07) {

					case TYPB:
					  argument[i] =
						(char)chkget(loc++,idsp);
					  curpos += sprintf(curpos, "0x%x",
						argument[i]);
					  continue;

					case TYPW:
					  argument[i] =
						(short)chkget(loc, idsp);
					  curpos += sprintf(curpos, "0x%x",
						argument[i]);
					  loc += 2;
					  continue;

					case TYPL:
					  argument[i] =
						chkget(loc, idsp);
					  curpos += sprintf(curpos, "0x%x",
						argument[i]);
					  loc += 4;
					  continue;

					case TYPQ:
					{
					  int temp;
					  temp = chkget(loc, idsp);
					  if (temp) {
						curpos += sprintf(curpos,
							"0x%x%x", temp,
							chkget(loc+4,idsp));
					  } else {
						curpos += sprintf(curpos,
							"0x%x",
							chkget(loc+4,idsp));
					  }
					  loc += 8;
					}
					  continue;

					case TYPF:
					  curpos += sprintf(curpos, "%f",
						chkget(loc, idsp));
					  loc += 4;
					  continue;

					case TYPD:
					{
					  union {
						double full;
						struct {
							int first;
							int second;
						} part;
					  } dbl;
					  dbl.part.first = chkget(loc, idsp);
					  dbl.part.second = chkget(loc+4,idsp);
					  curpos += sprintf(curpos, "%f",
						dbl.full);
					  loc += 8;
					  }
					  continue;
					} /* end of type switch */

				case 0xb:  /* byte displacement deferred */
					sprintf(curpos++, "*");
					/* fall through to case 0xa */

				case 0xa:  /* byte displacement */
					argument[i] = (char)chkget(loc++,idsp);
					/* if PC offset addressing, add loc */
					if (regno == PCNO) {
						argument[i] += loc;
						curpos += sprintf(curpos,
							"$0x%x",argument[i]);
					}
					else curpos += sprintf(curpos,
						"%ld(%s)", argument[i],
						regname[regno]);
					continue;

				case 0xd:  /* word (short) displacement
						deferred */
					sprintf(curpos++, "*");
					/* fall through to case 0xc */

				case 0xc:  /* word (short) displacement */
					argument[i] = (short)chkget(loc, idsp);
					loc += 2;
					/* if PC offset addressing, add loc */
					if (regno == PCNO) {
						argument[i] += loc;
						curpos += sprintf(curpos,
							"$0x%x",argument[i]);
					}
					else curpos += sprintf(curpos,
						"%ld(%s)", argument[i],
						regname[regno]);
					continue;

				case 0xf:  /* long displacement deferred */
					sprintf(curpos++, "*");
					/* fall through to case 0xe */

				case 0xe:  /* long displacement */
					argument[i] = (long)chkget(loc, idsp);
					loc += 4;
					/* if PC offset addressing, add loc */
					if (regno == PCNO) {
						argument[i] += loc;
						curpos += sprintf(curpos,
							"$0x%x",argument[i]);
					}
					else curpos += sprintf(curpos,
						"%ld(%s)", argument[i],
						regname[regno]);
					continue;

			} /* end of the mode switch */
		}
		else {   /* short literal */
			argument[i] = oprndbyte;
			if ((*ap & 07) == TYPF || (*ap & 07) == TYPD) {
				curpos += sprintf(curpos, "$%s", fltimm[oprndbyte]);
			}
			else {
				curpos += sprintf(curpos,"$0x%x",oprndbyte);
			}
		}
	}  /* end of operand loop ("i" loop) */

	if (ins == 0xcf || ins == 0xaf || ins == 0x8f) /*CASEx instruction */
	{
		caseflg = TRUE;
		loc += 2 * (argument[2] + 1);
	}
	else	caseflg = FALSE;

	return(loc);
}
