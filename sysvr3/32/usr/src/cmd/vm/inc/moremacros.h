/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/****************************************************************************
*
*	MACRO's replacing 1 line functions
*
****************************************************************************/

#ident	"@(#)vm:inc/moremacros.h	1.1"

extern	char	*strnsave();
#define strsave(s)	((s) ? strnsave(s, strlen(s)) : NULL )

extern	struct actrec		*AR_cur;
#define	ar_get_current()	AR_cur

extern	char	**Altenv;
extern	char	*getaltenv();
extern	void	copyaltenv();
extern	int	delaltenv();
extern	int	putaltenv();
#define	getAltenv(name)		getaltenv(Altenv, name)
#define	copyAltenv(an_env)	copyaltenv(an_env, &Altenv)
#define	delAltenv(name)		delaltenv(&Altenv, name)
#define	putAltenv(str)		putaltenv(&Altenv, str)
