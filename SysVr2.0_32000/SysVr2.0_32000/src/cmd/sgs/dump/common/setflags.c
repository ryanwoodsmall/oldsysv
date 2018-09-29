/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include	<stdio.h>
#include	<ctype.h>
#include	"flagdefs.h"
#include	"sgs.h"

extern int	ahread( ),
		fhread( ),
		ohseek( ),
		shread( ),
		sseek( ),
		rseek( ),
		olseek( ),
		zread( ),
		tbread( );
#if FLEXNAMES
extern int	strseek( );
#endif

FLAG	flag[NUMFLAGS] = {
	{0, ahread},
	{0, fhread},
	{0, ohseek},
	{0, shread},
	{0, sseek},
	{0, rseek},
	{0, olseek},
	{0, zread},
	{0, tbread},
#ifdef FLEXNAMES
	{0, strseek}
#endif
};

char	*name = NULL,
	*zname = NULL;
long	dstart = 0L,
	dstop = 0L,
	tstart = 0L,
	tstop = 0L;
unsigned short	zstart = 0,
		zstop = 0;
	int	vflag = 0;
	int	pflag = 0;
	int	uflag = 0;
#if PORTAR
	int	gflag = 0;
#endif


int
setflags(flagc, flagv)

int	flagc;
char	**flagv;

{
    extern		fprintf( );
    extern long		atol( );
    extern int		atoi( );
    extern char		*strchr( );

    extern FLAG		flag[ ];
    extern char		*name,
			*zname;
    extern long		dstart,
			dstop,
			tstart,
			tstop;
    extern unsigned short	zstart,
				zstop;

    extern   int	vflag;
    int			toggle;
    int			filec;
    char		**filev;
    char		*zptr;


    for (filec = 0, filev = flagv; flagc > 0; --flagc, ++flagv) {
	if (**flagv == '-') {
	    toggle = OFF;
	    while (toggle == OFF && *++*flagv != '\0') {
		switch (**flagv) {
		    case 'a':
			flag[A].setting = ON;
			break;

		    case 'f':
			flag[F].setting = ON;
			break;

		    case 'o':
			flag[O].setting = ON;
			break;

		    case 'h':
			flag[H].setting = ON;
			break;

		    case 's':
			flag[S].setting = ON;
			break;

		    case 'r':
			flag[R].setting = ON;
			break;

		    case 'l':
			flag[L].setting = ON;
			break;

		    case 'c':
#if FLEXNAMES
			flag[C].setting = ON;
#else
			fprintf( stderr, "-c option is not valid\n");
#endif
			break;

		    case 'g':
#if PORTAR
			gflag = ON;
#else
			fprintf( stderr, "-g option not valid\n");
#endif
			break;

		    case 'z':
			toggle = ON;
			flag[Z].setting = ON;
			if (*++*flagv == '\0') {
			    if (--flagc > 0) {
				zname = *++flagv;
			    } else {
				fprintf(stderr, "usage:  %sdump [flags] [-z _f_c_n_n_a_m_e [number] | -z_f_c_n_n_a_m_e[,number]] file ... \n", SGS);
			    }
			} else {
			    zname = *flagv;
			}
			if ((zptr = strchr(zname, ',')) != NULL) {
			    *zptr = '\0';
			    if (isdigit(*++zptr)) {
				zstart = atoi(zptr);
			    }
			}
			if ((flagc > 1) && (isdigit(*flagv[1]))) {
			    --flagc;
			    zstart = atoi(*++flagv);
			}
			break;

		    case 'n':
			toggle = ON;
			if (*++*flagv == '\0') {
			    if (--flagc > 0) {
				name = *++flagv;
			    } else {
				fprintf(stderr, "usage:  %sdump [flags] [-n _n_a_m_e | -n_n_a_m_e] file ...\n", SGS);
			    }
			} else {
			    name = *flagv;
			}
			break;

		    case 't':
			flag[T].setting = ON;
			if ((*flagv)[1] == '\0') {
			    toggle = ON;
			    if ((flagc > 1) && (isnum(*(flagv+1)))) {
				--flagc;
				tstart = atol(*++flagv);
			    }
			} else if (isnum(&(*flagv)[1])) {
			    toggle = ON;
			    tstart = atol(++*flagv);
			}
			break;

		    case 'v':
			vflag = 1;
			break;

		    case 'd':
			if ((*flagv)[1] == '\0') {
			    toggle = ON;
			    if ((flagc > 1) && (isdigit(*flagv[1]))) {
				--flagc;
				dstart = atol(*++flagv);
			    } else {
				fprintf(stderr, "usage:  %sdump [flags] [-d _n_u_m_b_e_r | -d_n_u_m_b_e_r] file ...\n", SGS);
			    }
			} else if (isdigit((*flagv)[1])) {
			    toggle = ON;
			    dstart = atol(++*flagv);
			} else {
			    fprintf(stderr, "usage:  %sdump [flags] [-d_n_u_m_b_e_r | -d _n_u_m_b_e_r] file...\n", SGS);
			}
			break;

		    case 'V':
			fprintf(stderr,"%s: dump -%s\n",SGSNAME,RELEASE);
			break;

		    case 'p':
			pflag = 1;
			break;

		    case 'u':
			uflag = 1;
			break;

		    default:
			fprintf(stderr, "%sdump:  unknown option \"%c\" ignored\n", SGS, **flagv);
			break;
		}
	    }

	} else if (**flagv == '+') {
	    if (*++*flagv == 't' ) {
		flag[T].setting = ON;
		if (*++*flagv == '\0') {
		    if ((flagc > 1) && (isdigit(*flagv[1]))) {
			--flagc;
			tstop = atol(*++flagv);
		    }
		} else if (isdigit(**flagv)) {
		    tstop = atol(*flagv);
		} else {
		    fprintf(stderr, "usage:  %sdump [flags] [+t _n_u_m_b_e_r | +t_n_u_m_b_e_r] file ...\n", SGS);
		}
	    } else if (**flagv == 'd') {
		if (*++*flagv == '\0') {
		    if ((flagc > 1) && (isdigit(*flagv[1]))) {
			--flagc;
			dstop = atol(*++flagv);
		    } else {
			fprintf(stderr, "usage:  %sdump [flags] [+d _n_u_m_b_e_r | +d_n_u_m_b_e_r] file ...\n", SGS);
		    }
		} else if (isdigit(**flagv)) {
		    dstop = atol(*flagv);
		} else {
		    fprintf(stderr, "usage:  %sdump [flags] [+d_n_u_m_b_e_r | +d _n_u_m_b_e_r] file ...\n", SGS);
		}
	    } else if (**flagv == 'z') {
		if (*++*flagv == '\0') {
		    if ((flagc > 1) && (isdigit(*flagv[1]))) {
			--flagc;
			zstop = atoi(*++flagv);
		    } else {
			fprintf(stderr, "usage:  %sdump [flags] [+z _n_u_m_b_e_r | +z_n_u_m_b_e_r] file ...\n", SGS);
		    }
		} else if (isdigit(**flagv)) {
		    zstop = atoi(*flagv);
		} else {
		    fprintf(stderr, "usage:  %sdump [flags] [+z_n_u_m_b_e_r | +z _n_u_m_b_e_r] file ...\n", SGS);
		}
	    } else {
		fprintf(stderr, "%sdump:  unknown option \"+%s\" ignored\n", SGS, *flagv);
	    }
	} else {
	    *filev++ = *flagv;
	    ++filec;
	}
    }

    return(filec);
}

isnum(s)

char *s;
{
	while ( *s != NULL )
		if (! isdigit(*s) )
			return(0);
		else
			++s;

	return(1);
}

/*
*	@(#) setflags.c: 1.4 3/23/83
*/
