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
/*	@(#)extern.h	1.2	*/
/*
 *  External data declarations
 */

extern MSG      ADWRAP;
extern MSG      BADCOM;
extern MSG      BADDAT;
extern MSG      BADEQ;
extern MSG      BADFIL;
extern MSG      BADKET;
extern MSG      BADLOC;
extern MSG      BADMAGIC;
extern MSG      BADMOD;
extern MSG      BADNAM;
extern MSG      BADRAD;
extern MSG      BADSYM;
extern MSG      BADSYN;
extern MSG      BADTXT;
extern MSG      BADVAR;
extern MSG      BADWAIT;
extern MSG      ENDPCS;
extern MSG      EXBKPT;
extern MSG      LONGFIL;
extern MSG      NOADR;
extern MSG      NOBKPT;
extern MSG      NOCFN;
extern MSG      NOEOR;
extern MSG      NOFORK;
extern MSG      NOMATCH;
extern MSG      NOPCS;
extern MSG      NOTOPEN;
extern MSG      SZBKPT;

extern int      adrflg;
extern int      adrval;
extern int      argcount;
extern BKPTR    bkpthead;
extern int      callpc;
extern int      cntflg;
extern int      cntval;
extern STRING   corfil;
extern int      datsiz;
extern int      ditto;
extern int      dot;
extern int      dotinc;
extern int      entrypt;
extern int      eof;
extern STRING   errflg;
extern int      executing;
extern int      expv;
extern int      fcor;
extern int      fsym;
extern int      infile;
extern char     lastc;
extern int      lastcom;
extern int      localval;
extern int      locval;
extern int      loopcnt;
extern STRING   lp;
extern short    magic;
extern int      maxfile;
extern int      maxoff;
extern int      maxpos;
extern unsigned maxstor;
extern int      mkfault;
extern int      outfile;
extern char     peekc;
extern int      pid;
extern MAP      qstmap;
extern int      radix;
extern REGLIST  reglist[];
extern REGLIST  freglist[];
extern SIGunion sigint;
extern int      signo;
extern SIGunion sigqit;
extern MAP      slshmap;
extern int      stksiz;
extern int      symbas;
extern SYMTAB   symbol;
extern STRING   symfil;
extern int      symnum;
extern SYMSLAVE *symvec;
extern int      trace;
extern int      txtsiz;
extern int      var[];
extern int      wtflag;
extern int locadr;
extern int numlocs;
