#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:cmd/lpadmin/Makefile	1.7"
#
# Makefile for the lpadmin command
#


RM	=	/bin/rm -f
AR	=	/bin/ar
INS	=	install
FUNCDCL	=


USRLIB	=	$(ROOT)/usr/lib

OWNER	=	lp
GROUP	=	bin

EMODES	=	0555


DEBUG	=	-g -DDEBUG

INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	$(DEBUG) -I. -I$(LPINC) -I$(INC) -O


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
		chkopts.c \
		default.c \
		do_align.c \
		do_fault.c \
		do_mount.c \
		do_printer.c \
		do_pwheel.c \
		done.c \
		fromclass.c \
		ismodel.c \
		lpadmin.c \
		options.c \
		output.c \
		rmdest.c \
		send_message.c \
		signals.c \
		startup.c \
		usage.c

OBJS	=	$(SRCS:.c=.o)


all:		lpadmin

install:	all
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRLIB) lpadmin

$(OBJS):	lpadmin.h

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) lpadmin

strip:
	$(STRIP) lpadmin


lpadmin:	$(OBJS) \
			$(LIBACC) \
			$(LIBCLS) \
			$(LIBMSG) \
			$(LIBPRT) \
			$(LIBFRM) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBLP) \
			$(LIBFS)
	$(CC) -o lpadmin $(OBJS) $(LDFLAGS) \
			$(LIBACC) \
			$(LIBCLS) \
			$(LIBMSG) \
			$(LIBPRT) \
			$(LIBFRM) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBLP) \
			$(LIBFS) \
				-lcurses

lint:
	lint $(CFLAGS) $(SRCS) \
			$(LINTACC) \
			$(LINTCLS) \
			$(LINTMSG) \
			$(LINTPRT) \
			$(LINTFRM) \
			$(LINTREQ) \
			$(LINTOAM) \
			$(LINTLP) \
			$(LINTFS)
