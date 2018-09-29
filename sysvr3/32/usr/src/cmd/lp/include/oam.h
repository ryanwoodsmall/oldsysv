/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/oam.h	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/*
 * Change the following lines to include the appropriate
 * standard header file when it becomes available.
 * Or change all the LP source to include it directly,
 * and get rid of the following stuff (up to the ====...==== line).
 */

char			*agettxt(
#if	defined(CPLUSPLUS) || defined(ANSI_C)
				long,		/* msg_id	*/
				char *,		/* buf		*/
				int		/* buflen	*/
#endif
			);

int			fmtmsg(
#if	defined(CPLUSPLUS) || defined(ANSI_C)
				char *,		/* label	*/
				long,		/* classification */
				int,		/* severity	*/
				char *,		/* text		*/
				char *,		/* tag		*/
				char *		/* action	*/
#endif
			);

/*
 * Possible values of "severity":
 */
#define	MIN_SEVERITY	0
#define	HALT		0
#define	ERROR		1
#define	WARNING		2
#define	INFO		3
#define	MAX_SEVERITY	3

/*
 * Possible values of "classification":
 */

	/*
	 * ``Major classification''
	 */
#define	HARD		0x00000000
#define	SOFT		0x00001000
#define	FIRM		0x00002000

	/*
	 * ``Sub-classification'' 1
	 */
#define	APPL		0x00000000
#define	UTIL		0x00000100
#define	OPSYS		0x00000200

	/*
	 * ``Sub-classification'' 2
	 */
#define	RECOVER		0x00000000
#define NRECOVER	0x00000010

/**======================================================================
 **
 ** LP Spooler specific error message handling.
 **/

#define	MSGSIZ		512

#if	defined(WHO_AM_I)

#include "oam_def.h"

#if	WHO_AM_I == I_AM_CANCEL
static char		*who_am_i = "UX:cancel"+3;

#elif	WHO_AM_I == I_AM_COMB
static char		*who_am_i = "UX:comb           "+3;
				  /* changed inside pgm */

#elif	WHO_AM_I == I_AM_LPMOVE
static char		*who_am_i = "UX:lpmove"+3;

#elif	WHO_AM_I == I_AM_LPUSERS
static char		*who_am_i = "UX:lpusers"+3;

#elif	WHO_AM_I == I_AM_LPNETWORK
static char		*who_am_i = "UX:lpnetwork"+3;

#elif	WHO_AM_I == I_AM_LP
static char		*who_am_i = "UX:lp"+3;

#elif	WHO_AM_I == I_AM_LPADMIN
static char		*who_am_i = "UX:lpadmin"+3;

#elif	WHO_AM_I == I_AM_LPFILTER
static char		*who_am_i = "UX:lpfilter"+3;

#elif	WHO_AM_I == I_AM_LPFORMS
static char		*who_am_i = "UX:lpforms"+3;

#elif	WHO_AM_I == I_AM_LPPRIVATE
static char		*who_am_i = "UX:lpprivate"+3;

#elif	WHO_AM_I == I_AM_LPSCHED
static char		*who_am_i = "UX:lpsched"+3;

#elif	WHO_AM_I == I_AM_LPSHUT
static char		*who_am_i = "UX:lpshut"+3;

#elif	WHO_AM_I == I_AM_LPSTAT
static char		*who_am_i = "UX:lpstat"+3;

#else
static char		*who_am_i = "UX:mysterious"+3;

#endif

/*
 * Simpler interfaces to the "fmtmsg()" and "agettxt()" stuff.
 */

#if	defined(lint)

#define LP_ERRMSG(C,X)			(void)printf("", C, X)
#define LP_ERRMSG1(C,X,A)		(void)printf("", C, X, A)
#define LP_ERRMSG2(C,X,A1,A2)		(void)printf("", C, X, A1, A2)
#define LP_ERRMSG3(C,X,A1,A2,A3)	(void)printf("", C, X, A1, A2, A3)

#else

#define	LP_ERRMSG(C,X) \
			(void)fmtmsg ( \
				who_am_i-3, \
				SOFT|UTIL|RECOVER, \
				C, \
				agettxt((X), _m_, MSGSIZ), \
				tagit((X)), \
				agettxt((X+1), _a_, MSGSIZ) \
			)
#define	LP_ERRMSG1(C,X,A) \
			(void)fmtmsg ( \
				who_am_i-3, \
				SOFT|UTIL|RECOVER, \
				C, \
				fmt1((X), A), \
				tagit((X)), \
				agettxt((X+1), _a_, MSGSIZ) \
			)
#define	LP_ERRMSG2(C,X,A1,A2) \
			(void)fmtmsg ( \
				who_am_i-3, \
				SOFT|UTIL|RECOVER, \
				C, \
				fmt2((X), A1, A2), \
				tagit((X)), \
				agettxt((X+1), _a_, MSGSIZ) \
			)
#define	LP_ERRMSG3(C,X,A1,A2,A3) \
			(void)fmtmsg ( \
				who_am_i-3, \
				SOFT|UTIL|RECOVER, \
				C, \
				fmt3((X), A1, A2, A3), \
				tagit((X)), \
				agettxt((X+1), _a_, MSGSIZ) \
			)

#define	vsp		(void)sprintf

#define	tagit(x)	(vsp(_t_, "%d", (x)), _t_)

#define fmt1(X,A)	(vsp(_m_, agettxt((X),_f_,MSGSIZ), A), _m_)
#define fmt2(X,A,B)	(vsp(_m_, agettxt((X),_f_,MSGSIZ), A,B), _m_)
#define fmt3(X,A,B,C)	(vsp(_m_, agettxt((X),_f_,MSGSIZ), A,B,C), _m_)

#endif	/* lint */

extern char		_m_[],
			_a_[],
			_f_[],
			*_t_;

#endif	/* WHO_AM_I */
