#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:cmd/lpstat/Makefile	1.5"
#
# Makefile for the lpstat command
#


RM	=	/bin/rm -f
AR	=	/bin/ar
INS	=	install
FUNCDCL	=


USRBIN	=	$(ROOT)/usr/bin

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
		accept.c \
		add_mounted.c \
		charset.c \
		class.c \
		device.c \
		done.c \
		form.c \
		lpstat.c \
		output.c \
		parse.c \
		printer.c \
		request.c \
		send_message.c

OBJS	=	$(SRCS:.c=.o)


all:		lpstat

install:	all
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) lpstat

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) lpstat

strip:
	$(STRIP) lpstat


do_printer.o:	lpstat.h do_printer.c

do_getpr.o:	lpstat.h do_getpr.c

dolist.o:	lpstat.h dolist.c

form.o:		lpstat.h form.c

lpstat.o:	lpstat.h lpstat.c

output.o:	lpstat.h output.c

parse.o:	lpstat.h parse.c

lpstat:	$(OBJS) \
			$(LIBACC) \
			$(LIBFRM) \
			$(LIBCLS) \
			$(LIBMSG) \
			$(LIBPRT) \
			$(LIBOAM) \
			$(LIBLP) \
			$(LIBFS)
	$(CC) -o lpstat $(OBJS) $(LDFLAGS) \
			$(LIBACC) \
			$(LIBFRM) \
			$(LIBCLS) \
			$(LIBMSG) \
			$(LIBPRT) \
			$(LIBOAM) \
			$(LIBLP) \
			$(LIBFS) \
				-lPW -lcurses


lint:
	lint $(CFLAGS) $(SRCS) \
			$(LINTACC) \
			$(LINTFRM) \
			$(LINTCLS) \
			$(LINTMSG) \
			$(LINTPRT) \
			$(LINTOAM) \
			$(LINTLP) \
			$(LINTFS)
