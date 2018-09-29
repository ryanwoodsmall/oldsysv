/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/rstatus.c	1.7"

#include "lpsched.h"

static RSTATUS		*FreeList	= 0;

/**
 ** freerstatus()
 **/

void			freerstatus (r)
	register RSTATUS	*r;
{
	if (r->exec) {
		if (r->exec->pid > 0)
			terminate (r->exec);
		r->exec->ex.request = 0;
	}

	if (r->secure) {
		if (r->secure->system)
			free (r->secure->system);
		if (r->secure->user)
			free (r->secure->user);
		if (r->secure->req_id)
			free (r->secure->req_id);
	}

	if (r->request)
		freerequest (r->request);

	if (r->req_file)
		free (r->req_file);
	if (r->slow)
		free (r->slow);
	if (r->fast)
		free (r->fast);
	if (r->pwheel_name)
		free (r->pwheel_name);

	remover (r);
	r->next = FreeList;
	FreeList = r;

	return;
}

/**
 ** allocr()
 **/

RSTATUS			*allocr ()
{
	register RSTATUS	*rp;
	register REQUEST	*req;
	register SECURE		*sec;
	

	TRACE_ON ("allocr");

	if ((rp = FreeList)) {

		FreeList = rp->next;
		req	= rp->request;
		sec	= rp->secure;

	} else if (
		!(rp = (RSTATUS *)malloc(sizeof(RSTATUS)))
	     || !(req = (REQUEST *)malloc(sizeof(REQUEST)))
	     || !(sec = (SECURE *)malloc(sizeof(SECURE)))
	)
		fail (MEMORY_ALLOC_FAILED);

	memset ((char *)rp, 0, sizeof(RSTATUS));
	memset ((char *)(rp->request = req), 0, sizeof(REQUEST));
	memset ((char *)(rp->secure = sec), 0, sizeof(SECURE));
	
	TRACE_OFF ("allocr");

	return (rp);
}
			
/**
 ** insertr()
 **/

void			insertr (r)
	RSTATUS			*r;
{
	RSTATUS			*rp;


	if (!Request_List) {
		Request_List = r;
		return;
	}
	
	for (rp = Request_List; rp; rp = rp->next) {
		if (rsort(&r, &rp) < 0) {
			r->prev = rp->prev;
			if (r->prev)
				r->prev->next = r;
			r->next = rp;
			rp->prev = r;
			if (rp == Request_List)
				Request_List = r;
			return;
		}

		if (rp->next)
			continue;

		r->prev = rp;
		rp->next = r;
		return;
	}
}

/**
 ** remover()
 **/

void			remover (r)
	RSTATUS			*r;
{
	if (r == Request_List)
		Request_List = r->next;
	
	if (r->next)
		r->next->prev = r->prev;
	
	if (r->prev)
		r->prev->next = r->next;
	
	r->next = 0;
	r->prev = 0;
	return;
}

/**
 ** request_by_id()
 **/

RSTATUS			*request_by_id (id)
	char			*id;
{
	register RSTATUS	*rp;
	
	for (rp = Request_List; rp; rp = rp->next)
		if (STREQU(id, rp->secure->req_id))
			return (rp);
	return (0);
}

/**
 ** rsort()
 **/

int			rsort (p1, p2)
	register RSTATUS	**p1,
				**p2;
{
	/*
	 * Of two requests needing immediate handling, the first
	 * will be the request with the LATER date. In case of a tie,
	 * the first is the one with the larger request ID (i.e. the
	 * one that came in last).
	 */
	if ((*p1)->request->outcome & RS_IMMEDIATE)
		if ((*p2)->request->outcome & RS_IMMEDIATE)
			if (later(*p1, *p2))
				return (-1);
			else
				return (1);
		else
			return (-1);

	else if ((*p2)->request->outcome & RS_IMMEDIATE)
		return (1);

	/*
	 * Of two requests not needing immediate handling, the first
	 * will be the request with the highest priority. If both have
	 * the same priority, the first is the one with the EARLIER date.
	 * In case of a tie, the first is the one with the smaller ID
	 * (i.e. the one that came in first).
	 */
	else if ((*p1)->request->priority == (*p2)->request->priority)
		if (!later(*p1, *p2))
			return (-1);
		else
			return (1);

	else
		return ((*p1)->request->priority - (*p2)->request->priority);
	/*NOTREACHED*/
}

static int		later (prs1, prs2)
	register RSTATUS	*prs1,
				*prs2;
{
	if (prs1->secure->date > prs2->secure->date)
		return (1);

	else if (prs1->secure->date < prs2->secure->date)
		return (0);

	/*
	 * The dates are the same, so compare the request IDs.
	 * One problem with comparing request IDs is that the order
	 * of two IDs may be reversed if the IDs wrapped around. This
	 * is a very unlikely problem, because the cycle should take
	 * more than one second to wrap!
	 */
	else {
		register int		len1 = strlen(prs1->req_file),
					len2 = strlen(prs2->req_file);

		/*
		 * Use the request file name (ID-0) for comparison,
		 * because the real request ID (DEST-ID) won't compare
		 * properly because of the destination prefix.
		 * The strlen() comparison is necessary, otherwise
		 * IDs like "99-0" and "100-0" will compare wrong.
		 */
		if (len1 > len2)
			return (1);
		else if (len1 < len2)
			return (0);
		else
			return (strcmp(prs1->req_file, prs2->req_file) > 0);
	}
	/*NOTREACHED*/
}
