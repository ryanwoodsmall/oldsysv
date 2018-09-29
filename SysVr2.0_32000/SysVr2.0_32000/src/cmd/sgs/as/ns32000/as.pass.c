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
# include <stdio.h>
# include "as.sym.h"
# include "as.cbuf.h"
# define YES 1
# define NO 0
# define SCTALIGN 4

extern char *ifile, *ofile;
extern short instr;
extern int errflag;
extern int opt;

long dottxt, dotdat, dotbss, newdot;
long uptxt, updat, upbss, txtsize, datsize, bsssize;
long txtrel = 0; datrel = 0;
short bitpos;

SYM dot;

aspass1() {
   dot.segmt = TXT;
   dot.value = newdot = 0L;
   instr = YES;
   set_reg();
   yyparse();
   if (! errflag) {
      optimize(opt);
      opt_done();
      init_rel();

      txtsize = dottxt;
      if (uptxt = txtsize % SCTALIGN)
         txtsize += (uptxt = SCTALIGN - uptxt);
      datsize = dotdat;
      if (updat = datsize % SCTALIGN)
         datsize += (updat = SCTALIGN - updat);
      bsssize = dotbss;
      if (upbss = bsssize % SCTALIGN)
         bsssize += (upbss = SCTALIGN - upbss);

      fixsymval(0L,txtsize,DAT);
      fixsymval(0L,txtsize+datsize,BSS);

      bufhead();
      symindex();
      headers();
      dot.value = newdot = 0L;
      translate(TXT,uptxt);
      translate(DAT,updat);
      dumprel();
      dumplnno();
      dumpsym();
      dumpstr();
      }
}

chsegmt(segment)
short segment;
{
   switch (dot.segmt) {
   case TXT:
      dottxt = newdot;
      break;
   case DAT:
      dotdat = newdot;
      break;
   case BSS:
      dotbss = newdot;
      break;
      }
   switch (segment) {
   case TXT:
      newdot = dottxt;
      break;
   case DAT:
      newdot = dotdat;
      break;
   case BSS:
      newdot = dotbss;
      break;
      }
   dot.segmt = segment;
   dot.value = newdot;
}
