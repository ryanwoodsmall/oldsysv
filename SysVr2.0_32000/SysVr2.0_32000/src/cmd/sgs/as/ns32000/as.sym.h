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
/* segmt types */
# define UNK 000
# define ABS 001
# define TXT 002
# define DAT 003
# define BSS 004
# define TYPE 007
# define EXT 010

# define MAXDIM 4

typedef struct aux_node AUX;
typedef struct sym_node SYM;

typedef struct inst_node {
   char mnemon[9];
   char format[24];
   short itype;
   short stype;
   } INST;

# define INSTSIZE sizeof(INST)

struct sym_node {
   struct sym_node *next;
   char *name;
   short segmt;
   long value;
   SYM *xsym;
   short type;
   short scl;
   long index;
   AUX *aux;
   short access;
   };

# define SYMSIZE sizeof(SYM)


struct aux_node {
   SYM *tag;
   unsigned short lnno;
   unsigned short size;
   union {
      struct {
         SYM *etag;
         int loff;
         } a_fn;
      unsigned short dim[MAXDIM];
      } a_m;
   };

# define AUXSIZE sizeof(AUX)

/* access types ( other than YES & NO ) */
# define TAG 2

typedef struct lnno_node LNNO;

struct lnno_node {
   int ad;
   int no;
   LNNO *link;
   };

# define LNNOSIZE sizeof(LNNO)
typedef struct str_node STR;

struct str_node {
   char *s_str;
   STR *s_lnk;
   } ;

# define STRSIZE sizeof(STR)
