/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/status.c	1.7"

#include "string.h"
#include "lpsched.h"

#define NCMP(X,Y)	(STRNEQU((X), (Y), sizeof(Y)-1))

extern long		atol(),
			time();

static void		load_pstatus(),
			load_cstatus(),
			put_multi_line();

static char		*pstatus	= 0,
			*cstatus	= 0;

/**
 ** load_status() - LOAD PRINTER/CLASS STATUS FILES
 **/

void			load_status ()
{
	load_pstatus ();
	load_cstatus ();
	return;
}

/**
 ** load_pstatus() - LOAD PRITNER STATUS FILE
 **/

static void		load_pstatus ()
{
	PSTATUS			*pps;

	char			*rej_reason,
				*dis_reason,
				*pwheel_name,
				buf[BUFSIZ],
				*name,
				*p;

	time_t			rej_date,
				dis_date;

	short			status;

	FSTATUS			*pfs;

	PWSTATUS		*ppws;

	int			i,
				len,
				total;

	time_t			now;

	register FILE		*fp;

	register int		f;


	(void) time(&now);

	if (
		!pstatus
	     && !(pstatus = makepath(Lp_System, PSTATUSFILE, (char *)0))
	) {
		fail (MEMORY_ALLOC_FAILED);
		/*NOTREACHED*/
	}
	if ((fp = open_lpfile(pstatus, "r")))
		while (!feof(fp)) {

			status = 0;

			total = 0;
			name = 0;
			rej_reason = 0;
			dis_reason = 0;

			for (f = 0; f < PST_MAX && fgets(buf, BUFSIZ, fp); f++) {
				if (p = strrchr(buf, '\n'))
					*p = '\0';

				switch (f) {
				case PST_BRK:
					break;

				case PST_NAME:
					name = strdup(buf);
					break;

				case PST_STATUS:
					if (NCMP(buf, NAME_DISABLED))
						status |= PS_DISABLED;
					p = strchr(buf, ' ');
					if (!p || !*(++p))
						break;
					if (NCMP(p, NAME_REJECTING))
						status |= PS_REJECTED;
					break;

				case PST_DATE:
					dis_date = (time_t)atol(buf);
					p = strchr(buf, ' ');
					if (!p || !*(++p))
						break;
					rej_date = (time_t)atol(p);
					break;

				case PST_DISREAS:
					len = strlen(buf);
					if (buf[len - 1] == '\\') {
						buf[len - 1] = '\n';
						f--;
					}
					if (dis_reason) {
						total += len;
						dis_reason = realloc(
							dis_reason,
							total+1
						);
						if (dis_reason)
							strcat (dis_reason, buf);
					} else {
						dis_reason = strdup(buf);
						total = len;
					}
					break;

				case PST_REJREAS:
					len = strlen(buf);
					if (buf[len - 1] == '\\') {
						buf[len - 1] = '\n';
						f--;
					}
					if (rej_reason) {
						total += len;
						rej_reason = realloc(
							rej_reason,
							total+1
						);
						if (rej_reason)
							strcat (rej_reason, buf);
					} else {
						rej_reason = strdup(buf);
						total = len;
					}
					break;

				case PST_PWHEEL:
					if (*buf) {
						ppws = search_pwtable(buf);
						pwheel_name = strdup(buf);
					} else {
						ppws = 0;
						pwheel_name = 0;
					}
					break;

				case PST_FORM:
					if (*buf)
						pfs = search_ftable(buf);
					else
						pfs = 0;
					break;
				}
			}

			if (ferror(fp) || f && f != PST_MAX) {
				close_lpfile (fp);
				note (READ_FILE_FAILED, pstatus);
				return;
			}

			if (!feof(fp) && name && (pps = search_ptable(name))) {
				pps->rej_date = rej_date;
				pps->status = status;
				if ((pps->form = pfs))
					pfs->mounted++;
				pps->pwheel_name = pwheel_name;
				if ((pps->pwheel = ppws))
					ppws->mounted++;
				pps->rej_reason = rej_reason;
				if (pps->printer->login) {
					pps->dis_date = now;
					pps->dis_reason = strdup(CUZ_LOGIN_PRINTER);
				} else {
					pps->dis_date = dis_date;
					pps->dis_reason = dis_reason;
				}

			} else {
				if (dis_reason)
					free(dis_reason);
				if (rej_reason)
					free(rej_reason);
			}
			if (name)
				free (name);
		}

	if (fp) {
		if (ferror(fp)) {
			close_lpfile (fp);
			note (READ_FILE_FAILED, pstatus);
			return;
		}
		close_lpfile (fp);
	}

	for (i = 0; i < PT_Size; i++)
		if (PStatus[i].printer->name && !PStatus[i].rej_reason) {
			PStatus[i].dis_reason = strdup(CUZ_NEW_PRINTER);
			PStatus[i].rej_reason = strdup(CUZ_NEW_DEST);
			PStatus[i].dis_date = now;
			PStatus[i].rej_date = now;
			PStatus[i].status = PS_DISABLED | PS_REJECTED;
		}

	return;
}

/**
 ** load_cstatus() - LOAD CLASS STATUS FILE
 **/

static void		load_cstatus ()
{
	CSTATUS			*pcs;

	char			*rej_reason,
				buf[BUFSIZ],
				*name,
				*p;

	time_t			rej_date;

	short			status;

	int			i,
				len,
				total;

	time_t			now;

	register FILE		*fp;

	register int		f;


	(void) time(&now);

	if (
		!cstatus
	     && !(cstatus = makepath(Lp_System, CSTATUSFILE, (char *)0))
	) {
		fail (MEMORY_ALLOC_FAILED);
		/*NOTREACHED*/
	}

	if ((fp = open_lpfile(cstatus, "r")))
		while (!feof(fp)) {
			status = 0;

			total = 0;
			name = 0;

			rej_reason = 0;
			for (f = 0; f < CST_MAX && fgets(buf, BUFSIZ, fp); f++) {
				if (p = strrchr(buf, '\n'))
					*p = '\0';
				switch (f) {
				case CST_BRK:
					break;

				case CST_NAME:
					name = strdup(buf);
					break;

				case CST_STATUS:
					if (NCMP(buf, NAME_REJECTING))
						status |= PS_REJECTED;
					break;

				case CST_DATE:
					rej_date = (time_t)atol(buf);
					break;

				case CST_REJREAS:
					len = strlen(buf);
					if (buf[len - 1] == '\\') {
						buf[len - 1] = '\n';
						f--;
					}
					if (rej_reason) {
						total += len;
						rej_reason = realloc(
							rej_reason,
							total+1
						);
						if (rej_reason)
							strcat (rej_reason, buf);
					} else {
						rej_reason = strdup(buf);
						total = len;
					}
					break;
				}
			}

			if (ferror(fp) || f && f != CST_MAX) {
				close_lpfile (fp);
				note (READ_FILE_FAILED, cstatus);
				return;
			}

			if (!feof(fp) && name && (pcs = search_ctable(name))) {
				pcs->rej_reason = rej_reason;
				pcs->rej_date = rej_date;
				pcs->status = status;

			} else
				if (rej_reason)
					free (rej_reason);

			if (name)
				free (name);
		}

	if (fp) {
		if (ferror(fp)) {
			close_lpfile (fp);
			note (READ_FILE_FAILED, cstatus);
			return;
		}
		close_lpfile (fp);
	}

	for (i = 0; i < CT_Size; i++)
		if (CStatus[i].class->name && !CStatus[i].rej_reason) {
			CStatus[i].status = CS_REJECTED;
			CStatus[i].rej_reason = strdup(CUZ_NEW_DEST);
			CStatus[i].rej_date = now;
		}

	return;
}

/**
 ** dump_status() - DUMP PRINTER/CLASS STATUS FILES
 **/

void			dump_status ()
{
	dump_pstatus ();
	dump_cstatus ();
	return;
}

/**
 ** dump_pstatus() - DUMP PRINTER STATUS FILE
 **/

void			dump_pstatus ()
{
	PSTATUS			*ppsend;

	FILE			*fp;

	register PSTATUS	*pps;

	register int		f;


	if (
		!pstatus
	     && !(pstatus = makepath(Lp_System, PSTATUSFILE, (char *)0))
	) {
		fail (MEMORY_ALLOC_FAILED);
		/*NOTREACHED*/
	}
	if (!(fp = open_lpfile(pstatus, "w", MODE_READ))) {
		note (CANNOT_OPEN_FILE, pstatus, PERROR);
		return;
	}

	for (pps = PStatus, ppsend = &PStatus[PT_Size]; pps < ppsend; pps++)
		if (pps->printer->name)
			for (f = 0; f < PST_MAX; f++) switch (f) {
			case PST_BRK:
				FPRINTF (fp, "%s\n", STATUS_BREAK);
				break;
			case PST_NAME:
				FPRINTF (fp, "%s\n", NB(pps->printer->name));
				break;
			case PST_STATUS:
				FPRINTF (
					fp,
					"%s %s\n",
					(pps->status & PS_DISABLED? NAME_DISABLED : NAME_ENABLED),
					(pps->status & PS_REJECTED? NAME_REJECTING : NAME_ACCEPTING)
				);
				break;
			case PST_DATE:
				FPRINTF (
					fp,
					"%ld %ld\n",
					pps->dis_date,
					pps->rej_date
				);
				break;
			case PST_DISREAS:
				put_multi_line (fp, pps->dis_reason);
				break;
			case PST_REJREAS:
				put_multi_line (fp, pps->rej_reason);
				break;
			case PST_PWHEEL:
				FPRINTF (fp, "%s\n", NB(pps->pwheel_name));
				break;
			case PST_FORM:
				FPRINTF (
					fp,
					"%s\n",
					(pps->form) ?
					pps->form->form->name : ""
				);
				break;
			}

	close_lpfile (fp);

	return;
}

/**
 ** dump_cstatus() - DUMP CLASS STATUS FILE
 **/

void			dump_cstatus ()
{
	CSTATUS			*pcsend;

	FILE			*fp;

	register CSTATUS	*pcs;

	register int		f;


	if (
		!cstatus
	     && !(cstatus = makepath(Lp_System, CSTATUSFILE, (char *)0))
	) {
		fail (MEMORY_ALLOC_FAILED);
		/*NOTREACHED*/
	}
	if (!(fp = open_lpfile(cstatus, "w", MODE_READ))) {
		note (CANNOT_OPEN_FILE, cstatus, PERROR);
		return;
	}

	for (pcs = CStatus, pcsend = &CStatus[CT_Size]; pcs < pcsend; pcs++)
		if (pcs->class->name)
			for (f = 0; f < CST_MAX; f++) switch (f) {
			case CST_BRK:
				FPRINTF (fp, "%s\n", STATUS_BREAK);
				break;
			case CST_NAME:
				FPRINTF (fp, "%s\n", NB(pcs->class->name));
				break;
			case CST_STATUS:
				FPRINTF (
					fp,
					"%s\n",
					(pcs->status & CS_REJECTED? NAME_REJECTING : NAME_ACCEPTING)
				);
				break;
			case CST_DATE:
				FPRINTF (fp, "%ld\n", pcs->rej_date);
				break;
			case CST_REJREAS:
				put_multi_line (fp, pcs->rej_reason);
				break;
			}

	close_lpfile (fp);

	return;
}

/**
 ** put_multi_line() - PRINT OUT MULTI-LINE TEXT
 **/

static void		put_multi_line (fp, buf)
	FILE			*fp;
	char			*buf;
{
	register char		*cp,
				*p;

	if (!buf) {
		(void)fprintf (fp, "\n");
		return;
	}

	for (p = buf; (cp = strchr(p, '\n')); ) {
		*cp++ = 0;
		(void)fprintf (fp, "%s\\\n", p);
		p = cp;
	}
	(void)fprintf (fp, "%s\n", p);
	return;
}
