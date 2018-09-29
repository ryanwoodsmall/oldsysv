/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:command.c	1.6"
#include "muse.h"
#include "mmuse.h"


VOID command_line()
{  
   register int j, k, loc, maxloc=0, test;
   int old = 1;
   register char *c_pt, *st_c_pt;
   int strings_n[MAXFIELDS];
   int bundle_n, isset();
   char delim;
   register struct field *f_pt;
   register struct segment *s_pt;
   register struct fix *p0_pt, *p1_pt, *cpo0_pt, *cpo1_pt;
   char *strings[MAXFIELDS][MAXSEGMS];
   char *bundles[MAXFIELDS];

   command[0] = null;

   for (f_pt=fields; f_pt<=last_field_pt; f_pt++) 
      if (maxloc<f_pt->loc) maxloc=f_pt->loc;

   for (loc=0; loc<=maxloc; loc++) {
      strings_n[loc] = 0;
      bundles[loc] = NULL;
      for (s_pt=segments, j=0; s_pt<=last_segm_pt; s_pt++, j++)
         strings[loc][j] = NULL;
   }
   bundle_n = 0;

   switch(clt_code)
   {
   case 0:

      for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
      {
         loc  = f_pt->loc;
         switch(f_pt->type)
         {
         case 0:                   /* Header: no effect on command line            */
         case 9:                   /* Header: no effect on command line            */
            break;
         case 1:                   /* No option, simply strings expected           */
            for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
            {
               if (s_pt->word!=NULL && *(s_pt->word)!=null)
               {
                  strings[loc][strings_n[loc]++] = " ";
                  strings[loc][strings_n[loc]++] = s_pt->word;
               }
            }
            break;
         case 2:                     /* Option implied by strings                  */
         case 3:                     /* Option implied if string != default        */
            s_pt = f_pt->first_s_pt;
            test = 0;
            if (s_pt!=NULL && s_pt->word!=NULL && *(s_pt->word)!=null)
            {
               if (f_pt->type==2)
                  test=1;
               else if (diff(s_pt->word,f_pt->def_arg) && f_pt->type==3)
                  test=1;
            }
            if (test)
            {
               for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
               {
                  if (s_pt->word!=NULL && *(s_pt->word)!=null)
                  {
                     strings[loc][strings_n[loc]++] = " ";
                     strings[loc][strings_n[loc]++] = f_pt->op_name;
                     strings[loc][strings_n[loc]++] = s_pt->word;
                  }
               }
            }
            break;
         case 4:                   /* Simple yes/no option without arguments      */
            if (*(f_pt->first_s_pt->word) == 'y' || *(f_pt->first_s_pt->word) == 'Y')
            {
               if (f_pt->bundle==1)
               {
                  if (strlen(f_pt->op_name)>1)   
                  {
                     delim = *(f_pt->op_name);
                     bundles[bundle_n++] = (f_pt->op_name + 1);
                  }
                  else if (strlen(f_pt->op_name))
                  {
                     delim = '-';
                     bundles[bundle_n++] = f_pt->op_name;
                  }
               }
               else
               {
                  strings[loc][strings_n[loc]++] = " ";
                  strings[loc][strings_n[loc]++] = f_pt->op_name;
               }
            }
            break;
         case 5:
            /* Assumes the following are not NULL: first_cpr_pt,
               first_cpr_pt->next, first_cpo_pt, first_cpo_pt->next  */
            for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
            {
               if (((s_pt->word==NULL || *(s_pt->word)==null) &&
                    (f_pt->first_cpo_pt->name==NULL
                        || *(f_pt->first_cpo_pt->name)==null))   ||
                   (s_pt->word!=NULL && f_pt->first_cpo_pt->name!=NULL &&
                        strcmp(s_pt->word,f_pt->first_cpo_pt->name)==0))
               {
                  if ((c_pt=f_pt->first_cpo_pt->next->name) != NULL)
                  {
                     strings[loc][strings_n[loc]++] = " ";
                     strings[loc][strings_n[loc]++] = c_pt;
                  }
               }
               else if (s_pt->word!=NULL && *(s_pt->word)!= null)
               {
                  strings[loc][strings_n[loc]++] = " ";
                  if ((c_pt=(p0_pt=f_pt->first_cpr_pt)->name)!=NULL)
                     strings[loc][strings_n[loc]++] = c_pt;
                  strings[loc][strings_n[loc]++] = s_pt->word;
                  if ((c_pt=(p0_pt->next)->name)!=NULL)
                     strings[loc][strings_n[loc]++] = c_pt;
               }
            }
            break;
         case 6: /*word is one of k pre-fixes; find out which one;
                   then put correspnding "command prefix" on command line */
            for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
            {
               if (s_pt->word!=NULL && *(s_pt->word)!=null)
               {
                  if (f_pt->bundle!=1)
                     strings[loc][strings_n[loc]++] = " ";
                  p1_pt = f_pt->first_cpr_pt;
                  for (p0_pt=f_pt->first_pr_pt; p0_pt!=NULL; p0_pt=p0_pt->next)
                  {
                     if (equal(s_pt->word,p0_pt->name))
                     {
                        if (f_pt->bundle!=1)
                           strings[loc][strings_n[loc]++] = p1_pt->name;
                        else if (strlen(p1_pt->name)>1)
                        {
                           delim = *(p1_pt->name);
                           bundles[bundle_n++] = (p1_pt->name + 1);
                        }
                     }
                     p1_pt = p1_pt->next;
                  }
               }
               else if (s_pt==f_pt->first_s_pt &&
                        (p0_pt=f_pt->first_cpo_pt)!=NULL &&
                        (c_pt = p0_pt->name) != NULL  && *c_pt!=null)
               {
                  strings[loc][strings_n[loc]++] = " ";
                  strings[loc][strings_n[loc]++] = c_pt;
               }
            }
            break;
         case 7:
            /* General mapping.
               Format: nms^XA^XB^Zxyxyxyxyxyxyxy */
 
            /* Use first cpr_pt character (n) to see whether
               there should be a space between strings in this field and
               strings in the PRECEDING field */
            old = 0;  /* Must be new-style command */
            if (*f_pt->first_cpr_pt->name == '1' 
                && (s_pt=f_pt->first_s_pt)!=NULL
                && isset(f_pt,s_pt))
                     strings[loc][strings_n[loc]++] = " ";

            for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
            {
               /* Use second cpr_pt character (m) to see whether
                  there should be a space between strings in this field */
               if (*(f_pt->first_cpr_pt->name+1) == '1' 
                   && s_pt!=NULL 
                   && s_pt!=f_pt->first_s_pt 
                   && isset(f_pt,s_pt))
                  strings[loc][strings_n[loc]++] = " ";

               /* Now check whether word matches any of the even
                  numbered cpo_pt->name's; if it does, map it onto
                  the next, odd-numbered cpo_pt->name.
                  The even-numbered cpo_pt->names (cpo0_pt->name) play
                  the role of x, the odd-numbered (cpo1_pt->name) play
                  the role of y.
                  Note the extensive checking for null- and NULL-ness.
               */
               test=0;
               for (cpo0_pt=f_pt->first_cpo_pt; 
                    test==0 && cpo0_pt!=NULL; cpo0_pt=cpo0_pt->next) {

                  cpo1_pt = cpo0_pt->next;

                  if (((s_pt->word   ==NULL || *(s_pt->word)   ==null) &&
                       (cpo0_pt->name==NULL || *(cpo0_pt->name)==null))  ||

                      (s_pt->word!=NULL && cpo0_pt->name!=NULL &&
                                      equal(s_pt->word,cpo0_pt->name))) 
                  {  
                     /* Match was found                             */
                     test = 1;
                     if ((c_pt = cpo1_pt->name) != NULL && *c_pt) 
                     {
                        if (f_pt->bundle!=1)
                           strings[loc][strings_n[loc]++] = c_pt;
                        else if (strlen(c_pt)>1) 
                        {
                           delim = *c_pt;
                           bundles[bundle_n++] = c_pt+1;
                        }
                        else if (strlen(c_pt)==1) 
                        {
                           delim = null;
                           bundles[bundle_n++] = c_pt;
                        }
                     }
                  }
                  cpo0_pt = cpo1_pt;
               }

               /* If no match was found, sandwich x between A and B */
               /* However, if match should have been found (*name+3=1)
                  then nothing from this field on command line      */
               if (test==0 && *(f_pt->first_cpr_pt->name+3)=='0'
                           && s_pt->word!=NULL && *(s_pt->word)!= null)
               {
                  p0_pt = f_pt->first_cpr_pt->next; /* skip first
                          cpr_pt -- used for spacing info */
                  c_pt = p0_pt->name;
                  if (c_pt!=NULL && *c_pt)
                     strings[loc][strings_n[loc]++] = c_pt;
                  strings[loc][strings_n[loc]++] = s_pt->word;
                  if ((c_pt=(p0_pt->next)->name)!=NULL && *c_pt)
                     strings[loc][strings_n[loc]++] = c_pt;
               }
            }
            if (*(f_pt->first_cpr_pt->name+2) == '1'
                && (s_pt=f_pt->first_s_pt)!=NULL
                && isset(f_pt,s_pt))
                     strings[loc][strings_n[loc]++] = " ";
            break;
         }
      }

      if (bundle_n)
      {
         sprintf(command," %1c",delim);
         for (k=0;k<bundle_n;k++)
            strcat(command,bundles[k]);
         strcat(command," ");
      }

      for (loc=0;loc<=maxloc;loc++)
      for (j=0;j<strings_n[loc];j++)    
      {
         if ((st_c_pt=strings[loc][j])!=NULL && *st_c_pt!=null)
            strcat(command,st_c_pt);
      }

      strcat(command,"\0");
      if (old) rmblnks(command+1);    /* +1 to preserve initial blank after command name */
      else rmblnks(command);
      break;
   case 10:
      chmod_cmd();
      break;
   case 11:
      umask_cmd();
      break;
   case 12:
      ar_r_cmd();
      break;
   default:
      break;
   }

}


int corder[] = {10,11,12,1,2,3,4,5,6,7,8,9,-1};
VOID chmod_cmd()
{
   int mask, s=0;
   struct field *f_pt=fields;
   struct segment *s_pt;
   register int *i_pt = corder, flag = 0;
   register char ch;

   for (mask=04000;mask;mask>>=1) {
      ch = *((fields + *i_pt)->first_s_pt->word);
      if (ch == 'y' || ch == 'Y') {
         s |= mask;
         if (*i_pt>9) flag=1;   /* Now we wanna use a 4-dig mode */
      }
      i_pt++;
   }

   if (flag==0)
      sprintf(command," %o%o%o ",(s>>6)&07,(s>>3)&07,s&07);
   else
      sprintf(command," %o%o%o%o ",(s>>9)&07,(s>>6)&07,(s>>3)&07,s&07);

   f_pt = fields;
   for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
   {
      strcat(command," ");
      if (s_pt->word!=NULL) strcat(command,s_pt->word);
   }
}



VOID umask_cmd()
{
   unsigned int mask, s=0;
   struct field *f_pt=fields;

   for (mask=0400;mask;f_pt++,mask>>=1)
      if (*((f_pt)->first_s_pt->word) == 'y' || *(f_pt->first_s_pt->word) == 'Y')
         s |= mask;

   if (clt_code==10)
      sprintf(command," %o%o%o ",(s>>6)&07,(s>>3)&07,s&07);
   else
      sprintf(command," %o%o%o ",(~(s>>6))&07,(~(s>>3))&07,(~s)&07);
}



int order[] = {5,6,7,8,2,3,0,1,-1};

VOID ar_r_cmd()
{
   register struct field *f_pt;
   register struct segment *s_pt;
   register int *i_pt=order;

   for (;*i_pt != -1;i_pt++)
   {
      f_pt = fields + *i_pt;
      for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
         switch(f_pt->type)
         {
         case 1:
            if (isset(f_pt,s_pt))
            {
               strcat(command," ");
               strcat(command,s_pt->word);
            }
            break;
         case 2:
         case 3:
            if (isset(f_pt,s_pt))
            {
               strcat(command,f_pt->op_name);
               strcat(command,s_pt->word);
            }
            break;
         case 4:
            if (isset(f_pt,s_pt))
               strcat(command,f_pt->op_name);
            break;
         }
   }
}

