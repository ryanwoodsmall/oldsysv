/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/init.c	1.10"

#include "lpsched.h"

ALERT		*Alert_Table;		/* Printer fault alerts    */
ALERT		*FAlert_Table;		/* Form mount alerts       */
ALERT		*PAlert_Table;		/* PrintWheel mount alerts */
CLASS		*Class_Table;		/* Known classes           */
CSTATUS		*CStatus;		/* Status of same          */
EXEC		*Exec_Table;		/* Running processes       */
EXEC		*Exec_Slow;		/*   Slow filters	   */
EXEC		*Exec_Notify;		/*   Notifications	   */
_FORM		*Form_Table;		/* Known forms             */
FSTATUS		*FStatus;		/* status of same	   */
PRINTER		*Printer_Table;		/* Known printers          */
PSTATUS		*PStatus;		/* Status of same          */
PWHEEL		*PrintWheel_Table;	/* Known print wheels      */
PWSTATUS	*PWStatus;		/* Status of same          */ 
RSTATUS		*Request_List;		/* Queue of print requests */

int		CT_Size,
		ET_Size,
		ET_SlowSize	= 1,
		ET_NotifySize	= 1,
		FT_Size,
		PT_Size,
		PWT_Size;

/*
**
**	CLRMEM clears memory pointed to by <addr> over the range of
**	<addr>[<cnt> .. <size>].  <datum> is the size of an element of <addr>.
**
*/
# define	CLRMEM(addr, cnt, size, datum) \
                      (void) memset((char *)(addr + cnt), 0, \
		                    (int)((size - cnt ) * sizeof(datum)))

static void	init_printers(),
		init_classes(),
		init_forms(),
		init_pwheels(),
		init_exec();


static RSTATUS	*init_requests();

void
init_memory()
{
    init_printers();
    init_classes();
    init_forms();
    init_pwheels();
    init_exec();

    /*
     * Load the status after the basic structures have been loaded,
     * but before loading requests still in the queue. This is so
     * the requests can be compared against accurate status information
     * (except rejection status--accept the jobs anyway).
     */
    load_status();

    Request_List = init_requests();
    Loadfilters(Lp_A_Filters);
}

static void
init_printers()
{
    PRINTER	*p;
    PRINTER	*pt_pointer;
    int		pt_allocation;
    int		at_allocation;
    int		PT_Count;
    int		i;

#if defined(DEBUG)
    if (debug)
    {
	printf ("Loading printer configurations: ");
	fflush (stdout);
    }
#endif

    PT_Size = 10;
    PT_Count = 0;
    pt_allocation = PT_Size * sizeof(PRINTER);

    Printer_Table = (PRINTER *)malloc(pt_allocation);

    if (Printer_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);
    
    pt_pointer = Printer_Table;
    
    while((p = Getprinter(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*pt_pointer = *p;
	pt_pointer++;

	if (++PT_Count < PT_Size)
	    continue;
	
	PT_Size += 10;
	pt_allocation = PT_Size * sizeof(PRINTER);

	Printer_Table = (PRINTER *)realloc(Printer_Table, pt_allocation);

	if (Printer_Table == NULL)
	    fail(MEMORY_ALLOC_FAILED);

	CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);

	pt_pointer = Printer_Table + PT_Count;
    }

    PT_Size = PT_Count + 40;

    pt_allocation = PT_Size * sizeof(PRINTER);

    Printer_Table = (PRINTER *)realloc(Printer_Table, pt_allocation);

    if (Printer_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Printer_Table, PT_Count, PT_Size, PRINTER);

    at_allocation = PT_Size * sizeof(ALERT);

    Alert_Table = (ALERT *)malloc(at_allocation);

    if (Alert_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Alert_Table, 0, PT_Size, ALERT);

    pt_allocation = PT_Size * sizeof(PSTATUS);
    
    PStatus = (PSTATUS *)malloc(pt_allocation);
    
    if (PStatus == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(PStatus, 0, PT_Size, PSTATUS);

    for (i = 0; i < PT_Size; i++)
    {
	char	buf[15];
	
	PStatus[i].printer = Printer_Table + i;
	PStatus[i].alert = Alert_Table + i;
	sprintf(buf, "A-%d", i);
	Alert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(Alert_Table[i].msgfile);
	if (i < PT_Count)
	{
	    void	load_sdn();
	    
	    load_userprinter_access (
		    Printer_Table[i].name,
		    &(PStatus[i].users_allowed),
		    &(PStatus[i].users_denied)
	    );
	    load_formprinter_access (
		    Printer_Table[i].name,
		    &(PStatus[i].forms_allowed),
		    &(PStatus[i].forms_denied)
	    );
	    load_sdn (&(PStatus[i].cpi), Printer_Table[i].cpi);
	    load_sdn (&(PStatus[i].lpi), Printer_Table[i].lpi);
	    load_sdn (&(PStatus[i].plen), Printer_Table[i].plen);
	    load_sdn (&(PStatus[i].pwid), Printer_Table[i].pwid);
	}
    }
    
#if defined(DEBUG)
    if (debug)
    {
	if (PT_Count > 1)
	    printf ("%d printers\n", PT_Count);
	else
	    if (PT_Count == 1)
		printf ("1 printer\n");
	    else
		printf ("No printers\n");
	fflush (stdout);
    }
#endif
}

static void
init_classes()
{
    CLASS	*p;
    CLASS	*ct_pointer;
    int		ct_allocation;
    int		CT_Count;
    int		i;

#if defined(DEBUG)
    if (debug)
    {
	printf ("Loading class configurations: ");
	fflush (stdout);
    }
#endif

    CT_Size = 10;
    CT_Count = 0;
    ct_allocation = CT_Size * sizeof(CLASS);

    Class_Table = (CLASS *)malloc(ct_allocation);

    if (Class_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);
    
    ct_pointer = Class_Table;

    while((p = Getclass(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*ct_pointer = *p;
	ct_pointer++;

	if (++CT_Count < CT_Size)
	    continue;
	
	CT_Size += 10;
	ct_allocation = CT_Size * sizeof(CLASS);

	Class_Table = (CLASS *)realloc(Class_Table, ct_allocation);

	if (Class_Table == NULL)
	    fail(MEMORY_ALLOC_FAILED);

	CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);

	ct_pointer = Class_Table + CT_Count;

    }

    CT_Size = CT_Count + 40;

    ct_allocation = CT_Size * sizeof(CLASS);

    Class_Table = (CLASS *)realloc(Class_Table, ct_allocation);

    if (Class_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Class_Table, CT_Count, CT_Size, CLASS);

    ct_allocation = CT_Size * sizeof(CSTATUS);
    
    CStatus = (CSTATUS *)malloc(ct_allocation);
    
    if (CStatus == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(CStatus, 0, CT_Size, CSTATUS);

    for (i = 0; i < CT_Size; i++)
	CStatus[i].class = Class_Table + i;

#if defined(DEBUG)
    if (debug)
    {
	if (CT_Count > 1)
	    printf ("%d classes\n", CT_Count);
	else
	    if (CT_Count == 1)
		printf ("1 class\n");
	    else
		printf ("No classes\n");
	fflush (stdout);
    }
#endif
}

static void
init_forms()
{
    _FORM	*ft_pointer,
		*f;
    int		at_allocation;
    int		ft_allocation;
    int		FT_Count;
    int		i;

#if defined(DEBUG)
    if (debug)
    {
	printf ("Loading form definitions: ");
	fflush (stdout);
    }
#endif

    FT_Size = 10;
    FT_Count = 0;
    ft_allocation = FT_Size * sizeof(_FORM);

    Form_Table = (_FORM *)malloc(ft_allocation);

    if (Form_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);
    
    ft_pointer = Form_Table;

    while((f = Getform(NAME_ALL)) != NULL)
    {
	*(ft_pointer++) = *f;

	if (++FT_Count < FT_Size)
	    continue;
	
	FT_Size += 10;
	ft_allocation = FT_Size * sizeof(_FORM);

	Form_Table = (_FORM *)realloc(Form_Table, ft_allocation);

	if (Form_Table == NULL)
	    fail(MEMORY_ALLOC_FAILED);

	CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);

	ft_pointer = Form_Table + FT_Count;
    }

    FT_Size = FT_Count + 40;

    ft_allocation = FT_Size * sizeof(_FORM);

    Form_Table = (_FORM *)realloc(Form_Table, ft_allocation);

    if (Form_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(Form_Table, FT_Count, FT_Size, _FORM);

    at_allocation = FT_Size * sizeof(ALERT);

    FAlert_Table = (ALERT *)malloc(at_allocation);

    if (FAlert_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(FAlert_Table, 0, FT_Size, ALERT);

    ft_allocation = FT_Size * sizeof(FSTATUS);
    
    FStatus = (FSTATUS *)malloc(ft_allocation);
    
    if (FStatus == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(FStatus, 0, FT_Size, FSTATUS);

    for (i = 0; i < FT_Size; i++)
    {
	char	buf[15];
	
	FStatus[i].form = Form_Table + i;
	FStatus[i].alert = FAlert_Table + i;
	FStatus[i].trigger = Form_Table[i].alert.Q;
	sprintf(buf, "F-%d", i);
	FAlert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(FAlert_Table[i].msgfile);

	if (i < FT_Count)
	{
	    void	load_sdn();
	    
	    load_userform_access (
		    Form_Table[i].name,
		    &(FStatus[i].users_allowed),
		    &(FStatus[i].users_denied)
	    );
	    load_sdn (&(FStatus[i].cpi), Form_Table[i].cpi);
	    load_sdn (&(FStatus[i].lpi), Form_Table[i].lpi);
	    load_sdn (&(FStatus[i].plen), Form_Table[i].plen);
	    load_sdn (&(FStatus[i].pwid), Form_Table[i].pwid);
	}
    }
    
#if defined(DEBUG)
    if (debug)
    {
	if (FT_Count > 1)
	    printf ("%d forms\n", FT_Count);
	else
	    if (FT_Count == 1)
		printf("1 form\n");
	    else
		printf("No forms\n");
	fflush (stdout);
    }
#endif
}

static void
init_pwheels()
{
    PWHEEL	*pwt_pointer;
    PWHEEL	*p;
    int		at_allocation;
    int		pwt_allocation;
    int		PWT_Count;
    int		i;
    
#if defined(DEBUG)
    if (debug)
    {
	printf ("Loading print wheel list: ");
	fflush (stdout);
    }
#endif

    PWT_Count = 0;
    PWT_Size = 10;
    pwt_allocation = PWT_Size * sizeof(PWHEEL);

    PrintWheel_Table = (PWHEEL *)malloc(pwt_allocation);

    if (PrintWheel_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);
    
    pwt_pointer = PrintWheel_Table;

    while((p = Getpwheel(NAME_ALL)) != NULL || errno != ENOENT)
    {
	if (!p)
	    continue;

	*pwt_pointer = *p;
	pwt_pointer++;

	if (++PWT_Count < PWT_Size)
	    continue;
	
	PWT_Size += 10;
	pwt_allocation = PWT_Size * sizeof(PWHEEL);

	PrintWheel_Table = (PWHEEL *)realloc(PrintWheel_Table, pwt_allocation);

	if (PrintWheel_Table == NULL)
	    fail(MEMORY_ALLOC_FAILED);

	CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);

	pwt_pointer = &PrintWheel_Table[PWT_Count];

    }

    PWT_Size = PWT_Count + 40;

    pwt_allocation = PWT_Size * sizeof(PWHEEL);

    PrintWheel_Table = (PWHEEL *)realloc(PrintWheel_Table, pwt_allocation);

    if (PrintWheel_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(PrintWheel_Table, PWT_Count, PWT_Size, PWHEEL);

    at_allocation = PWT_Size * sizeof(ALERT);

    PAlert_Table = (ALERT *)malloc(at_allocation);

    if (PAlert_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(PAlert_Table, 0, PWT_Size, ALERT);

    pwt_allocation = PWT_Size * sizeof(PWSTATUS);
    
    PWStatus = (PWSTATUS *)malloc(pwt_allocation);
    
    if (PWStatus == NULL)
	fail(MEMORY_ALLOC_FAILED);

    CLRMEM(PWStatus, 0, PWT_Size, PWSTATUS);

    for (i = 0; i < PWT_Size; i++)
    {
	char	buf[15];
	
	PWStatus[i].pwheel = PrintWheel_Table + i;
	PWStatus[i].trigger = PrintWheel_Table[i].alert.Q;
	PWStatus[i].alert = PAlert_Table + i;
	sprintf(buf, "P-%d", i);
	PAlert_Table[i].msgfile = makepath(Lp_Temp, buf, (char *)0);
	(void) Unlink(PAlert_Table[i].msgfile);
    }
    
#if defined(DEBUG)
    if (debug)
    {
	if (PWT_Count > 1)
	    printf ("%d print wheels\n", PWT_Count);
	else
	    if (PWT_Count == 1)
		printf ("1 print wheel\n");
	    else
		printf ("No print wheels\n");
	fflush (stdout);
    }
#endif
}

static RSTATUS	*
init_requests()
{
    RSTATUS	*rp = NULL;
    int		i;
    REQUEST	*r;
    SECURE	*s;
    char	*name;
    char	*next_file();
    char	*makestr();
    long	addr = -1;
    RSTATUS	**table;
    int		count;
    int		size;

    size = 20;
    count = 0;
    
#if defined(DEBUG)
    if (debug)
    {
	printf ("Loading requests that haven't printed: ");
	fflush (stdout);
    }
#endif

    table = (RSTATUS **) malloc(size * sizeof(RSTATUS *));
    
    if (table == NULL)
	fail(MEMORY_ALLOC_FAILED);
    
    while((name = next_file(Lp_Requests, &addr)) != NULL)
    {
	table[count] = allocr();

	if ((s = Getsecure(name)) == NULL)
	{
	    rmfiles(table[count], 0);
	    freerstatus(table[count]);
	    continue;
	}
	*(table[count]->secure) = *s;

	if((r = Getrequest(name)) == NULL)
	{
	    rmfiles(table[count], 0);
	    freerstatus(table[count]);
	    continue;
	}
	r->outcome &= ~RS_ACTIVE;	/* it can't be! */
	*(table[count]->request) = *r;

	table[count]->req_file = strdup(name);

	if ((r->outcome & RS_CANCELLED) && !(r->outcome & RS_NOTIFY))
	{
	    rmfiles(table[count], 0);
	    freerstatus(table[count]);
	    continue;
	}

	/*
	 * So far, the only way RS_NOTIFY can be set without there
	 * being a notification file containing the message to the
	 * user, is if the request was cancelled. This is because
	 * cancelling a request does not cause the creation of the
	 * message if the request is currently printing or filtering.
	 * (The message is created after the child process dies.)
	 * Thus, we know what to say.
	 *
	 * If this behaviour changes, we may have to find another way
	 * of determining what to say in the message.
	 */
	if (r->outcome & RS_NOTIFY)
	{
	    char	*file = makereqerr(s->req_id);

	    if (Access(file, F_OK) == -1)
	    {
		if (!(r->outcome & RS_CANCELLED))
		{
		    free(file);
		    rmfiles(table[count], 0);
		    freerstatus(table[count]);
		    continue;
		}
		notify(table[count], (char *)0, 0, 0, 0);
	    }
	    free(file);
	}

	if (validate_request(table[count], (char *)0, 1) != MOK)
	    cancel(table[count], 1);
	

	if (++count < size)
	    continue;
	
	size += 20;
	
	table = (RSTATUS **) realloc((char *)table, size * sizeof(RSTATUS *));
	
	if (table == NULL)
	    fail(MEMORY_ALLOC_FAILED);
    }
    
    if (!count)
	free ((char *)table);
    else if (size = count)
    {
	table = (RSTATUS **) realloc((char *)table, size * sizeof(RSTATUS *));
	
	if (table == NULL)
	    fail(MEMORY_ALLOC_FAILED);
    
	qsort((char *)table, size, sizeof(RSTATUS *), rsort);

    
	for (i = 0; i < size - 1; i++)
	{
	    table[i]->next = table[i + 1];
	    table[i + 1]->prev = table[i];
	}

	table[0]->prev = 0;
	table[size - 1]->next = 0;

	rp = *table;
	free(table);

    }

#if defined(DEBUG)
    if (debug)
    {
	if (count > 1)
	    printf ("%d print requests\n", count);
	else
	    if (count == 1)
		printf ("1 print request\n");
	    else
		printf ("No print requests\n");
	fflush (stdout);
    }
#endif

    return(rp);
}

static void
init_exec()
{
    EXEC	*et_pointer;
    int		et_allocation;
    int		i;

    ET_Size	= ET_SlowSize
		+ ET_NotifySize
    		+ PT_Size * 2	/* 1 for interface, 1 for alert */
		+ PWT_Size
		+ FT_Size;

    et_allocation = ET_Size * sizeof(EXEC);

    Exec_Table = (EXEC *) malloc(et_allocation);

    if (Exec_Table == NULL)
	fail(MEMORY_ALLOC_FAILED);
    
    CLRMEM(Exec_Table, 0, ET_Size, EXEC);

    et_pointer = Exec_Table;

    Exec_Slow = et_pointer;
    for (i = 0; i < ET_SlowSize; i++)
	(et_pointer++)->type = EX_SLOWF;

    Exec_Notify = et_pointer;
    for (i = 0; i < ET_NotifySize; i++)
	(et_pointer++)->type = EX_NOTIFY;

    for (i = 0; i < PT_Size; i++)
    {
	PStatus[i].exec = et_pointer;
	et_pointer->type = EX_INTERF;
	et_pointer->ex.printer = PStatus + i;
	et_pointer++;

	PStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_ALERT;
	et_pointer->ex.printer = PStatus + i;
	et_pointer++;
    }

    for (i = 0; i < PWT_Size; i++)
    {
	PWStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_PALERT;
	et_pointer->ex.pwheel = PWStatus + i;
	et_pointer++;
    }

    for (i = 0; i < FT_Size; i++)
    {
	FStatus[i].alert->exec = et_pointer;
	et_pointer->type = EX_FALERT;
	et_pointer->ex.form = FStatus + i;
	et_pointer++;
    }
    
}
