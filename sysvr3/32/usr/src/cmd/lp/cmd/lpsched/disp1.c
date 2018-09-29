/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/disp1.c	1.11"

#include "dispatch.h"

void			endpwent();

RSTATUS			*NewRequest;

/**
 ** alloc_files() - S_ALLOC_FILES
 **/

void
alloc_files (m,n)
char			*m;
NODE			*n;
{
    char			*file_prefix;
    ushort			count;

    getmessage (m, S_ALLOC_FILES, &count);

    if ((file_prefix = _alloc_files(count, (char *)0, n->uid, n->gid)))
    {
	send (n, R_ALLOC_FILES, MOK, file_prefix);
	add_flt_act(n, FLT_FILES, file_prefix, count);
    }
    else
	send (n, R_ALLOC_FILES, MNOMEM, "");

    return;
}

/**
 ** print_request() - S_PRINT_REQUEST
 **/

void
print_request (m,n)
	char			*m;
	NODE			*n;
{
    extern char			*Local_System;
    extern struct passwd	*getpwuid();
    extern long			time();
    char			*file;
    char			*p;
    char			*req_id	= 0;
    RSTATUS			*rp;
    REQUEST			*r;
    struct passwd		*pw;
    short			err;
    short			status;
    off_t			size;


    (void) getmessage (m, S_PRINT_REQUEST, &file);

    /*
     * "NewRequest" points to a request that's not yet in the
     * request list but is to be considered with the rest of the
     * requests (e.g. calculating # of requests awaiting a form).
     */
    if (!(rp = NewRequest = allocr()))
	status = MNOMEM;

    else
    {
	p = makepath(Lp_Temp, file, (char *)0);
	(void) Chmod(p, 0600);
	(void) Chown(p, Lp_Uid, Lp_Gid);
	free(p);
    
	if (!(r = Getrequest(file)))
	    status = MNOOPEN;
    
	else
	{
	    *(rp->request) = *r;
	    rp->request->outcome = 0;
	    rp->secure->uid = n->uid;
	    rp->secure->gid = n->gid;
	    rp->req_file = strdup(file);
    
	    if (!STREQU(n->system, Local_System))
		rp->secure->system = strdup(n->system);

	    pw = getpwuid(n->uid);
	    endpwent();
	    if (pw && pw->pw_name && *pw->pw_name)
		rp->secure->user = strdup(pw->pw_name);
	    else
	    {
#define BIGGEST "65536"		/* the biggest 16-bit number */
		rp->secure->user = strdup(BIGGEST);
		sprintf (rp->secure->user, "%d", (n->uid & 0xFFFF));
	    }

	    if ((rp->request->actions & ACT_SPECIAL) == ACT_HOLD)
		rp->request->outcome |= RS_HELD;
	    if ((rp->request->actions & ACT_SPECIAL) == ACT_RESUME)
		rp->request->outcome &= ~RS_HELD;
	    if((rp->request->actions & ACT_SPECIAL) == ACT_IMMEDIATE)
	    {
		if (!isadmin(n->uid))
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->request->outcome |= RS_IMMEDIATE;
	    }
    
	    size = chfiles(rp->request->file_list, Lp_Uid, Lp_Gid);
	    if (size < 0)
	    {
		status = MUNKNOWN;
		goto Return;
	    }
	    if (!(rp->request->outcome & RS_HELD) && size == 0)
	    {
		status = MNOPERM;
		goto Return;
	    }
	    rp->secure->size = size;
    
	    (void) time(&rp->secure->date);
    
	    if((err = validate_request(rp, &req_id, 0)) != MOK)
		status = err;

	    else
	    {
		if (p = strchr(file, '-'))
		    *p = '\0';
		req_id = makestr(req_id, "-", file, (char *)0);
		rp->secure->req_id = req_id;
		if (p)
		    *p = '-';
    
		if (
		    putsecure(file, rp->secure) == -1
		 || putrequest(file, rp->request) == -1
		)
		    status = MNOMEM;

		else
		{
		    status = MOK;

		    insertr(rp);
		    NewRequest = 0;
    
		    if (rp->slow)
			schedule (EV_SLOWF, rp);
		    else
			schedule (EV_INTERF, rp->printer);

		    del_flt_act(n, FLT_FILES);
		}
	    }
	}
    }

Return:
    NewRequest = 0;
    if (status != MOK && rp)
    {
	rmfiles(rp, 0);
	freerstatus(rp);
    }
    send(n, R_PRINT_REQUEST, status, NB(req_id), chkprinter_result);
    return;
}

/**
 ** start_change_request()
 **/

void
start_change_request (m,n)
	char			*m;
	NODE			*n;
{
    char		*req_id;
    char		*req_file;
    char		*path;
    short		status;
    RSTATUS		*rp;

    (void) getmessage(m, S_START_CHANGE_REQUEST, &req_id);

    if (!(rp = request_by_id(req_id)))
	status = MUNKNOWN;

    else if (rp->request->outcome & RS_DONE)
	status = M2LATE;

    else if (n->uid && n->uid != Lp_Uid && n->uid != rp->secure->uid)
	status = MNOPERM;

    else if (rp->request->outcome & RS_CHANGING)
	status = MNOOPEN;
    
    else if (rp->request->outcome & RS_NOTIFYING)
	status = MBUSY;

    else
    {
	status = MOK;

	if (
	    rp->request->outcome & RS_FILTERING
	 && !(rp->request->outcome & RS_STOPPED)
	)
	{
	    rp->request->outcome |= (RS_REFILTER|RS_STOPPED);
	    terminate (rp->exec);
	}

	if (
	    rp->request->outcome & RS_PRINTING
	 && !(rp->request->outcome & RS_STOPPED)
	)
	{
	    rp->request->outcome |= RS_STOPPED;
	    terminate (rp->printer->exec);
	}

	rp->request->outcome |= RS_CHANGING;	
    
	/*
	 * Change the ownership of the request file to be "n->uid".
	 * Either this is identical to "rp->secure->uid", or it is
	 * "Lp_Uid" (see above) or it is root. The idea is that the
	 * person at the other end needs access, and that may not
	 * be who queued the request.
	 */
	path = makepath(Lp_Temp, rp->req_file, (char *)0);
	(void) Chown(path, n->uid, rp->secure->gid);
	free(path);

	add_flt_act(n, FLT_CHANGE, rp);
	req_file = rp->req_file;

    }
    send(n, R_START_CHANGE_REQUEST, status, req_file);
    return;
}

/**
 ** end_change_request()
 **/

void
end_change_request (m,n)
	char			*m;
	NODE			*n;
{
    char		*req_id;
    RSTATUS		*rp;
    off_t		size;
    off_t		osize;
    short		err;
    short		status;
    char		*file;
    REQUEST		*r		= 0;
    REQUEST		oldr;
    int			call_schedule	= 0;
    int			move_ok		= 0;


    (void) getmessage(m, S_END_CHANGE_REQUEST, &req_id);
    if (!(rp = request_by_id(req_id)))
	status = MUNKNOWN;

    else if (!(rp->request->outcome & RS_CHANGING))
	status = MNOSTART;

    else
    {
	file = makepath(Lp_Temp, rp->req_file, (char *)0);
	(void) Chmod(file, 0600);
	(void) Chown(file, Lp_Uid, Lp_Gid);
	free(file);

	rp->request->outcome &= ~(RS_CHANGING);
	del_flt_act(n, FLT_CHANGE);
	/*
	 * The RS_CHANGING bit may have been the only thing preventing
	 * this request from filtering or printing, so regardless of what
	 * happens below, we must check to see if the request can proceed.
	 */
	call_schedule = 1;

	if (!(r = Getrequest(rp->req_file)))
	    status = MNOOPEN;

	else
	{
	    oldr = *(rp->request);
	    *(rp->request) = *r;

	    move_ok = STREQU(oldr.destination, r->destination);

	    /*
	     * Preserve the current request status!
	     */
	    rp->request->outcome = oldr.outcome;

	    if ((rp->request->actions & ACT_SPECIAL) == ACT_HOLD)
	    {
		rp->request->outcome |= RS_HELD;
		/*
		 * To be here means either the user owns the request
		 * or he or she is the administrator. Since we don't
		 * want to set the RS_ADMINHELD flag if the user is
		 * the administrator, the following compare will work.
		 */
		if (n->uid != rp->secure->uid)
		    rp->request->outcome |= RS_ADMINHELD;
	    }

	    if ((rp->request->actions & ACT_SPECIAL) == ACT_RESUME)
	    {
		if (
		    (rp->request->outcome & RS_ADMINHELD)
		 && !isadmin(n->uid)
		)
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->request->outcome &= ~(RS_ADMINHELD|RS_HELD);
	    }

	    if((rp->request->actions & ACT_SPECIAL) == ACT_IMMEDIATE)
	    {
		if (!isadmin(n->uid))
		{
		    status = MNOPERM;
		    goto Return;
		}
		rp->request->outcome |= RS_IMMEDIATE;
	    }

	    size = chfiles(rp->request->file_list, Lp_Uid, Lp_Gid);
	    if (size < 0)
	    {
		status = MUNKNOWN;
		goto Return;
	    }
	    if (!(rp->request->outcome & RS_HELD) && size == 0)
	    {
		status = MNOPERM;
		goto Return;
	    }

	    osize = rp->secure->size;
	    rp->secure->size = size;

	    if ((err = validate_request(rp, (char **)0, move_ok)) != MOK)
	    {
		status = err;
		rp->secure->size = osize;

	    }
	    else
	    {
		status = MOK;

		if(rp->request->outcome & RS_IMMEDIATE)
		{
		    remover(rp);
		    insertr(rp);
		}

		(void) time(&rp->secure->date);

		freerequest(&oldr);
		(void) putrequest(rp->req_file, rp->request);
		(void) putsecure(rp->req_file, rp->secure);

	    }
	}
    }

Return:
    if (status != MOK && rp)
    {
	if (r)
	{
	    freerequest(r);
	    *(rp->request) = oldr;
	}
	if (status != MNOSTART)
	    (void) putrequest(rp->req_file, rp->request);
    }
    if (call_schedule)
	maybe_schedule(rp);
    send(n, R_END_CHANGE_REQUEST, status, chkprinter_result);
    return;
}

/**
 ** cancel_request()
 **/

void
cancel_request (m,n)
	char			*m;
	NODE			*n;
{
    char		*req_id;
    RSTATUS		*rp;

    (void) getmessage(m, S_CANCEL_REQUEST, &req_id);

    if (rp = request_by_id(req_id))
    {
	if (n->uid && n->uid != Lp_Uid && n->uid != rp->secure->uid)
	{
	    send(n, R_CANCEL_REQUEST, MNOPERM);
	    return;
	}

	rp->reason = MOK;

	if (cancel(rp, (n->uid != rp->secure->uid)))
	    send(n, R_CANCEL_REQUEST, MOK);
	else
	    send(n, R_CANCEL_REQUEST, M2LATE);
	return;
    }
    send(n, R_CANCEL_REQUEST, MUNKNOWN);
}

/**
 ** inquire_request() - S_INQUIRE_REQUEST
 **/

void
inquire_request (m,n)
	char			*m;
	NODE			*n;
{
    char	*form;
    char	*dest;
    char	*pwheel;
    char	*user;
    char	*req_id;
    RSTATUS	*rp;
    RSTATUS	*found;

    found = (RSTATUS *)0;

    (void) getmessage(m, S_INQUIRE_REQUEST,&form,&dest,&req_id,&user,&pwheel);

    for(rp = Request_List; rp != NULL; rp = rp->next)
    {
	if (*form && !SAME(form, rp->request->form))
	    continue;
	if (*dest && !STREQU(dest, rp->request->destination))
	    continue;
	if (*req_id && !STREQU(req_id, rp->secure->req_id))
	    continue;
	if (*user && !STREQU(user, rp->request->user))
	    continue;
	if (*pwheel && !SAME(pwheel, rp->pwheel_name))
	    continue;
	
	if (found)
	    send(n, R_INQUIRE_REQUEST,
		 MOKMORE,
		 found->secure->req_id,
		 found->request->user,
		 found->secure->size,
		 found->secure->date,
		 found->request->outcome,
		 found->printer->printer->name,
		 (found->form? found->form->form->name : ""),
		 NB(found->pwheel_name)
	    );
	found = rp;
    }

    if (found)
	send(n, R_INQUIRE_REQUEST,
	     MOK,
	     found->secure->req_id,
	     found->request->user,
	     found->secure->size,
	     found->secure->date,
	     found->request->outcome,
	     found->printer->printer->name,
	     (found->form? found->form->form->name : ""),
	     NB(found->pwheel_name)
	);
    else
	send(n, R_INQUIRE_REQUEST, MNOINFO, "", "", 0L, 0L, 0, "", "", "");

    return;
}

/**
 ** move_request()
 **/

void
move_request  (m, n)
	char			*m;
	NODE			*n;
{
    char	*olddest;
    RSTATUS	*rp;
    short	err;
    char	*req_id;
    char	*dest;
    EXEC	*oldexec;


    (void) getmessage(m, S_MOVE_REQUEST, &req_id, &dest);

    if (!(search_ptable(dest)) && !(search_ctable(dest)))
    {
	send(n, R_MOVE_REQUEST, MNODEST, 0L);
	return;
    }

    if (rp = request_by_id(req_id))
    {
	if (STREQU(rp->request->destination, dest))
	{
	    send(n, R_MOVE_REQUEST, MOK, 0L);
	    return;
	}
	if (rp->request->outcome & (RS_DONE|RS_NOTIFYING))
	{
	    send(n, R_MOVE_REQUEST, M2LATE, 0L);
	    return;
	}
	if (rp->request->outcome & RS_CHANGING)
	{
	    send(n, R_MOVE_REQUEST, MBUSY, 0L);
	    return;
	}
	oldexec = rp->printer->exec;
	olddest = rp->request->destination;
	rp->request->destination = strdup(dest);
	if ((err = validate_request(rp, (char **)0, 1)) == MOK)
	{
	    free(olddest);
	    (void) putrequest(rp->req_file, rp->request);
	    send(n, R_MOVE_REQUEST, MOK, 0L);

	    /*
	     * If the request was being filtered or was printing,
	     * it would have been stopped in "validate_request()",
	     * but only if it has to be refiltered. Thus, the
	     * filtering has been stopped if it has to be stopped,
	     * but the printing may still be going.
	     */
	    if (
		rp->request->outcome & RS_PRINTING
	     && !(rp->request->outcome & RS_STOPPED)
	    )
	    {
		rp->request->outcome |= RS_STOPPED;
		terminate (oldexec);
	    }

	    maybe_schedule(rp);

	    return;
	}
	send(n, R_MOVE_REQUEST, err, chkprinter_result);
	free(rp->request->destination);
	rp->request->destination = olddest;
	return;
    }
    send(n, R_MOVE_REQUEST, MUNKNOWN, 0L);
}

/**
 ** move_dest()
 **/

void
move_dest  (m, n)
	char			*m;
	NODE			*n;
{
    char		*dest;
    char		*fromdest;
    register RSTATUS	*rp;
    char		*olddest;
    EXEC		*oldexec;
    register char	*found = (char *)0;
    register short	num_ok = 0;

    (void) getmessage(m, S_MOVE_DEST, &fromdest, &dest);

    if (!search_ptable(fromdest) && !search_ctable(fromdest))
    {
	send(n, R_MOVE_DEST, MNODEST, fromdest, 0);
	return;
    }

    if (!(search_ptable(dest)) && !(search_ctable(dest)))
    {
	send(n, R_MOVE_DEST, MNODEST, dest, 0);
	return;
    }

    if (STREQU(dest, fromdest))
    {
	send(n, R_MOVE_DEST, MOK, "", 0);
	return;
    }

    BEGIN_WALK_BY_DEST_LOOP (rp, fromdest)
	if (!(rp->request->outcome & (RS_DONE|RS_CHANGING|RS_NOTIFYING)))
	{
	    oldexec = rp->printer->exec;
	    olddest = rp->request->destination;
	    rp->request->destination = strdup(dest);
	    if (validate_request(rp, (char **)0, 1) == MOK)
	    {
		num_ok++;
		free(olddest);
		(void) putrequest(rp->req_file, rp->request);

		/*
		 * If the request was being filtered or was printing,
		 * it would have been stopped in "validate_request()",
		 * but only if it has to be refiltered. Thus, the
		 * filtering has been stopped if it has to be stopped,
		 * but the printing may still be going.
		 */
		if (
		    rp->request->outcome & RS_PRINTING
		 && !(rp->request->outcome & RS_STOPPED)
		)
		{
		    rp->request->outcome |= RS_STOPPED;
		    terminate (oldexec);
	        }

		maybe_schedule(rp);

		continue;
	    }
	    free(rp->request->destination);
	    rp->request->destination = olddest;
	}
	
	if (found)
	    send(n, R_MOVE_DEST, MMORERR, found, 0);

	found = rp->secure->req_id;
    END_WALK_LOOP

    if (found)
	send(n, R_MOVE_DEST, MERRDEST, found, num_ok);
    else
	send(n, R_MOVE_DEST, MOK, "", num_ok);
}
