#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rmount:rmount.mk	1.2.2.1"
ROOT = 
TESTDIR = .
INSDIR = $(ROOT)/usr/nserve
INC = $(ROOT)/usr/include
INS = :
CFLAGS=$(DEBUG) $(PROFILE) -x -s
LDFLAGS=-lc_s -lns
CC = cc
PROFILE=
DEBUG=
EXECS=rmount rmnttry rumount
SOURCE=rmount.c rmnttry.c rumount.c fqn.c mntlock.c rd_rmnttab.c wr_rmnttab.c
RMOBJECTS = rmount.o mntlock.o rd_rmnttab.o wr_rmnttab.c fqn.o
RUOBJECTS = rumount.o mntlock.o rd_rmnttab.o wr_rmnttab.c fqn.o
RTOBJECTS = rmnttry.o mntlock.o rd_rmnttab.o wr_rmnttab.o fqn.o
FRC =

all:	$(EXECS)
rmount: $(RMOBJECTS)
	$(CC) -I$(INC) $(CFLAGS) $(RMOBJECTS) $(LIB) -o $(TESTDIR)/rmount $(LDLIBS) $(LDFLAGS)
rmnttry: $(RTOBJECTS)
	$(CC) -I$(INC) $(CFLAGS) $(RTOBJECTS) $(LIB) -o $(TESTDIR)/rmnttry $(LDLIBS) $(LDFLAGS)
rumount: $(RUOBJECTS)
	$(CC) -I$(INC) $(CFLAGS) $(RUOBJECTS) $(LIB) -o $(TESTDIR)/rumount $(LDLIBS) $(LDFLAGS)
install: all
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		mkdir $(INSDIR) ; \
	fi ;
	/etc/install -f $(INSDIR) -m 555 -u root -g sys rmount
	/etc/install -f $(INSDIR) -m 550 -u root -g sys rmnttry
	/etc/install -f $(INSDIR) -m 554 -u root -g sys rumount
uninstall:
	(cd $(INSDIR); rm -f $(EXECS))

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(EXECS)
FRC: 

#### dependencies now follow

fqn.o: $(INC)/sys/rfsys.h $(INC)/nserve.h $(INC)/string.h
mntlock.o: $(INC)/unistd.h $(INC)/fcntl.h $(INC)/stdio.h
rd_rmnttab.o: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h
rmnttry.c: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h
rmount.o: $(INC)/sys/types.h  $(INC)/mnttab.h $(INC)/stdio.h $(INC)/nserve.h
rumount.o: $(INC)/sys/types.h  $(INC)/mnttab.h $(INC)/stdio.h $(INC)/nserve.h
wr_rmnttab.o: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h $(INC)/unistd.h
