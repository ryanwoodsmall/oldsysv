/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:util.c	1.21"

#include "../forms/muse.h"
#include "vdefs.h"

struct vfunc *addval();

/*
 * in_to_fl() is called from the master attribute form, and is
 * used to set multi-field valfuncs.  It prepares input for
 * the select() routine.  It is always called with the current
 * field as first argument. The second argument is the first
 * icmp pointer, at_least_one pointer, or requires pointer.
 * Its output is an allocated linked list of fieldlink pointers.
 */

struct fieldlink *in_to_fl(f_pt,code)
struct field *f_pt;             /* Current field                      */
char code;                      /* first_i_pt, first_a_pt, first_m_pt */
{  struct index *in0_pt, *in_pt;
   struct fieldlink *fl, *fl0;

   if (f_pt == (struct field*)0) return((struct fieldlink *)0);

   switch(code) {
      case 'i':
        in_pt = f_pt->first_i_pt;
        break;
      case 'a':
        in_pt = f_pt->first_a_pt;
        break;
      case 'm':
        in_pt = f_pt->first_m_pt;
        break;
      default:
        return((struct fieldlink *)0);
        break;
   }
   fl = (struct fieldlink *)malloc(sizeof(struct fieldlink));
   fl->f_pt = f_pt;
   fl->next = (struct fieldlink*)0;

   if (in_pt==NULL) return(fl);
   
   for (in0_pt=in_pt, fl0=fl; in0_pt!=NULL; in0_pt = in0_pt->next) {
      fl0 = fl0->next =
           (struct fieldlink *)malloc(sizeof(struct fieldlink));
      fl0->f_pt = fields+in0_pt->num;
      fl0->next = NULL;
   }
   return(fl);
}


/*
 * fl_to_in() is called from the master attribute form, and is
 * used to set multi-field valfuncs.  It processes output from
 * the select() routine.  It is always called with the current
 * field as first argument. The third argument is the first
 * icmp pointer, at_least_one pointer, or requires pointer.
 * The second argument is the output from select().
 */

int fl_to_in(f_pt,fl,code)
struct field *f_pt;             /* Current field                      */
struct fieldlink *fl;           /* Output from select()               */
char code;                      /* first_i_pt, first_a_pt, first_m_pt.
                                   Must have been allocated.          */
{  struct index *in0_pt, **in_pt_pt;
   struct fieldlink *fl0=fl, *fl1;

   if (f_pt == (struct field*)0) return(0);
   if (fl ==(struct fieldlink*)0) return(0);

   switch(code) {
      case 'i':
        in_pt_pt = &f_pt->first_i_pt;
        break;
      case 'a':
        in_pt_pt = &f_pt->first_a_pt;
        break;
      case 'm':
        in_pt_pt = &f_pt->first_m_pt;
        break;
      default:
        return(0);
        break;
   }

   if (fl->next == NULL) {
      *in_pt_pt = NULL;
      return(1);
   }

   if (*in_pt_pt == NULL)
      *in_pt_pt = (struct index *)malloc(sizeof(struct index));

   for (fl0=fl->next, in0_pt = *in_pt_pt; fl0!=NULL; fl0 = fl1) {
      in0_pt->num = (int)(fl0->f_pt - fields);
      if ((fl1 = fl0->next) != NULL)
         in0_pt = in0_pt->next =
           (struct index *)malloc(sizeof(struct index));
      in0_pt->next = NULL;
      free(fl0);
   }
   return(1);
}


int set_icmp(f_pt)
register struct field *f_pt;
{  char s[256];
   struct fieldlink *in_to_fl(), *select(), *fl;
   struct index *in_pt, *addidx();
   struct field *f0_pt;

   strcpy(s,"Select Fields Incompatible With Current Field");

   if ((fl = in_to_fl(f_pt,'i')) == (struct fieldlink *)0) return(0);
   if ((fl = select(fl,s,1)) == (struct fieldlink *)0) return(0);

/*
 * Remove all old "links" of other fields with f_pt.
 */
   for (in_pt = f_pt->first_i_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields+in_pt->num;
/*    Remove all references to f_pt */
      while (rm_idx(&(f0_pt->first_i_pt),(int)(f_pt-fields)) == 1);
/*    If current field is only field referred to, remove vfunc
      alltogether */
      if (f0_pt->first_i_pt == (struct index*)0)
         while (rmvaln(f0_pt,5)==1);
   }

/*
 * Done if fl_to_in fails or first_i_pt has nothing.
 */
   while (rmvaln(f_pt,5)==1);
   if (fl_to_in(f_pt,fl,'i') == 0) return(0);
   if (f_pt->first_i_pt==NULL) return(1);
   addval(f_pt,5);

/*
 * Next, establish new links.  Symmetrisize.
 */

   for (in_pt=f_pt->first_i_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields + in_pt->num;
      while (rmvaln(f0_pt,5) == 1);
      addval(f0_pt,5);
      f0_pt->first_i_pt = addidx((f0_pt->first_i_pt),(int)(f_pt-fields));
   }



   return(1);
}


int set_atlone(f_pt)
register struct field *f_pt;
{  char s[256];
   struct fieldlink *in_to_fl(), *fl, *select();
   struct index *in_pt, *addidx();
   struct field *f0_pt;

   sprintf(s,"Select \"At Least One\" Fields");

   if ((fl = in_to_fl(f_pt,'a')) == (struct fieldlink *)0) return(0);
   if ((fl = select(fl,s,1)) == (struct fieldlink *)0) return(0);

/*
 * Remove all old "links" of other fields with f_pt.
 */
   for (in_pt = f_pt->first_a_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields+in_pt->num;
/*    Remove all references to f_pt */
      while (rm_idx(&(f0_pt->first_a_pt),(int)(f_pt-fields)) == 1);
/*    If current field is only field referred to, remove vfunc
      alltogether */
      if (f0_pt->first_a_pt == (struct index*)0)
         while (rmvaln(f0_pt,31)==1);
   }

/*
 * Done if fl_to_in fails or first_a_pt has nothing.
 */
   while (rmvaln(f_pt,31)==1);
   if (fl_to_in(f_pt,fl,'a') == 0) return(0);  /* Error in fl_to_in() */
   if (f_pt->first_a_pt==NULL) return(1);
   addval(f_pt,31);

/*
 * Next, establish new links.  Symmetrisize.
 */

   for (in_pt=f_pt->first_a_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields + in_pt->num;
      while (rmvaln(f0_pt,31) == 1);
      addval(f0_pt,31);
      f0_pt->first_a_pt = addidx((f0_pt->first_a_pt),(int)(f_pt-fields));
   }

   return(1);
}

int set_reqs(f_pt)
register struct field *f_pt;
{  char s[256];
   struct fieldlink *in_to_fl(), *fl, *select();
   struct field *f0_pt;
   struct index *in_pt;

   sprintf(s,"Select Fields Required By Current Field");

   if ((fl = in_to_fl(f_pt,'m')) == (struct fieldlink *)0) return(0);
   if ((fl = select(fl,s,1)) == (struct fieldlink *)0) return(0);

   for (in_pt = f_pt->first_m_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields+in_pt->num;
      while (rm_idx(&(f0_pt->first_m_pt),(int)(f_pt-fields)) == 1);
      if (f0_pt->first_m_pt == (struct index*)0)
         while (rmvaln(f0_pt,19) == 1);
   }

   while (rmvaln(f_pt,16) == 1);
   if (fl_to_in(f_pt,fl,'m') == 0) return(0);
   if (f_pt->first_m_pt==NULL) return(1);

   addval(f_pt,16);
   for (in_pt=f_pt->first_m_pt; in_pt!=NULL; in_pt=in_pt->next) {
      f0_pt = fields+in_pt->num;
      while (rmvaln(f0_pt,19) == 1);
      addval(f0_pt,19);
   }

   return(1);
}





/*
 * Adds new assigned vfunc pointer to linked list;
 * sets num equal to num, next to NULL.
 * Returns this pointer.
 */

struct vfunc *addval(f_pt,num)
struct field *f_pt;
int num;
{  struct vfunc *v_pt;

   if (f_pt == (struct field *)0) return((struct vfunc*)0);
   if (num<0 || num>35) return((struct vfunc*)0);

   if ((v_pt = f_pt->first_v_pt) == NULL)
      v_pt = f_pt->first_v_pt
           = (struct vfunc *)malloc(sizeof(struct vfunc));
   else {
      for (;v_pt->next!=NULL; v_pt=v_pt->next);
      v_pt = v_pt->next 
           = (struct vfunc *)malloc(sizeof(struct vfunc));
   }

   v_pt->num = num;
   v_pt->next= NULL;
   v_pt->pr_pt = v_pt->po_pt = NULL;
   return(v_pt);
}



int rmval(f_pt,v_pt)
struct field *f_pt;
struct vfunc *v_pt;
{  struct vfunc *v0_pt;

   if (f_pt==(struct field*)0) return(0);
   if (v_pt==(struct vfunc*)0) return(0);
   if (f_pt->first_v_pt==(struct vfunc*)0) return(0);

   if ((v0_pt = f_pt->first_v_pt) == v_pt)
      f_pt->first_v_pt = v0_pt->next;                  /* OK if NULL */

   else {
      for (v0_pt=f_pt->first_v_pt; v0_pt->next!=v_pt && v0_pt->next!=NULL; 
           v0_pt=v0_pt->next);
      if (v0_pt->next==NULL) return(0);
      v0_pt->next = v_pt->next;
   }
   return(1);
}


int rmvaln(f_pt,num)
struct field *f_pt;
int num;
{  struct vfunc *v0_pt;

   if (f_pt==(struct field*)0) return(0);
   if (f_pt->first_v_pt==(struct vfunc*)0) return(0);
   
   if ((v0_pt = f_pt->first_v_pt)->num == num)
      f_pt->first_v_pt = v0_pt->next;                  /* OK if NULL */

   else {
      for (v0_pt=f_pt->first_v_pt; v0_pt->next!=NULL && v0_pt->next->num!=num; 
           v0_pt=v0_pt->next);
      if (v0_pt->next==NULL) return(0);
      v0_pt->next = v0_pt->next->next;
   }
   return(1);
}

int location()
{  struct fieldlink *select();
   struct field *f_pt, *f0_pt;
   int *locs, retflag=1;

/*
 * Run the select() module, in "location" mode; return code does not
 * indicate abort, just return to TOP.
 */
   if (select((struct fieldlink*)0,
"Command Line Order [BU = bundled option]",
  2) == 0) retflag = 0;

/*
   locs = (int*)calloc((unsigned)((int)(last_field_pt-fields)+2),sizeof(int));

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++)  {
      if (f_pt->bundle!=1)  {
         f_pt->loc++;
         *(locs + (int)(f_pt-fields)) = 1;
      }
      else   {
         f_pt->loc = 0;
         *(locs + (int)(f_pt-fields)) = 0;
      }
   }

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++)
   for (f0_pt=fields; f0_pt<=last_field_pt; f0_pt++) 
      if (f_pt!=f0_pt && f_pt->loc>f0_pt->loc && f0_pt->bundle!=1) 
         (*(locs+(int)(f_pt-fields)))++;

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++) 
      f_pt->loc = *(locs+(int)(f_pt-fields));

   free(locs);
*/

   return(retflag);
}

/* Checks whether s has valid drwxrwxrwx format. Not used.
*  permchk(v_pt)  
*  struct vfunc *v_pt;
*  {  char *c0_pt, *s, *c_pt;
*     char *pr = v_pt->pr_pt->name;
*     char *po = v_pt->po_pt->name;
*  
*     if (pr==NULL || po==NULL)
*        return(14);
*     s = pr;
*     while (s!=NULL) {
*        if (strlen(s)!=10) return(14);
*        c0_pt = "drwxrwxrwx";
*        c_pt = s;
*        for (; *c_pt; c_pt++, c0_pt++)
*          if (*c_pt != *c0_pt && *c_pt != '-') 
*             return(14);
*        s = (s==pr) ? po : NULL;
*     }
*     return(0);
*  }
*/


length_chk(v_pt)  /* Checks whether s is a 1 or 2 dig. number */
struct vfunc *v_pt;
{  char *pr;
   int num;

   if (v_pt==NULL || v_pt->pr_pt==NULL || v_pt->pr_pt->name==NULL) 
      return(15);
   pr = v_pt->pr_pt->name;
   if (*pr==null) return(0); /* OK to have nothing */
   if (strlen(pr)>2) return(15);
   if (!isdigit(*pr) || (*(pr+1)!=null && !isdigit(*(pr+1))))
      return(15);
   num = atoi(pr);
   if (num<0 || num>80)
      return(15);
   return(0);
}

haspre(v_pt)  /* Checks whether v_pt has >= 1 prefix */
struct vfunc *v_pt;
{  struct fix *pr_pt;
   int test = 0;

   if (v_pt == NULL) return(16);

   pr_pt = v_pt->pr_pt;
   for (;pr_pt!=NULL && test==0; pr_pt = pr_pt->next)
      if (pr_pt->name!=NULL)
         test = 1;
   if (test==0)
      return(16);
   return(0);
}

haspost(v_pt)  /* Checks whether v_pt has >= 1 postfix */
struct vfunc *v_pt;
{  struct fix *po_pt;
   int test = 0;

   if (v_pt==NULL) return(16);
   po_pt = v_pt->po_pt;
   test = 0;
   for (;po_pt!=NULL && test==0; po_pt = po_pt->next)
      if (po_pt->name!=NULL)
         test = 1;
   if (test==0)
      return(16);

   return(0);
}

int chckstr(s,flag)
char *s;
int flag;
{  register int Yflag = 0, count=0;
   if (s==NULL || *s==null) return(0);
   do {
      count++;
      switch(*s) {
      case CTRL(X):
         return(20);
         break;
      case CTRL(Z):
         return(21);
         break;
      case CTRL(R):
         return(22);
         break;
/*    OK: would harm only if put prior to real ^E in command form,
          which is impossible.
      case CTRL(E):
         return(23);
         break;
*/
      case CTRL(J):
         if (flag==0) return(24);
         break;
      case CTRL(M):
         if (flag==0) return(25);
         break;
      case CTRL(Y):
         Yflag++;
         break;
      default:
         break;
      }
   }
   while (*++s);
   if (count>506) return(53);
   if (Yflag%2) return(26);
   return(0);
}

int chckchar(ch)
char ch;
{ 
   switch(ch) {
   case CTRL(X):
      return(20);
      break;
   case CTRL(Z):
      return(21);
      break;
   case CTRL(R):
      return(22);
      break;
/*    OK: would harm only if put prior to real ^E in command form,
          which is impossible.
   case CTRL(E):
      return(23);
      break;
*/
   case CTRL(J):
      return(24);
      break;
   case CTRL(M):
      return(25);
      break;
   default:
      break;
   }

   return(0);
}

/* Not used.
*  struct fix *get_dflt(f_pt)
*  struct field *f_pt;
*  {  struct fix *cpo_pt=f_pt->first_cpo_pt;
*     if (cpo_pt == NULL || mode==MENU || (f_pt->type!=7 && f_pt->type!=1))
*        return((struct fix*)0);
*     while (cpo_pt!=NULL && cpo_pt->next!=NULL) {
*        if (*cpo_pt->next->name == null) return(cpo_pt);
*        cpo_pt = cpo_pt->next->next;
*     }
*     return((struct fix*)0);
*  }
*/


int isinpath(t)
char *t;
{  int test=1;
   char *s0, *s, **b;
   char eds[128], *getenv();

   s0 = s = getenv("PATH");
   b = binpaths;
   if (s==NULL) *b=NULL;
   else while (test)
   {
      if (*s==':' || *s==null)
      {
         *b = (char*)calloc((unsigned)(s-s0+1),sizeof(char)); /* Also 0's *b */
         strncpy(*b++,s0,s-s0);
         *b = NULL;
         s0 = s+1;
         if (*s==null) test=0;
      }
      s++;
   }

   test = 0;
   if (access(eds,04)==0 && access(eds,01)==0) test=1;   
                                /* In current dir; ok even if no PATH */
   else for (b=binpaths; *b!=NULL && test==0; b++) {
      if (**b!=null)
         sprintf(eds,"%s/%s",*b,t);
      else (strcpy(eds,t));
      if (access(eds,04)==0 && access(eds,01)==0) test=1;
   }

   return(test);
}

int chckformat(v_pt,flag)
struct vfunc *v_pt;
int flag;
{  char *c_pt;
   struct fix *pr_pt;
   
   if (flag==0) {   /* Check ^P's */
      if (v_pt==NULL || v_pt->pr_pt==NULL || 
          (c_pt=v_pt->pr_pt->name)==NULL)
         return(0);   /* No ^P */
      else {
         for (pr_pt=v_pt->pr_pt; pr_pt!=NULL; pr_pt=pr_pt->next)
            for (c_pt = pr_pt->name; *c_pt; c_pt++)
               if (*c_pt==CTRL(P)) return(1);
         return(0);
      }
   }
   else {  /* Check whether there is a postfix */
      if (v_pt==NULL || v_pt->po_pt==NULL || 
          (c_pt=v_pt->po_pt->name)==NULL || *c_pt==null)
      return(1);   /* No postfix */
      else return(0);  /* Postfix */
   }
}


VOID rm_ref(f_pt,t)  /* Removes all references to an removed field 
                        from all icmp, req, and a_l_o lists */
struct field *f_pt;
int *t;
{  struct field *f0_pt;
   int *t_pt = t;

   for (f0_pt = fields; f0_pt<=last_field_pt; f0_pt++, t_pt++)
      if (*t_pt!=0)  {
      if (f0_pt->first_i_pt!=(struct index*)0)
         rm_idx(&(f0_pt->first_i_pt),(int)(f_pt-fields));
      if (f0_pt->first_a_pt!=(struct index*)0)
         rm_idx(&(f0_pt->first_a_pt),(int)(f_pt-fields));
      if (f0_pt->first_m_pt!=(struct index*)0)
         rm_idx(&(f0_pt->first_m_pt),(int)(f_pt-fields));
      if (f0_pt->first_i_pt == (struct index*)0) rmvaln(f0_pt,5);
      if (f0_pt->first_a_pt == (struct index*)0) rmvaln(f0_pt,31);
      if (f0_pt->first_m_pt == (struct index*)0) rmvaln(f0_pt,16);
   }
}

rm_idx(in_pt_pt,fn)   /* Removes index matching fn from linked index 
                         list pointed at by in_pt_pt   */
struct index **in_pt_pt;
int fn;
{  struct index *in0_pt;

   in0_pt = *in_pt_pt;

   if (in0_pt == (struct index *)0) return(0);

   if (in0_pt->num == fn) {   /* fn matches first index */
      *in_pt_pt = in0_pt->next;
      return(1);
   }
   
   for (; in0_pt->next != (struct index *)0 && in0_pt->next->num != fn;
      in0_pt = in0_pt->next);

   if (in0_pt->next == (struct index *)0) return(0);  /* fn not found */

   in0_pt->next = in0_pt->next->next;  /* fm found in in0_pt->next */

   return(1);
}


 /* Removes multiple validations ; not used
*  v_double(f_pt) 
*  struct field *f_pt;
*  {  int vals[50], i;
*     struct vfunc *v_pt;
*  
*     for (i=0; i<50; i++) vals[i] = 0;
*  
*     for (v_pt = f_pt->first_v_pt; v_pt != (struct vfunc *)0; 
*          v_pt = v_pt->next)
*        vals[v_pt->num]++;
*     
*     for (i=0; i<50; i++) {
*        while (vals[i] > 1) {
*           rmvaln(f_pt,i);
*           vals[i]--;
*        }
*     }
*  }
*  
*/
struct index *addidx(in_pt,num)
struct index *in_pt;
int num;
{  struct index *in0_pt;

   if (num<0 || num>MAXFIELDS) return((struct index*)0);

   if (in_pt == NULL)
      in0_pt = in_pt 
             = (struct index *)malloc(sizeof(struct index));
   else {
      for (in0_pt = in_pt; in0_pt->next!=NULL; in0_pt=in0_pt->next);
      in0_pt = in0_pt->next 
             = (struct index *)malloc(sizeof(struct index));
   }

   in0_pt->num = num;
   in0_pt->next= NULL;
   return(in_pt);
}


