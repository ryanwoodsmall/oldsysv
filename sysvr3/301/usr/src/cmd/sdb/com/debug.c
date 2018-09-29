/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/debug.c	1.12"

#include "head.h"
#include "coff.h"
/*
 * debugging for sdb
 */
extern MSG		NOPCS;

prstentry(stp)	/* debug */
register struct syment *stp;
{
	fprintf( FPRT2, "debug: print symbol table entry:\n" );
	fprintf( FPRT2, "	n_name=  %s;\n", stp->n_name );
	fprintf( FPRT2, "	n_value= %#lx;\n", stp->n_value );
	fprintf( FPRT2, "	n_scnum= %d;\n", (int)stp->n_scnum );
	fprintf( FPRT2, "	n_type=  %#x;\n", (int)stp->n_type );
	fprintf( FPRT2, "	n_sclass=%d;\n", (int)stp->n_sclass );
	fprintf( FPRT2, "	n_numaux=%d;\n", (int)stp->n_numaux );
}

prprocaux(axp, name)	/* debug */
AUXENT *axp;
char *name;
{
#define AX	axp->x_sym
#define AXF	AX.x_fcnary
	fprintf(FPRT2, " proc auxent for %s:\n", name);
	fprintf(FPRT2, "	x_tagndx= 0x%8.8lx\n", AX.x_tagndx);
	fprintf(FPRT2, "	x_fsize=  0x%8.8lx\n", AX.x_misc.x_fsize);
	fprintf(FPRT2, "	x_lnnoptr=0x%8.8lx\n", AXF.x_fcn.x_lnnoptr);
	fprintf(FPRT2, "	x_endndx= 0x%8.8lx\n", AXF.x_fcn.x_endndx);
}

prprstk()	/* debug */
{
	register int n, i;
#if vax || u3b
	fprintf(FPRT2, "((struct user *)uu)->u.ar0 = %#o\n",
		((struct user *)uu)->u_ar0);
#else
#if u3b5 || u3b15 || u3b2
	readregs();
#endif
#endif
	fprintf(FPRT2, "debug process stack:\n");
	if( pid == 0 && PC < 0 )
	{
		error( NOPCS );
		return( -1 );
	}
	else
	{
#if u3b5 || u3b15 || u3b2
		fprintf(FPRT2, "	SDBREG(PC) = %#o\n", regvals[PC]);
#else
		fprintf(FPRT2, "	SDBREG(PC) = %#o\n", SDBREG(PC));
#endif
	}

#if u3b
	fprintf(FPRT2, "	SDBREG(PS) = %#o\n", SDBREG(PS));
	fprintf(FPRT2, "	SDBREG(PSBR) = %#o\n", SDBREG(PSBR));
	fprintf(FPRT2, "	SDBREG(SSBR) = %#o\n", SDBREG(SSBR));
#else
#if vax
	fprintf(FPRT2, "	SDBREG(PS) = %#o\n", SDBREG(PS));
#else
#if u3b5 || u3b15 || u3b2
	for (i = 0; i <= 12; i++)
		fprintf(FPRT2,"%s = 0x%8.8lx\n",reglist[i].rname,regvals[reglist[i].roffs]);
#endif
#endif
#endif
}
static int debug_margin;

#define TAB "  "

init1()
{
	debug_margin=0;
}

incr1()
{
	++debug_margin;
}

decr1()
{
	--debug_margin;
}

indent()
{
	register int i;

	for ( i=1 ; i<= debug_margin ; i ++ )
		printf(TAB);
}

enter1(s)
char * s;
{
	indent();
	printf("%s\n",s);
	incr1();
}

exit1()
{
	decr1();
}

enter2(s)
char * s;
{
	indent();
	printf("%s(",s);
	incr1();
}

exit2(s)
char * s;
{
	decr1();
	indent();
	printf("%s() returns ",s);
}

closeparen()
{
	printf(")\n");
}

endofline()
{
	printf("\n");
}

arg(s)
char * s;
{
	printf("%s == ",s);
}

comma()
{
	printf(",");
}
