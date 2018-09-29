#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:cmd/Makefile	1.12"
#
# Makefile for the LP commands
#


RM	=	/bin/rm -f
CP	=	/bin/cp
LN	=	/bin/ln
INS	=	install


USRBIN	=	$(ROOT)/usr/bin
USRLIB	=	$(ROOT)/usr/lib

OWNER	=	lp
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
		cancel.c \
		comb.c \
		lp.c \
		lpfilter.c \
		lpforms.c \
		lpmove.c \
		lpshut.c \
		lpusers.c

OBJS	=	$(SRCS:.c=.o)


LILCMDS	= \
		accept \
		cancel \
		disable \
		enable \
		lp \
		lpfilter \
		lpforms \
		lpmove \
		lpshut \
		lpusers \
		reject

BIGCMDS	= \
		lpadmin \
		lpsched \
		lpstat

CMDS	=	$(BIGCMDS) $(LILCMDS)


USRCMDS1= \
		cancel \
		lp

USRCMDS2= \
		disable \
		enable

SYSCMDS	= \
		accept \
		lpfilter \
		lpforms \
		lpmove \
		lpshut \
		lpusers \
		reject


all:		$(CMDS) \
		dumpolp

install:	all
	for cmd in $(USRCMDS1); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) $$cmd; done
	for cmd in $(USRCMDS2); do $(INS) -m $(SMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) $$cmd; done
	for cmd in $(SYSCMDS); do $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRLIB) $$cmd; done
	for cmd in $(BIGCMDS); do cd ./$$cmd; $(MAKE) install; cd ..; done
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRLIB) dumpolp

clean:
	$(RM) $(OBJS)
	for cmd in $(BIGCMDS); do cd ./$$cmd; $(MAKE) clean; cd ..; done
	$(RM) dumpolp.o

clobber:
	$(RM) $(OBJS)
	$(RM) $(LILCMDS)
	for cmd in $(BIGCMDS); do cd ./$$cmd; $(MAKE) clobber; cd ..; done
	$(RM) dumpolp.o
	$(RM) dumpolp

strip:
	$(STRIP) $(LILCMDS)
	for cmd in $(BIGCMDS); do cd ./$$cmd; $(MAKE) STRIP=$(STRIP) strip; cd ..; done
	$(STRIP) dumpolp


accept:		comb.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o accept comb.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

disable:	accept
	$(CP) accept disable

enable:		disable
	$(LN) disable enable

reject:		accept
	$(LN) accept reject

cancel:		cancel.o \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o cancel cancel.o $(LDFLAGS) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

lp:		lp.o \
			$(LIBPRT) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lp lp.o $(LDFLAGS) \
			$(LIBPRT) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

lpadmin:	lpadmin_b

lpadmin_b:
	cd ./lpadmin; $(MAKE)

lpfilter:	lpfilter.o \
			$(LIBFLT) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpfilter lpfilter.o $(LDFLAGS) \
			$(LIBFLT) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

lpforms:	lpforms.o \
			$(LIBFRM) \
			$(LIBMSG) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBACC) \
			$(LIBLP) \
			$(LIBFS)
	$(CC) -o lpforms lpforms.o $(LDFLAGS) \
			$(LIBFRM) \
			$(LIBMSG) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBACC) \
			$(LIBLP) \
			$(LIBFS)

lpmove:		lpmove.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpmove lpmove.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

lpsched:	lpsched_b

lpsched_b:
	cd ./lpsched; $(MAKE)

lpshut:		lpshut.o \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpshut lpshut.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP) \
		$(LIBFS)

lpstat:		lpstat_b

lpstat_b:
	cd ./lpstat; $(MAKE)

lpusers:	lpusers.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBUSR) \
			$(LIBLP) \
			$(LIBFS)
	$(CC) -o lpusers lpusers.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBUSR) \
			$(LIBLP) \
			$(LIBFS)

dumpolp:	dumpolp.o \
			$(LIBLP)
	$(CC) -o dumpolp dumpolp.o $(LDFLAGS) \
			$(LIBLP)


prof:
	cd lpsched; $(MAKE) prof

lprof:
	cd lpsched; $(MAKE) lprof


lint:
	lint $(CFLAGS) comb.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) cancel.c \
			$(LINTREQ) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) lp.c \
			$(LINTREQ) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) lpfilter.c \
			$(LINTFLT) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) lpforms.c \
			$(LINTFRM) \
			$(LINTMSG) \
			$(LINTACC) \
			$(LINTFS) \
			$(LINTLP)
	lint $(CFLAGS) lpmove.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) lpshut.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	lint $(CFLAGS) lpusers.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTUSR) \
			$(LINTLP)
	for cmd in $(BIGCMDS); do cd ./$$cmd; $(MAKE) lint; cd ..; done

now:
	lint $(CFLAGS) lpforms.c \
			$(LINTFRM) \
			$(LINTMSG) \
			$(LINTACC) \
			$(LINTFS) \
			$(LINTLP)
