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
/* address modes */
# define AFREG 000
# define AREG 001
# define ARREL 002
# define AMREL 003
# define AEXPR 004
# define AEXTR 005
# define ATOS 006
# define AMSP 007
# define AINDX 010
# define AIMM 020

/* expression data types */
# define TYPB 001
# define TYPW 002
# define TYPL 003
# define TYPQ 004
# define TYPF 010
# define TYPD 020
# define TINT 007
# define TFLT 030
# define TSYM 040
# define TDFF 0100

typedef struct expr_node {
   short xtype;
   SYM *psym;
   SYM *nsym;
   long lngval;
   double dblval;
   } EXPR;

typedef struct opr_node {
   short mode;
   short reg1;
   short reg2;
   short scale;
   EXPR  disp1;
   EXPR  disp2;
   } OPR;
