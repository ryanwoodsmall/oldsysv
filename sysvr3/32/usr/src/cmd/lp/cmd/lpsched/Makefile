#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:cmd/lpsched/Makefile	1.10"
#
# Makefile for the lpsched command
#


RM	=	/bin/rm -f
INS	=	install


USRLIB	=	$(ROOT)/usr/lib
USRSPOOL=	$(ROOT)/usr/spool

BINDIR	=	$(USRSPOOL)/lp/bin

OWNER	=	lp
SUPER	=	root
GROUP	=	bin

EMODES	=	0555
SMODES	=	04555

#
# Possible uses of TRACE_MALLOC and MALLOC_INC:
#
#	make TRACE_MALLOC=-DTRACE_MALLOC
#	make MALLOC_INC=-I/usr/foo/include
#	make TRACE_MALLOC=-DMDL MALLOC_INC=-I/usr/foo/include
#
# Leave USE_FIFOS alone unless you really want to try the IPC
# version. Note, though, that only the FIFO version completed
# system test.
#
# Define MALLOC_3X if you are using malloc(3X). Link it in
# by defining LDMALLOC=-lmalloc.
#
# Define LOST_LOCK if the mysterious problem of losing the lock
# on the SCHEDLOCK file occurs. It's been seen to happen when
# the system is under heavy load.
#
# Define CHECK_CHILDREN to cause the scheduler to look for children
# which have terminated without sending S_CHILD_DONE.  This may
# occur if the intermediate child core dumps or is killed by someone.
# It's also needed (and automatically defined if necessary) if LPSCHED_JR
# is defined, since Junior is vulnerable to dying from SIGTERM.
#
# Define LPSCHED_JR if you will be using the lpsched.jr progam to
# wait for children. This saves on process space by having a tiny
# process wait, instead of a forked lpsched. If you don't want
# this, also undefine LPSCHED_JR in $(LPINC)/lp.h.
#

CHECK_CHILDREN	= -DCHECK_CHILDREN
USE_FIFOS	= -DUSE_FIFOS
LOST_LOCK	= -DLOST_LOCK
MALLOC_3X	= -DMALLOC_3X -DDEF_MXFAST=32 -DDEF_NLBLKS=32 -DDEF_GRAIN=4
LDMALLOC	= -lmalloc
#TRACE_MALLOC	= -DTRACE_MALLOC
#LDDEBUG	= malloc/mymalloc.c
LPSCHED_JR	= lpsched.jr

DEBUG	=	-g -DDEBUG $(TRACE_MALLOC) $(MALLOC_INC)


INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	-I. -I$(LPINC) -I$(INC) \
			$(DEBUG) \
			$(CHECK_CHILDREN) \
			$(USE_FIFOS) \
			$(LOST_LOCK) \
			$(MALLOC_3X)


LIBDIR	=	../../lib

LIBACC	=	$(LIBDIR)/access/liblpacc.a
LIBCLS	=	$(LIBDIR)/class/liblpcls.a
LIBFLT	=	$(LIBDIR)/filters/liblpflt.a
LIBFRM	=	$(LIBDIR)/forms/liblpfrm.a
LIBFS	=	$(LIBDIR)/fs/liblpfs.a
LIBLP	=	$(LIBDIR)/lp/liblp.a
LIBMSG	=	$(LIBDIR)/msgs/liblpmsg.a
LIBOAM	=	$(LIBDIR)/oam/liblpoam.a
LIBPRT	=	$(LIBDIR)/printers/liblpprt.a
LIBREQ	=	$(LIBDIR)/requests/liblpreq.a
LIBUSR	=	$(LIBDIR)/users/liblpusr.a

PLIBACC	=	$(LIBDIR)/access/pliblpacc.a
PLIBCLS	=	$(LIBDIR)/class/pliblpcls.a
PLIBFLT	=	$(LIBDIR)/filters/pliblpflt.a
PLIBFRM	=	$(LIBDIR)/forms/pliblpfrm.a
PLIBFS	=	$(LIBDIR)/fs/pliblpfs.a
PLIBLP	=	$(LIBDIR)/lp/pliblp.a
PLIBMSG	=	$(LIBDIR)/msgs/pliblpmsg.a
PLIBOAM	=	$(LIBDIR)/oam/pliblpoam.a
PLIBPRT	=	$(LIBDIR)/printers/pliblpprt.a
PLIBREQ	=	$(LIBDIR)/requests/pliblpreq.a
PLIBUSR	=	$(LIBDIR)/users/pliblpusr.a

QLIBACC	=	$(LIBDIR)/access/qliblpacc.a
QLIBCLS	=	$(LIBDIR)/class/qliblpcls.a
QLIBFLT	=	$(LIBDIR)/filters/qliblpflt.a
QLIBFRM	=	$(LIBDIR)/forms/qliblpfrm.a
QLIBFS	=	$(LIBDIR)/fs/qliblpfs.a
QLIBLP	=	$(LIBDIR)/lp/qliblp.a
QLIBMSG	=	$(LIBDIR)/msgs/qliblpmsg.a
QLIBOAM	=	$(LIBDIR)/oam/qliblpoam.a
QLIBPRT	=	$(LIBDIR)/printers/qliblpprt.a
QLIBREQ	=	$(LIBDIR)/requests/qliblpreq.a
QLIBUSR	=	$(LIBDIR)/users/qliblpusr.a

LINTACC	=	$(LIBDIR)/access/llib-llpacc.ln
LINTCLS	=	$(LIBDIR)/class/llib-llpcls.ln
LINTFLT	=	$(LIBDIR)/filters/llib-llpflt.ln
LINTFRM	=	$(LIBDIR)/forms/llib-llpfrm.ln
LINTFS	=	$(LIBDIR)/fs/llib-llpfs.ln
LINTLP	=	$(LIBDIR)/lp/llib-llp.ln
LINTMSG	=	$(LIBDIR)/msgs/llib-llpmsg.ln
LINTOAM	=	$(LIBDIR)/oam/llib-llpoam.ln
LINTPRT	=	$(LIBDIR)/printers/llib-llpprt.ln
LINTREQ	=	$(LIBDIR)/requests/llib-llpreq.ln
LINTUSR	=	$(LIBDIR)/users/llib-llpusr.ln


SRCS	= \
		alerts.c \
		cancel.c \
		checkchild.c \
		daisyforms.c \
		dial.c \
		disena.c \
		disp1.c \
		disp2.c \
		disp3.c \
		disp4.c \
		disptab.c \
		dowait.c \
		exec.c \
		faults.c \
		files.c \
		flt.c \
		fncs.c \
		getkey.c \
		getpwent.c \
		init.c \
		log.c \
		logmsg.c \
		lpsched.c \
		msgs.c \
		notify.c \
		pickfilter.c \
		ports.c \
		requeue.c \
		rstatus.c \
		schedule.c \
		secure.c \
		status.c \
		terminate.c \
		validate.c

JR_SRCS	= \
		exec.c \
		msgs.c \
		log.c

OBJS	=	$(SRCS:.c=.o)
POBJS	=	$(SRCS:.c=.p)
QOBJS	=	$(SRCS:.c=.q)
JR_OBJS	=	$(JR_SRCS:.c=.j)

.SUFFIXES:	.o .p .q .j

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

.c.j:
	@-if [ -f $*.o ] ; then mv $*.o tmp ; fi
	$(CC) $(CFLAGS) -DI_AM_LPSCHED_JR -c $*.c
	@-mv $*.o $*.j
	@-if [ -f tmp ] ; then mv tmp $*.o ; fi


all:		lpsched $(LPSCHED_JR)

install:	all
	$(INS) -m $(SMODES) -u $(SUPER) -g $(GROUP) -f $(USRLIB) lpsched
	if [ -n "$(LPSCHED_JR)" ]; then $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(BINDIR) $(LPSCHED_JR); fi

clean:
	$(RM) $(OBJS) $(POBJS) $(QOBJS) msgs.c $(JR_OBJS)

clobber:	clean
	$(RM) lpsched plpsched qlpsched $(LPSCHED_JR)

strip:
	$(STRIP) lpsched $(LPSCHED_JR)


change:
	@-touch msgs.c flt.c disptab.c disp1.c

msgs.c:		msgs_ipc.c \
		msgs_fifo.c
	@echo '#if	defined(USE_FIFOS)' > msgs.c
	@cat msgs_fifo.c >> msgs.c
	@echo '#else	/* USE_FIFOS */' >> msgs.c
	@cat msgs_ipc.c >> msgs.c
	@echo '#endif	/* USE_FIFOS */' >> msgs.c

$(OBJS):	dispatch.h \
		errmsg.h \
		lpsched.h

lpsched:	$(OBJS) \
			$(LIBMSG) \
			$(LIBFRM) \
			$(LIBREQ) \
			$(LIBPRT) \
			$(LIBCLS) \
			$(LIBACC) \
			$(LIBFLT) \
			$(LIBUSR) \
			$(LIBLP) \
			$(LIBFS)
	$(CC) -o lpsched $(OBJS) $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBFRM) \
			$(LIBREQ) \
			$(LIBPRT) \
			$(LIBCLS) \
			$(LIBACC) \
			$(LIBFLT) \
			$(LIBUSR) \
			$(LIBOAM) \
			$(LIBLP) \
			$(LIBFS) \
				-lcurses \
					$(LDMALLOC) \
					$(LDDEBUG)

prof:		plpsched

plpsched:	$(POBJS) \
			$(PLIBMSG) \
			$(PLIBFRM) \
			$(PLIBREQ) \
			$(PLIBPRT) \
			$(PLIBCLS) \
			$(PLIBACC) \
			$(PLIBFLT) \
			$(PLIBUSR) \
			$(PLIBLP) \
			$(PLIBFS)
	$(CC) -p -o plpsched $(POBJS) $(LDFLAGS) \
			$(PLIBMSG) \
			$(PLIBFRM) \
			$(PLIBREQ) \
			$(PLIBPRT) \
			$(PLIBCLS) \
			$(PLIBACC) \
			$(PLIBFLT) \
			$(PLIBUSR) \
			$(PLIBOAM) \
			$(PLIBLP) \
			$(PLIBFS) \
				-lcurses \
					$(LDDEBUG)

lprof:		qlpsched

qlpsched:	$(QOBJS) \
			$(QLIBMSG) \
			$(QLIBFRM) \
			$(QLIBREQ) \
			$(QLIBPRT) \
			$(QLIBCLS) \
			$(QLIBACC) \
			$(QLIBFLT) \
			$(QLIBUSR) \
			$(QLIBLP) \
			$(QLIBFS)
	$(CC) -ql -o qlpsched $(QOBJS) $(LDFLAGS) \
			$(QLIBMSG) \
			$(QLIBFRM) \
			$(QLIBREQ) \
			$(QLIBPRT) \
			$(QLIBCLS) \
			$(QLIBACC) \
			$(QLIBFLT) \
			$(QLIBUSR) \
			$(QLIBOAM) \
			$(QLIBLP) \
			$(QLIBFS) \
				-lcurses \
					$(LDDEBUG)

lpsched.jr:	$(JR_OBJS) \
			$(LIBMSG) \
			$(LIBLP)
	$(CC) -o lpsched.jr $(JR_OBJS) $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBLP) \
				$(LDDEBUG)


lint:	msgs.c
	lint -ux $(CFLAGS) $(SRCS) \
			$(LINTMSG) \
			$(LINTFRM) \
			$(LINTREQ) \
			$(LINTPRT) \
			$(LINTCLS) \
			$(LINTACC) \
			$(LINTFLT) \
			$(LINTUSR) \
			$(LINTLP) \
			$(LINTFS)

$(OBJS):	nodes.h
