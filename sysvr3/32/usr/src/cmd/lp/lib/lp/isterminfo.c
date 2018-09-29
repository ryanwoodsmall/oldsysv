/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/isterminfo.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "fcntl.h"
#include "errno.h"
#include "string.h"
#include "unistd.h"
#include "search.h"

#include "lp.h"

extern void		free();

/*
 * Define the following if you want to do a stronger check:
 * that a type names a valid entry in the Terminfo database.
 * The stronger check reads the entry and verifies the magic
 * number in the header. The weaker check is to see if we
 * have read access to the file. The weaker check will be a
 * tad faster.
 */
/* #define STRONG_CHECK	1	/* */

/*
 * Define the following if you want to cache hits and/or misses.
 * One reason for NOT caching is to guard against crazies who try
 * submitting print requests with goofball types, or every valid type
 * under the sun. Since Terminfo is limited, the hit cache is effectively
 * limited, so it shouldn't be a problem searching the cache (the search
 * is binary) but the cache can become big. The miss cache, on the other
 * hand, could cause a problem. This problem can become severe, so
 * consider limiting the size of the cache (see below).
 * Another reason for NOT caching is to pick up changes in the Terminfo
 * database. The "terminfo" type is not likely to be an oft used feature,
 * though, so this shouldn't be a big problem.
 * The reason FOR caching is to limit the number of file system accesses.
 * This routine is called OFTEN, so limiting the number of open() or
 * access() calls is a good idea.
 */
#define CACHE_HITS 1	/* */
#define CACHE_MISSES 1	/* */

/*
 * Define the following if you want to limit the sizes of the caches.
 */
#define HIT_CACHE_LIMIT		100	/* */
#define MISS_CACHE_LIMIT	100	/* */

#if	defined(CACHE_HITS)
static char		*hit_cache	= 0;

# if	defined(HIT_CACHE_LIMIT)
static int		hit_cache_size	= 0;
# endif
#endif

#if	defined(CACHE_MISSES)
static char		*miss_cache	= 0;

# if	defined(MISS_CACHE_LIMIT)
static int		miss_cache_size	= 0;
# endif
#endif

#if	defined(CACHE_HITS) || defined(CACHE_MISSES)
extern char		*tsearch(),
			*tfind();
#endif

static int		_isterminfo();

/**
 ** isterminfo() - SEE IF TYPE IS IN TERMINFO DATABASE
 **/

int			isterminfo (type)
	char			*type;
{
	register int		ret;

	static char		*envTERMINFO	= 0;

	extern char		*getenv();


	if (!type || !*type)
		return (0);

#if	defined(CACHE_HITS)
	if (tfind(type, &hit_cache, strcmp))
		return (1);
#endif

#if	defined(CACHE_MISSES)
	if (tfind(type, &miss_cache, strcmp))
		return (0);
#endif

	if (!envTERMINFO)
		envTERMINFO = getenv("TERMNIFO");
	if (
		envTERMINFO
	     && _isterminfo(type, envTERMINFO)
#if	defined(TERMINFO)
	     || _isterminfo(type, TERMINFO)
#endif
	) {
		ret = 1;

#if	defined(CACHE_HITS)
# if	defined(HIT_CACHE_LIMIT)
		if (hit_cache_size++ < HIT_CACHE_LIMIT)
# endif
			(void)tsearch (strdup(type), &hit_cache, strcmp);
#endif

	} else {
		ret = 0;

#if	defined(CACHE_MISSES)
# if	defined(MISS_CACHE_LIMIT)
		if (miss_cache_size++ < MISS_CACHE_LIMIT)
# endif
			(void)tsearch (strdup(type), &miss_cache, strcmp);
#endif
	}
	return (ret);
}

/**
 ** _isterminfo()
 **/

static int		_isterminfo (type, parent)
	char			*type,
				*parent;
{
	char			*path,
				*type_letter	= "X";

	int			ret;

#if	defined(STRONG_CHECK)
	int			fd;
#endif


	type_letter[0] = type[0];
	if (!(path = makepath(parent, type_letter, type, (char *)0)))
		return (0);

#if	defined(STRONG_CHECK)
	if (!(fd = Open(path, O_RDONLY))) {

		/*
		 * If we can't open the TERMINFO file because we
		 * don't have any open channels left, let's err on
		 * the side of likelihood--if the file can be
		 * accessed, figure that it's okay.
		 */
		if (errno == EMFILE && Access(path, R_OK) == 0)
			ret = 1;
		else
			ret = 0;

	} else {

		char			buf[2];

		if (Read(fd, buf, 2) == 2 && buf[0] == 26 && buf[1] == 1)
			ret = 1;
		else
			ret = 0;

		Close (fd);

	}
#else
	ret = (Access(path, R_OK) == 0);
#endif

	free (path);

	return (ret);
}
