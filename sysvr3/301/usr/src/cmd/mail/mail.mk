#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:mail.mk	1.13"
#
#	mail make file
#
ROOT = 
MAILDIR = $(ROOT)/usr/mail
INSDIR = $(ROOT)/bin
CFLAGS = -O
LDFLAGS = -s $(LDLIBS)
INS=:

mail:	mail.c
	$(CC) $(CFLAGS) -o mail mail.c $(LDFLAGS)
	$(INS) $(INSDIR) -m 2755 -g mail -u bin mail
	if [ "$(INS)" != ":" ]; \
	then \
		rm -f $(INSDIR)/rmail; \
		ln $(INSDIR)/mail $(INSDIR)/rmail; \
	fi

all:	install clobber

ckdirs:
	@echo
	@echo mail requires the directories:
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
	chmod 775 $(MAILDIR)
	chgrp mail $(MAILDIR)
	chmod 775 $(MAILDIR)/:saved
	chgrp mail $(MAILDIR)/:saved

install: ckdirs
	$(MAKE) -f mail.mk INS="/etc/install -f"

clean:
	-rm -f mail.o rmail.o

clobber: clean
	rm -f mail rmail
