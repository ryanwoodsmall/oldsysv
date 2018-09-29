#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:model/Makefile	1.8"

#
# Makefile for lp/model
#


RM	=	/bin/rm -f
AR	=	/bin/ar
INS	=	install
FUNCDCL	=


USRBIN	=	$(ROOT)/usr/bin
USRLIB	=	$(ROOT)/usr/lib
USRSPOOL=	$(ROOT)/usr/spool

BINDIR	=	$(USRSPOOL)/lp/bin
MODELDIR=	$(USRSPOOL)/lp/model

OWNER	=	lp
GROUP	=	bin

EMODES	=	0555
SMODES	=	04555
FMODES	=	0444


DEBUG	=	-g

INC	=	$(ROOT)/usr/include
LPINC	=	../include

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) -O


LIBDIR	=	../lib

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
		lp.set.c \
		lp.cat.c \
		lp.page.c \
		lp.tell.c \
		drain.output.c

#
# Don't use $(SRCS:.c=.o) because it doesn't work on lp.cat.c
OBJS	= \
		lp.set.o \
		lp.cat.o \
		lp.page.o \
		lp.tell.o \
		drain.output.o

CMDS	= \
		lp.set \
		lp.cat \
		lp.page \
		lp.tell \
		drain.output

MODELS	= \
		standard \
		1640 \
		5310 \
		dqp10 \
		dumb \
		f450 \
		hp \
		lqp40 \
		ph.daps \
		pprx \
		prx

MISC	= \
		alert.proto


all:		$(CMDS)

install:	all
	for cmd in $(CMDS); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(BINDIR) $$cmd; done
	for model in $(MODELS); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(MODELDIR) $$model; done
	for misc in $(MISC); do $(INS) -m $(FMODES) -u $(OWNER) -g $(GROUP) -f $(BINDIR) $$misc; done

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMDS)

strip:
	$(STRIP) $(CMDS)


lp.set:		lp.set.o \
			$(LIBLP)
	$(CC) -o lp.set lp.set.o $(LDFLAGS) \
			$(LIBLP) \
			-lcurses

lp.cat:		lp.cat.o \
			$(LIBLP)
	$(CC) -o lp.cat lp.cat.o $(LDFLAGS) \
			$(LIBLP)

lp.page:	lp.page.o
	$(CC) -o lp.page lp.page.o $(LDFLAGS)

lp.tell:	lp.tell.o \
			$(LIBMSG) \
			$(LIBLP)
	$(CC) -o lp.tell lp.tell.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBLP)

drain.output:	drain.output.o \
			$(LIBLP)
	$(CC) -o drain.output drain.output.o $(LDFLAGS) \
			$(LIBLP)

lint:
	lint $(CFLAGS) lp.set.c \
			$(LINTLP)
	lint $(CFLAGS) lp.cat.c
	lint $(CFLAGS) lp.page.c
	lint $(CFLAGS) lp.tell.c \
			$(LINTLP) \
			$(LINTMSG)
	lint $(CFLAGS) drain.output.c
