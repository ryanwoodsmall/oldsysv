/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:read_in.c	1.12"
#include    "muse.h"

#define     NOT_IN_SCRIPT      0
#define     IN_SCRIPT          1


VOID read_in(flag)
int flag;      /* 0: do not execute shell scripts, 1: do ... */
{  
   register int c, i, k, j;
   register int maxrow, state, CR_flag;
   register int count, n;
   register int flip_flop, def_arg;
   register char *c_pt;
   static char *files_name=NULL;
   register struct field *f_pt;
   register struct segment *s_pt;
   register struct index *in_pt;
   register struct fix *fi_pt;
   register struct vfunc *v_pt;
   char *lbuffer=NULL, *lbuf_pt=NULL;
   FILE *fp0;
   int implement();
   struct stat buf;
   int ctrl_R = 0;
   int ign_CR_flag = 0;


/*
 *
 * Construct file names for input and output files. 
 *
 */

   if (flag && implement(lab_pt->files_name,1)==0) {
      mvaddstr(LINES-2,0,error_mess);
      move(LINES-1,0);
      refresh();
      endwin();
      exit(1);
   }

   if (files_name!=NULL) free(files_name);
   files_name = (char*)calloc((unsigned)(strlen(lab_pt->files_name)+
                                         strlen(fs_forms) + 10),
                              sizeof(char));
   sprintf(files_name,"%s%s.fs",fs_forms,lab_pt->files_name);
   stat(files_name,&buf);
   if (buffer!=NULL) free(buffer);
   if (lbuffer!=NULL) free(lbuffer);

   BUFSIZE = 2 * (buf.st_size + 2);
   buf_pt = buffer = (char*)calloc((unsigned)BUFSIZE,sizeof(char));
   lbuf_pt = lbuffer = (char*)calloc((unsigned)BUFSIZE,sizeof(char));

/*
 *
 * Reset everything. 
 *
 */

   form_help = exit_mess = NULL;

   maxpage = page = 1;
   maxrow = 0;

   last_field_pt = fields+MAXFIELDS-1;
   last_segm_pt = segments+MAXSEGMS-1;
   last_fi_pt = fixes+NFIX-1;
   last_in_pt = indices+MAXINDICES-1;
   last_v_pt = vfuncs+NUMVFUNCS-1;

   mode = NEWFIELD;
   for (f_pt=last_field_pt; f_pt>=fields; f_pt--)
   {
      f_pt->first_s_pt = NULL;
      f_pt->first_v_pt = NULL;
      f_pt->first_i_pt = NULL;
      f_pt->first_m_pt = NULL;
      f_pt->first_a_pt = NULL;
      f_pt->op_name    = NULL;
      f_pt->def_arg    = NULL;
      f_pt->loc = f_pt->bundle = 0;
      f_pt->caption = buf_pt;
      f_pt->action = buf_pt;
      f_pt->help = buf_pt;
      f_pt->first_pr_pt = NULL;
      f_pt->first_po_pt = NULL;
   }
   last_field_pt = fields;

   for (s_pt = last_segm_pt; s_pt>=segments; s_pt--)
   {
      s_pt->word = NULL;
      s_pt->row = NULL;
      s_pt->col = NULL;
      s_pt->next = NULL;
      s_pt->previous = NULL;
   }
   last_segm_pt = segments;


   for (fi_pt = last_fi_pt; fi_pt>=fixes; fi_pt--)
   {
      fi_pt->name = NULL;
      fi_pt->next = NULL;
   }
   last_fi_pt = fixes;

   for (in_pt = last_in_pt; in_pt>=indices; in_pt--)
   {
      in_pt->num = 0;
      in_pt->next = NULL;
   }
   last_in_pt = indices;

   for (v_pt = last_v_pt; v_pt>=vfuncs; v_pt--)
   {
      v_pt->num = 0;
      v_pt->next = NULL;
      v_pt->pr_pt = NULL;
      v_pt->po_pt = NULL;
   }
   last_v_pt = vfuncs;

   count = 0;

   for (i=0;i<NRETS;i++)  rets[i] = -1;
   buf_pt++;

/*
 *
 * Read in caption file:  "fftrrcc<caption>", ff:field#, t:type, rr:row
 * cc:col, and <caption> is caption; 
 *
 */

   fp = fopen(files_name,"r");
   if (fp==NULL)
   {
      status = 2;
      done();
   }
   
/*
 * Now put information in temporary buffer lbuffer[].  Strip
 * text precedinf (first) ^E and coming after each ^R (but on same line).
 * Execute shell scripts, enclosed by ^Y's.  buffer[] is used to
 * store these scripts.
 */

   state = NOT_IN_SCRIPT;

/*
 * Store pre-^E stuff in comment[], but only if astgen is used.
 */
   if (flag==0) {
      c_pt = comment;
      while ((c=getc(fp))!=CTRL(E) && (int)(c_pt-comment)<1023) 
        *c_pt++ = c; 
      *c_pt = null;
   }
   else while ((c=getc(fp))!=CTRL(E));

   *lbuf_pt++ = CTRL(E);
   mode = ((c=getc(fp))=='4') ? MENU : 0;
   ungetc(c,fp);

   j = 0;
   CR_flag = 0;
   while ((c=getc(fp))!=EOF && ++j < BUFSIZE-1)
      switch(c)
      {
      case CTRL(Y):
         if (flag) {              /* Execute shell script only of flag is 1 */
            switch(state) {
            case NOT_IN_SCRIPT:   /* First ^Y: get into script mode         */
               state = IN_SCRIPT;
               buf_pt=buffer;     /* Get set up to read script in buffer[]  */
               break;
            case IN_SCRIPT:       /* Second ^Y: get out of script mode      */
               *buf_pt=null;      /* Terminate buffer[]                     */
                                  /* Execute script; add result to lbuffer[] */
               CR_flag = 0;
               if (mode==MENU && ctrl_R>=1 && ctrl_R<=2) 
                  ign_CR_flag = 1;
               else if (mode!=MENU && ctrl_R>=2 && ctrl_R<=4) 
                  ign_CR_flag = 1;
               else
                  ign_CR_flag = 0;
               PR_POPEN;
               fp0 = popen(buffer,"r"); 
               while ((k=getc(fp0))!=EOF && ++j < BUFSIZE-1) 
               {
                  if (k!=CTRL(Z) && k!=CTRL(R) && k!=CTRL(X) &&
                      (!CR_flag || ign_CR_flag ))
                     *lbuf_pt++ = k;   
                  else 
                  {
                     pclose(fp0);
                     PO_POPEN;
                     status = 16;
                     done();
                  }
                  if (k=='\n') CR_flag=1;
               }
               if (CR_flag)  *--lbuf_pt = null;    
               else *lbuf_pt=null;
               pclose(fp0);
               PO_POPEN;
               state = NOT_IN_SCRIPT;
               break;
            }
         }
         else
            *lbuf_pt++ = c;    /* Add to ordinary buffer */
         break;
      case CTRL(R):            /* Skip rest of line, but not in script mode */
         if (state==NOT_IN_SCRIPT)
         {
            ctrl_R++;
            *lbuf_pt++ = CTRL(R);
            while ((c=getc(fp))!='\n' && ++j < BUFSIZE-1);
            *lbuf_pt++ = '\n';
         }
         break;
      default:
         switch(state)
         {
         case NOT_IN_SCRIPT:
            *lbuf_pt++ = c;    /* Add to ordinary buffer */
            break;
         case IN_SCRIPT:
            *buf_pt++ = c;     /* Add to script-buffer */
            break;
         }
         break;
      }
   fclose(fp);

   if (j> BUFSIZE-3)
   {
      status = 15;
      done();
   }

   lbuf_pt = lbuffer;
   buf_pt = buffer;
   *buf_pt = null;
/*
 * Header line first: ^Embb\n, where m is mode and bb is clt_code.   
 */
   if (*lbuf_pt++!=CTRL(E))                 /* Read ^E */
   {
      status = 3;
      done();
   }

   if ((c = *lbuf_pt++)>'0' && c<='9')            /* Read m */
   {
      mode=MENU;   
      SCRLINES = LINES-3;
   }
   else if (c=='0')
   {
      mode = NEWFIELD;
      SCRLINES = LINES-5;
   }
   else 
   {
      status = 3;
      done();
   }

   if ((c = *lbuf_pt++)<'0' || c>'9')             /* Read b */
   {
      status = 3;
      done();
   }
   else  clt_code = (c-'0')*10;
   if ((c = *lbuf_pt++)<'0' || c>'9')             /* Read b */
   {
      status = 3;
      done();
   }
   else  clt_code += c-'0';

   if (mode==MENU) c = *lbuf_pt++;
   else                               /* Read rets[] and \n               */
   {
      i = flip_flop = 0;
      while ((c = *lbuf_pt++) >= '0' && c <= '9')
      {
         switch(flip_flop)
         {
         case 0:
            rets[i] = 10*(c-'0');
            break;
         case 1:
            rets[i++] += c-'0';
            break;
         }
         flip_flop = 1-flip_flop;
      }
   }

   if (c!='\n')   
   {
      *(lab_pt->screen_name) = c;
      c_pt = (lab_pt->screen_name)+1;
      while ((c = *lbuf_pt++) != '\n') *c_pt++ = c;
      *c_pt = null;
   }
   else *(lab_pt->screen_name) = null;

   while ((c = *lbuf_pt++)!=CTRL(R))
   {
      switch(c)
      {
      case '\n':                         /* End of input line: next field     */
         *buf_pt++ = null;               /* Terminate caption                 */
         if (f_pt->type!=0 && f_pt->type!=9) /* f_pt->first_s_pt initialized      */
         {
            TOOMANY(5,last_segm_pt+1,segments,MAXSEGMS);
            s_pt = f_pt->first_s_pt = last_segm_pt++;
            s_pt->row = f_pt->row;
            s_pt->col = f_pt->col + strlen(f_pt->caption) + 1;
            s_pt->word = buf_pt;
            *buf_pt++ = null;
         }
         maxrow = (maxrow>f_pt->row) ? maxrow : f_pt->row;
         count=0;                        /* Set digit counter to 0            */
         break;
      default:                           /* Digit or caption character:       */
         switch(count)                   /* If count<5, c must be a digit:    */
         {
         case 0:
            n = c-'0';
            break;
         case 1:
            f_pt = fields + c-'0' + 10*n;
            last_field_pt = (last_field_pt>f_pt)? last_field_pt : f_pt;
            TOOMANY(8,last_field_pt+1,fields,MAXFIELDS);
            f_pt->caption = buf_pt;
            break;
         case 2:                         /* type                              */
            f_pt->type = c-'0';
            break;
         case 3:                         /* row                               */
            f_pt->row = 10*(c-'0');
            break;
         case 4:                         /* row                               */
            f_pt->row += c-'0';
            f_pt->last_row = f_pt->row;
            break;
         case 5:                         /* col                               */
            f_pt->col = 10*(c-'0');
            break;
         case 6:                         /* col                               */
            f_pt->col += c-'0';
            break;
         case 7:                         /* minfsegms                         */
            f_pt->minfsegms = 10*(c-'0');
            break;
         case 8:                         /* minfsegms                         */
            f_pt->minfsegms += c-'0';
            break;
         case 9:                         /* maxfsegms                         */
            f_pt->maxfsegms = 10*(c-'0');
            break;
         case 10:                         /* maxfsegms                         */
            f_pt->maxfsegms += c-'0';
            break;
         default:                        /* Caption character  if count>=5    */
            *buf_pt++ = c;
            break;
         }
         count++;
         break;
      }
   }


/*
 *
 * Read in option specification file, for command forms only.
 *
 */

   if (mode!=MENU)
   {
      count = 0;
      def_arg = 0;
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                               /* End of input line: next field */
            *buf_pt++ = null;
            if (def_arg==1)
            {
               i = strlen(f_pt->def_arg);
               if (i && flag) 
               {
/*                shexp(f_pt->def_arg);   */
                  *(buf_pt += strlen(f_pt->def_arg)-i) = null;
               }
            }
            def_arg=0;
            count=0;
            break;
         case CTRL(Z):
            *buf_pt++ = null;
            def_arg = 1;
            f_pt->def_arg = buf_pt;
            break;
         default:
            if (def_arg==0)
            {
               switch(count)
               {
               case 0:
                  n = c-'0';
                  break;
               case 1:
                  f_pt = fields + c-'0' + 10*n;
                  break;
               case 2:                        /* Type of option                   */
                  n  = c-'0';
                  break;
               case 3:                        /* Type of option                   */
                  f_pt->loc  = c-'0' + 10*n;
                  break;
               case 4:                        /* Location of option in cmndline   */
                  f_pt->bundle  = c-'0';
                  f_pt->op_name = buf_pt;
                  break;
               default:                       /* Next char of option name         */
                  *buf_pt++ = c;
                  break;
               }
               count++;
            }
            else *buf_pt++ = c;
                                              /* Next char of default arg.        */
            break;
         }
      }
   }


   if (flag!=0 && mode!=MENU) close_gaps();

   if (mode!=MENU)
   {
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      count = 0;
      exit_mess = buf_pt;
      exit_field = -1;
      exit_strs_pt = exit_strs;
      *exit_strs_pt = NULL;
      while ((c = *lbuf_pt++)!=CTRL(R)) 
         switch(c)
         {
            case CTRL(Z):
               count=1;
               *buf_pt++ = '\n';
               *buf_pt++ = null;
               break;
            case CTRL(X):
               *buf_pt++ = null;
               *exit_strs_pt++ = buf_pt;
               *exit_strs_pt = NULL;
               break;
            default:
               switch(count)
               {
               case 0:
                  *buf_pt++=c;
                  break;
               case 1:
                  exit_field = 10*(c-'0');
                  count++;
                  break;
               case 2:
                  exit_field += (c-'0');
                  count++;
                  break;
               default:
                  if (c!='\n') *buf_pt++=c;
                  break;
               }
         }
      if (count==0 && buf_pt>exit_mess) *buf_pt++ = '\n';
      *buf_pt++ = null;
      if (*exit_mess==null) exit_mess=NULL;
   }



/*
 *
 * Read in form help.
 *
 */

   if ((c = *lbuf_pt++)!='\n')
   {
      status = 3;
      done();
   }
   form_help = buf_pt;
   while ((c = *lbuf_pt++)!=CTRL(R)) *buf_pt++=c;
   *buf_pt++ = null;


/*
 *
 * Read in item help.
 *
 */

   count=0;
   if ((c = *lbuf_pt++)!='\n')
   {
      status = 3;
      done();
   }
   while ((c = *lbuf_pt++)!=CTRL(R))
   {
      switch(c)
      {
      case CTRL(Z):
         *buf_pt++ = '\n';
         *buf_pt++ = null;
         count=0;
         break;
      default:
         switch(count)
         {
            case 0:
               if (c!='\n') n = c-'0';  /* Tricky way to skip \n after $     */
               else count--;
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               f_pt->help = buf_pt;
               break;
            default:
               if (f_pt->help<buf_pt || c!='\n') *buf_pt++ = c;
               break;
         }
         count++;
         break;
      }
   }




/*
 *
 * Pairs of incompatible options, for command forms only.
 *
 */

   if (mode!=MENU)
   {
      flip_flop = 0;
      count = 0;
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                          /* End of input line: next field    */
            count=0;
            break;
         default:
            switch(count)
            {
            case 0:
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               break;
            default:
               switch(flip_flop)
               {
               case 0:                    /* First digit of 2-digit funx indx */
                  TOOMANY(7,last_in_pt+1,indices,MAXINDICES);
                  if (count==2)
                     in_pt = f_pt->first_i_pt = last_in_pt++;
                  else
                     in_pt = in_pt->next = last_in_pt++;
                  in_pt->num = 10*(c-'0');
                  break;
               case 1:                    /* Second digit of indx             */
                  in_pt->num += c-'0';
                  break;
               }
               flip_flop = 1 - flip_flop;
               break;
            }
            count++;
            break;
         }
      }
   }


   if (mode!=MENU)   /* Conditionally mandatory options   */
   {
      flip_flop = 0;
      count = 0;
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                          /* End of input line: next field    */
            count=0;
            break;
         default:
            switch(count)
            {
            case 0:
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               break;
            default:
               switch(flip_flop)
               {
               case 0:                    /* First digit of 2-digit funx indx */
                  TOOMANY(7,last_in_pt+1,indices,MAXINDICES);
                  if (count==2)
                     in_pt = f_pt->first_m_pt = last_in_pt++;
                  else
                     in_pt = in_pt->next = last_in_pt++;
                  in_pt->num = 10*(c-'0');
                  break;
               case 1:                    /* Second digit of indx             */
                  in_pt->num += c-'0';
                  break;
               }
               flip_flop = 1 - flip_flop;
               break;
            }
            count++;
            break;
         }
      }
   }



/*
 *
 *  Read in input file:  "rrff<input>SPACErrff<...", where r is row, f is first 
 *  character column, and <input> is input string.   Length is computed. 
 *  First initialize pointers and counters and indices.
 *
 */

   f_pt = fields;
   count     = 0;

   if ((c = *lbuf_pt++)!='\n') /* starts directly with real input; it ends on ^R     */
   {
      status = 3;
      done();
   }


   while((c = *lbuf_pt++) != CTRL(R))
   {
      switch(c)
      {
      case '\n':                          /* End of input line: next field    */
         *buf_pt++ = null;                /* Terminate current segment        */
         i = strlen(s_pt->word);
         if (i && mode!=MENU && flag) 
         {
/*          shexp(s_pt->word);   */
            *(buf_pt += strlen(s_pt->word)-i) = null;
         }
         f_pt->row = f_pt->first_s_pt->row; 
         count = 0;                       /* Digit counter to 0               */
         break;
      case CTRL(X):                       /* Next segment on same input line  */
         *buf_pt++ = null;
         i = strlen(s_pt->word);
         if (i && flag) 
         {
/*          shexp(s_pt->word); */
            *(buf_pt += strlen(s_pt->word)-i) = null;
         }
         TOOMANY(5,last_segm_pt+1,segments,MAXSEGMS);
         s_pt->next = last_segm_pt++;
         (s_pt->next)->previous = s_pt;
         s_pt = s_pt->next;
         count = 2;                       /* Digit counter to 0               */
         break;
      default:
         switch(count)                    /* If count<4 c is digit:           */
         {
         case 0: 
            n = c-'0';
            break;
         case 1:
            f_pt = fields + c-'0' + 10*n;
            s_pt  = f_pt->first_s_pt;     /* Initialized during caption read */
            break;
         case 2:                          /* row                              */
            s_pt->row = 10*(c-'0');
            s_pt->word = buf_pt;
            break;
         case 3:                          /* row                              */
            s_pt->row += c-'0';
            f_pt->last_row = s_pt->row;
            maxrow = (maxrow>s_pt->row) ? maxrow : s_pt->row;
            break;
         case 4:                          /* col                              */
            s_pt->col = 10*(c-'0');
            break;
         case 5:                          /* col                              */
            s_pt->col += c-'0';
            break;
         default:                         /* IF count>=4 c IS SEGMENT CHAR.:  */
            *buf_pt++ = c;
            break;
         }
         count++;
         break;
      }
   }

/*
 * Set maxpage.
 */
   maxpage = (maxrow-1)/(SCRLINES-2) + 1;


/*
 *
 * Initialize command line, for command forms only.
 *
 */

   if (mode != MENU)
   {
      strcpy(init_cmd," COMMAND LINE: ");
      strcat(init_cmd,lab_pt->screen_name);
      strcpy(dum_cmd_name,lab_pt->screen_name);
      cmd_col = strlen(init_cmd);
   }



/*
 *
 * Read validation functions, for command forms only.
 *
 */

   if (mode!=MENU)
   {
      flip_flop = 0;
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                          /* End of input line: next field    */
            *buf_pt++ = null;
            count=0;                         /* Re-assign field pointer          */
            break;
         default:
            switch(count)
            {
            case 0:
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;;
               break;
            case 2:        /* valfunc number */
               TOOMANY(7,last_v_pt+1,vfuncs,NUMVFUNCS);
               if (f_pt->first_v_pt == NULL) 
                  v_pt=f_pt->first_v_pt=last_v_pt++;
               else 
                  v_pt=v_pt->next = last_v_pt++;
               n = c-'0';
               break;
            case 3:
               v_pt->num = c-'0' + 10*n;
               flip_flop = -1;
               break;
            default:       /* valfunc has arguments */
               if (flip_flop == -1)  /*First character of first prefix*/
               {
                  TOOMANY(6,last_fi_pt+1,fixes,NFIX);
                  fi_pt = v_pt->pr_pt = last_fi_pt++;
                  fi_pt->name = buf_pt;
                  flip_flop = 0;
               }
               switch(c)
               {
               case CTRL(X):     
                  *buf_pt++ = null;
                  TOOMANY(6,last_fi_pt+1,fixes,NFIX);
                  fi_pt = fi_pt->next = last_fi_pt++;
                  fi_pt->name = buf_pt;
                  break;
               case CTRL(Z): 
                  *buf_pt++ = null;
                  TOOMANY(6,last_fi_pt+1,fixes,NFIX);
                  fi_pt = v_pt->po_pt = last_fi_pt++;
                  fi_pt->name = buf_pt;
                  break;
               default:
                  *buf_pt++ = c;
                  break;
               }
               break;
            }
            count++;
            break;
         }
      }
   }


/*
 * Read action information, for menus only.
 */
   if (mode==MENU)
   {
      if ((c = *lbuf_pt++)!='\n')                    /* Read \n */
      {
         status = 3;
         done();
      }
      count=0;
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                       /* End of input line: next field    */
            *buf_pt++ = null;             /* Terminate action[]               */
            count=0;                      /* Set digit counter to 0           */
            break;
         default:                         /* Digit or action character:       */
            switch(count)                 /* If count<2, c must be a digit.   */
            {
            case 0:
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               f_pt->action = buf_pt;
               break;
            default:                     /* Caption character  if count>=2    */
               *buf_pt++ = c;
               break;
            }
            count++;
            break;
         }
      }
   }


/*
 * Finally, read pre- and post-fixes needed for certain validation functions. 
 */
   if (mode!=MENU)
   {
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                            /* End of input line: next field    */
            *buf_pt++ = null;                  /* Terminate current segment        */
            count = 0;                         /* Digit counter to 0               */
            break;
         case CTRL(Z):
            *buf_pt++ = null;
            TOOMANY(6,last_fi_pt+1,fixes,NFIX);
            fi_pt = f_pt->first_po_pt = last_fi_pt++;
            fi_pt->name = buf_pt;
            break;
         case CTRL(X):                             /* Next segment on same input line  */
            *buf_pt++ = null;
            TOOMANY(6,last_fi_pt+1,fixes,NFIX);
            fi_pt = fi_pt->next = last_fi_pt++;
            fi_pt->name = buf_pt;
            break;
         default:
            switch(count)                      /* If count<4 c is digit:           */
            {
            case 0: 
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               TOOMANY(6,last_fi_pt+1,fixes,NFIX);
               fi_pt = f_pt->first_pr_pt = last_fi_pt++;
               fi_pt->name = buf_pt;
               break;
            default:                         /* IF count>=2 c IS SEGMENT CHAR.:  */
               *buf_pt++ = c;
               break;
            }
            count++;
            break;
         }
      }
   }


   if (mode!=MENU)     /* Read special command-line generation information */
   {
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                       /* End of input line: next field    */
            *buf_pt++ = null;             /* Terminate current segment        */
            count = 0;                    /* Digit counter to 0               */
            break;
         case CTRL(Z):
            *buf_pt++ = null;
            TOOMANY(6,last_fi_pt+1,fixes,NFIX);
            fi_pt = f_pt->first_cpo_pt = last_fi_pt++;
            fi_pt->name = buf_pt;
            break;
         case CTRL(X):                    /* Next segment on same input line  */
            *buf_pt++ = null;
            TOOMANY(6,last_fi_pt+1,fixes,NFIX);
            fi_pt = fi_pt->next = last_fi_pt++;
            fi_pt->name = buf_pt;
            break;
         default:
            switch(count)                 /* If count<=2 c is digit:          */
            {
            case 0: 
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               TOOMANY(6,last_fi_pt+1,fixes,NFIX);
               fi_pt = f_pt->first_cpr_pt = last_fi_pt++;
               fi_pt->name = buf_pt;
               break;
            default:                      /* IF count>=2 c IS name CHAR.:  */
               *buf_pt++ = c;
               break;
            }
            count++;
            break;
         }
      }
   }

   if (mode!=MENU)     /* At-least-one fields   */
   {
      flip_flop = 0;
      count = 0;
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R))
      {
         switch(c)
         {
         case '\n':                          /* End of input line: next field    */
            count=0;
            break;
         default:
            switch(count)
            {
            case 0:
               n = c-'0';
               break;
            case 1:
               f_pt = fields + c-'0' + 10*n;
               break;
            default:
               switch(flip_flop)
               {
               case 0:                    /* First digit of 2-digit funx indx */
                  TOOMANY(7,last_in_pt+1,indices,MAXINDICES);
                  if (count==2)
                     in_pt = f_pt->first_a_pt = last_in_pt++;
                  else
                     in_pt = in_pt->next = last_in_pt++;
                  in_pt->num = 10*(c-'0');
                  break;
               case 1:                    /* Second digit of indx             */
                  in_pt->num += c-'0';
                  break;
               }
               flip_flop = 1 - flip_flop;
               break;
            }
            count++;
            break;
         }
      }
   }


   if (mode!=MENU)     /* Third placeholder control-R   */
   {
      if ((c = *lbuf_pt++)!='\n')
      {
         status = 3;
         done();
      }
      while ((c = *lbuf_pt++)!=CTRL(R));
   }


   if (lbuffer!=NULL) {
      free(lbuffer);
      lbuffer = NULL;
   }

   TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
   TOOMANY(5,last_segm_pt+1,segments,MAXSEGMS);
   TOOMANY(6,last_fi_pt+1,fixes,NFIX);
   TOOMANY(7,last_in_pt+1,indices,MAXINDICES);
   TOOMANY(8,last_field_pt+1,fields,MAXFIELDS);
}
