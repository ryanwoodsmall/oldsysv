#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cron:cron.mk	1.16"

INS=cpset

ROOT=
OL=$(ROOT)/
ETC=$(OL)etc
USR=$(OL)usr
INSDIR=$(OL)usr/bin
SPOOL=$(USR)/spool/cron
LIB=$(USR)/lib

CRONLIB=$(LIB)/cron
CRONSPOOL=$(SPOOL)/crontabs
ATSPOOL=$(SPOOL)/atjobs

XDIRS= $(ROOT) $(ETC) $(USR) $(INSDIR) $(LIB) $(SPOOL)\
      $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

DIRS= $(SPOOL) $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

CMDS= cron at crontab batch

CFLAGS= -O
LDFLAGS= -s
DEFS=

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all:	$(CMDS)

install:	dirs install_cron install_at install_crontab install_batch
#	make -f cron.mk INS="install -f" $(ARGS)

install_cron:	cron
	$(INS) -o cron $(ETC) 700 root sys

install_at:	at
	$(INS) at $(INSDIR) 4755 root sys

install_crontab:	crontab
	$(INS) crontab $(INSDIR) 4755 root sys

install_batch:	batch
	$(INS) batch $(INSDIR) 755 bin bin

libelm.a: elm.o
	ar cr libelm.a elm.o

cron:	cron.o funcs.o libelm.a
	$(CC) $(CFLAGS) cron.o funcs.o libelm.a -o cron $(LDFLAGS) $(LDLIBS)

crontab:	crontab.o permit.o funcs.o
	$(CC) $(CFLAGS) crontab.o permit.o funcs.o -o crontab

at:	at.o att1.o att2.o funcs.o permit.o
	$(CC) $(CFLAGS) at.o att1.o att2.o funcs.o permit.o -o at $(LDFLAGS)

batch:	batch.sh
	cp batch.sh batch

att1.c att1.h:	att1.y
	yacc -d att1.y
	mv y.tab.c att1.c
	mv y.tab.h att1.h

att2.c:	att2.l
	lex att2.l
	ed - lex.yy.c < att2.ed
	mv lex.yy.c att2.c

att2.o:	att1.h

cron.o:	cron.c cron.h
crontab.o:	crontab.c cron.h
at.o:	at.c cron.h

dirs:	$(DIRS)

$(DIRS):
	-if [ "$(OL)" = "/" ]; \
		then mkdir $@; chmod 755 $@; chgrp sys $@; chown root $@; \
	fi

clean:
	rm -f *.o libelm.a att1.h att1.c att2.c

clobber:	clean
	rm -f $(CMDS)
