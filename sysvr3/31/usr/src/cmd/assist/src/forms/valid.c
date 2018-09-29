/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:valid.c	1.27"

/* valid.c -- validation functions.
 * val_all() runs all validation functions for all segments
 * in all fields.
 * validate runs all validation functions for a given
 * segment.  It is called by val_all() and by expand() [in expand.c].
 * Validation functions are stored in an array (*valfuncs[]()).
 */

#include "muse.h"
#include "mmuse.h"

struct vfunc *V_pt;

extern char *visible[];

int show_segm;

int prepoexist(), length14(), exists(), writeperm();
int prepo(), incomp(), cp_ok(), notsame();
int format(), ownfile(), isdir(), notdir();
int cdirwp(), noexist(), existread();
int existstr(), requires(), matchlist();
int notprepo(), requiredby(), prepo_noexist(),strlist();
int splitlist();
int superf(), superf_1(), nomatch(), existwrite(), regulx();
int mustquote(), filelen();
int no_n_match(), least_one(), shval(), permission(), dpermission();
int version3();
int isset();
int validate();
int (*val_funcs[])() = /* Array of validation functions                       */
{
   prepoexist,         /* 0: Verifies whether string w/pre- or post-fix exists*/
                       /* Not used in astgen */

   length14,           /* 1:                                   14             */
                       /* astgen name: Be no longer than specified length     */

   exists,             /* 2:           file exists and has read permission    */
                       /* astgen name: Exist */

   writeperm,          /* 3: Warns if writeperm existing file                 */
                       /* astgen name: Be writable or can be created  */

   matchlist,          /* 4: Verifies  string is equal to one of NFIX prefixes*/
                       /* astgen name: Match specified string(s)  */

   incomp,             /* 5: Highlights incompatible "y"'s                    */
                       /* Called in astgen incompitible fields module */

   cp_ok,              /* 6: For cp-like commands: check whether: target file is
                          directory OR there is only one input file.
                          Assumes that target file field immediately follows
                          input file field.                                   */
                       /* Not used in astgen */

   notsame,            /* 7: For cp-like commands, checks whether input file 
                             matches target file.                             */
                       /* astgen name: Differ from file in last field  */

   format,             /* 8: Tests whether string is of specfied format */
                       /* astgen name: Match specified format(s)  */

   ownfile,            /* 9: Tests whether file is owned by current user.     */
                       /* astgen name: Be owned by user. */

   isdir,              /*10: Tests whether name is existing directory.        */
                       /* Not used in astgen */

   cdirwp,             /*11: Tests whethe current directory has write perm.   */
                       /* Not used in astgen */

   noexist,            /*12: Tests whether file does not exist.               */
                       /* astgen name: Not exist. */

   notdir,             /*13: Tests whether name is not existing directory.    */
                       /* Not used in astgen */

   existread,          /*14: Warns if file does not exist of does not have
                             read permission.                                 */
                       /* Not used in astgen */

   existstr,           /*15: Checks if name = one of several strings;
                           if not, runs exists().     */
                       /* astgen name: Be readable or match string(s) */

   requires,           /*16: Like incomp, for conditionally mandatory options */
                       /* Called in astgen required fields module */

   prepo,              /*17: Verifies  string begins with one of NFIX prefixes*/
                       /* astgen name: Start/end with specified string(s) */

   notprepo,           /*18: Verifies  string does not begin with one of 
                             NFIX prefixes    */
                       /* astgen name: Not start/end with specified string(s) */

   requiredby,         /*19: Checks whether option is set in current field,
                             as dictated by cman().                           */
                       /* Called in astgen required fields module */

   prepo_noexist,      /*20: Negative of prepoexist                           */
                       /* Not used in astgen */

   strlist,            /*21: Tests whether input is either "a" or
                             "asbs...sz", where s is an arbitrary character and
                              a,b,... are arbitrary-length strings.           */
                       /* Not used in astgen */

   superf,             /*22: Not implemented */
                       /* Not used in astgen */

   superf_1,           /*23: Not implemented */
                       /* Not used in astgen */

   nomatch,            /*24: Negative of matchst                              */
                       /* astgen name: Not match specified string(s) */

   existwrite,         /*25: File has write permission                        */
                       /* Not used in astgen */

   regulx,             /*26: String is valid regular expression               */
                       /* astgen name: Be a regular expression */

   mustquote,          /*27: Checks whether string contains special
                             characters that require enclosing by
                             other characters, e.g., " or '.                  */
                       /* astgen name: Be quoted if it has specified char(s) */

   splitlist,          /*28: Parses string into substrings using
                             separator; then applies validation
                             functions. */
                       /* Not used in astgen */

   filelen,            /*29: Checks whether atoi(word)>#lines in file */
                       /* Not used in astgen */

   no_n_match,         /*30: Checks whether preceding and current 
                             fields share strings  */
                       /* Not used in astgen */

   least_one,          /*31: Checks whether at least one of a group
                             fields are set   */
                       /* Called in astgen at-least-one fields module */

   shval,              /*32: Runs shell */
                       /* astgen name: Run a shellscript */

   permission,         /*33: Checks whether file has/doesn't have
                           any specified (rwx) permission pattern */
                       /* Not used in astgen */

   dpermission,        /*34: Checks whether file has/doesn't have
                           access() permossion pattern.   */
                       /* astgen name: Have specified permission(s) */

   version3            /*35: Tests for version three */
                       /* astgen name: Version check */
};

/*
 * Routine that validates all segments in all fields, using validate().
 */
int val_all(upd)
int upd;   /* Obsolete. Always 0 */
{
   register int test = 1;
   register struct segment *s_pt, *save_s_pt;
   register struct field *f_pt, *save_f_pt;

   save_s_pt = Segm_pt;
   save_f_pt = Field_pt;
   for (f_pt=fields; f_pt<=last_field_pt && test!=0 && test!=3; f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9) /* No need to check headers.*/
      {
         Field_pt = f_pt;
         for (s_pt=f_pt->first_s_pt;
              s_pt!=NULL && test;
              s_pt=s_pt->next)
         {
            Segm_pt=s_pt;
            test = validate(upd,1);
            if (test==0 || test==3) /* No execution when INCORRECT or
                                       GRERROR is found */
            {
               Field_pt = f_pt;
               Segm_pt = s_pt;
               Row = Segm_pt->row;
               Col = Segm_pt->col;
               test = 0;
            }
         }
      }
   }
   if (test==1 || test==2)          /* Execution OK when at most some
                                       warnings */
   {
      Segm_pt=save_s_pt;
      Field_pt=save_f_pt;
   }
   return(test);
}


/*
 * Routine that applies all validation functiosn applicable to the
 * current segment, Segm_pt.
 */
int validate(upd,ss)
int upd;  /* Obsolete. Always 0 */
int ss;   /* Flag to determine if current user input is to be highlighted */
{  char *restore();
   register int test=1, ist;
   register int maxtest=0, musttest;

   show_segm = ss;

   *warn_mess = null;
   *error_mess = null;

   if (upd==UPDATE)
      update(Field_pt->last_row);

   /* First handle empty string case:                                  */
   if (Field_pt->minfsegms > 0)  
         if (Field_pt->first_s_pt->word==NULL ||
               *(Field_pt->first_s_pt->word)==null)
         {
            Segm_pt = Field_pt->first_s_pt;
            if (Field_pt->type==3 || Field_pt->type==6 
                                  || Field_pt->type==7)
               Segm_pt->word=restore(Field_pt);

            if (Segm_pt->word==NULL || *Segm_pt->word==null)
            {
               strcpy(error_mess," INPUT REQUIRED HERE ");
               mode=GRERROR;
               return(3);             /* At least one string required */
            }
         }

   if (Field_pt->first_v_pt==NULL) return(1); /*No validation functions */

/*
   if (!isset(Field_pt,Segm_pt)) return(1);
*/

/*
 * validate only if MV valfunc (5, 16, 19, 31) OR segment is set OR 
 * mustmatch flag is set.
 */

   ist = isset(Field_pt,Segm_pt);

   musttest = (ist ||
               (Field_pt->first_cpr_pt!=NULL &&
                Field_pt->first_cpr_pt->name!=NULL &&
                *(Field_pt->first_cpr_pt->name+3)=='1'));

   for (V_pt = Field_pt->first_v_pt; 
        V_pt!=NULL && test!=0 && test!=3; 
        V_pt = V_pt->next)
   {
      *error_mess = null;
      if (V_pt->num==5 || 
          V_pt->num==16 || 
          V_pt->num==19 || 
          V_pt->num==31 || 
          (musttest && ist) || 
          (musttest && V_pt->num != 35))
         test = (*val_funcs[V_pt->num])();
      maxtest = (maxtest>test) ? maxtest : test;
   }
   if (test==0 || test==3)
   {
      *warn_mess = null;
      mode = GRERROR;
      Col = Segm_pt->col;
      return(3);
   }

   if (maxtest==2) mode=WARN;
   return(maxtest);
}

/*
 * Routine that checks of for some prefix and some suffix,
 * <prefix><word><suffix> exists.
 */
int prepoexist()
{
   register struct fix *pr_pt, *po_pt;
   char s[100];
   char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   rmquotes(Segm_pt->word,val_str);
   for (pr_pt = V_pt->pr_pt; pr_pt!=NULL;  pr_pt=pr_pt->next)
   for (po_pt = V_pt->po_pt; po_pt!=NULL;  po_pt=po_pt->next)
   {
      strcpy(s,pr_pt->name);                    /* Empty string allowed.  */
      strcat(s,val_str);  
      strcat(s,po_pt->name);                    /* Empty string allowed.  */
      if (access(s,00)==0) return(1);
   }
   if (strlen(s) < 60)
      sprintf(error_mess," %s DOES NOT EXIST ",s);
   else
      sprintf(error_mess," DOES NOT EXIST ");
   return(0);
}

/*
 * Compute length -- after stripping basename.  Compare with
 * specified length; 14 is default.
 * astgen name: Be no longer than specified length.
 */
int length14()
{
   register int slen=14;
   register int len = 0;
   register char *c_pt;
   char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   strcpy(val_str,Segm_pt->word);
   if (V_pt->pr_pt != NULL) slen=atoi(V_pt->pr_pt->name);
   c_pt = val_str;
   while (*c_pt)
   {
      if (*c_pt++ == '/') len=0;
      else len++;
   }
   if (len <= slen) return(1);
   if (strlen(Segm_pt->word) + strlen(V_pt->pr_pt->name) < 50)
      sprintf(error_mess," %s HAS MORE THAN %d CHARACTERS ",Segm_pt->word,slen);
   else
      sprintf(error_mess," HAS MORE THAN %d CHARACTERS ",slen);
   return(0);
}

/*
 * Check is <word> is existing file.
 * astgen name: Exist.
 */
int exists()
{  char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (access(val_str,00)!=0)
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," DOES NOT EXIST ");
      return(0);
   }
   return(1);
}


/*
 * Checks if <word> can be written onto or created.
 * Distinguishes between several cases.
 * astgen name: Be writable or can be created.
 */
int writeperm()
{  struct stat buf;
   int stat_return;
   char val_str[128];
   char s[128], *c_pt, v[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   stat_return = stat(val_str,&buf);
   if (strlen(val_str) <40) strcpy(v,val_str);
   else *v = null;

   if (stat_return==0)   /* File or directory exists */
   {
      if (buf.st_mode & S_IFDIR)
      {
         sprintf(error_mess," CANNOT BE A DIRECTORY ");
         return(0);
      }
      else if (access(val_str,02) != 0)
      {
         sprintf(error_mess," FILE %s HAS NO WRITE PERMISSION ",v);
         return(0);
      }
      else
      {
         sprintf(warn_mess," WARNING: WILL OVERWRITE %s ",v);
         return(2);
      }
   }
   else /* file does not exist; check whether path has
           write permission */
   {
      if (strpbrk(val_str,"/")!=NULL) /* There is a pathname prior to a 
                              non-existing file name */
      {
         strcpy(s,val_str);  /* Strip file name */
         for (c_pt = s+strlen(s)-1; c_pt>s && *c_pt!='/'; 
            *c_pt=null, c_pt--);
         if (c_pt>s) *c_pt = null;
      }
      else
         strcpy(s,".");
      if (access(s,00) != 0)
      {
         if (strlen(s) < 40) strcpy(v,s);
         else *v = null;
         sprintf(error_mess," DIRECTORY %s DOES NOT EXIST ",v);
         return(0);
      }
      
      if (access(s,02) != 0)  
                                    /* No pathname precedes file name */
      {
         if (strlen(s) < 40) strcpy(v,s);
         else *v = null;
         if (strcmp(s,".")!=0)
            sprintf(error_mess," DIRECTORY %s HAS NO WRITE PERMISSION ",v);
         else
            sprintf(error_mess," CURRENT DIRECTORY HAS NO WRITE PERMISSION ");
         return(0);
      }
      if (access(s,01) != 0)  
                                    /* No pathname precedes file name */
      {
         if (strlen(s) < 40) strcpy(v,s);
         else *v = null;
         sprintf(error_mess," DIRECTORY %s CANNOT BE ACCESSED ",v);
         return(0);
      }
      return(1);
   }
}




/*
 * Matchstr() check whether Segm_pt->word matches one of a list of
 * "prefixes" i.e., strings stored before the ^Z in the fs file.
 * astgen name: Match specified string(s) 
 */
int matchlist()
{
   register int test=0;
   register struct fix *pr_pt;
   char mess[256];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL;pr_pt=pr_pt->next)
      if (equal(Segm_pt->word,pr_pt->name)) 
         test=1;

   if (test) return(1);

   sprintf(mess,"SHOULD BE ");
   for (pr_pt=V_pt->pr_pt;
        pr_pt!=NULL && pr_pt->name!=NULL
        && strlen(mess) + strlen(pr_pt->name)<74;
        pr_pt=pr_pt->next)
   {
      strcat(mess,pr_pt->name);
      strcat(mess," OR ");
   }
   
   if (strlen(mess)>=79 || pr_pt!=NULL)
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   else
   {
      mess[strlen(mess)-3] = null;  /* Blank out final "OR "         */
      strcpy(error_mess,mess);
   }
   return(0);
}



/*
 * Find set fields incompatible with current field.
 * Called in astgen incompatible fields module.
 */
int incomp()
{
   register int test, flag;
   register struct segment *s_pt;
   register struct field *f_pt;
   register struct index *in_pt;

   test = 1;
   flag = isset(Field_pt,Segm_pt);
   for (in_pt = Field_pt->first_i_pt; in_pt!=NULL; in_pt = in_pt->next)
   {
      f_pt = fields+in_pt->num;
      s_pt = f_pt -> first_s_pt;
      if (isset(f_pt,s_pt))
      {
         if (flag)
         {
            if (s_pt->row-Stdscr_loc<=SCRLINES && s_pt->row-Stdscr_loc>2)
            {
               highlight(s_pt,1,f_pt);
               strcpy(error_mess," INCOMPATIBLE WITH HIGHLIGHTED ITEM(S) ");
            }
            else
               strcpy(error_mess," INCOMPATIBLE WITH ITEM(S) ON OTHER PAGE(S) ");
            test=3;
         }
         else if (s_pt->row-Stdscr_loc<=SCRLINES && s_pt->row-Stdscr_loc>2)
            highlight(s_pt,0,f_pt);
      }
   }
   return(test);
}


/*
 * Check if previous field has more than one string.  If yes, require
 * current field to be a directory.  For cp, ln, mv.
 */
int cp_ok()
{  struct stat buf;
   char val_str[128];
   int stat_return;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   stat_return = stat(val_str,&buf);  
                                            /* Get status of current word */

   if ((Field_pt-1)->first_s_pt->next!=NULL)  /* > 1 source file */
   {
      if (stat_return==0) /* Current word exists */
      {
         if (!(buf.st_mode & S_IFDIR))     /* Current word not a directory */
         {
            if (strlen(val_str)<50)
               sprintf(error_mess," %s IS NOT A DIRECTORY ",val_str);
            else
               sprintf(error_mess," NOT A DIRECTORY ");
            return(0);
         }
         else
            return(1);
      }
      else
      {
         strcpy(error_mess," TYPE NAME OF EXISTING DIRECTORY ");
         return(0);
      }
   }
   if (stat_return==0 && !(buf.st_mode & S_IFDIR))  /* 1 source file */
   {
      if (strlen(val_str)<50)
         sprintf(warn_mess," WOULD OVERWRITE %s ",val_str);
      else
         sprintf(warn_mess," WOULD OVERWRITE ");
      return(2);
   }
   return(1);
}



/*
 * Check if file in current field has same inode as file in preceding field.
 * astgen name: Differ from file in last field 
 */
int notsame()
{
   struct stat buf1, buf2;
   struct segment *s_pt;
   char val_str1[128], val_str2[128];
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str2);
   for (s_pt=(Field_pt-1)->first_s_pt; s_pt!=NULL; s_pt=s_pt->next)
   {
      rmquotes(s_pt->word,val_str1);
      if (stat(val_str1,&buf1) ==0 &&
          stat(val_str2,&buf2) ==0)
         if (buf1.st_ino == buf2.st_ino)
         {
            if (strlen(val_str1) + strlen(val_str2) < 35)
               sprintf(error_mess," INPUT FILE %s AND TARGET FILE %s ARE THE SAME ",
                      val_str1,val_str2);
             else
               sprintf(error_mess," INPUT AND TARGET FILES ARE THE SAME ");
            return(0);
         }
   }
   return(1);
}




#define   DIGITS    CTRL(N)
#define   DIGIT     CTRL(B)
#define   LETTER    CTRL(L)
#define   LETTERS   CTRL(E)
#define   LIST      CTRL(P)
#define   GRAPH     CTRL(C)
#define   GRAPHS    CTRL(K)

/* Routine that checks format of string, using ideosyncratic conventions.
 * Really should have used regular expressions.
 * Prefixes contain format strings.  Postfix lists characters for which
 * the LIST item is wild.
 * astgen name: Match specified format(s).
 */
int format()
{
   register struct fix *pr_pt, *po_pt;
   register char *c_pt, *cw_pt, *c0_pt;
   register int test;
   char s[512], val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);

/* rmquotes(Segm_pt->word,val_str);  */
   strcpy(val_str,Segm_pt->word);
   strcpy(s," ACCEPTABLE: ");
   test=0;

/* Now the real parsing starts */
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL && test==0;pr_pt=pr_pt->next)
   {
      test=1;
      c_pt = pr_pt->name;
      cw_pt = val_str;
      while (*c_pt && strlen(s)<500)
      {
         switch(*c_pt)
         {
         case DIGIT:
            if (*cw_pt && isdigit(*cw_pt)) cw_pt++;
            else test=0;
            strcat(s,"<digit>");
            break;
         case DIGITS:
            if (*cw_pt && isdigit(*cw_pt)) cw_pt++;
            else test=0;
            while (*cw_pt && isdigit(*cw_pt)) cw_pt++;
            strcat(s,"<digits>");
            break;
         case LETTER:
            if (*cw_pt && isalpha(*cw_pt)) cw_pt++;
            else test=0;
            strcat(s,"<letter>");
            break;
         case LETTERS:
            if (*cw_pt && isalpha(*cw_pt)) cw_pt++;
            else test=0;
            while (*cw_pt && isalpha(*cw_pt)) cw_pt++;
            strcat(s,"<letters>");
            break;
         case GRAPH:
            if (*cw_pt && isgraph(*cw_pt)) cw_pt++;
            else test=0;
            strcat(s,"<character>");
            break;
         case GRAPHS:
            if (*cw_pt && isgraph(*cw_pt)) cw_pt++;
            else test=0;
            while (*cw_pt && isgraph(*cw_pt)) cw_pt++;
            strcat(s,"<characters>");
            break;
         case LIST:
            if ((po_pt=V_pt->po_pt)==NULL ||
                po_pt->name==NULL || *(po_pt->name)==null) test=0;
            else
            {
               test=0;
               for (c0_pt=po_pt->name; *c0_pt && test==0; c0_pt++)
                  if (*cw_pt && *cw_pt == *c0_pt) test=1;
               if (*cw_pt && test==1) cw_pt++;
            }
            strcat(s,"[");
            strcat(s,po_pt->name);
            strcat(s,"]");
            break;
         default:
            if (*cw_pt && *cw_pt == *c_pt) cw_pt++;
            else test=0;
            if ((int)*c_pt<=27)
               strcat(s,visible[(int)*c_pt]);
            else
               strncat(s,c_pt,1);
            break;
         }
         c_pt++;
      }
      if (*cw_pt != null) test=0;
      if (test==1) return(1);
      strcat(s," ");
   }

   if (strlen(s)<79) strcpy(error_mess,s);
   else 
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   return(0);
}

/*
 * Check if user owns file.
 * astgen name: Be owned by user.
 */
int ownfile()
{  char val_str[128];
   struct stat buf;
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (stat(val_str,&buf) !=0)
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," DOES NOT EXIST ");
      return(0);
   }
   if (buf.st_uid != geteuid())
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," YOU DO NOT OWN %s ",val_str);
      else
         sprintf(error_mess," YOU DO NOT OWN FILE OR DIRECTORY ");
      return(0);
   }
   return(1);
}


/*
 * Check if string is a directory.
 */
int isdir()
{  char val_str[128];
   struct stat buf;
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (stat(val_str,&buf) != 0)
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," DOES NOT EXIST ",val_str);
      return(0);
   }
   if (!(buf.st_mode & S_IFDIR))
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," %s IS NOT A DIRECTORY ",val_str);
      else
         sprintf(error_mess," NOT A DIRECTORY ",val_str);
      return(0);
   }
   return(1);
}


/*
 * Check if user has write permission in current directory.
 */
int cdirwp()
{
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   if (access(".",02) == 0) return(1);

   strcpy(error_mess," NO WRITE PERMISSION IN CURRENT DIRECTORY ");
   return(0);
}


/*
 * Checks if string does NOT eixst.
 * astgen name: Not exist.
 */
int noexist()
{  char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (access(val_str,00)==0)
   {
      if (strlen(val_str)<70)
         sprintf(error_mess," %s EXISTS ",val_str);
      else
         sprintf(error_mess," EXISTS ",val_str);
      return(0);
   }
   return(1);
}




/*
 * Checks if string is NOT a directory.
 */
int notdir()
{  char val_str[128];
   struct stat buf;
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (stat(val_str,&buf) == 0 && (buf.st_mode & S_IFDIR))
   {
      if (strlen(val_str)<60)
         sprintf(error_mess," %s IS A DIRECTORY ",val_str);
      else
         sprintf(error_mess," IS A DIRECTORY ");
      return(0);
   }
   return(1);
}



/*
 * Checks if string exists and has read permission.
 */
int existread()
{  char val_str[128];
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (access(val_str,00)==0)
   {
      if (access(val_str,04)!=0)
      {
         if (strlen(val_str) < 50)
            sprintf(warn_mess," WARNING: %s CANNOT BE NOT READ ",val_str);
         else
            sprintf(warn_mess," WARNING: CANNOT BE NOT READ ");
         return(2);
      }
   }
   else
   {
      if (strlen(val_str) < 50)
         sprintf(warn_mess," WARNING: %s DOES NOT EXIST ",val_str);
      else
         sprintf(warn_mess," WARNING: DOES NOT EXIST ");
      return(2);
   }
   return(1);
}

/*
 * Checks if string either is a readable file OR matches one of a list
 * of strings.
 * astgen name: Be readable or match string(s).
 */
int existstr()
{  register struct fix *pr_pt;   /* First check literal match;
                                    then do usal file chevk on rmquoted
                                    string */
   char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL;pr_pt=pr_pt->next)
      if (equal(Segm_pt->word,pr_pt->name)) return(1);
   rmquotes(Segm_pt->word,val_str);
   if (access(val_str,00)!=0)
   {
      if (strlen(val_str) < 50)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," DOES NOT EXIST ");
      return(0);
   }
   else if (access(val_str,04)!=0)
   {
      if (strlen(val_str) < 50)
         sprintf(error_mess," %s DOES NOT HAVE READ PERMISSION ",val_str);
      else
         sprintf(error_mess," DOES NOT HAVE READ PERMISSION ");
      return(0);
   }
   return(1);
}




/*
 * Checks of at least one of a list of required fields
 * is set.
 * Called in astgen required fields module.
 */
int requires()
{
   register int test, flag;
   register struct segment *s_pt;
   register struct field *f_pt;
   register struct index *in_pt;

   test = 1;
   flag = isset(Field_pt,Segm_pt);

   for (in_pt = Field_pt->first_m_pt; in_pt!=NULL; in_pt = in_pt->next)
   {
      f_pt = fields+in_pt->num;
      s_pt = f_pt -> first_s_pt;
      if (isset(f_pt,s_pt)==0)
      {
         if (flag)
         {
            if (s_pt->row-Stdscr_loc<=SCRLINES && s_pt->row-Stdscr_loc>2)
            {
               highlight(s_pt,1,f_pt);
               strcpy(error_mess," REQUIRES HIGHLIGHTED ITEM(S) ");
            }
            else
               strcpy(error_mess," REQUIRES ITEM(S) ON OTHER PAGE(S) ");
            test=3;
         }
         else if (s_pt->row-Stdscr_loc<=SCRLINES && s_pt->row-Stdscr_loc>2)
            highlight(s_pt,0,f_pt);
      }
   }
   return(test);
}




/*
 * prepo() checks whether an initial (final) segment of a string
 * matches one of a list of prefixes and one of a list of suffixes.
 * astgen name: Start/end with specified string(s).
 */
int prepo()
{
   register int len0, len1, test=0;
   register struct fix *pr_pt, *po_pt;
   register char *c0_pt, *c1_pt;
   char mess[256];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);

   pr_pt = V_pt->pr_pt;
   if (*(pr_pt->name)==null)
         test=1;        /* Empty string is trivially initial segment of word  */

   else
      for (; pr_pt!=NULL; pr_pt=pr_pt->next)
         if (strlen(Segm_pt->word) >= (len1=strlen(pr_pt->name)))
            if (equaln(Segm_pt->word,pr_pt->name,len1)) 
               test=1;

   if (test==0)
   {
      sprintf(mess," SHOULD BEGIN WITH ");

      for (pr_pt=V_pt->pr_pt; pr_pt!=NULL 
           && strlen(mess) +strlen(pr_pt->name)<74;
            pr_pt=pr_pt->next)
      {
         strcat(mess,pr_pt->name);
         strcat(mess," OR ");
      }
      if (strlen(mess)>=79 || pr_pt!=NULL)
         strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
      else
      {
         mess[strlen(mess)-3] = null;      /* Blank out final "OR " */
         strcpy(error_mess,mess);
      }
      return(0);
   }


   po_pt = V_pt->po_pt;
   if (po_pt==NULL || po_pt->name==NULL || *(po_pt->name)==null)
         return(1);     /* Empty string is trivially initial segment of word  */

   len0 = strlen(Segm_pt->word);
   for (; po_pt!=NULL; po_pt=po_pt->next)
   {
      len1 = strlen(po_pt->name);
      test=1;
      if (len1<=len0)
      {
         c0_pt = Segm_pt->word+len0-len1;
         for (c1_pt = po_pt->name; *c1_pt ; c1_pt++)
            if (*c0_pt++ != *c1_pt) test=0;
      }
      else test=0;
      if (test==1) return(1);
   }

   sprintf(mess," SHOULD END WITH ");

   for (po_pt=V_pt->po_pt; po_pt!=NULL 
        && strlen(mess) + strlen(po_pt->name) < 74; 
        po_pt=po_pt->next)
   {
      strcat(mess,po_pt->name);
      strcat(mess," OR ");
   }
   
   if (strlen(mess)>=79 || po_pt!=NULL)
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   else
   {
      mess[strlen(mess)-3] = null;  /* Blank out final "OR "        */
      strcpy(error_mess,mess);
   }
   return(0);
}



/*
 * Checks whether an initial (final) segment of Segm_pt->word 
 * does NOT match one of a list of prefixes and one of a list of suffixes.
 * astgen name: Not start/end with specified string(s).
 */
int notprepo()
{
   register int len0, len1, pr_test=0, po_test=0;
   register struct fix *pr_pt, *po_pt;
   register char *c0_pt, *c1_pt;
   char mess[256];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
 
   pr_test=0;
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL;pr_pt=pr_pt->next)
      if (strlen(Segm_pt->word) >= (len1=strlen(pr_pt->name)))
         if (equaln(Segm_pt->word,pr_pt->name,len1)) 
            pr_test=1;

   len0 = strlen(Segm_pt->word);
   po_test=0;
   for (po_pt=V_pt->po_pt;
        po_test!=1 && po_pt!=NULL;
        po_pt=po_pt->next)
   {
      len1 = strlen(po_pt->name);
      po_test=1;
      if (len1>0 && len1<=len0)
      {
         c0_pt = Segm_pt->word+len0-len1;
         for (c1_pt = po_pt->name; *c1_pt ; c1_pt++)
            if (*c0_pt++ != *c1_pt) po_test=0;
      }
      else po_test=0;
   }

   *mess = null;
   if (pr_test==0 && po_test==0) return(1);
   if (pr_test || po_test)
   {
      if (pr_test==1)
      {
         strcat(mess," SHOULD NOT BEGIN WITH ");
         for (pr_pt=V_pt->pr_pt;pr_pt!=NULL 
              && strlen(mess) + strlen(pr_pt->name) < 74;
              pr_pt=pr_pt->next)
         {
            strcat(mess,pr_pt->name);
            strcat(mess," OR ");
         }
         mess[strlen(mess)-3] = null;  /* Blank out final "OR "           */
      }

      if (po_test==1)
      {
         strcat(mess," SHOULD NOT END WITH ");
         for (po_pt=V_pt->po_pt;po_pt!=NULL
              && strlen(mess) + strlen(po_pt->name) < 74;
              po_pt=po_pt->next)
         {
            strcat(mess,po_pt->name);
            strcat(mess," OR ");
         }
         mess[strlen(mess)-3] = null;  /* Blank out final "OR "           */
      }
   }
   
   if (strlen(mess)>=79 ||  
       (pr_test==1 && pr_pt!=NULL) ||
       (po_test==1 && po_pt!=NULL))
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   else
      strcpy(error_mess,mess);
   return(0);
}



/*
 * Checks if current field is required by some other field.
 * Called in astgen required fields module.
 */
int requiredby()
{  register int flag;
   register struct segment *s_pt;
   register struct field *f_pt;
   register struct index *in_pt;
   register int r;

   flag = isset(Field_pt,Segm_pt);

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++)
   {
      if (isset(f_pt,s_pt=f_pt->first_s_pt))
         for (in_pt = f_pt->first_m_pt;in_pt!=NULL;in_pt = in_pt->next)
         {
            if (in_pt->num == Field_pt-fields && flag == 0)
            {
               if ((r=s_pt->row-Stdscr_loc)<=SCRLINES && r>2)
               {
                  highlight(s_pt,1,f_pt);
                  strcpy(error_mess," REQUIRED BY HIGHLIGHTED ITEM(S) ");
               }
               else
                  strcpy(error_mess," REQUIRED BY ITEM(S) ON OTHER PAGE(S) ");
               return(3);
            }
            else if ((r=s_pt->row-Stdscr_loc)<=SCRLINES && r>2)
                  highlight(s_pt,0,f_pt);
      }
   }
   return(1);
}



/*
 * Verifies if there is not <prefix> and <suffix> such that
 * <prefix><word><suffix> is and existing file.
 */
int prepo_noexist()
{
   register struct fix *pr_pt, *po_pt;
   register int test=0;
   char s[100], val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   rmquotes(Segm_pt->word,val_str);
   for (pr_pt = V_pt->pr_pt; pr_pt!=NULL && test==0;  pr_pt=pr_pt->next)
   for (po_pt = V_pt->po_pt; po_pt!=NULL && test==0;  po_pt=po_pt->next)
   {
      strcpy(s,pr_pt->name);                    /* Empty string allowed.  */
      strcat(s,val_str);  
      strcat(s,po_pt->name);                    /* Empty string allowed.  */
      if (access(s,00)==0) test=1;
   }
   if (test==0) return(1);
   if (strlen(s)<70)
      sprintf(error_mess," %s EXISTS ",s);
   else 
   {
      *(s+65) = null;
      strcat(s," ...");
      sprintf(error_mess," %s EXISTS ",s);
   }
   return(0);
}



/*
 * Checks if string is an X-separated list of specified strings.
 * X is a separator character ("sep").
 */
int strlist()
{
   register int test=1;
   register struct fix *pr_pt;
   char s[80];
   register char *c_pt;
   char *strs[NFIX], **strs_pt;
   char mess[256], sep;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);

   strcpy(s,Segm_pt->word);
   sep = *(V_pt->po_pt->name);

   *strs = c_pt = s;
   strs_pt = strs;
   while (*c_pt)
      if (*c_pt==sep)
      {
         *c_pt++=null;
         *(++strs_pt) = c_pt;
      }
      else c_pt++;
   *(++strs_pt) = NULL;

   test=1;
   for (strs_pt=strs;*strs_pt!=NULL && test;strs_pt++)
   {
      test=0;
      for (pr_pt=V_pt->pr_pt;pr_pt!=NULL;pr_pt=pr_pt->next)
      {
         if (equal(*strs_pt,pr_pt->name)) test=1;
      }
   }
   if (test) return(1);

   sprintf(mess," SHOULD BE \"%c\"-SEPARATED LIST OF: ",sep);
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL 
        && strlen(mess) + strlen(pr_pt->name) < 74;
        pr_pt=pr_pt->next)
   {
      strcat(mess,pr_pt->name);
      strcat(mess," ");
   }
   
   if (strlen(mess)>=79 || pr_pt!=NULL)
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   else strcpy(error_mess,mess);
   return(0);
}

int superf()
{
}

int superf_1()
{
}



/*
 * nomatch() checks whether Segm_pt->word does not match a stored
 * "pre-fix".
 * astgen name: Not match specified string(s).
 */
int nomatch()
{
   register int test=0;
   register struct fix *pr_pt;
   char mess[256];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL;pr_pt=pr_pt->next)
      if (equal(Segm_pt->word,pr_pt->name)) 
         test=1;

   if (test==0) return(1);

   sprintf(mess," SHOULD NOT BE ");

   for (pr_pt=V_pt->pr_pt;pr_pt!=NULL 
        && strlen(mess) + strlen(pr_pt->name) < 74;
        pr_pt=pr_pt->next)
   {
      strcat(mess,pr_pt->name);
      strcat(mess," OR ");
   }
   
   
   if (strlen(mess)>=79 || pr_pt!=NULL)
      strcpy(error_mess," NOT ACCEPTABLE; CHECK ITEM HELP ");
   else
   {
      mess[strlen(mess)-3] = null;  /* Blank out final "OR "          */
      strcpy(error_mess,mess);
   }
   return(0);
}


/*
 * Checks if string exists and has writepermission.
 */
int existwrite()
{  char val_str[128];

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(Segm_pt->word,val_str);
   if (access(val_str,00)==0)
   {
      if (access(val_str,02)!=0)
      {
         if (strlen(val_str) < 40)
            sprintf(error_mess," %s DOES NOT HAVE WRITE PERMISSION ",
            val_str);
         else
            sprintf(error_mess," DOES NOT HAVE WRITE PERMISSION ");
         return(0);
      }
   }
   else
   {
      if (strlen(val_str) < 60)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      return(0);
   }
   return(1);
}



/*
 * Routine that checks if string is valid regular experssion.
 * Part of regulx() routine is stolen from /usr/src/cmd/grep.c.
 * astgen name: Be a regular expression.
 */

#define INIT   register char *sp = instring;
#define GETC()      (*sp++)
#define PEEKC()      (*sp)
#define UNGETC(c)   (--sp)
#define RETURN(c)   return;
#define ERROR(c)   regerr(c)

#include <regexp.h>

int regulx()
{
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   compile(Segm_pt->word, expbuf, &expbuf[ESIZE], '\0');
   if (*error_mess != null) return(0);
   else return(1);
}

regerr(err)
register err;
{
   switch(err) {
      case 11:
         strcpy(error_mess," RANGE ENDPOINT TOO LARGE ");
         break;
      case 16:
         strcpy(error_mess," BAD NUMBER ");
         break;
      case 25:
         strcpy(error_mess," ``\\DIGIT'' OUT OF RANGE ");
         break;
      case 36:
         strcpy(error_mess," ILLEGAL OR MISSING DELIMITER ");
         break;
      case 41:
         strcpy(error_mess," NO REMEMBERED SEARCH STRING ");
         break;
      case 42:
         strcpy(error_mess," \\( \\) IMBALANCE ");
         break;
      case 43:
         strcpy(error_mess," TOO MANY \\( ");
         break;
      case 44:
         strcpy(error_mess," MORE THAN 2 NUMBERS GIVEN IN \\{ \\} ");
         break;
      case 45:
         strcpy(error_mess," } EXPECTED AFTER \\ ");
         break;
      case 46:
         strcpy(error_mess," FIRST NUMBER EXCEEDS SECOND IN \\{ \\} ");
         break;
      case 49:
         strcpy(error_mess," [ ] IMBALANCE ");
         break;
      case 50:
         strcpy(error_mess," REGULAR EXPRESSION OVERFLOW ");
         break;
      default:
         strcpy(error_mess," UNKNOWN REGEXP ERROR CODE!! ");
         break;
   }
}
/* End of regulx() routine   */


/*
 * Routine that checks if string has certain characters. If yes,
 * tells user how to quote them.
 * astgen name: Be quoted if it has specified char(s).
 */
int mustquote()
{
   register struct fix *pr_pt, *po_pt;
   register char *c_pt, *w;
   register int test;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   w = Segm_pt->word;
   test=0;
   for (c_pt=w; *c_pt && test==0; c_pt++)
      for (pr_pt = V_pt->pr_pt; pr_pt!=NULL && test==0;  
            pr_pt=pr_pt->next)
         if (*c_pt == *(pr_pt->name) && (c_pt==w || *(c_pt-1)!='\\')) 
            test=1;
   if (test==0) return(1);
   if (test==1 && *w== *(w+strlen(w)-1))
      for (po_pt=V_pt->po_pt;po_pt!=NULL;po_pt=po_pt->next)
         if (*w == *(po_pt->name)) return(1);
   sprintf(error_mess," CONTAINS A '%c'; MUST BE ENCLOSED BY ",*(c_pt-1));
   for (po_pt = V_pt->po_pt; po_pt!=NULL 
        && strlen(error_mess) + strlen(po_pt->name) < 74;  
        po_pt=po_pt->next)
   {
      strcat(error_mess,"'"); strcat(error_mess,po_pt->name); 
      strcat(error_mess,"'"); strcat(error_mess," OR ");
   }
   
   if (po_pt!=NULL)
      sprintf(error_mess,
" CONTAINS A '%c'; MUST BE ENCLOSED BY APPROPRIATE QUOTATION SYMBOLS");
   else
      error_mess[strlen(error_mess)-3] = null;
   return(0);
}

/*
 * Routine that allows application of a list of valfuncs on the
 * sub-strings of a X-separated string list (X="sep").
 */
int splitlist()
{
   register int test=1;
   register struct fix *pr_pt;
   register char sep;
   char s[80];
   register char *c_pt;
   char *strs[NFIX], **strs_pt;
   struct segment segm, *s_Segm_pt;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);

   strcpy(s,Segm_pt->word);
   s_Segm_pt = Segm_pt;
   Segm_pt = &segm;
   sep = *(V_pt->po_pt->name);

   *strs = c_pt = s;
   strs_pt = strs;
   while (*c_pt)
      if (*c_pt==sep)
      {
         *c_pt++=null;
         *(++strs_pt) = c_pt;
      }
      else c_pt++;
   *(++strs_pt) = NULL;

   test=1;
   for (strs_pt=strs;*strs_pt!=NULL && test;strs_pt++)
   {
      for (pr_pt=V_pt->pr_pt;pr_pt!=NULL && test;pr_pt=pr_pt->next)
      {
         Segm_pt->word = *strs_pt;
         test = (*val_funcs[atoi(pr_pt->name)])();
      }
   }
   Segm_pt = s_Segm_pt;
   return(test);
}


/*
 * Check if number of lines in a file is larger than
 * user provided number.
 */
int filelen()
{  register char *word = NULL;
   register struct segment *s_pt;
   register int nlines=0, flines, c;
   register FILE *fp0;
   char val_str[128];

   s_pt = (Field_pt-1)->first_s_pt;
   if (s_pt!=NULL) word = (Field_pt-1)->first_s_pt->word;
   if (s_pt==NULL || word==NULL || *word==null)
      return(1);
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   rmquotes(word,val_str);
   if (access(val_str,00)==0)
   {
      sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      return(0);
   }
   else if (access(val_str,04)==0)
   {
      sprintf(error_mess," %s DOES NOT HAVE READ PERMISSION ",val_str);
      return(0);
   }
   fp0 = fopen(word,"r");
   while ((c=getc(fp0))!=EOF) if (c=='\n') nlines++;
   if ((flines=atoi(Segm_pt->word)) > nlines)
   {
      sprintf(error_mess," %s HAS ONLY %d LINES ",word,flines);
      return(0);
   }
   return(1);
}

/*
 * Checks if current field and preceding field share a string.
 */
int no_n_match()
{
   struct segment *s_pt;
   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
      return(1);
   for (s_pt=(Field_pt-1)->first_s_pt; s_pt!=NULL; s_pt=s_pt->next)
   {
      if (equal(s_pt->word,Segm_pt->word))
         {
            if (strlen(Segm_pt->word) < 30)
               sprintf(error_mess,
               "%s CANNOT OCCUR BOTH IN CURRENT AND IN PREVIOUS ITEM ",
                      Segm_pt->word);
            else
               sprintf(error_mess,
               " CANNOT OCCUR BOTH IN CURRENT AND IN PREVIOUS ITEM ");
            return(0);
         }
   }
   return(1);
}


/*
 * Checks if at least one of a group of listed fields is set.
 * Called in astgen at-least-one fields module.
 */
int least_one()
{
   register int test, flag;
   register struct segment *s_pt;
   register struct field *f_pt;
   register struct index *in_pt;

/* First check what the situation is */

   test = (isset(Field_pt,Segm_pt)) ? 1 : 3;
   for (in_pt = Field_pt->first_a_pt; in_pt!=NULL && test==3; in_pt = in_pt->next)
   {
      f_pt = fields+in_pt->num;
      s_pt = f_pt -> first_s_pt;
      if (isset(f_pt,s_pt)) test=1;
   }

/* Now take care of the highlighting  */

   flag = (test==3)? 1 : 0;
   if (show_segm) highlight(Segm_pt,flag,Field_pt);

   for (in_pt = Field_pt->first_a_pt; in_pt!=NULL; in_pt = in_pt->next)
   {
      f_pt = fields+in_pt->num;
      s_pt = f_pt -> first_s_pt;
      if (s_pt->row-Stdscr_loc<=SCRLINES && s_pt->row-Stdscr_loc>2)
      {
         highlight(s_pt,flag,f_pt);
         if (flag)
            strcpy(error_mess,
            " AT LEAST ONE OF THE HIGHLIGHTED ITEM(S) IS REQUIRED ");
      }
      else if (flag)
         strcpy(error_mess,
" AT LEAST ONE OF THE HIGHLIGHTED ITEM(S) ON THIS AND ON NEXT PAGE IS REQUIRED ");
   }

   return(test);
}


/*
 * Routine that runs a shell script.  Fields are referred to
 * using $Fzz notation (zz = 00, 01 02, ...).
 * Shell script should return message IFF there is an error.
 * astgen name: Run a shellscript.
 */
int shval()
{
   register FILE *fp0;
   register char *c_pt, *c0_pt, *c1_pt;
   register struct field *f_pt;
   char val_str[128];
   int k;
   char s[1024];
   extern int done();

   if (V_pt->pr_pt==NULL || (c_pt=V_pt->pr_pt->name)==NULL) return(1);

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);

   mvaddstr(LINES-1,40," VALIDATING INPUT, PLEASE WAIT ... ");
   refresh();
   move(LINES-1,40); clrtoeol();

   c1_pt = s;
   while (*c_pt)
   {
      switch(*c_pt)
      {
         case '$':
            c0_pt = c_pt;
            if (*++c_pt=='F' && *++c_pt>='0' && *c_pt<='9' &&
                                *++c_pt>='0' && *c_pt<='9')
            {
               f_pt = fields + (*c_pt-'0') + 10*(*(c_pt-1)-'0');
               if (f_pt==Field_pt)
                  rmquotes(Segm_pt->word,val_str);
               else {
                  if (f_pt->first_s_pt==NULL || 
                      f_pt->first_s_pt->word==NULL) *val_str = null;
                  else 
                     rmquotes(f_pt->first_s_pt->word,val_str);
               }
               strcpy(c1_pt,val_str);
               c1_pt += strlen(val_str);
            }
            else
            {
              c_pt = c0_pt;
              *c1_pt++ = *c_pt;
            }
            break;
         default:
           *c1_pt++ = *c_pt;
            break;
      }
      c_pt++;
   }
   *c1_pt = null;

   PR_POPEN;
   if ((fp0 = popen(s,"r")) !=NULL)
   {
      c0_pt = error_mess;
      while ((k=getc(fp0))!=EOF && c0_pt-error_mess<79) *c0_pt++ = k;
      *c0_pt=null;
   }
   else strcpy(error_mess," VALIDATION INTERRUPTED ");
   pclose(fp0);
   PO_POPEN;
   if (*error_mess==null || *error_mess=='\n') return(1);
   else return(0);
}


/*
 * Specify in prefix and suffix permissions a file
 * should (not) have, using drwx... notation.
 */
int permission()
{  register struct fix *pr_pt, *po_pt;
   register int test = -1, P;
   char val_str[128];
   struct stat buf;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   rmquotes(Segm_pt->word,val_str);

   if (access(val_str,00)!=0)
   {
      if (strlen(val_str) < 60)
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      else
         sprintf(error_mess," %s DOES NOT EXIST ",val_str);
      return(0);
   }

   stat(val_str,&buf);
   P = buf.st_mode;

   for (pr_pt=V_pt->pr_pt;
        pr_pt!=NULL && pr_pt->name!=NULL && *(pr_pt->name)!=null && test!=1;
        pr_pt=pr_pt->next)
   {
      test = 0;
      *error_mess = null;
      if (permcode(pr_pt->name,val_str,0,P) ==1) test = 1;
   }
   if (test==0) return(0);

   test = -1;
   for (po_pt=V_pt->po_pt;
        po_pt!=NULL && po_pt->name!=NULL && *(po_pt->name)!=null && test!=1;
        po_pt=po_pt->next)
   {
      test = 0;
      *error_mess = null;
      if (permcode(po_pt->name,val_str,1,P) == 1) test = 1;
   }
   if (test==0) return(0);
   else return(1);
}

/* subroutine for permissions() */
int permcode(s,t,c,p)
char *s,*t;
int c, p;
{  int mask = 0400;
   char T[128];

   if (strlen(t)<50) strcpy(T,t);
   else *T = null;

   if (*s == 'd' && c==0 && !(p&S_IFDIR))
   {
      sprintf(error_mess," %s IS NOT A DIRECTORY ",T);
      return(0);
   }
   if (*s == 'd' && c==1 && p&S_IFDIR)
   {
      sprintf(error_mess," %s IS A DIRECTORY ",T);
      return(0);
   }

   s++;

   while (*s)
   {
      switch(*s)
      {
      case 'r':
         if (c==0 && !(mask&p))
         {
            sprintf(error_mess," %s DOES NOT HAVE READ PERMISSION ",T);
            return(0);
         }
         if (c==1 && mask&p) 
         {
            sprintf(error_mess," %s HAS READ PERMISSION ",T);
            return(0);
         }
         break;
      case 'w':
         if (c==0 && !(mask&p))
         {
            sprintf(error_mess," %s DOES NOT HAVE WRITE PERMISSION ",T);
            return(0);
         }
         if (c==1 && mask&p) 
         {
            sprintf(error_mess," %s HAS WRITE PERMISSION ",T);
            return(0);
         }
         break;
      case 'x':
         if (c==0 && !(mask&p))
         {
            sprintf(error_mess,
                   " %s DOES NOT HAVE EXECUTE OR SEARCH PERMISSION ",T);
            return(0);
         }
         if (c==1 && mask&p)
         {
            sprintf(error_mess,
                   " %s HAS EXECUTE OR SEARCH PERMISSION ",T);
            return(0);
         }
         break;
      default:
         break;
      }
      s++;
      mask>>=1;
   }
   return(1);
}

/*
 * General purpose permissions checker.
 * Uses 8-digit code to specify permissions.
 * astgen name: Have specified permission(s).
 */
int dpermission()
{  char val_str[128], s[128], *i_pt;
   struct stat buf;
   int d=0, r=0, w=0, x=0, e=0;
   int wflag = 1;

   if (Segm_pt==NULL || Segm_pt->word==NULL || *(Segm_pt->word)==null)
       return(1);
   rmquotes(Segm_pt->word,val_str);

   if (access(val_str,00)!=0) d = r = w = x = e = 0;
   else {
      e = 1;
      if (access(val_str,04)==0) r = 1;
      if (access(val_str,02)==0) w = 1;
      if (access(val_str,01)==0) x = 1;
      stat(val_str,&buf);
      if (buf.st_mode & S_IFDIR) d = 1;
   }

   if (strlen(val_str) > 40) *val_str = null;

   *s = null;

   i_pt = V_pt->pr_pt->name;
   if (!e && *i_pt!='1') {
      if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
      else wflag=0;
      strcat(s,val_str);
      strcat(s," DOES NOT EXIST");
   }

   else
   for (i_pt = V_pt->pr_pt->name; *i_pt && *s==null; i_pt+=2) {
      if (!e && *i_pt=='0') {
         if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
         else wflag=0;
         strcat(s,val_str);
         strcat(s," DOES NOT EXIST");
      }
      else 
         switch((int)(i_pt-V_pt->pr_pt->name)) {
         case 0:
            if (d && *i_pt=='2') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," IS A DIRECTORY");
            }
            else if (!d && *i_pt=='0') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," IS NOT A DIRECTORY");
            }
            break;
         case 2:
            if (r && *i_pt=='2') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," HAS READ PERMISSION");
            }
            else if (!r && *i_pt=='0') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," DOES NOT HAVE READ PERMISSION");
            }
            break;
         case 4:
            if (w && *i_pt=='2') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," HAS WRITE PERMISSION");
            }
            else if (!w && *i_pt=='0') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               strcat(s," DOES NOT HAVE WRITE PERMISSION ");
            }
            break;
         case 6:
            if (x && *i_pt=='2') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               if (d)
                  strcat(s," HAS SEARCH PERMISSION");
               else
                  strcat(s," HAS EXECUTE PERMISSION");
            }
            else if (!x && *i_pt=='0') {
               if (*(i_pt+1)=='1') strcpy(s,"WARNING: ");
               else wflag=0;
               strcat(s,val_str);
               if (d)
                  strcat(s," DOES NOT HAVE SEARCH PERMISSION");
               else
                  strcat(s," DOES NOT HAVE EXECUTE PERMISSION");
            }
            break;
         default:
            break;
      }
   }

   if (*s!=null) {
      if (wflag==1) {
         strcpy(warn_mess,s);
         return(2);
      }
      else {
         strcpy(error_mess,s);
         return(0);
      }
   }
   else return(1);
}

#include <sys/utsname.h>
#include <stdio.h>

/*
 * Checks of UNIX version on your system is a release 3 version.
 * astgen name: Version check.
 */
version3()
{  struct utsname name;
   uname(&name);
   if (*(name.release) != '3') {
      if (strlen(name.release)<10)
         sprintf(error_mess," YOU NEED UNIX SYSTEM V RELEASE 3 FOR THIS FIELD (YOU ARE ON RELEASE %s) ",name.release);
      else
         sprintf(error_mess," YOU NEED UNIX SYSTEM V RELEASE 3 FOR THIS FIELD ");
      return(0);
   }
   return(1);
}
