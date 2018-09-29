#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lib/msgs/Makefile	1.6"
#
# Makefile for messagemgmt(lp) routines
#


RM	=	/bin/rm -f
AR	=	ar
INS	=	install

USE_FIFOS=-DUSE_FIFOS

DEBUG	=	-g

INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) -O $(USE_FIFOS)


LIBRARY	=	liblpmsg.a
PLIBRARY=	p$(LIBRARY)
QLIBRARY=	q$(LIBRARY)

LINTSRC	=	llib-llpmsg
LINTLIB	=	$(LINTSRC).ln


SRCS	= \
		_getmessage.c \
		_putmessage.c \
		getmessage.c \
		hslconv.c \
		putmessage.c \
		msgfmts.c \
		mclose.c \
		mneeds.c \
		mopen.c \
		mrecv.c \
		msend.c \
		read_fifo.c \
		write_fifo.c

LINTS	= \
		../lp/llib-llp.ln

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
	$(RM) $(OBJS) $(POBJS) $(QOBJS) mopen.c mclose.c mrecv.c msend.c

clobber:	clean
	$(RM) $(LIBRARY) $(PLIBRARY) $(QLIBRARY) $(LINTLIB)

strip:
	$(STRIP) $(LIBRARY)


$(OBJS):	$(LPINC)/msgs.h

$(LIBRARY):	$(OBJS)
	$(AR) rv $(LIBRARY) $?

prof:		$(PLIBRARY)

$(PLIBRARY):	$(POBJS)
	$(AR) rv $(PLIBRARY) $?

lprof:		$(QLIBRARY)

$(QLIBRARY):	$(QOBJS)
	$(AR) rv $(QLIBRARY) $?

change:
	@-touch mopen.c mclose.c mrecv.c msend.c msgfmts.c

concat:
	@( \
		echo '#if	defined(USE_FIFOS)'; \
		cat $(COMPONENT)_fifo.c; \
		echo '#else	/* USE_FIFOS */'; \
		cat $(COMPONENT)_ipc.c; \
		echo '#endif	/* USE_FIFOS */'; \
	) >$(COMPONENT).c

mopen.c:	mopen_fifo.c mopen_ipc.c
	$(MAKE) COMPONENT=$(@:.c=) concat

mclose.c:	mclose_fifo.c mclose_ipc.c
	$(MAKE) COMPONENT=$(@:.c=) concat

mrecv.c:	mrecv_fifo.c mrecv_ipc.c
	$(MAKE) COMPONENT=$(@:.c=) concat

msend.c:	msend_fifo.c msend_ipc.c
	$(MAKE) COMPONENT=$(@:.c=) concat


lint:
	lint -ux $(CFLAGS) $(SRCS) $(LINTS)

lintsrc:	$(LINTSRC)

$(LINTSRC):	$(SRCS)
	if [ -n "$(FUNCDCL)" ]; then $(FUNCDCL) $(SRCS) >$(LINTSRC); fi

lintlib:	$(LINTLIB)

$(LINTLIB):	$(LINTSRC)
	$(CC) -E -C $(CFLAGS) -Dlint $(LINTSRC) | /usr/lib/lint1 -vx -H./LINT_WORK > $(LINTLIB)
	$(RM) ./LINT_WORK
