#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lib/lp/Makefile	1.9"
#
# Makefile for LP support routines
#


RM	=	/bin/rm -f
AR	=	ar
INS	=	install


DEBUG	=	-g

##########
#
# Define SPOOLDEF if you want the spooling directory to be
# other than the standard place.
# Define SPOOLDIR_FROM_ENV if you want the software to determine
# the spooling directory from the SPOOLDIR environment variable.
##########
# SPOOLDEF		=	-DSPOOLDIR=\"/usr/spool/foo\"
SPOOLDIR_FROM_ENV	=	-DSPOOLDIR_FROM_ENV

INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) $(SPOOLDEF) $(SPOOLDIR_FROM_ENV) -O


LIBRARY	=	liblp.a
PLIBRARY=	p$(LIBRARY)
QLIBRARY=	q$(LIBRARY)

LINTSRC	=	llib-llp
LINTLIB	=	$(LINTSRC).ln


SRCS	= \
		Syscalls.c \
		addlist.c \
		appendlist.c \
		alerts.c \
		charset.c \
		cs_strcmp.c \
		cs_strncmp.c \
		curdir.c \
		dellist.c \
		dashos.c \
		dirs.c \
		duplist.c \
		files.c \
		freelist.c \
		getlist.c \
		getname.c \
		getpaths.c \
		getspooldir.c \
		isterminfo.c \
		lenlist.c \
		lp_errno.c\
		makepath.c \
		makestr.c \
		mergelist.c \
		printlist.c \
		sdn.c \
		sprintlist.c \
		searchlist.c \
		set_charset.c \
		set_pitch.c \
		set_size.c \
		sop.c \
		strip.c \
		syntax.c \
		tidbit.c \
		tinames.c \
		wherelist.c \
		which.c

LINTS	= \
		../fs/llib-llpfs.ln

OBJS	=	$(SRCS:.c=.o)
POBJS	=	$(SRCS:.c=.p)
QOBJS	=	$(SRCS:.c=.q)

.SUFFIXES:	.o .p .q

.c.p:
	@-if [ -f $*.o ] ; then mv $*.o tmp ; fi
	$(CC) -p $(CFLAGS) -c $*.c
	@-mv $*.o $*.p
	@-if [ -f tmp ] ; then mv tmp $*.o ; fi

.c.q:
	@-if [ -f $*.o ] ; then mv $*.o tmp ; fi
	$(CC) -ql $(CFLAGS) -c $*.c
	@-mv $*.o $*.q
	@-if [ -f tmp ] ; then mv tmp $*.o ; fi


all:		$(LIBRARY) $(LINTLIB)

install:	all
	@echo "Not a delivered part of LP; not installed"
	@false

clean:	
	$(RM) $(OBJS) $(POBJS) $(QOBJS)

clobber:	clean
	$(RM) $(LIBRARY) $(PLIBRARY) $(QLIBRARY) $(LINTLIB)

strip:
	$(STRIP) $(LIBRARY)

$(LIBRARY):	$(OBJS)
	$(AR) rv $(LIBRARY) $?

prof:		$(PLIBRARY)

$(PLIBRARY):	$(POBJS)
	$(AR) rv $(PLIBRARY) $?

lprof:		$(QLIBRARY)

$(QLIBRARY):	$(QOBJS)
	$(AR) rv $(QLIBRARY) $?

lint:
	lint -ux $(CFLAGS) $(SRCS) $(LINTS)

lintsrc:	$(LINTSRC)

$(LINTSRC):	$(SRCS)
	if [ -n "$(FUNCDCL)" ]; then $(FUNCDCL) $(SRCS) >$(LINTSRC); fi

lintlib:	$(LINTLIB)

$(LINTLIB):	$(LINTSRC)
	$(CC) -E -C $(CFLAGS) -Dlint $(LINTSRC) | /usr/lib/lint1 -vx -H./LINT_WORK > $(LINTLIB)
	$(RM) ./LINT_WORK
