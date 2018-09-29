/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/flt.c	1.4"
# include	<varargs.h>
# include	"lpsched.h"

static void	free_flt();

void
add_flt_act(va_alist)
va_dcl
{
    va_list	arg;
    FLT		*f;
    NODE	*n;
    
    va_start(arg);
    
    n = (NODE *)va_arg(arg, NODE *);

    if ((f = (FLT *)malloc(sizeof(FLT))) == NULL)
	return;
    (void) memset((char *)f, 0, sizeof(FLT));
    
    f->type = (int)va_arg(arg, int);
    
    switch(f->type)
    {
	case FLT_FILES:
	f->s1 = strdup((char *)va_arg(arg, char *));
	f->i1 = (int)va_arg(arg, int);
	break;
	
	case FLT_CHANGE:
	f->r1 = (RSTATUS *)va_arg(arg, RSTATUS *);
	break;
    }

    if (n->flt)
	f->next = n->flt;

    n->flt = f;
    va_end(arg);
}


void
del_flt_act(va_alist)
va_dcl
{
    va_list	arg;
    NODE	*n;
    int		type;
    FLT		*fp;
    FLT		*f;
    
    va_start(arg);

    n = (NODE *)va_arg(arg, NODE *);

    type = (int)va_arg(arg, int);
    
    if (n->flt)
    {
	if (n->flt->type == type)
	{
	    fp = n->flt;
	    n->flt = n->flt->next;
	    free_flt(fp);
	}
	else
	{
	    for (f = n->flt; f->next && f->next->type != type; f = f->next);
	    if (f->next)
	    {
		fp = f->next;
		f->next = f->next->next;
		free_flt(fp);
	    }
	}
    }
    va_end(arg);
}

void
do_flt_acts(n)
NODE	*n;
{
    FLT		*fp;
    FLT		*f = n->flt;
    char	*file;
    char	id[14];
    
    while (f)
    {
	switch (f->type)
	{
	    case FLT_FILES:
	    /* remove files created with alloc_files */
	    while(f->i1--)
	    {
		(void) sprintf(id, "%s-%d", f->s1, f->i1);
		file = makepath(Lp_Temp, id, (char *)0);
		(void) Unlink(file);
		free(file);
	    }
	    break;
	    

	    case FLT_CHANGE:
	    /* clear RS_CHANGE bit, write request file, and schedule */
	    f->r1->request->outcome &= ~RS_CHANGING;
	    putrequest(f->r1->req_file, f->r1->request);
	    if (NEEDS_FILTERING(f->r1))
		schedule(EV_SLOWF, f->r1);
	    else
		schedule(EV_INTERF, f->r1->printer);
	    break;
	}
	fp = f;
	f = f->next;
	free_flt(fp);
    }
    n->flt = (FLT *)0;
}

static void
free_flt(f)
FLT	*f;
{
    if (f->s1)
	free(f->s1);
    free((char *)f);
}
