/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/conv.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "errno.h"

#include "lp.h"
#include "filters.h"

extern char		*malloc();

static char		*keyword_list[] = {
	"INPUT",
	"OUTPUT",
	"TERM",
	"CPI",
	"LPI",
	"LENGTH",
	"WIDTH",
	"PAGES",
	"CHARSET",
	"FORM",
	"COPIES",
	"MODES",
	0
};

static char		*q_strchr(),
			*q_strdup();

/**
 ** s_to_filtertype() - CONVERT (char *) TO (FILTERTYPE)
 **/

FILTERTYPE		s_to_filtertype (str)
	char			*str;
{
	/*
	 * The default type, if none is given, is ``slow''.
	 */
	if (STREQU(str, FL_FAST))
		return (fl_fast);
	else
		return (fl_slow);
}

/**
 ** s_to_type() - CONVERT (char *) TO (TYPE) 
 **/

TYPE			s_to_type (str)
	register char		*str;
{
	TYPE			ret;

	if ((ret.name = strdup(str)))
		ret.info = isterminfo(str);
	return (ret);
}

/**
 ** s_to_template() - CONVERT (char *) TO (TEMPLATE)
 **/

TEMPLATE		s_to_template (str)
	register char		*str;
{
	TEMPLATE		ret;

	register char		*p,
				*q,
				c;
	
	register int		e;


	if (!*(str += strspn(str, " "))) {
		lp_errno = LP_ETEMPLATE;
		ret.keyword = 0;
		goto Done;
	}

	if (!(p = strchr(str, ' '))) {
		lp_errno = LP_EPATTERN;
		ret.keyword = 0;
		goto Done;
	}

	c = *p;
	*p = 0;
	ret.keyword = strdup(str);
	*p = c;

	if (!ret.keyword) {
		lp_errno = LP_ENOMEM;
		goto Done;
	}
	if (!searchlist(ret.keyword, keyword_list)) {
		lp_errno = LP_EKEYWORD;
		ret.keyword = 0;
		goto Done;
	}

	str = p + strspn(p, " ");
	if (!(p = q_strchr(str, '='))) {
		lp_errno = LP_ERESULT;
		ret.keyword = 0;
		goto Done;
	}
	while (p[-1] == ' ' && p > str)
		p--;

	c = *p;
	*p = 0;
	ret.pattern = q_strdup(str);
	*p = c;

	if (!ret.pattern) {
		lp_errno = LP_ENOMEM;
		ret.keyword = 0;
		goto Done;
	}

	if (!*ret.pattern) {
		lp_errno = LP_EPATTERN;
		ret.keyword = 0;
		goto Done;
	}

#if	defined(PATT_RE)
	{
		extern char		*regcmp();

		if (!(ret.re = regcmp(ret.pattern, (char *)0))) {
			lp_errno = LP_EREGEX;
			ret.keyword = 0;
			goto Done;
		}
	}
#endif

	if (!*(str = p + strspn(p, " ="))) {
		lp_errno = LP_ERESULT;
		ret.keyword = 0;
		goto Done;
	}
	ret.result = strdup(str);
	if (!ret.result) {
		lp_errno = LP_ENOMEM;
		ret.keyword = 0;
	}

Done:	return (ret);		
}

/**
 ** sl_to_typel() - CONVERT (char **) LIST TO (TYPE *) LIST
 **/

TYPE			*sl_to_typel (src)
	char			**src;
{
	register TYPE		*dst;

	register int		nitems,
				n;

	if (!src || !*src)
		return (0);

	for (nitems = 0; src[nitems]; nitems++)
		;

	if (!(dst = (TYPE *)malloc((nitems + 1) * sizeof(TYPE)))) {
		errno = ENOMEM;
		return (0);
	}

	for (n = 0; n < nitems; n++)
		dst[n] = s_to_type(src[n]);
	dst[nitems].name = 0;

	return (dst);
}

/**
 ** sl_to_templatel() - DUPLICATE A (char **) LIST AS (TEMPLATE *) LIST
 **/

TEMPLATE		*sl_to_templatel (src)
	register char		**src;
{
	register TEMPLATE	*dst;

	register int		nitems,
				n;

	if (!src || !*src)
		return (0);

	for (nitems = 0; src[nitems]; nitems++)
		;

	if (!(dst = (TEMPLATE *)malloc((nitems + 1) * sizeof(TEMPLATE)))){
		errno = ENOMEM;
		return (0);
	}

	for (n = 0; n < nitems; n++)
		if ((dst[n] = s_to_template(src[n])).keyword == 0) {
			freetempl (dst);
			return (0);
		}
	dst[nitems].keyword = 0;

	return (dst);
}

/**
 ** type_to_s() - CONVERT (TYPE) TO (char *)
 **/

char			*type_to_s (t)
	TYPE			t;
{
	return (strdup(t.name));
}

/**
 ** template_to_s() - CONVERT (TEMPLATE) TO (char *)
 **/

char			*template_to_s (t)
	TEMPLATE		t;
{
	register char		*ret,
				*p,
				*r;

	register unsigned int	len;


	len  = strlen(t.keyword) + 1;
	for (p = t.pattern; *p; p++) {
		if (*p == '=')
			len++;
		len++;
	}
	len += 3 + strlen(t.result);

	ret = malloc(len + 1);
	if (!ret) {
		errno = ENOMEM;
		return (0);
	}

	r = ret;
	for (p = t.keyword; *p; )
		*r++ = *p++;
	*r++ = ' ';
	for (p = t.pattern; *p; ) {
		if (*p == '=')
			*r++ = '\\';
		*r++ = *p++;
	}
	*r++ = ' ';
	*r++ = '=';
	*r++ = ' ';
	for (p = t.result; *p; )
		*r++ = *p++;
	*r = 0;

	return (ret);
}

/**
 ** typel_to_sl() - DUPLICATE (TYPE *) LIST AS (char **) LIST
 **/

char			**typel_to_sl (src)
	TYPE			*src;
{
	register char		**dst;

	register int		nitems,
				n;

	if (!src || !src->name)
		return (0);

	for (nitems = 0; src[nitems].name; nitems++)
		;

	if (!(dst = (char **)malloc((nitems + 1) * sizeof(char *)))) {
		errno = ENOMEM;
		return (0);
	}

	for (n = 0; n < nitems; n++)
		dst[n] = type_to_s(src[n]);
	dst[nitems] = 0;

	return (dst);
}

/**
 ** templatel_to_sl() - DUPLICATE A (TEMPLATE *) LIST AS (char **) LIST
 **/

char			**templatel_to_sl (src)
	register TEMPLATE	*src;
{
	register char		**dst;

	register int		nitems,
				n;

	if (!src || !src->keyword)
		return (0);

	for (nitems = 0; src[nitems].keyword; nitems++)
		;

	if (!(dst = (char **)malloc((nitems + 1) * sizeof(char *)))) {
		errno = ENOMEM;
		return (0);
	}

	for (n = 0; n < nitems; n++)
		dst[n] = template_to_s(src[n]);
	dst[nitems] = 0;

	return (dst);
}

/**
 ** q_strpbrk() - strpbrk() WITH BACKSLASH QUOTING
 ** q_strdup() - strdup() WITH BACKSLASHES OMITTED
 **/

char			*q_strchr (sp, c)
	register char		*sp,
				c;
{
	do {
		if (*sp == '\\' && sp[1])
			sp += 2;
		if (*sp == c)
			return (sp);
	} while (*sp++);
	return (0);
}

char			*q_strdup (str)
	char			*str;
{
	char			*ret;

	register char		*p,
				*q;

	register int		len	= 0;


	for (p = str; *p; p++) {
		if (*p == '\\' && p[1] == '=')
			p++;
		len++;
	}

	if (!(ret = q = malloc(len + 1)))
		return (0);

	for (p = str; *p; p++) {
		if (*p == '\\' && p[1] == '=')
			p++;
		*q++ = *p;
	}
	*q = 0;

	return (ret);
}
