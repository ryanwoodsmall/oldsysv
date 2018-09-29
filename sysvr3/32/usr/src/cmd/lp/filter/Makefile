#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:filter/Makefile	1.3"
#
# Makefile for lp/filter
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

OWNER	=	bin
GROUP	=	bin

EMODES	=	0555
SMODES	=	04555


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
		5310.c \
		hp2631a.c \
		pprx.c \
		prx.c

OBJS	=	$(SRCS:.c=.o)

CMDS	= \
		5310 \
		hp2631a \
		pprx \
		prx

MISC	= \
		slow.filter


all:		$(CMDS) $(MISC)

install:	all
	for cmd in $(CMDS); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRLIB) $$cmd; done
	for misc in $(MISC); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(BINDIR) $$misc; done

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMDS)

strip:
	$(STRIP) $(CMDS)


5310:		5310.o
	$(CC) -o 5310 5310.o $(LDFLAGS)

hp2631a:	hp2631a.o
	$(CC) -o hp2631a hp2631a.o $(LDFLAGS)

prx:		prx.o
	$(CC) -o prx prx.o $(LDFLAGS)

pprx:		pprx.o
	$(CC) -o pprx pprx.o $(LDFLAGS)


lint:
	lint $(CFLAGS) 5310.c
	lint $(CFLAGS) hp2631a.c
	lint $(CFLAGS) pprx.c
	lint $(CFLAGS) prx.c
