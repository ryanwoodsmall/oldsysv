/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:xtd.c	2.4"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	Print out Xt Link data structure
*/

char		Usage[]	=	"Usage: %s [-01234567f]\n";

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/tty.h>
#include	<sys/jioctl.h>
#include	<sys/xtproto.h>
#include	<sys/xt.h>
#include	"stdio.h"
#include	"errno.h"

char *		name;
char		display;
char		inhibitchans;
char		notchan[MAXPCHAN];

#if	XTDATA == 1

#define	INDENT		2		/* sizeof relative indent */

struct
{
	struct Linkinfo	l;
	struct Channel	chans[MAXPCHAN];
}
		Data;

typedef struct print_list
{
	char *		desc;
	char *		format;
	short		type;
	short		size;
}
		Print_list;

#define	CHAR		0
#define	UCHAR		1
#define	SHORT		2
#define	INT		3
#define	LONG		3
#define	POINTER		4
#define	STRUCT		5
#define	PKTS		6
#define	CHARARRAY	7

char		nullstr[]	= "";
char		inthex[]	= "%#x";
char		intdec[]	= "%d";
char		intoct[]	= "%#o";

#define		pointer	inthex
#define		charoct	intoct
#define		chardec	intdec

Print_list	null_list[1];

#define		NULL_LIST	(char *)null_list

Print_list	clist_list[]	=
{
	 {"c_cc",	intdec,		INT,		sizeof(int)		}
	,{"c_cf",	pointer,	POINTER,	sizeof(struct cblock *)	}
	,{"c_cl",	pointer,	POINTER,	sizeof(struct cblock *)	}
	,{0}
};

Print_list	ccblock_list[]	=
{
	 {"c_ptr",	pointer,	POINTER,	sizeof(caddr_t)		}
	,{"c_count",	intdec,		SHORT,		sizeof(short)		}
	,{"c_size",	intdec,		SHORT,		sizeof(short)		}
	,{0}
};

Print_list	pks_list[]	=
{
	 {"Packet",	NULL_LIST,	STRUCT,		0			}
	,{"header",	charoct,	UCHAR,		sizeof(char)		}
	,{"dsize",	chardec,	UCHAR,		sizeof(char)		}
	,{"data",	charoct,	CHARARRAY,	MAXPKTDSIZE+EDSIZE	}
	,{"timo",	intdec,		SHORT,		sizeof(short)		}
	,{"state",	charoct,	UCHAR,		sizeof(char)		}
	,{"size",	chardec,	UCHAR,		sizeof(char)		}
	,{0}
};

Print_list	l_list[]	=
{
	 {"Link data structure", NULL_LIST, STRUCT,	0			}
	,{"line",	pointer,	POINTER,	sizeof(struct tty *)	}
	,{"rdatap",	pointer,	POINTER,	sizeof(char *)		}
	,{"rpkt",	(char *)pks_list, STRUCT,	sizeof(struct Pktstate)	}
	,{"lihiwat",	intdec,		SHORT,		sizeof(short)		}
	,{"xtimo",	intdec,		SHORT,		sizeof(short)		}
	,{"rtimo",	intdec,		CHAR,		sizeof(char)		}
	,{"old",	intdec,		CHAR,		sizeof(char)		}
	,{"nchans",	intdec,		CHAR,		sizeof(char)		}
	,{"lchan",	intdec,		CHAR,		sizeof(char)		}
	,{"open",	intoct,		UCHAR,		sizeof(char)		}
	,{"xopen",	intoct,		UCHAR,		sizeof(char)		}
#	if	XTRACE == 1
	,{"(tracing not shown)", NULL_LIST, STRUCT,	sizeof(struct Tbuf)	}
#	endif
#	if	XTSTATS == 1
	,{"(statistics not shown)", NULL_LIST, STRUCT,	sizeof(Stats_t)*S_NSTATS}
#	endif
	,{0}
};

Print_list	Pc_list[]	=
{
	 {"Protocol data", NULL_LIST,	STRUCT,		0			}
	,{"pkts",	nullstr,	PKTS,	sizeof(struct Pktstate)*NPCBUFS	}
	,{"nextpkt",	pointer,	POINTER,	sizeof(Pks_p)		}
	,{"cdata",	charoct,	CHARARRAY,	sizeof(char)*SEQMOD	}
	,{"seq-x/r",	charoct,	UCHAR,		sizeof(char)		}
	,{"outen",	chardec,	CHAR,		sizeof(char)		}
	,{"flags",	charoct,	UCHAR,		sizeof(char)		}
	,{"channo",	chardec,	CHAR,		sizeof(char)		}
	,{"link",	chardec,	CHAR,		sizeof(char)		}
	,{0}
};

Print_list	tty_list[]	=
{
	 {"Tty structure", NULL_LIST,	STRUCT,		0			}
	,{"t_rawq",	(char *)clist_list, STRUCT,	sizeof(struct clist)	}
	,{"t_canq",	(char *)clist_list, STRUCT,	sizeof(struct clist)	}
	,{"t_outq",	(char *)clist_list, STRUCT,	sizeof(struct clist)	}
	,{"t_tbuf",	(char *)ccblock_list, STRUCT,	sizeof(struct ccblock)	}
	,{"t_rbuf",	(char *)ccblock_list, STRUCT,	sizeof(struct ccblock)	}
	,{"t_proc",	pointer,	POINTER,	sizeof(int (*)())	}
	,{"t_iflag",	intoct,		SHORT,		sizeof(short)		}
	,{"t_oflag",	intoct,		SHORT,		sizeof(short)		}
	,{"t_cflag",	intoct,		SHORT,		sizeof(short)		}
	,{"t_lflag",	intoct,		SHORT,		sizeof(short)		}
	,{"t_state",	intoct,		SHORT,		sizeof(short)		}
	,{"t_pgrp",	intdec,		SHORT,		sizeof(short)		}
	,{"t_line",	intdec,		CHAR,		sizeof(char)		}
	,{"t_delct",	intdec,		CHAR,		sizeof(char)		}
	,{"t_term",	intdec,		CHAR,		sizeof(char)		}
	,{"t_tmflag",	intoct,		CHAR,		sizeof(char)		}
	,{"t_col",	intdec,		CHAR,		sizeof(char)		}
	,{"t_row",	intdec,		CHAR,		sizeof(char)		}
	,{"t_vrow",	intdec,		CHAR,		sizeof(char)		}
	,{"t_lrow",	intdec,		CHAR,		sizeof(char)		}
	,{"t_hqcnt",	intdec,		CHAR,		sizeof(char)		}
	,{"t_dstat",	intdec,		CHAR,		sizeof(char)		}
	,{"t_cc",	charoct,	CHARARRAY,	sizeof(char)*NCC	}
	,{0}
};

static void	print_list();
#endif
static int	xtdata();
static void	error();

extern char	_sobuf[];


#define		Fprintf		(void)fprintf
#define		SYSERROR	(-1)



int
main(argc, argv)
	int		argc;
	register char *	argv[];
{
	register int	c, i;

	name = *argv++;

	while ( --argc )
	{
		if ( argv[0][0] == '-' )
			while ( c = *++argv[0] )
				switch ( c )
				{
				 case '0': case '1': case '2': case '3':
				 case '4': case '5': case '6': case '7':
						if ( !inhibitchans )
						{
							inhibitchans = 1;
							for ( i = 0 ; i < MAXPCHAN ; i++ )
								notchan[i] = 1;
						}
						notchan[c-'0'] = 0;
						break;
				 case 'f':	display++;
						break;
				 default:	error("bad flag '%c'", (char *)c);
						exit(1);
				}
		else
		{
			error("unrecognised argument '%s'", argv[0]);
			exit(1);
		}
		argv++;
	}

	setbuf(stdout, _sobuf);

	if ( xtdata(0, stdout) ) {
		error("xt ioctl returned errno '%d'", errno);
		exit(1);
	}

	if ( display )
		Fprintf(stdout, "\f");

	exit(0);
}



static void
error(s1, s2)
	char *	s1;
	char *	s2;
{
	Fprintf(stderr, "%s - error - ", name);
	Fprintf(stderr, s1, s2);
	Fprintf(stderr, "\n");
}



static int
xtdata(cfd, ofd)
	int		cfd;
	register FILE *	ofd;
{
#	if	XTDATA == 1
	register int	i;

	if ( ioctl(cfd, XTIOCDATA, &Data) == SYSERROR )
		return(1);

	print_list(ofd, l_list, (char *)&Data, 0);

	for ( i = 0 ; i < Data.l.nchans ; i++ )
		if ( !notchan[i] )
		{
			Fprintf(ofd, "\nChannel %d", i);
			print_list(ofd, Pc_list, (char *)&Data.chans[i].chan, INDENT);
			print_list(ofd, tty_list, (char *)&Data.chans[i].tty, INDENT);
		}

	Fprintf(ofd, "\n");
#endif
	return(0);
}



#if	XTDATA == 1
static void
print_list(ofd, lp, dp, indent)
	register FILE *		ofd;
	register Print_list *	lp;
	register char *		dp;
	int			indent;
{
	register int		i;

	for ( ; lp->desc != (char *)0 ; lp++ )
	{
		Fprintf(ofd, "\n%*s%-10s ", indent, nullstr, lp->desc);

		switch ( lp->type )
		{
		 case CHAR:	Fprintf(ofd, lp->format, *dp);
				break;
		 case UCHAR:	Fprintf(ofd, lp->format, *(unsigned char *)dp);
				break;
		 case SHORT:	Fprintf(ofd, lp->format, *(short *)dp);
				break;
		 case LONG:	Fprintf(ofd, lp->format, *(long *)dp);
				break;
		 case POINTER:	Fprintf(ofd, lp->format, *(char **)dp);
				break;
		 case STRUCT:	print_list(ofd, (Print_list *)lp->format, dp, indent+INDENT);
				break;
		 case PKTS:	for ( i = 0 ; i < (lp->size/sizeof(struct Pktstate)) ; i++ )
				{
					print_list(ofd, pks_list, dp, indent+INDENT);
					dp += sizeof(struct Pktstate);
				}
				continue;
		 case CHARARRAY:for ( i = 0 ; i < lp->size ; i++, dp++ )
				{
					if ( (i % 4) == 0 )
						Fprintf(ofd, "\n%*s", indent, nullstr);
					Fprintf(ofd, "<%o>", *(unsigned char *)dp);
				}
				continue;
		 default:	Fprintf(ofd, "** unrecognised type '%d' **", lp->type);

		}

		dp += lp->size;
	}
}
#endif
