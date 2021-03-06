/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:expand.c	1.19"

/*
 * expand() -- routine that shell-expands user input and then
 * passes it to validate().
 */
#include "muse.h"
#include "mmuse.h"


int validate();

int expand(upd)
int upd;
{
   FILE *e_fp;
   char *e_string[100], *t, *t_pt;
   register char *e_cmd, **e_s_pt=e_string;
   register int hassq=0, hasdq=0, slflag=0, expflag=0;
   register int flaw=0, numsq=0, numdq=0;
   register char *c_pt, *s, *word;
   register int j, k, test, kount;
   register int last_row;
   register struct segment *s_last_segm_pt, *s_Segm_pt;
   register struct segment *s_pt, *final_s_pt;
   struct segment *errs_pt = (struct segment *)0;
   extern char *visible[];
   char *s_buf_pt;
   char first, last;
   extern int done();
   int numsegms, lstrcount;

   /* char *e_string[] -- storage for strings generated by shell
      expansion */
   for (j=0; j<100; j++) e_string[j] = NULL;

   numsegms = 0;
   lstrcount = 0;

   /* Count how many strings the current field has */
   for (s_pt=Field_pt->first_s_pt; s_pt!=(struct segment*)0;
        s_pt=s_pt->next)
   {
      numsegms++;
      if (s_pt->word!=NULL && strlen(s_pt->word) > (COLS/2)-10)
         lstrcount++;
   }

   /* Terminate inserted string */
   TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
   if (mode==INSERT)  *buf_pt++ = null;          

/* Validation needed, even when input is empty  */
   if (Segm_pt->word==NULL || *(Segm_pt->word)==null)    
   {
      Segm_pt->word=NULL;
      update(Field_pt->last_row);
      goto val;
   }

/* Expansion switched off.  Go directly to validation   */
   if (Field_pt->bundle>=1) goto val;     

/* Check if string has quotes */
   s = Segm_pt->word;
   while (*s)
   {
      switch(*s)
      {
         case '\'':
            if (!slflag) numsq++;
            else slflag=0;
            break;
         case '"':
            if (!slflag) numdq++;
            else slflag=0;
            break;
         case '\\':
            if (slflag) slflag=0;
            else slflag=1;
            break;
         default:
            if (slflag) slflag=0;
            break;
      } 
      s++;
   }


   /* Check whether string is single or double quoted */
   s = Segm_pt->word;      
   if (*s != *(s+strlen(s)-1)) {
      first = *s;
      last = *(s+strlen(s)-1);
      slflag = 0;
      if (strlen(s)>1 && *(c_pt = s+strlen(s)-2) =='\\') slflag=1;
      while (--c_pt>s && *c_pt=='\\') slflag=1-slflag;
      if (slflag) last = null;
      if (first=='\'' || first=='\"' ||
          last=='\'' || last=='\"') {
        flaw = 7;     /* unbalanced quotes */
        goto pretest;
      }
   }
   else if (*s == *(s+strlen(s)-1))
      switch(*s)
      {
      case '\'':   /* string enclosed by single quotes */
         hassq=1;
         if (strlen(s)<3)  /* Check whether string is empty */
         {
            flaw=3;     /* string is: '' */
            goto pretest;
         }
         else if (numsq%2 !=0)
         {
            flaw=5;    /* unbalance single quotes */
            goto pretest;
         }
         else goto val;    /* A lot is skipped with single quotes */
         break;
      case '"':   /* string enclosed by double quotes */
         if (strlen(s)<3)  /* Check whether string is empty */
         {
            flaw=3;  /* string is "" */
            goto pretest;
         }
         else if (numdq%2 != 0)
         {
            flaw=6;  /* unbalanced number of quotes */
            goto pretest;
         }
         hasdq=1;
         break;
      }

/* At this point, we know that the string has sound (if any) quoting,
   and is NOT enclosed by single quotes.  However, it may be
   double-quoted (hasdq=1).  We are now going to look at
   the interior of the string -- shtring w/ initial and final
   balanced quotes removed.  We are going to test soundness and
   check if unquoted special shell characters are present */

   t_pt = t = (char*)calloc((unsigned)(strlen(s)+10),sizeof(char));
   word = s+strlen(s)-hasdq-hassq; /* Just an end point marker */
   slflag = 0;
   for (c_pt=s+hassq+hasdq; c_pt<word && !flaw; c_pt++)
      switch(*c_pt)
      {
      case ';':        /* Word terminators */
      case '&':
      case '(':
      case ')':
      case '|':
      case '^':
      case '<':
      case '>':
      case CTRL(M):
      case ' ':
      case CTRL(I):
         *t_pt++ = *c_pt;
         if (!hasdq && !slflag) flaw=1; /* Unquoted word terminator */
         if (slflag) slflag=0;
         break;
      case '`':        /* Special characters, even when double quoted */
      case '$':
         *t_pt++ = *c_pt;
         if (!slflag) expflag=1; /* Shell expansion required w/ ` or $ */
         else slflag=0;
         break;
      case '*':        /* Special characters, unless slash- or double quoted */
      case '?':
      case '[':
      case ']':
         *t_pt++ = *c_pt;
         if (!slflag &&!hasdq) expflag=1; /* Shell expansion required */
         if (slflag) slflag=0;
         break;
      case '\'':       /* Must be quoted */
         *t_pt++ = *c_pt;
         if (!slflag && !hasdq) flaw=2;
         if (slflag) slflag=0;
         break;
      case '"':       /* Must be quoted */
         *t_pt++ = *c_pt;
         if (!slflag) flaw=2;
         else slflag=0;
         break;
      case '\\':      /* Flip slash flag */
         *t_pt++ = *c_pt;
         *t_pt++ = *c_pt;
         slflag = 1-slflag;
         break;
      default:
         *t_pt++ = *c_pt;
         if (slflag) slflag=0;
         break;
      }

   *t_pt = null; /* Terminate interior string */
   if (slflag) flaw=4;  /* String ends with backslash */

pretest:    /* Generate error message from "flaw" index */
   if (flaw)
   {
      switch(flaw)
      {
      case 1:
	 strcpy(error_mess, " SURROUND");
         if ((int)(*--c_pt) <= 27)
            sprintf(error_mess, " \'%s\'",visible[(int)(*c_pt)]);
         else
            sprintf(error_mess, " \'%c\'",*c_pt);
         strcat(error_mess, 
                " WITH QUOTES OR PRECEDE WITH BACKSLASH ");
         break;
      case 2:
         strcpy(error_mess,
             " CONTAINS \" INSIDE STRING.  USE \'\' OR \\ ");
         break;
      case 3:
         strcpy(error_mess, " EMPTY STRING ");
         break;
      case 4:
         strcpy(error_mess,
             " BACKSLASH MUST BE FOLLOWED BY OTHER CHARACTER ");
         break;
      case 5:
         strcpy(error_mess,
             " ODD NUMBER OF SINGLE QUOTES ");
         break;
      case 6:
         strcpy(error_mess,
             " ODD NUMBER OF DOUBLE QUOTES ");
         break;
      case 7:
         strcpy(error_mess,
             " UNBALANCED SINGLE OR DOUBLE QUOTES ");
         break;
      }

      mode=INCORRECT;
      return(0);
   }

val:

   /* If no shell expansion is needed, validate and return */
   if (hassq || !expflag || Field_pt->bundle>=1)
   {
      if ((test=validate(0,1)))
      {
         command_line();
         return(test);
      }
      else  {
         mode = GRERROR;
         return(0);
      }
   }

/*
 * At this point, we know that the user string is sound AND needs
 * shell expansion.
 */
 /* Save Segm_pt, buf_pt, last_segm_pt, and last_row,
    in case generated segments are invalid.  */
   s_Segm_pt      = Segm_pt;
   s_buf_pt       = buf_pt;
   s_last_segm_pt = last_segm_pt;
   last_row       = Field_pt->last_row;

/*
 * Now expand string with echo(1).  Store in e_string[].
 * char **e_s_pt has been initialized already.
 */
   e_cmd = (char*)calloc((unsigned)(strlen(mecho)+strlen(t)+5),sizeof(char));
   sprintf(e_cmd,"%s %s",mecho,Segm_pt->word);
   *e_s_pt = buf_pt;
   TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
   if (hasdq) *buf_pt++ = '"';
   PR_POPEN;
   e_fp = popen(e_cmd,"r");
   if (e_fp==NULL)  /* popen() failed */
   {
      strcpy(error_mess, " SHELL EXPANSION FAILED ");
      mode = INCORRECT;
      return(0);
   }
/*
 * First check size of shell expansion, both in terms of total
 * number of strings, total number of characters needed,
 * and maximum individual string length.
 * Store in e_strings[].
 */
   kount = 0;

   while ((k=getc(e_fp))!=EOF && k!='\n' && kount++<1023)
      switch(k)
      {
      case ' ':
         if (!hasdq)
         {
            if (length(*e_s_pt) > COLS - 4 - Field_pt->col)
            {
               strcpy(error_mess, " STRING MUCH TOO LONG ");
               mode = INCORRECT;
               return(0);
            }
            else *++e_s_pt = ++buf_pt;
         }
         else  
         {
            TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
            *buf_pt++ = k;
            *buf_pt = null;
         }
         break;
      default:
         TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
         *buf_pt++ = k;
         *buf_pt=null;
         break;
      }
   TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
   if (hasdq) *buf_pt++ = '"';
   *buf_pt++ = null;
   *++e_s_pt=NULL;
   pclose(e_fp);
   PO_POPEN;

   if (kount >= 1022)    /* Too many characters */
   {
      strcpy(error_mess, " EXPANSION MUCH TOO LONG ");
      buf_pt = s_buf_pt;
      mode = INCORRECT;
      return(0);
   }
   if (e_s_pt > e_string+1 && Field_pt->maxfsegms<=1)
   {
      strcpy(error_mess," ONE STRING ONLY ");
      buf_pt = s_buf_pt;
      mode=INCORRECT;
      return(0);
   }
   else if (strlen(*e_string)==0)
   {
      strcpy(error_mess,
" EITHER NOT RECOGNIZED BY THE SHELL OR GENERATED TOO MANY STRINGS ");
      buf_pt = s_buf_pt;
      mode=INCORRECT;
      return(0);
   }

/*
 * Transfer contents of e_string[] to appropriate segments.
 * Validate string in the process, and exit when a fatal error
 * is found (mode - INCORRECT).
 */

   s_pt = last_segm_pt++;

   e_s_pt = e_string;
   while (*e_s_pt!=NULL && mode!=INCORRECT)
   {
      s_pt->word = *e_s_pt;
      Segm_pt = s_pt;
      if (validate(0,0) == 3 && errs_pt==(struct segment *)0) 
         errs_pt = s_pt;
      if ((last_segm_pt-segments  >= MAXSEGMS || 
           (int)(e_s_pt-e_string) > Field_pt->maxfsegms - numsegms)
          && mode!=INCORRECT)
      {
         strcpy(error_mess," TOO MANY STRINGS ");
         mode = INCORRECT;
      }
      else numsegms++;
      if (strlen(s_pt->word) > COLS/2-10 && lstrcount++ > 5 && mode!=INCORRECT)
      {
         strcpy(error_mess," TOO MANY STRINGS THAT REQUIRE AN ENTIRE LINE ");
         mode = INCORRECT;
      }
      s_pt->next = last_segm_pt++;
      (s_pt->next)->previous = s_pt;
      s_pt = s_pt->next;
      e_s_pt++;
   }

   final_s_pt = s_pt->previous;
   Segm_pt = s_Segm_pt;

/*
 * If some string was INCORRECT, reset everything.
 */
   if (mode==INCORRECT)
   {
      buf_pt       = s_buf_pt;
      for (s_pt = s_last_segm_pt; s_pt<last_segm_pt; s_pt++)
      {
         s_pt->next = s_pt->previous = NULL;
         s_pt->word = NULL;
      }
      last_segm_pt = s_last_segm_pt;
      show(Segm_pt);
      Col = Segm_pt->col;
      return(0);

   }

/*
 * If all strings are OK, put new segments in chain.
 * First blank out Segm_pt.
 */

   if (Segm_pt->word!=NULL && *(Segm_pt->word)!=null)
   {
      j = length(Segm_pt->word);
      mvaddch(Segm_pt->row-Stdscr_loc,Segm_pt->col,SPACE);
      while (--j>0) addch(SPACE);
   }

   /* Link new chain with preceding segment */
   if (Segm_pt->previous!=NULL)
   {
      (Segm_pt->previous)->next = s_last_segm_pt;
      s_last_segm_pt->previous = Segm_pt->previous;
   }
   else
      Field_pt->first_s_pt = s_last_segm_pt;

   
   /* Link new chain with following segment */
   if (Segm_pt->next!=NULL)
   {
      (Segm_pt->next)->previous = final_s_pt;
      final_s_pt->next = Segm_pt->next;
   }

   Segm_pt=final_s_pt;  /* Critical if expand followed by SPACE   */

/*
 * Update screen.
 */
   if (upd) update(last_row);
   command_line();
   if (errs_pt != (struct segment *)0)  /*Reset to first wrong segment*/
   {
      Segm_pt = errs_pt;
      validate(0,0);
   }
   if (mode == INCORRECT || mode == GRERROR) return(0);
   else return(1);
}
