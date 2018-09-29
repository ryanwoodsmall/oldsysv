#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sa:sa.mk	1.6.1.6"
#/*	sa.mk 1.6.1.6 of 2/25/86	*/
# how to use this makefile
# to make sure all files  are up to date: make -f sa.mk all
#
# to force recompilation of all files: make -f sa.mk all FRC=FRC 
#
# to test new executables before installing in 
# /usr/lib/sa:	make -f sa.mk testbi
#
# to install just one file:	make -f sa.mk safile "INS=/etc/install"
#
# The sadc and sadp modules must be able to read /dev/kmem,
# which standardly has restricted read permission.
# They must have set-group-ID mode
# and have the same group as /dev/kmem.
# The chmod and chgrp commmands below ensure this.
#
ROOT =
TESTDIR = .
FRC =
INS = @:
INSDIR = $(ROOT)/usr/lib/sa
CRONDIR= $(ROOT)/usr/spool/cron/crontabs
CRONTAB= $(ROOT)/usr/spool/cron/crontabs/sys
ENTRY1= '0 * * * 0-6 /usr/lib/sa/sa1'
ENTRY2= '20,40 8-17 * * 1-5 /usr/lib/sa/sa1'
ENTRY3= '5 18 * * 1-5 /usr/lib/sa/sa2 -s 8:00 -e 18:01 -i 1200 -A'
CFLAGS = -O
FFLAG =
LDFLAGS = -s
ARGS = all
 

all:	sadc sar sa1 sa2 perf timex sag sadp


sadc:: sadc.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadc sadc.c 
sadc::
	$(INS) -o -f $(INSDIR) -m 2755 -g sys $(TESTDIR)/sadc $(INSDIR)
sar:: sar.c sa.h
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sar sar.c
sar::
	$(INS) -f $(ROOT)/usr/bin $(TESTDIR)/sar 
sa2:: sa2.sh
	cp sa2.sh sa2
sa2::
	$(INS) -f $(INSDIR) $(TESTDIR)/sa2 $(INSDIR)
sa1:: sa1.sh
	cp sa1.sh sa1
sa1::
	$(INS) -f $(INSDIR) $(TESTDIR)/sa1 $(INSDIR)
 
perf:: perf.sh
	cp perf.sh perf
perf::
	$(INS) -f $(ROOT)/etc/init.d -u root -g sys -m 444 $(TESTDIR)/perf

timex::	timex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/timex timex.c 
timex::
	$(INS) -f $(ROOT)/usr/bin $(TESTDIR)/timex
sag::	saga.o sagb.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sag saga.o sagb.o 
sag::
	$(INS) -f $(ROOT)/usr/bin $(TESTDIR)/sag
saga.o:	saga.c saghdr.h
	$(CC) -c $(CFLAGS) saga.c
sagb.o:	sagb.c saghdr.h
	$(CC) -c $(CFLAGS) sagb.c
sadp:: sadp.c 
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadp sadp.c
sadp::
	$(INS) -f $(ROOT)/usr/bin -m 2755 -g sys $(TESTDIR)/sadp 
test:		testai

testbi:		#test for before installing
	sh  $(TESTDIR)/runtest new $(ROOT)/usr/src/cmd/sa

testai:		#test for after install
	sh $(TESTDIR)/runtest new

install:
	if [ ! -d $(INSDIR) ];\
	then	mkdir $(INSDIR);\
		chmod 775 $(INSDIR);\
		$(CH) chown adm $(INSDIR);\
		$(CH) chgrp bin $(INSDIR);\
	fi;
	-make -f sa.mk $(ARGS) FFLAG=$(FFLAG) "INS=install" INSDIR=$(INSDIR)
	rm -rf $(ROOT)/etc/rc2.d/S21perf
	ln $(ROOT)/etc/init.d/perf $(ROOT)/etc/rc2.d/S21perf
	if [ -d $(CRONDIR) ];\
		then	if [ ! -f $(CRONTAB) ];\
			   then  >  $(CRONTAB);\
			   chmod 644  $(CRONTAB);\
			   $(CH) chown root $(CRONTAB);\
			   $(CH) chgrp sys  $(CRONTAB);\
		fi;\
	else\
			echo "***WARNING**** cannot install cron entries";\
	fi;
	if [ -f $(CRONTAB) ];\
		then  if grep "sa1" $(CRONTAB) >/dev/null 2>&1 ; then :;\
		else\
			echo $(ENTRY1) >> $(CRONTAB);\
			echo $(ENTRY2) >> $(CRONTAB);\
		fi;\
		if grep "sa2" $(CRONTAB) >/dev/null 2>&1 ; then :;\
		else\
			echo $(ENTRY3) >> $(CRONTAB);\
		fi;\
	fi;

clean:
	-rm -f *.o
 
clobber:	clean
		-rm -f sadc sar sa1 sa2 perf sag timex sadp

FRC:
