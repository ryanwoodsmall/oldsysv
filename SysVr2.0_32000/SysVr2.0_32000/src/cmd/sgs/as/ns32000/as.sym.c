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
# define SYMLOOP(x,y) for(x=0;x<256&&full[x]>0;x++) for(y=0;y<full[x];y++)
/* ordering definitions */
# define O_TYPES 001
# define O_LOCAL 002
# define O_STATIC 004
# define O_GLOBAL 010
# define O_DECLRD 020
# define O_UNDEF 040

# include <stdio.h>
# include "syms.h"
# include "as.sym.h"
# include "as.addr.h"
# include "y.tab.h"


# define NHASH 373
# define YES 1
# define NO 0

extern char *cfile;
extern long txtsize, datsize;

AUX *getaux();
char *strcpy();

long FILINDX, TXTINDX, DATINDX, BSSINDX;

char *cond[] = {"eq","ne","cs","cc","hi","ls","gt",
                "le","fs","fc","lo","hs","lt","ge"};

INST _INTAB[] = {
# include "as.ops.h"
};

INST *itabl[] = {
   &_INTAB[0], &_INTAB[12], &_INTAB[21], &_INTAB[35], &_INTAB[39],
   &_INTAB[43], 0, 0, &_INTAB[46], &_INTAB[50],
   0, &_INTAB[52], &_INTAB[57], &_INTAB[78], &_INTAB[82],
   0, &_INTAB[83], &_INTAB[84], &_INTAB[93], &_INTAB[108],
   0, 0, &_INTAB[110], &_INTAB[112], 0, 0
   };

long nsyms = 0;
SYM *table[NHASH];

extern short intbits, fltbits, ccode;

SYM *sym_tab[256];
short full[256];
int level = 0;

SYM *
newsym() {
   SYM *p;
   int i;

   if ( full[level] >= 256 )
      if (++level >= 256)
         error("symbol table full");
   if (! sym_tab[level]) {
      sym_tab[level] = (SYM *) allocate(256 * SYMSIZE);
      full[level] = 0;
      }
   p = sym_tab[level]+full[level]++;
   p->aux = (AUX *) 0;
   p->segmt = UNK;
   p->xsym = (SYM *) 0;
   p->value = 0;
   p->next = (SYM *) 0;
   p->scl = 0;
   p->type = 0;
   return p;
}

SYM *
lookup(name,mode)
   char *name;
   int mode;
{
   int index;
   SYM *p;

   if ( p = table[index = hash(name)] ) {
      for ( ; p != (SYM *) 0; p = p->next)
         if ( (mode & p->access) && (! strcmp(name,p->name)) )
            return p;
      p = newsym();
      p->name = strcpy(allocate(1+strlen(name)),name);
      p->access = mode & 1;
      p->next = table[index];
      table[index] = p;
      return p;
      }
   p = table[index] = newsym();
   p->name = strcpy(allocate(1+strlen(name)),name);
   p->access = mode & 1;
   return p;
}

SYM *L_tag;
int F_glb;
SYM *L_bb[10];
SYM *L_bf;
SYM *L_fn;
int I_bb = 0;

spec_sym(p)
   SYM *p;
{
   switch ( p->scl ) {
   case C_EOS:
      L_tag->aux->tag = p;
      return;
   case C_STRTAG: case C_UNTAG: case C_ENTAG:
      L_tag = p;
      return;
   case C_FCN:
      if ( p->name[1] == 'b')
         L_bf = p;
      else {
         L_bf->aux->tag = p;
         L_fn->aux->a_m.a_fn.etag = p;
         }
      return;
   case C_BLOCK:
      if ( p->name[1] == 'b')
         L_bb[I_bb++] = p;
      else
         L_bb[--I_bb]->aux->tag = p;
      return;
      }
   if ( ISFCN(p->type) ) {
      if (! p->aux)
         p->aux = getaux();
      L_fn = p;
      add_lnno(p,0);
      }
}

int scn_map[] = { 0, -1, 1, 2, 3 };

int
scl_map(scl)
   int scl;
{
   switch ( scl ) {
   case C_EXT: case C_STAT: case C_LABEL:
      return 0;
   case C_AUTO: case C_REG: case C_MOS: case C_ARG: case C_MOU: case C_MOE:
   case C_REGPARM: case C_FIELD: case C_EOS:
      return N_ABS;
   case C_STRTAG: case C_UNTAG: case C_TPDEF: case C_ENTAG:
   case C_FILE: case C_ALIAS:
      return N_DEBUG;
   default:
      return 1;
      }
}

symwalk(acc,fun)
   int acc;
   int (*fun)();
{
   int lvl, i;
   SYM *p;

   SYMLOOP(lvl,i) {
      p = sym_tab[lvl]+i;

      if (acc & p->access)
         (*fun)(p);
      }
}

setindex(p)
   SYM *p;
{
   p->index = nsyms++;
   if ( p->aux )
      nsyms++;
}

putsym(p)
   SYM *p;
{
   struct syment s;
   int i;

   if ( strlen(p->name) > SYMNMLEN ) {
      s.n_zeroes = 0;
      s.n_offset = add_str(p->name);
      }
   else
      strncpy(s.n_name,p->name,SYMNMLEN);
   s.n_value = p->value;
   if (! (s.n_scnum = scl_map(p->scl)))
      s.n_scnum = scn_map[p->segmt & TYPE];
   s.n_type = p->type;
   s.n_sclass = p->scl;
   s.n_numaux = ( p->aux ) ? 1 : 0;
   s.n_env = 0;
   s.n_dummy = 0;
   fwrite(&s, SYMESZ, 1, stdout);
   if ( p->aux )
      putaux(p);
}

putaux(p)
   SYM *p;
{
   union auxent a;
   int i;

   strncpy(&a,"",SYMESZ);
   switch ( p->scl ) {
   case C_STRTAG: case C_UNTAG: case C_ENTAG:
      a.x_sym.x_misc.x_lnsz.x_size = p->aux->size;
      a.x_sym.x_fcnary.x_fcn.x_endndx = p->aux->tag->index + 2;
      break;
   case C_EOS:
      a.x_sym.x_misc.x_lnsz.x_size = p->aux->size;
      a.x_sym.x_tagndx = p->aux->tag->index;
      break;
   case C_BLOCK: case C_FCN:
      a.x_sym.x_misc.x_lnsz.x_lnno = p->aux->lnno;
      if ( p->name[1] == 'b' )
         a.x_sym.x_fcnary.x_fcn.x_endndx = p->aux->tag->index + 2;
      break;
   default:
      if ( p->aux->tag )
         a.x_sym.x_tagndx = p->aux->tag->index;
      if ( ISFCN(p->type) ) {
         a.x_sym.x_misc.x_fsize = p->aux->size;
         a.x_sym.x_fcnary.x_fcn.x_endndx = p->aux->a_m.a_fn.etag->index + 2;
         a.x_sym.x_fcnary.x_fcn.x_lnnoptr = p->aux->a_m.a_fn.loff;
         break;
         }
      a.x_sym.x_misc.x_lnsz.x_size = p->aux->size;
      a.x_sym.x_misc.x_lnsz.x_lnno = p->aux->lnno;
      for ( i = 0; i < MAXDIM; i++)
         a.x_sym.x_fcnary.x_ary.x_dimen[i] = p->aux->a_m.dim[i];
      break;
      }
   fwrite(&a,SYMESZ,1,stdout);
}

dumpsym() {
   int lvl, i;
   SYM *p;
   int infunc = 0;
   struct syment s;
   union auxent a;

   strncpy(s.n_name,".file",SYMNMLEN);
   s.n_value = F_glb;
   s.n_scnum = N_DEBUG;
   s.n_type = T_NULL;
   s.n_sclass = C_FILE;
   s.n_numaux = 1;
   s.n_env = 0;
   s.n_dummy = 0;
   fwrite(&s,SYMESZ,1,stdout);
   if (! cfile)
      cfile = "";
   memset(&a, 0, SYMESZ);
   strncpy(a.x_file.x_fname,cfile,FILNMLEN);
   fwrite(&a,SYMESZ,1,stdout);
   symwalk(O_TYPES,putsym);
   symwalk(O_LOCAL,putsym);
   symwalk(O_STATIC,putsym);
   strncpy(s.n_name,".text",SYMNMLEN);
   s.n_value = 0;
   s.n_scnum = 1;
   s.n_type = 0;
   s.n_sclass = C_STAT;
   s.n_numaux = 1;
   fwrite(&s,SYMESZ,1,stdout);
   scn_aux(&a,1);
   fwrite(&a,SYMESZ,1,stdout);
   strncpy(s.n_name,".data",SYMNMLEN);
   s.n_value = txtsize;
   s.n_scnum = 2;
   fwrite(&s,SYMESZ,1,stdout);
   scn_aux(&a,2);
   fwrite(&a,SYMESZ,1,stdout);
   strncpy(s.n_name,".bss",SYMNMLEN);
   s.n_value += datsize;
   s.n_scnum = 3;
   fwrite(&s,SYMESZ,1,stdout);
   scn_aux(&a,3);
   fwrite(&a,SYMESZ,1,stdout);
   symwalk(O_GLOBAL,putsym);
   symwalk(O_DECLRD,putsym);
   symwalk(O_UNDEF,putsym);
}

symindex() {
   int lvl, i;
   SYM *p;
   int infunc = 0;
   int intag = 0;

   SYMLOOP(lvl,i) {
      p = sym_tab[lvl]+i;
      if (! p->name)
         continue;
      p->access = ( p->access != 1 );   /* mark entries created by .def's */
      if ( p->xsym ) {
         if ( p->xsym->segmt & TYPE ) {
            if ( p->segmt & TYPE )
               if ( (p->segmt & TYPE) != (p->xsym->segmt & TYPE) ) {
                  fprintf(stderr,"symbols %s and %s not in same segment\n",
                     p->name, p->xsym->name);
                  error("cannot resolve symbol value");
                  }
            if (! strcmp(p->name, p->xsym->name) ) {
               p->xsym->name = (char *) 0;
               p->xsym->access = 0;
               p->xsym->xsym = p;
               }
            }
         p->segmt |= p->xsym->segmt;
         p->value += p->xsym->value;
         p->xsym = (SYM *) 0;
         }
      if (p->type == 0 && p->scl == 0) {
         if ( (p->segmt & TYPE) && ! (p->segmt & EXT) )
            p->scl = C_STAT;
         else
            p->scl = C_EXT;
         p->type = T_NULL;
         }
      if ((! infunc) && ISFCN(p->type) )
         infunc = 1;
      if ((! intag) && ISTAG(p->scl) )
         intag = 1;
      if ( p->access ) {        /* .def symbol */
         if ( infunc ) {
            p->access = O_LOCAL;
            if (p->scl == C_FCN && p->name[1] == 'e')
               infunc = 0;
            }
         else if ( intag ) {
            p->access = O_TYPES;
            if (p->scl == C_EOS)
               intag = 0;
            }
         else if ( (p->segmt & TYPE) == UNK )
            p->access = O_DECLRD;
         else
            p->access = 0;
         }
      if (! p->access) {        /* normal symbols */
         if ( p->scl == C_STAT )
            p->access = O_STATIC;
         else if ( p->segmt & TYPE )
            p->access = O_GLOBAL;
         else /*  p->scl == C_EXT && (p->segmt & TYPE) == UNK */
            p->access = O_UNDEF;
         }
      if ( *(p->name) == '.' && p->scl == C_STAT)
         p->access = 0;
      }
   nsyms += 2; /* .file entry */
   symwalk(O_TYPES,setindex);
   symwalk(O_LOCAL,setindex);
   symwalk(O_STATIC,setindex);
   TXTINDX = nsyms; nsyms += 2;
   DATINDX = nsyms; nsyms += 2;
   BSSINDX = nsyms; nsyms += 2;
   F_glb = nsyms;
   symwalk(O_GLOBAL,setindex);
   symwalk(O_DECLRD,setindex);
   symwalk(O_UNDEF,setindex);
}

AUX *
getaux() {
   AUX *a;
   int i;

   a = (AUX *) allocate(AUXSIZE);
   memset(a, 0, AUXSIZE);
}

INST *
find(name)
   char *name;
{
   INST *index;
   char *c, *s, sh;

   if ( *name < 'a' || *name > 'z' || (index = itabl[*name-'a']) == (INST *) 0)
      return (INST *) 0;
   c = index->mnemon;
   do {
      for (s = name; sh = *s; s++, c++) {
         if (*c == 'I') {
            if (sh != 'b' && sh != 'w' && sh != 'd')
               break;
            intbits = sh == 'b' ? 0 : ( sh == 'w' ? 1 : 3 );
            }
         else if (*c == 'F') {
            if (sh != 'f' && sh != 'l')
               break;
            fltbits = sh == 'f' ? 1 : 0;
            }
         else if (*c == 'C') {
            for (ccode = 0; ccode < 14; ccode++)
               if (sh == *cond[ccode])
                  if (*(s+1) == cond[ccode][1])
                     break;
            if (ccode == 14)
               break;
            s++;
            }
         else if (*c != sh)
            break;
         }
      if (*c == 'B') { intbits = 0; c++; }
      else if (*c == 'W') { intbits = 1; c++; }
      else if (*c == 'D') { intbits = 3; c++; }
      if ( (! *s) && (! *c) )
         return index;
      c = (++index)->mnemon;
      } while (*c == *name);
   return (INST *) 0;
}

int
hash(name)
   char *name;
{
   int val;

   for (val = 0; *name; name++)
      val += (int) *name;
   return val % NHASH;
}

fixsymval(addr,incr,type)
   long addr, incr;
   short type;
{
   int lvl, i;
   register SYM *p;

   SYMLOOP(lvl,i) {
      p = sym_tab[lvl]+i;

      if (! *(p->name))
         continue;
      if ((p->segmt & TYPE) == type && p->value >= addr)
         p->value += incr;
      }
}

readonly()
{
   int lvl, i;
   SYM *p;

   SYMLOOP(lvl,i) {
      p = sym_tab[lvl]+i;

      if ((p->segmt & TYPE) == DAT)
         p->segmt =TXT | p->segmt & ~TYPE;
      }
}
