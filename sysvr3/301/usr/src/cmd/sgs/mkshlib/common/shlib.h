/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/shlib.h	1.3"

#define FALSE	0
#define TRUE	1

extern char	*specname;
extern char	*trgname;
extern char	*hstname;
extern char	*idstr,
		*progname,
		*prefix,
		*ldname,
		*asname;
extern char	**trgobjects;
extern long	numobjs;
extern int	maketarg,
		makehost,
		qflag;


/* System supplied functions */
extern FILE     *fopen();
extern char     *strcat(), *strcpy(), *strncpy(), *strchr(), *strrchr(), *strtok();
extern char     *malloc(), *realloc(), *calloc(), *ldgetname();
extern long     time(), strtol();
extern void	exit(), free(), perror();
extern LDFILE	*ldopen();


/* User supplied functions */
extern void	catchsigs();
extern void	cmdline();
extern char	*concat();
extern int	copy();
extern void	dexit();
extern void	directive();
extern int	execute();
extern void	fatal();
extern char	*getbase();
extern char	*getident();
extern char	*getname();
extern char	*getpref();
extern char	*gettok();
extern long	hash();
extern char	*makename();
extern void	mktmps();
extern void	rdspec();
extern void	rm();
extern void	rmtmps();
extern void	spec();
extern char	*stralloc();
extern void	usagerr();
