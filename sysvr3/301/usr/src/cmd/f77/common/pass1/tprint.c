/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)f77:common/pass1/tprint.c	1.1.1.1"
/*	@(#)f77:common/pass1/tprint.c	1.1.1.1	*/
#include "defs"

static int level = 0;

static char *opname[] = 
	{
	"unknown",
	"+",
	"-",
	"*",
	"/",
	"**",
	"u-",
	"||",
	"&&",
	".eqv.",
	".neqv.",
	"!",
	"|",
	".lt.",
	".eq.",
	".gt.",
	".le.",
	".ne.",
	".ge.",
	"call",
	"ccall",
	"=",
	"+=",
	"*=",
	"sconv",
	"<<",
	"%",
	",,",
	"?",
	":",
	"abs",
	"min",
	"max",
	"addr",
	"u*",
	"or()",
	"and()",
	"xor()",
	"not()",
	">>"
	};

static char *typnames[] =
	{
	"tyunknown",
	"tyaddr",
	"tyshort",
	"tylong",
	"tyreal",
	"tydouble",
	"tycomplex",
	"tydcomplex",
	"tylogical",
	"tychar",
	"tysubr",
	"tyerror"
	};

static char *consnames[] =
	{
	"unkcon",
	"acon",
	"icon",
	"icon",
	"rcon",
	"dcon",
	"cxcon",
	"dxcon",
	"logcon",
	"chcon",
	"sbcon",
	"errcon"
	};

static char *stgname[] =
	{
	"unknown\t",
	"arg\t(ap)",
	"auto\t(fp)",
	"bss\t",
	"init\t",
	"const\t",
	"ext\t",
	"intr\t",
	"stfunct\t",
	"common\t",
	"equiv\t",
	"register\tr",
	"leng\t",
	"null\t"
	};

void tprint(px)
expptr px;
	{
	int i;
	chainp pl;

	for (i = level; i > 0; --i)
		putchar('\t');
	switch (px->headblock.tag)
		{
		case TCONST:
			if (0 > px->headblock.vtype ||
			    px->headblock.vtype > TYERROR)
				{
				fputs("unknown constant type\n", stdout);
				return;
				}
			fputs(consnames[px->headblock.vtype], stdout);
			putchar('\t');
			switch (px->headblock.vtype)
				{
				case TYADDR:
				case TYSHORT:
				case TYLONG:
					printf("%d\n", px->constblock.const.ci);
					break;
				case TYREAL:
				case TYDREAL:
					printf("%g\n",
					    px->constblock.const.cd[0]);
					break;
				case TYCOMPLEX:
				case TYDCOMPLEX:
					printf("(%g, %g)",
					    px->constblock.const.cd[0],
					    px->constblock.const.cd[1]);
					break;
				case TYLOGICAL:
					if (px->constblock.const.ci)
						putchar('t');
					else
						putchar('f');
					putchar('\n');
					break;
				case TYCHAR:
				case TYSUBR:
					puts(px->constblock.const.ccp);
					break;
				default:
					puts("ERROR");
				}
			break;
		case TEXPR:
			fputs(opname[px->exprblock.opcode], stdout);
			putchar('\t');
			puts(typnames[px->headblock.vtype]);
			if (px->exprblock.leftp)
				{
				++level;
				tprint(px->exprblock.leftp);
				--level;
				}
			if (px->exprblock.rightp)
				{
				++level;
				tprint(px->exprblock.rightp);
				--level;
				}
			break;
		case TADDR:
			fputs("TADDR", stdout);
			if (px->addrblock.istemp)
				fputs("\ttemp", stdout);
			putchar('\t');
			fputs(stgname[px->addrblock.vstg], stdout);
			if (px->addrblock.vstg == STGEXT ||
			    px->addrblock.vstg == STGBSS ||
			    px->addrblock.vstg == STGINIT ||
			    px->addrblock.vstg == STGCONST ||
			    px->addrblock.vstg == STGEQUIV ||
			    px->addrblock.vstg == STGCOMMON)
				fputs(memname(px->addrblock.vstg,
						px->addrblock.memno), stdout);
			else
				printf("%d", px->addrblock.memno);
			putchar('\t');
			puts(typnames[px->headblock.vtype]);
			if (px->addrblock.memoffset)
				{
				++level;
				tprint(px->addrblock.memoffset);
				--level;
				}
			break;
		case TPRIM:
			fputs("TPRIM", stdout);
			putchar('\t');
			fputs(px->primblock.namep->varname, stdout);
			goto prtype;
		case TLIST:
			fputs("TLIST:\n", stdout);
			++level;
			pl = px->listblock.listp;
			while (pl != NULL)
				{
				tprint((expptr) pl->datap);
				pl = pl->nextp;
				}
			--level;
			break;
		case TIMPLDO:
			fputs("TIMPLDO", stdout);
	prtype:
			putchar('\t');
			puts(typnames[px->headblock.vtype]);
			break;
		default:
			puts("unknown tag");
		}
	}
