/*	@(#)sqrt.c	2.3	*/
# include "e.h"

sqrt(p2) int p2; {
	int nps;

	nps = EFFPS(((eht[p2]*9)/10+(res/POINT-1))/(res/POINT));
	yyval = p2;
	if (ttype == DEVCAT || ttype == DEVAPS)
		eht[yyval] = VERT( EM(1.2, nps) );
	else if (ttype == DEV202 || ttype == DEVX97)
		eht[yyval] = VERT( EM(1.0, nps) );
	if(dbg)printf(".\t\\\" sqrt: S%d <- S%d;b=%d, h=%d\n", 
		yyval, p2, ebase[yyval], eht[yyval]);
	if (rfont[yyval] == ITAL)
		printf(".as %d \\|\n", yyval);
	nrwid(p2, ps, p2);
	printf(".ds %d \\v'%du'\\s%d", yyval, ebase[p2], nps);	/* proper position for sqrt */
	if (ttype == DEVCAT || ttype == DEVAPS)
		printf("\\v'-.2m'\\(sr\\l'\\n(%du\\(rn'\\v'.2m'", p2);
	else
		printf("\\(sr\\l'\\n(%du\\(rn'", p2);	/* .95 best for 10; .9 best for 16 */
	printf("\\s%d\\v'%du'\\h'-\\n(%du'\\*(%d\n", ps, -ebase[p2], p2, p2);
	lfont[yyval] = ROM;
}
