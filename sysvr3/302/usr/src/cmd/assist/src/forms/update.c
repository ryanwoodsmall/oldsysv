/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:update.c	1.7"
#include "muse.h"
#include "mmuse.h"


/* THIS ROUTINE DOES USER INPUT STRING WRAP                                      */
VOID update(last_row)
register int last_row;
{  
   register int first_col, i, k, row, lower;
   struct segment *s_pt;
   struct field *f_pt;

   first_col = Field_pt->col+2;

   i = 0;
   for (s_pt=Field_pt->first_s_pt ; s_pt!=NULL ; s_pt=s_pt->next)
   {
      if (s_pt->word==NULL)              /* Remove from chain   */
      {
         if (s_pt->previous!=NULL)    /* s_pt not first segment       */
         {
            (s_pt->previous)->next = s_pt->next;
            if (s_pt->next != NULL)   /* s_pt not last segment        */
               (s_pt->next)->previous = s_pt->previous;
         }
         else                              /* s_pt must be first segment   */
         {
            if (s_pt->next !=NULL)    /* s_pt not last segment        */
            {
               (s_pt->next)->previous = NULL;
               Field_pt->first_s_pt = s_pt->next;
            }
            else                      /* s_pt must be ONLY segment   */
            {
               s_pt->word = buf_pt;
               TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
               *buf_pt++ = null;
            }
         }
         if (Segm_pt==s_pt)             /* Reassign Segm_pt.             */
         {
            if (s_pt->next!=NULL) Segm_pt=s_pt->next;
            else if (s_pt->previous!=NULL) Segm_pt=s_pt->previous;
            else   /* Special stuff if current segment is only segment */
            {
               Segm_pt=Field_pt->first_s_pt;
               Segm_pt->word = buf_pt;
               TOOMANY(4,buf_pt+1,buffer,BUFSIZE);
               *buf_pt++ = null;
               Segm_pt->row = Field_pt->row;
               Segm_pt->col = Field_pt->col + strlen(Field_pt->caption) + 1;
            }
         }
      }
      else                   /* Keep in chain       */
      {
         row = s_pt->row-Stdscr_loc;
         if (row>2 && row<=SCRLINES && s_pt->word!=NULL &&
             strlen(s_pt->word)>0)
         {
            mvaddch(row,s_pt->col,SPACE);
            k=length(s_pt->word);
            while (--k>0)  addch(SPACE);
         }
         if (s_pt->previous==NULL)     /* First non-zero segment  */
         {
            s_pt->row = Field_pt->row;
            s_pt->col = Field_pt->col + strlen(Field_pt->caption) + 1;
         }
         else          /* Second or subsequent non-zero segment.   */
         {
            s_pt->row = (s_pt->previous)->row;
            s_pt->col = (s_pt->previous)->col +
                        length((s_pt->previous)->word) + 1;
         }
         if (s_pt->col + length(s_pt->word) >= COLS-2)  
                                 /* Put on next line if it doesn't fit.          */
         {
            s_pt->col = first_col;
            s_pt->row++;
         }
      }
   }

   for (s_pt = Field_pt->first_s_pt;s_pt;s_pt=s_pt->next)
   {
      Field_pt->last_row = s_pt->row;
      row = s_pt->row-Stdscr_loc;
      if (row>2 && row<=SCRLINES)    show(s_pt);
   }

   if ((lower = Field_pt->last_row - last_row) != 0)
   {
      for (f_pt=Field_pt+1;f_pt<=last_field_pt;f_pt++)
      {
         for (i=f_pt->row;i<=f_pt->last_row;i++)
         {
            move(i,0); clrtoeol();
         }
         f_pt->row+=lower;
         f_pt->last_row+=lower;
         for (s_pt=f_pt->first_s_pt;s_pt!=NULL;s_pt=s_pt->next)
            s_pt->row+=lower;
      }
      copy_form_std(2);
   }

   Row = Segm_pt->row;
   Col = (mode==INSERT) ? Segm_pt->col+length(Segm_pt->word) : Segm_pt->col;
}

