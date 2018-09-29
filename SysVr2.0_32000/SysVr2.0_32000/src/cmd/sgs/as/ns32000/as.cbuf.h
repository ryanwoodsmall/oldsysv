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
# define FLUSH 0
# define GENINST 1
# define GENVALUE 2
# define GENRELOC 3
# define GENPCREL 4
# define GENIMVAL 5
# define GENSTMT 6
# define GENRLVAL 7
# define GENNRDSP 8

typedef struct cbuf_node {
   short cn;
   short cact;
   long cval;
   SYM *cpsym;
   SYM *cnsym;
   } CBUF;

# define CBUFSIZE sizeof(CBUF)
# define CBLEN 512

typedef struct clist_node {
   struct clist_node *link;
   CBUF *cptr;
   short cnt;
   CBUF cblock[CBLEN];
   } CLIST;

# define CLISTSIZE sizeof(CLIST)
