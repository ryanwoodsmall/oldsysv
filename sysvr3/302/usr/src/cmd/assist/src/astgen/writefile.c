/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:writefile.c	1.13"


#include "../forms/muse.h"
#include "vdefs.h"

#define twoprint(k) {if (k>9) fprintf(fp0,"%2d",k); else fprintf(fp0,"0%1d",k); }
#define NOT_0(X)    ((X)!=NULL && *(X)!=null)
#define RMCR(X)     {if (X!=NULL && *(c_pt=(X+strlen(X)-1))=='\n') *c_pt=null;}


write_out(fp0)
register FILE *fp0;
{  register int i, j, dumflag;
   register struct field *f_pt;
   register struct index *in_pt;
   register struct fix *fi_pt;
   register struct vfunc *v_pt;
   register char *c_pt;

/*
 * Header.
 */
   if (*comment != null)
      for (c_pt = comment; *c_pt; c_pt++) 
         putc(*c_pt,fp0);
   fprintf(fp0,"%c0",CTRL(E));
   twoprint(clt_code);
   fprintf(fp0,"00%s\n",lab_pt->screen_name);


/*
 * Field captions
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      i = f_pt-fields;
      twoprint(i);
      fprintf(fp0,"%1d",f_pt->type);
      twoprint(f_pt->row);
      twoprint(f_pt->col);
      twoprint(f_pt->minfsegms);
      twoprint(f_pt->maxfsegms);
      fprintf(fp0,"%s\n",f_pt->caption);
   }
 
   fprintf(fp0,"%c\n",CTRL(R));
   
/*
 * Option specifications.
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9)
      {
         i = f_pt-fields;
         twoprint(i);
         twoprint(f_pt->loc); 
         fprintf(fp0,"%1d",f_pt->bundle); 
         if NOT_0(f_pt->op_name) {
            if (f_pt->bundle && strlen(f_pt->op_name)>1)
               fprintf(fp0,"%s",(f_pt->op_name)+1);  /* Skip "-"   */
            else
               fprintf(fp0,"%s",f_pt->op_name);
         }
         if NOT_0(f_pt->def_arg) fprintf(fp0,"%1c%s",CTRL(Z),f_pt->def_arg);
         fprintf(fp0,"\n");
      }
   }

/*
 * Exit message.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   if NOT_0(exit_mess)  {
      RMCR(exit_mess); RMCR(exit_mess);
      fprintf(fp0,"%s",exit_mess);
      if (exit_field != -1) {
         fprintf(fp0,"%1c",CTRL(Z));
         twoprint(exit_field);
         for (exit_strs_pt = exit_strs;
              *exit_strs_pt != NULL; exit_strs_pt++)
            fprintf(fp0,"%1c%s",CTRL(X),*exit_strs_pt);
      }
      fprintf(fp0,"\n");
   }

/*
 * Command form help.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   if NOT_0(form_help) {
      RMCR(form_help);
      fprintf(fp0,"%s\n",form_help);
   }
   
/*
 * Item help.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9)
      {
         i = f_pt-fields;
         twoprint(i);
         if (NOT_0(f_pt->help)) {
            RMCR(f_pt->help);
            fprintf(fp0,"%s%c\n",f_pt->help,CTRL(Z));
         }
         else
            fprintf(fp0,"%c\n",CTRL(Z));
      }
   }

/*
 * Incompatible options.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->first_i_pt!=NULL && f_pt->type!=9)
      {
         i = f_pt-fields;
         twoprint(i);
         for (in_pt=f_pt->first_i_pt; in_pt!=NULL; in_pt = in_pt->next)
            twoprint(in_pt->num);
         fprintf(fp0,"\n");
      }
   }

/*
 * Conditionally mandatory options.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->first_m_pt!=NULL && f_pt->type!=9)
      {
         i = f_pt-fields;
         twoprint(i);
         for (in_pt=f_pt->first_m_pt; in_pt!=NULL; in_pt = in_pt->next)
            twoprint(in_pt->num);
         fprintf(fp0,"\n");
      }
   }

/*
 * Segments.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type !=0 && f_pt->first_s_pt!=NULL 
          && NOT_0(f_pt->first_s_pt->word) && f_pt->type!=9) {
         i = f_pt-fields;
         twoprint(i);
         twoprint(f_pt->row);
         j = f_pt->col+strlen(f_pt->caption)+1;
         twoprint(j);
         if (f_pt->first_s_pt!=NULL && f_pt->first_s_pt->word!=NULL)
            fprintf(fp0,"%s",f_pt->first_s_pt->word);
         fprintf(fp0,"\n");
      }
   }

/*
 * Validation functions
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9 && f_pt->first_v_pt!=NULL) 
      {
         for (v_pt=f_pt->first_v_pt; v_pt!=NULL; v_pt = v_pt->next)
         {
            dumflag=0;
            i = f_pt-fields;
            twoprint(i);
            twoprint(v_pt->num);
            if ((fi_pt=v_pt->pr_pt)!=NULL && fi_pt->name!=NULL) {
               dumflag=1;
               fprintf(fp0,"%s",fi_pt->name);
               fi_pt = fi_pt->next;
               for (; fi_pt!=NULL && fi_pt->name!=NULL; 
                      fi_pt=fi_pt->next)
                  fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
            }
            if (dumflag) fprintf(fp0,"%1c",CTRL(Z));
            if ((fi_pt=v_pt->po_pt)!=NULL && fi_pt->name!=NULL) {
               if (!dumflag) fprintf(fp0,"%1c",CTRL(Z));
               fprintf(fp0,"%s",fi_pt->name);
               fi_pt = fi_pt->next;
               for (; fi_pt!=NULL && fi_pt->name!=NULL;
                      fi_pt=fi_pt->next)
                  fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
            }
            fprintf(fp0,"\n");
         }
      }
   }

/*
 * Pre- and suffixes.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type == 6 && 
          (f_pt->first_pr_pt!=NULL || f_pt->first_po_pt!=NULL)) {
         i = f_pt-fields;
         twoprint(i);
         if ((fi_pt=f_pt->first_pr_pt)!=NULL && fi_pt->name!=NULL) {
            fprintf(fp0,"%s",fi_pt->name);
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL && fi_pt->name!=NULL; fi_pt=fi_pt->next)
               fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
         }
         fprintf(fp0,"%1c",CTRL(Z));
         if ((fi_pt=f_pt->first_po_pt)!=NULL && fi_pt->name!=NULL) {
            fprintf(fp0,"%s",fi_pt->name);
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL && fi_pt->name!=NULL; fi_pt=fi_pt->next)
               fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
         }
         fprintf(fp0,"\n");
      }
   }

/*
 * Command line generation strings
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type != 0  && f_pt->type!=9 && 
          ((fi_pt=f_pt->first_cpr_pt)!=NULL  && fi_pt->name!=NULL) || 
          ((fi_pt=f_pt->first_cpo_pt)!=NULL  && fi_pt->name!=NULL) ) {
         i = f_pt-fields;
         twoprint(i);
         if ((fi_pt=f_pt->first_cpr_pt)!=NULL && fi_pt->name!=NULL) {
            fprintf(fp0,"%s",fi_pt->name);
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL && fi_pt->name!=NULL; fi_pt=fi_pt->next)
               fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
         }
         fprintf(fp0,"%1c",CTRL(Z));
         if ((fi_pt=f_pt->first_cpo_pt)!=NULL && fi_pt->name!=NULL) {
            fprintf(fp0,"%s",fi_pt->name);
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL &&fi_pt->name!=NULL; fi_pt=fi_pt->next)
               fprintf(fp0,"%1c%s",CTRL(X),fi_pt->name);
         }
         if (f_pt->type==7)
            if ((fi_pt=f_pt->first_cpo_pt)==NULL || fi_pt->name==NULL ||
                (fi_pt=fi_pt->next)==NULL || fi_pt->name==NULL)
                   fprintf(fp0,"%1c",CTRL(X));
         fprintf(fp0,"\n");
      }
   }

   fprintf(fp0,"%c\n",CTRL(R));

   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9 && f_pt->first_a_pt!=NULL)
      {
         i = f_pt-fields;
         twoprint(i);
         for (in_pt=f_pt->first_a_pt; in_pt!=NULL; in_pt = in_pt->next)
            twoprint(in_pt->num);
         fprintf(fp0,"\n");
      }
   }

/*
 * Placeholder sections.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   fprintf(fp0,"%c\n",CTRL(R));

}






mwrite_out(fp0)
register FILE *fp0;
{  register int i, j;
   register struct field *f_pt;
   char *c_pt;

   if (*comment != null)
      for (c_pt = comment; *c_pt; c_pt++) 
         putc(*c_pt,fp0);

/*
 * Header.
 */
   fprintf(fp0,"%c400%s\n",CTRL(E),lab_pt->screen_name);


/*
 * Field captions
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      i = f_pt-fields;
      twoprint(i);
      fprintf(fp0,"%1d",f_pt->type);
      twoprint(f_pt->row);
      twoprint(f_pt->col);
      twoprint(f_pt->minfsegms);
      twoprint(f_pt->maxfsegms);
      fprintf(fp0,"%s\n",f_pt->caption);
   }
 

/*
 * Menu form help.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   if NOT_0(form_help)  {
      RMCR(form_help);
      fprintf(fp0,"%s\n",form_help);
   }
   
/*
 * Item help.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9)
      {
         i = f_pt-fields;
         twoprint(i);
         RMCR(f_pt->help);
         if (f_pt->help!=NULL) fprintf(fp0,"%s%c\n",f_pt->help,CTRL(Z));
         else fprintf(fp0,"%c\n",CTRL(Z));
      }
   }

/*
 * Segments.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type !=0 && f_pt->type!=9 && f_pt->first_s_pt!=NULL
                         && NOT_0(f_pt->first_s_pt->word)) {
         i = f_pt-fields;
         twoprint(i);
         twoprint(f_pt->row);
         j = f_pt->first_s_pt->col;
         twoprint(j);
         fprintf(fp0,"%s",f_pt->first_s_pt->word);
         fprintf(fp0,"\n");
      }
   }

/*
 * Actions.
 */
   fprintf(fp0,"%c\n",CTRL(R));
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type !=0 && f_pt->type!=9
                         && NOT_0(f_pt->first_s_pt->word)) {
         i = f_pt-fields;
         twoprint(i);
         if (f_pt->action != NULL) fprintf(fp0,"%s\n",f_pt->action);
         else fprintf(fp0,"\n");
      }
   }

   fprintf(fp0,"%c\n",CTRL(R));

}



int chkwrite()
{  register struct field *f_pt;
   register struct segment *s_pt;
   register struct index *in_pt;
   register struct fix *fi_pt;
   register struct vfunc *v_pt;
   int n_segment=0, n_fix=0, n_index=0, n_vfunc=0;

   if ((int)(last_field_pt-fields) > MAXFIELDS) return(40);

   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   for (s_pt=f_pt->first_s_pt; s_pt!=(struct segment*)0; s_pt=s_pt->next)
         n_segment++;
/*
 * Incompatible options.
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
      if (f_pt->type!=0 && f_pt->first_i_pt!=NULL && f_pt->type!=9)
         for (in_pt=f_pt->first_i_pt; in_pt!=NULL; in_pt = in_pt->next)
            n_index++;

/*
 * Conditionally mandatory options.
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
      if (f_pt->type!=0 && f_pt->first_m_pt!=NULL && f_pt->type!=9)
         for (in_pt=f_pt->first_m_pt; in_pt!=NULL; in_pt = in_pt->next)
            n_index++;


/*
 * Validation functions
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type!=0 && f_pt->type!=9 && f_pt->first_v_pt!=NULL) 
      {
         for (v_pt=f_pt->first_v_pt; v_pt!=NULL; v_pt = v_pt->next)
         {
            n_vfunc++;
            if ((fi_pt=v_pt->pr_pt)!=NULL && fi_pt->name!=NULL) {
               n_fix++;
               fi_pt = fi_pt->next;
               for (; fi_pt!=NULL; fi_pt=fi_pt->next)
                  n_fix++;
            }
            if ((fi_pt=v_pt->po_pt)!=NULL && fi_pt->name!=NULL) {
               n_fix++;
               fi_pt = fi_pt->next;
               for (; fi_pt!=NULL && fi_pt->name!=NULL;
                      fi_pt=fi_pt->next)
                  n_fix++;
            }
         }
      }
   }

/*
 * Pre- and suffixes.
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type == 6 && 
          (f_pt->first_pr_pt!=NULL || f_pt->first_po_pt!=NULL)) {
         if ((fi_pt=f_pt->first_pr_pt)!=NULL && fi_pt->name!=NULL) {
            n_fix++;
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL &&fi_pt->name!=NULL; fi_pt=fi_pt->next)
               n_fix++;
         }
         if ((fi_pt=f_pt->first_po_pt)!=NULL && fi_pt->name!=NULL) {
            n_fix++;
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL &&fi_pt->name!=NULL; fi_pt=fi_pt->next)
               n_fix++;
         }
      }
   }

/*
 * Command line generation strings
 */
   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
   {
      if (f_pt->type != 0  && f_pt->type!=9 && 
          ((fi_pt=f_pt->first_cpr_pt)!=NULL  && fi_pt->name!=NULL) || 
          ((fi_pt=f_pt->first_cpo_pt)!=NULL  && fi_pt->name!=NULL) ) {
         if ((fi_pt=f_pt->first_cpr_pt)!=NULL && fi_pt->name!=NULL) {
            n_fix++;
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL && fi_pt->name!=NULL; fi_pt=fi_pt->next)
               n_fix++;
         }
         if ((fi_pt=f_pt->first_cpo_pt)!=NULL && fi_pt->name!=NULL) {
            n_fix++;
            fi_pt = fi_pt->next;
            for (; fi_pt!=NULL &&fi_pt->name!=NULL; fi_pt=fi_pt->next)
               n_fix++;
         }
      }
   }


   for (f_pt=fields;f_pt<=last_field_pt;f_pt++)
      if (f_pt->type!=0 && f_pt->type!=9 && f_pt->first_a_pt!=NULL)
         for (in_pt=f_pt->first_a_pt; in_pt!=NULL; in_pt = in_pt->next)
            n_index++;

   if (n_segment > MAXSEGMS) return(41);
   if (n_index > MAXINDICES) return(43);
   if (n_vfunc > NUMVFUNCS) return(42);
   if (n_fix > NFIX) return(44);
   return(0);
}
