#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident "@(#)lp:lp.mk	1.11"
# Makefile for lp line printer spooler system

OL	= /
BINCOMP  = cancel disable enable lp lpstat
LPCOMP   = accept lpmove lpshut reject
ROOTCOMP = lpadmin lpsched
SPOOL	= $(OL)usr/spool/lp
ADMDIR	= $(OL)usr/lib
USRDIR	= $(OL)usr/bin
LIB	= lib.a
CFLAGS	= -O
LDFLAGS	= -s -n
COMPILE	= $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIB)

all:	$(BINCOMP) $(LPCOMP) $(ROOTCOMP)
	-mkdir $(SPOOL) $(SPOOL)/model
	for i in model/*;\
		do \
			cpset $$i $(ROOT)/$(SPOOL)/model 644 lp ; \
		done
	touch pstatus; cpset pstatus $(SPOOL) 644 lp
	touch qstatus; cpset qstatus $(SPOOL) 644 lp

	-cd $(SPOOL) ; \
		mkdir class interface member request ; \
		chmod 755 . class interface member model request

	$(CH) cd $(SPOOL) ; chown lp . * ; chgrp bin . *


.c.a:

.PRECIOUS: $(LIB)

accept:	accept.c lp.h $(LIB)
	$(COMPILE)
cancel:	cancel.c lp.h $(LIB)
	$(COMPILE)
disable:	disable.c lp.h $(LIB)
	$(COMPILE)
enable:	enable.c lp.h $(LIB)
	$(COMPILE)
lp:	lp.c lp.h $(LIB)
	$(COMPILE)
lpadmin:	lpadmin.c lp.h $(LIB)
	$(COMPILE)
lpmove:	lpmove.c lp.h $(LIB)
	$(COMPILE)
lpsched:	lpsched.c lpsched.h lp.h $(LIB)
	$(COMPILE)
lpshut:	lpshut.c lp.h $(LIB)
	$(COMPILE)
lpstat:	lpstat.c lpsched.h lp.h $(LIB)
	$(COMPILE)
reject:	reject.c lp.h $(LIB)
	$(COMPILE)
$(LIB): \
	$(LIB)(dest.o) \
	$(LIB)(destlist.o) \
	$(LIB)(fullpath.o) \
	$(LIB)(gwd.o) \
	$(LIB)(enter.o) \
	$(LIB)(fifo.o) \
	$(LIB)(getname.o) \
	$(LIB)(isclass.o) \
	$(LIB)(isprinter.o) \
	$(LIB)(isrequest.o) \
	$(LIB)(outlist.o) \
	$(LIB)(outputq.o) \
	$(LIB)(pstatus.o) \
	$(LIB)(date.o) \
	$(LIB)(isdest.o) \
	$(LIB)(eaccess.o) \
	$(LIB)(qstatus.o) \
	$(LIB)(fatal.o) \
	$(LIB)(lock.o) \
	$(LIB)(request.o) \
	$(LIB)(sendmail.o) \
	$(LIB)(trim.o) \
	$(LIB)(wrtmsg.o) \
	$(LIB)(findtty.o)
	$(CC) -c $(CFLAGS) $(?:.o=.c)
	ar r $(LIB) $?
	rm -f $?
	chmod 664 $@

$(LIB)(date.o): date.c lp.h
$(LIB)(dest.o): dest.c lpsched.h lp.h
$(LIB)(destlist.o): destlist.c lpsched.h
$(LIB)(eaccess.o): eaccess.c lp.h
$(LIB)(enter.o): enter.c lp.h
$(LIB)(fatal.o): fatal.c lp.h
$(LIB)(fifo.o): fifo.c lp.h
$(LIB)(findtty.o): findtty.c lp.h lpsched.h
$(LIB)(fullpath.o): fullpath.c lp.h
$(LIB)(getname.o): getname.c lp.h
$(LIB)(gwd.o): gwd.c lp.h
$(LIB)(isclass.o): isclass.c lp.h
$(LIB)(isdest.o): isdest.c lp.h
$(LIB)(isprinter.o): isprinter.c lp.h
$(LIB)(isrequest.o): isrequest.c lp.h
$(LIB)(lock.o): lock.c lp.h
$(LIB)(outlist.o): outlist.c lp.h lpsched.h
$(LIB)(outputq.o): outputq.c lp.h
$(LIB)(pstatus.o): pstatus.c lp.h
$(LIB)(qstatus.o): qstatus.c lp.h
$(LIB)(request.o): request.c lp.h
$(LIB)(sendmail.o): sendmail.c lp.h
$(LIB)(trim.o): trim.c lp.h
$(LIB)(wrtmsg.o): wrtmsg.c lp.h lpsched.h

install: $(BINCOMP) $(LPCOMP) $(ROOTCOMP)
	for c in $(BINCOMP) ;\
	do \
		cpset $$c $(USRDIR) 6775 lp ; \
	done
	for c in $(LPCOMP) ;\
	do \
		cpset $$c $(ADMDIR) 6775 lp ; \
	done
	for c in $(ROOTCOMP) ;\
	do \
		cpset $$c $(ADMDIR) 6775 root ; \
	done
	cd filter;  $(MAKE) -f filter.mk OL=$(ROOT)/$(OL) install
	cd model;   $(MAKE) -f model.mk  OL=$(ROOT)/$(OL) install

clean:	
	cd filter; $(MAKE) -f filter.mk clean
	cd model;  $(MAKE) -f model.mk  clean
	rm -f $(LIB) *.o pstatus qstatus

clobber:	clean
	cd filter; $(MAKE) -f filter.mk clobber
	cd model;  $(MAKE) -f model.mk  clobber
	rm -f $(BINCOMP) $(LPCOMP) $(ROOTCOMP)
