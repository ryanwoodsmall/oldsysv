#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mailx:mailx.mk	1.14"
#
# mailx -- a modified version of a University of California at Berkeley
#	mail program
#
# for standard Unix
#

ROOT=
VERSION=3.0
HDR=	hdr
HELP=	help
DESTDIR= $(ROOT)/usr/bin
DESTLIB= $(ROOT)/usr/lib/mailx

DEFINES= -DUSG -DUSG_TTY
CFLAGS=	-O -I$(HDR) $(DEFINES) 
LDFLAGS= -n -i -s
LIBES=
MAILDIR = $(ROOT)/usr/mail


HOSTCC=	/bin/cc
INS=install

OBJS=	version.o \
	aux.o \
	cmd1.o \
	cmd2.o \
	cmd3.o \
	cmd4.o \
	cmdtab.o \
	collect.o \
	config.o \
	edit.o \
	fio.o \
	getname.o \
	hostname.o \
	head.o \
	usg.local.o \
	lock.o \
	lex.o \
	list.o \
	main.o \
	names.o \
	optim.o \
	popen.o \
	quit.o \
	send.o \
	stralloc.o \
	temp.o \
	tty.o \
	vars.o \
	sigretro.o \
	lpaths.o \
	extern.o 

SRCS=	aux.c \
	cmd1.c \
	cmd2.c \
	cmd3.c \
	cmd4.c \
	cmdtab.c \
	collect.c \
	config.c \
	edit.c \
	fio.c \
	getname.c \
	head.c \
	hostname.c \
	usg.local.c \
	lock.c \
	lex.c \
	list.c \
	main.c \
	names.c \
	optim.c \
	popen.c \
	quit.c \
	send.c \
	stralloc.c \
	temp.c \
	tty.c \
	vars.c \
	sigretro.c \
	lpaths.c \
	extern.c

HDRS=	$(HDR)/rcv.h \
	$(HDR)/configdefs.h \
	$(HDR)/def.h \
	$(HDR)/glob.h \
	$(HDR)/usg.local.h \
	$(HDR)/local.h \
	$(HDR)/uparm.h \
	$(HDR)/sigretro.h

S=	$(SRCS) version.c $(HDRS)

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all:	mailx rmmail

mailx:	$S $(OBJS)
	-rm -f mailx
	$(CC) $(LDFLAGS) -o mailx $(OBJS) $(LIBES) $(LDLIBS)

rmmail:
	$(CC) $(CFLAGS) $(LDFLAGS) -o rmmail rmmail.c $(LDLIBS)

install: ckdirs all
	$(INS) -n $(DESTDIR) mailx
	if [ ! -d $(DESTLIB) ] ; then mkdir $(DESTLIB) ; fi
	$(INS) -f $(DESTLIB) rmmail
	$(CH)-chgrp mail $(DESTDIR)/mailx
	$(CH)-chmod 2755 $(DESTDIR)/mailx
	$(CH)-chgrp mail $(DESTLIB)/rmmail
	$(CH)-chmod 2755 $(DESTLIB)/rmmail
	$(INS) -f $(DESTLIB) $(HELP)/mailx.help
	$(INS) -f $(DESTLIB) $(HELP)/mailx.help.~
	$(CH)-chmod 0664 $(DESTLIB)/mailx.help*

version.o:	/tmp version.c
	$(CC) -c version.c
	-rm -f version.c

version.c:
	echo \
	"char *version=\"mailx version $(VERSION)\";"\
		> version.c

clean:
	-rm -f *.o
	-rm -f version.c a.out core 

clobber:	clean
	-rm -f mailx rmmail

lint:	version.c
	lint $(CFLAGS) $(SRCS)
	-rm -f version.c

cmdtab.o:	cmdtab.c
	$(CC) -c $(CFLAGS) cmdtab.c

mailx.cpio:	$(SRCS) $(HDRS) mailx.mk 
	@echo $(SRCS) $(HDRS) mailx.mk | \
		tr " " "\012" | \
		cpio -oc >mailx.cpio

listing:
	pr mailx.mk hdr/*.h [a-l]*.c | lp
	pr [m-z]*.c | lp

ckdirs:
	@echo
	@echo mailx requires the directories:
	@echo "\t$(MAILDIR)"
	@echo "\t$(MAILDIR)/:saved"
	@echo
	@echo Checking for existence of directories
	@echo
	@if [ -d $(MAILDIR) ]; \
	then \
		echo "\t$(MAILDIR) exists";\
		echo; \
	else \
		echo "\t$(MAILDIR) does not exist";\
		echo "\tCreating $(MAILDIR)"; \
		mkdir $(MAILDIR); \
		echo; \
	fi
	@if [ -d $(MAILDIR)/:saved ]; \
	then \
		echo "\t$(MAILDIR)/:saved exists";\
		echo; \
	else \
		echo "\t$(MAILDIR)/:saved does not exist";\
		echo "\tCreating $(MAILDIR)/:saved"; \
		mkdir $(MAILDIR)/:saved; \
		echo; \
	fi
	chmod 775 $(MAILDIR);
	chgrp mail $(MAILDIR);
	chmod 775 $(MAILDIR)/:saved; 
	chgrp  mail $(MAILDIR)/:saved;
