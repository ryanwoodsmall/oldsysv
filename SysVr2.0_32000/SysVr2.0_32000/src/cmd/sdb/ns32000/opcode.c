/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include <stdio.h>
#include "defs.h"

#include "../com/mode.h"
extern MSG      ADWRAP;

struct o1 {
	char *name;
	short val;
	char extra;
} ops1[] = {
	{"beq",   0x0a, 16},
	{"bne",   0x1a, 16},
	{"bcs",   0x2a, 16},
	{"bcc",   0x3a, 16},
	{"bhi",   0x4a, 16},
	{"bls",   0x5a, 16},
	{"bgt",   0x6a, 16},
	{"ble",   0x7a, 16},
	{"bfs",   0x8a, 16},
	{"bfc",   0x9a, 16},
	{"blo",   0xaa, 16},
	{"bhs",   0xba, 16},
	{"blt",   0xca, 16},
	{"bge",   0xda, 16},
	{"bpt",   0xf2, 0},
	{"br",    0xea, 16},
	{"bsr",   0x02, 16},
	{"cxp",   0x22, 2},
	{"dia",   0xc2, 0},
	{"enter", 0x82, 10},
	{"exit",  0x92, 1},
	{"flag",  0xd2, 0},
	{"nop",   0xa2, 0},
	{"restore", 0x72, 1},
	{"ret",   0x12, 2},
	{"reti",  0x52, 0},
	{"rett",  0x42, 2},
	{"rxp",   0x32, 2},
	{"save",  0x62, 8},
	{"svc",   0xe2, 0},
	{"wait",  0xb2, 0},
	{0,       0,    0}
};

struct o2 {
	short val, mask;
	char  *name;
	char  allow, type;
	char  extra;
} ops2[] = {
	{0x13,  0x1f,  "acb",    0, 1, 16},
	{0x00,  0x0f,  "add",    0, 0, 0},
	{0x04,  0x0f,  "addc",   0, 0, 0},
	{0x03,  0x1f,  "addq",   0, 1, 0},
	{0x09,  0x0f,  "addr",   2, 0, 0},
	{0x15f, 0x1ff, "adjsp",  0, 2, 0},
	{0x0a,  0x0f,  "and",    0, 0, 0},
	{0x02,  0x0f,  "bic",    0, 0, 0},
	{0x5f,  0x1ff, "bicpsr", 1, 2, 0},
	{0xdf,  0x1ff, "bispsr", 1, 2, 0},
	{0x1df, 0x1ff, "case",   0, 2, 0},
	{0x01,  0x0f,  "cmp",    0, 0, 0},
	{0x07,  0x1f,  "cmpq",   0, 1, 0},
	{0x1f,  0x1ff, "cxpd",   2, 2, 0},
	{0x19f, 0x1ff, "jsr",    2, 2, 0},
	{0x9f,  0x1ff, "jump",   2, 2, 0},
	{0x1b,  0x1f,  "lpr",    0, 3, 0},
	{0x05,  0x0f,  "mov",    0, 0, 0},
	{0x17,  0x1f,  "movq",   0, 1, 0},
	{0x06,  0x0f,  "or",     0, 0, 0},
	{0x0f,  0x1ff, "seq",    0, 2, 0},
	{0x2f,  0x1ff, "sne",    0, 2, 0},
	{0x4f,  0x1ff, "scs",    0, 2, 0},
	{0x6f,  0x1ff, "scc",    0, 2, 0},
	{0x8f,  0x1ff, "shi",    0, 2, 0},
	{0xaf,  0x1ff, "sls",    0, 2, 0},
	{0xcf,  0x1ff, "sgt",    0, 2, 0},
	{0xef,  0x1ff, "sle",    0, 2, 0},
	{0x10f, 0x1ff, "sfs",    0, 2, 0},
	{0x12f, 0x1ff, "sfc",    0, 2, 0},
	{0x14f, 0x1ff, "slo",    0, 2, 0},
	{0x16f, 0x1ff, "shs",    0, 2, 0},
	{0x18f, 0x1ff, "slt",    0, 2, 0},
	{0x1af, 0x1ff, "sge",    0, 2, 0},
	{0x0b,  0x1f,  "spr",    0, 3, 0},
	{0x08,  0x0f,  "sub",    0, 0, 0},
	{0x0c,  0x0f,  "subc",   0, 0, 0},
	{0x0d,  0x0f,  "tbit",   0, 0, 0},
	{0x0e,  0x0f,  "xor",    0, 0, 0},
	{-1,    0,     0,        0, 0, 0}
};

struct o3 {
	short val, mask;
	char  *name;
	char  f1, f2, regf, typef;
	char  extra;
} ops3[] = {
	{0x697, 0x7df, "abs",    0, 0, 1, 1, 0},
	{0x609, 0x79f, "abs",    0, 0, 1, 0, 0},
	{0x17,  0x7df, "add",    0, 0, 1, 1, 0},
	{0x789, 0x79f, "addp",   0, 0, 1, 0, 0},
	{0x89,  0x79f, "ash",    4, 0, 1, 0, 0},
	{0x109, 0x79f, "cbit",   0, 0, 1, 0, 0},
	{0x189, 0x79f, "cbiti",  0, 0, 1, 0, 0},
	{0x1d,  0x9f,  "check",  0, 0, 0, 0, 0},
	{0x117, 0x7df, "cmp",    0, 0, 1, 1, 0},
	{0x99,  0x79f, "cmpm",   0, 0, 1, 0, 2},
	{0x81,  0x1f9f,"cmps",   1, 1, 1, 0, 0},
	{0x1081,0x1fff,"cmpst",  1, 1, 1, 4, 0},
	{0x689, 0x79f, "com",    0, 0, 1, 0, 0},
	{0x6d,  0xff,  "cvtp",   0, 0, 0, 4, 0},
	{0x599, 0x79f, "dei",    0, 0, 1, 0, 0},
	{0x417, 0x7df, "div",    0, 0, 1, 1, 0},
	{0x799, 0x79f, "div",    0, 0, 1, 0, 0},
	{0x05,  0x9f,  "ext",    0, 0, 0, 0, 2},
	{0x199, 0x79f, "exts",   0, 0, 1, 0, 4},
	{0x8d,  0x79f, "ffs",    0, 4, 1, 0, 0},
	{0x707, 0x71f, "floor",  0, 0, 1, 2, 0},
	{0x709, 0x79f, "ibit",   0, 0, 1, 0, 0},
	{0x85,  0x9f,  "index",  0, 0, 0, 0, 0},
	{0x15,  0x9f,  "ins",    0, 0, 0, 0, 2},
	{0x119, 0x79f, "inss",   0, 0, 1, 0, 4},
	{0x1e7, 0xffff,"lfsr",   0, 4, 1, 4, 0},
	{0x163, 0xfff, "lmr",    0, 3, 1, 4, 0},
	{0x289, 0x79f, "lsh",    4, 0, 1, 0, 0},
	{0x499, 0x79f, "mei",    0, 0, 1, 0, 0},
	{0x719, 0x79f, "mod",    0, 0, 1, 0, 0},
	{0x97,  0x7df, "mov",    0, 0, 1, 1, 0},
	{0x07,  0x71f, "mov",    0, 0, 1, 3, 0},
	{0x367, 0x7ff, "movfl",  0, 0, 1, 6, 0},
	{0x2c7, 0x7ff, "movlf",  0, 0, 1, 5, 0},
	{0x19,  0x79f, "movm",   0, 0, 1, 0, 2},
	{0x01,  0x1f9f,"movs",   1, 1, 1, 0, 0},
	{0x1001,0x1fff,"movst",  1, 1, 1, 4, 0},
	{0x195, 0x79f, "movsu",  0, 0, 1, 0, 0},
	{0x395, 0x79f, "movus",  0, 0, 1, 0, 0},
	{0x399, 0x7ff, "movxbd", 4, 7, 1, 4, 0},
	{0x3b9, 0x7ff, "movxwd", 5, 7, 1, 4, 0},
	{0x219, 0x7ff, "movxbw", 4, 5, 1, 4, 0},
	{0x319, 0x7ff, "movzbd", 4, 7, 1, 4, 0},
	{0x339, 0x7ff, "movzwd", 5, 7, 1, 4, 0},
	{0x299, 0x7ff, "movzbw", 4, 5, 1, 4, 0},
	{0x617, 0x7df, "mul",    0, 0, 1, 1, 0},
	{0x419, 0x79f, "mul",    0, 0, 1, 0, 0},
	{0x297, 0x7df, "neg",    0, 0, 1, 1, 0},
	{0x409, 0x79f, "neg",    0, 0, 1, 0, 0},
	{0x489, 0x79f, "not",    0, 0, 1, 0, 0},
	{0x619, 0x79f, "quo",    0, 0, 1, 0, 0},
	{0x63,  0xffff,"rdval",  0, 4, 1, 4, 0},
	{0x699, 0x79f, "rem",    0, 0, 1, 0, 0},
	{0x09,  0x79f, "rot",    4, 0, 1, 0, 0},
	{0x407, 0x71f, "round",  0, 0, 1, 2, 0},
	{0x309, 0x79f, "sbit",   0, 0, 1, 0, 0},
	{0x389, 0x79f, "sbiti",  0, 0, 1, 0, 0},
	{0x161, 0xfff, "setcfg", 1, 2, 1, 4, 0},
	{0x6e7, 0x7ff, "sfsr",   1, 0, 1, 4, 0},
	{0x181, 0x1f9f,"skps",   1, 1, 1, 0, 0},
	{0x1181,0x1fff,"skpst",  1, 1, 1, 4, 0},
	{0x1e3, 0xfff, "smr",    0, 3, 1, 4, 0},
	{0x217, 0x7df, "sub",    0, 0, 1, 1, 0},
	{0x589, 0x79f, "subp",   0, 0, 1, 0, 0},
	{0x507, 0x71f, "trunc",  0, 0, 1, 2, 0},
	{0xe3,  0xffff,"wrval",  0, 4, 1, 4, 0},
	{0,     0,     0,        0, 0, 0, 0, 0}
};

char *preg[] = {
	"upsr", "?",   "?",       "?",
	"?",    "?",   "?",       "?",
	"fp",   "sp",  "sb",      "?",
	"?",    "psr", "intbase", "mod"
};

char *rnm[] = {
	"fp", "sp", "sb", "pc"
};

short flen[2] = {15, 11};	/* length parameters for f, l */

static int incp;
static int Idsp;
static int dot;

int errlev;	/* to keep track of errors encountered during	*/
		/* the disassembly, probably due to being out	*/
		/* of sync.					*/

dis_dot(loc, idsp, fmt)
{
	int ins;

	dot = loc;
	/* get opcode of current instruction */
	ins = chkget(loc,idsp);

	errlev = 0;
	initpbuf();

	Idsp = idsp;
	switch (ins & 7) {
	case 2:
		incp = 1;
		op1(ins);
		break;
	case 6:
		incp = 3;
		op3(ins);
		break;
	default:
		incp = 2;
		op2(ins);
		break;
	}
	return(dot+incp);
}

op1(a)
{
	register struct o1 *p;
	int i;

	a &= 0xff;
	for (p=ops1; p->val != 0; p++) {
		if (p->val == a) {
			sdbpr("%s%8t", p->name);
			if (p->extra) {
				do_extra(p->extra);
			}
			return;
		}
	}
	sdbpr("?");
	errlev++;
}

op2(a)
{
	int i;
	register struct o2 *p;
	short cmp;

	a &= 0xffff;
	cmp = a>>2;
	for (p=ops2; p->mask != 0; p++) {
		if (p->val == (cmp & p->mask)) {
			sdbpr(p->name);
			if (p->allow != 2) {
				printc("bw?d"[a&3]);
			}
			sdbpr("%8t");
			switch (p->type) {
			case 0:
				gen2(a>>11, (a>>6)&0x1f, a&3, a&3);
				break;
			case 1:
				quick((a>>7)&0xf);
				printc(',');
				gen1(a>>11, a&3);
				break;
			case 2:
				gen1(a>>11, a&3);
				break;
			case 3:
				sdbpr("%s,", preg[(a>>7)&0xf]);
				gen1(a>>11, a&3);
				break;
			}
			if (p->extra) {
				printc(',');
				do_extra(p->extra);
			}
			return;
		}
	}
	sdbpr("??");
	errlev++;
}

op3(a)
{
	int itype, ftype;
	int i;
	short l1, l2;
	register struct o3 *p;
	short cmp;

	a &= 0xffffff;
	cmp = a >> 3;
	for (p=ops3; p->mask != 0; p++) {
		if (p->val == (cmp & p->mask)) {
			sdbpr(p->name);
			switch (p->typef) {
			case 0:
				itype = (a>>8)&3;
				l1 = l2 = itype;
				printc("bw?d"[itype]);
				break;
			case 1:
				ftype = (a>>8)&1;
				l1 = l2 = flen[ftype];
				printc("lf"[ftype]);
				break;
			case 2:
				itype = (a>>8)&3;
				ftype = (a>>10) & 1;
				l1 = flen[ftype];
				l2 = itype;
				printc("lf"[ftype]);
				printc("bw?d"[itype]);
				break;
			case 3:
				itype = (a>>8)&3;
				ftype = (a>>10) & 1;
				l1 = itype;
				l2 = flen[ftype];
				printc("bw?d"[itype]);
				printc("lf"[ftype]);
				break;
			case 4:
				l1 = l2 = 3;
				break;
			case 5:
				l1 = flen[0];
				l2 = flen[1];
				break;
			case 6:
				l1 = flen[1];
				l2 = flen[0];
				break;
			}
			sdbpr("%8t");
			if (p->regf == 0) {
				sdbpr("r%d,", (a>>11) & 7);
			}
			if (p->f1 == 0 && p->f2 == 0) {
				gen2(a>>19, (a>>14)&0x1f, l1, l2);
			} else {
				i = (a>>14) & 0x1f;
				if (p->f2 == 3) {
					do_ss(i>>1);
					printc(',');
				}
				if (p->f1 == 0) {
					gen1(a>>19, l1);
					if (p->f2 != 3)
						printc(',');
				}
				if (p->f1 >= 4) {
					gen1(a>>19, p->f1 - 4);
					printc(',');
				}
				switch (p->f2) {
				case 0:
					gen1(i, l1);
					break;
				case 1:
					do_uwb(i>>2);
					break;
				case 2:
					do_xx(i>>1);
					break;
				case 3:
					break;
				default:
					gen1(i, p->f2 - 4);
					break;
				}
			}
			if (p->extra) {
				printc(',');
				do_extra(p->extra);
			}
			return;
		}
	}
	sdbpr("??");
	errlev++;
}

gen2(m1, m2, l1, l2)
{
	int idx1, idx2;

	if (m1 >= 0x1c) { /* scaled indexing */
		idx1 = egetchar();
	}
	if (m2 >= 0x1c) {
		idx2 = egetchar();
	}
	if (m1 >= 0x1c)
		sc_gen(idx1, m1&3);
	else
		sub_gen(m1, l1);
	printc(',');
	if (m2 >= 0x1c)
		sc_gen(idx2, m2&3);
	else
		sub_gen(m2, l2);
}
gen1(mode, len)
{
	int idx;

	if (mode >= 0x1c) {	/* scaled indexing */
		idx = egetchar();
		sc_gen(idx, mode&3);
	} else
		sub_gen(mode, len);
}
quick(x)
{
	if (x <= 7)
		sdbpr("$%R", x);
	else
		sdbpr("$%R", x-16);
}

/* scaled indexing - index byte in idx, scale factor in slen */
sc_gen(idx, slen)
{
	int mode, reg;

	mode = idx>>3;
	reg = idx & 7;
	if (mode == 0x14) {	/* immediate */
		sdbpr("BAD");
		return;
	}
	sub_gen(mode, 0);
	sdbpr("[r%d:%c]", reg, "bwdq"[slen]);
}

/* generic, but not scaled - */
sub_gen(mode, len)
{
	int d1, d2;

	if (mode <= 7) {	/* register */
		sdbpr("%c%d", (len & 8) ? 'f' : 'r', mode);
		return;
	}
	if (mode <= 15) {	/* register relative */
		d1 = getdisp();
		sdbpr("%R(r%d)", d1, mode&7);
		return;
	}
	if (mode <= 18) {	/* memory relative */
		d1 = getdisp();
		d2 = getdisp();
		sdbpr("%R(%R(%s))", d2, d1, rnm[mode-16]);
		return;
	}
	if (mode >= 0x18) {	/* memory space */
		d1 = getdisp();
		sdbpr("%R(%s)", d1, rnm[mode-0x18]);
		return;
	}
	switch (mode) {
	case 0x14:	/* immediate */
		putimm(len);
		return;
	case 0x15:	/* absolute */
		d1 = getdisp();
		sdbpr("%R", d1);
		return;
	case 0x16:	/* external */
		d1 = getdisp();
		d2 = getdisp();
		sdbpr("ext(%R)%R", d1, d2);
		return;
	case 0x17:	/* top of stack */
		sdbpr("tos");
		return;
	default:
		sdbpr("??");
	}
}

/* get a 1 to 4 byte signed displacement */
int
getdisp()
{
	int a, b, c, d;

	a = egetchar();
	if ((a & 0x80) == 0) {
		if (a < 0x40)
			return(a);
		else
			return(a-0x80);
	}
	if ((a & 0xc0) == 0x80) {	/* short */
		b = egetchar();
		a = a<<8 | b;
		a &= 0x3fff;
		if (a < 0x2000)
			return(a);
		else
			return(a-0x4000);
	}
	/* long */
	b = egetchar(); c = egetchar(); d = egetchar();
	a = (a << 24) | (b << 16) | (c << 8) | d;
	a &= 0x3fffffff;
	if (a < 0x20000000)
		return(a);
	else
		return(a - 0x40000000);
}

/* print a 1 to 4 byte immediate */
/* or a 4 or 8 byte floating immediate */
int
putimm(len)
{
	int a, x;
	int flo;
	int i;

	flo = len & 8;
	len &= 7;
	x = 0;
	for (i=0; i<=len; i++) {
		a = egetchar();
		if (a == EOF) {
			sdbpr("??");
			return;
		}
		if (i <= 3)
			x = x<<8 | a;
	/* note that bytes are backwards in immediates! */
	}
	if (flo) {
		int f, e;
		double ldexp();
		double d;

		if (len == 3) {	/* float */
			f = (x & 0x7fffff) | 0x800000;
			e = ((x>>23) & 0xff) - 127;
			e -= 23;
		} else {
			f = (x & 0xfffff) | 0x100000;
			e = ((x>>20) & 0x7ff) - 1023;
			e -= 20;
		}
		d = ldexp((double)f, e);
		if (x < 0)
			d = -d;
		sdbpr("$%-32.18F", d);
	} else {
		if (len == 0 && (x & 0x80) != 0)
			x |= 0xffffff00;
		else if (len == 1 && (x & 0x8000) != 0)
			x |= 0xffff0000;
		sdbpr("$%R",x);
	}
}

egetchar()
{
	int a;

	a = bchkget(inkdot(incp), Idsp);
	incp++;
	return a;
}

do_extra(x)
{
	int some = 0;
	int disp;
	int a, i, j;

	if (x & (1|8)) {	/* reglist */
		a = egetchar();
		printc('[');
		for (i=0; i<8; i++) {
			j = (x&1) ? 0x80>>i : 1<<i;
			if (a & j) {
				if (some)
					printc(',');
				sdbpr("r%d", i);
				some++;
			}
		}
		printc(']');
		some++;
	}
	if (x & (2|16)) {	/* disp */
		if (some)
			printc(',');
		else
			some++;
		disp = getdisp();
		if (x & 2)
			sdbpr("%R", disp);
		else
			sdbpr("%R", dot+disp);
	}
	if (x & 4) {	/* short offset,len */
		if (some)
			printc(',');
		a = egetchar();
		sdbpr("%R,%R", a>>5, (a&0x1f)+1);
	}
}

char *sname[] = {
	"bpr0", "bpr1", "?",   "?",
	"pf0",  "pf1",  "?",   "?",
	"sc",   "?",    "msr", "bcnt",
	"ptb0", "ptb1", "?",   "eia"
};

do_ss(x)
{
	sdbpr(sname[x]);
}

do_uwb(x)
{
	int i, some;

	some = 0;
	printc('[');
	for (i=0; i<4; i++) {
		if (x & (1<<i)) {
			if (some)
				printc(',');
			else
				some++;
			printc("bwu"[i]);
		}
	}
	printc(']');
}

do_xx(x)
{
	int i, some;

	some = 0;
	printc('[');
	for (i=0; i<4; i++) {
		if (x & (1<<i)) {
			if (some)
				printc(',');
			else
				some++;
			printc("ifmc"[i]);
		}
	}
	printc(']');
}

int inkdot(incr)
{
	int newdot;

	newdot = dot + incr;
	if ((dot ^ newdot) >> 24 )
		error(ADWRAP);
	return(newdot);
}
