#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)listen:listen.mk	1.14"
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# listen.mk:
# makefile for network listener - System V release 3.0
#

INC	= $(ROOT)/usr/include
OPT	= -O
CFLAGS	= -Um68k -Um68 -Umc68k -US4 -Uvax  -UMICROOMS -Updp11 -Uu3b -Du3b2 -I$(INC) ${OPT}
LDFLAGS	= $(LLDFLAGS)
LIBNLS = libnls.a
LDLIBS = -lnsl_s
LSTINC	= .

# change the next two lines to compile with -g
# OPT	= -g
LLDFLAGS= -s

CHKDIR = $(ROOT)/usr/net $(ROOT)/usr/net/nls
INSDIR = $(ROOT)/usr/net/nls
LIBDIR = $(ROOT)/usr/lib

LSUID	= listen
LSGID	= adm
LIBID	= bin

LLDLIBS	= \
	$(LIBNLS)


# The DEBUG module can always be included...
# if DEBUGMODE is undefined, no code gets compiled.
# doprnt.o is from system V rel 5.0.5.

DBGOBJ	= doprnt.o
DBGSRC	= doprnt.c

SRC	= \
	listen.c \
	lsdata.c \
	lsdbf.c \
	lslog.c \
	lssmb.c \
	nlsaddr.c \
	nlsdata.c $(DBGSRC)

LIBSRC = \
	nlsenv.c \
	nlsrequest.c

S4SRC = \
	lsnames.c \
	nlsconnect.c \
	nlsname.c \
	nlsestab.c \
	putenv.c

PRODUCT	= $(INSDIR)/listen

LSTINCS = \
	$(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/varargs.h \
	$(INC)/string.h \
	$(INC)/errno.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/tiuser.h \
	$(INC)/sys/poll.h \
	$(INC)/sys/param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/stat.h \
	$(INC)/values.h \
	$(INC)/ctype.h \
	$(INC)/pwd.h \
	$(INC)/grp.h \
	$(LSTINC)/lsparam.h \
	$(LSTINC)/lsfiles.h \
	$(LSTINC)/lserror.h \
	$(LSTINC)/lsnlsmsg.h \
	$(LSTINC)/lssmbmsg.h \
	$(LSTINC)/lsdbf.h

DBGINCS	= \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/varargs.h \
	$(INC)/values.h \
	$(LSTINC)/lsparam.h \
	$(LSTINC)/print.h

SMBINCS = \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(LSTINC)/lssmbmsg.h \
	$(LSTINC)/lsparam.h \
	$(INC)/sys/tiuser.h \
	$(LSTINC)/lsdbf.h

CONINCS = \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/fcntl.h \
	$(INC)/errno.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/tiuser.h \
	$(LSTINC)/listen.h

ALLINCS = $(LSTINCS) $(DBGINCS) $(SMBINCS) $(CONINCS)

LSOBJS	= \
	listen.o \
	lslog.o \
	lsdbf.o \
	lssmb.o \
	lsdata.o \
	nlsdata.o \
	nlsaddr.o

NLSOBJS = \
	$(LIBNLS)(nlsenv.o) \
	$(LIBNLS)(nlsrequest.o)

S4OBJS = \
	lsnames.o \
	nlsconnect.o \
	nlsname.o \
	nlsestab.o \
	putenv.o

all:	listen libnls

# 
# SHAREDLIB version
#

listen:		$(LSOBJS) $(DBGOBJ) $(LLDLIBS) 
		$(CC) $(CFLAGS) -o listen $(LSOBJS) $(DBGOBJ) \
			$(LDFLAGS) $(LLDLIBS) $(LDLIBS)

libnls:	$(LIBNLS)

$(LIBNLS):	$(NLSOBJS)

.PRECIOUS:	$(LIBNLS)

s4stuff:	$(S4OBJS)
		$(CC) -c $(S4OBJS)

listen.o:	$(LSTINCS)
lsdbf.o:	$(LSTINCS)
lslog.o:	$(LSTINCS)
lssmb.o:	$(INC)/stdio.h $(INC)/string.h $(LSTINC)/lssmbmsg.h \
		$(LSTINC)/lsparam.h $(INC)/sys/tiuser.h $(LSTINC)/lsdbf.h
lsdata.o:	$(LSTINCS)
nlsdata.o:	$(INC)/sys/tiuser.h
nlsaddr.o:	$(INC)/ctype.h $(INC)/sys/tiuser.h
doprnt.o:	$(DBGINCS)

# listener library routines and /usr/include headers:
# putenv.o should only be included on an S4. (not included in s4's libc.a)
# In addition, nlsconnect.o, nlsestab.o, and nlsname.o are only on the S4.

$(LIBNLS)(nlsenv.o):	$(INC)/ctype.h $(LSTINC)/nlsenv.h $(INC)/sys/tiuser.h
$(LIBNLS)(nlsrequest.o):	$(INC)/stdio.h $(INC)/ctype.h $(INC)/fcntl.h \
				$(INC)/errno.h $(INC)/string.h $(INC)/sys/tiuser.h \
				$(LSTINC)/listen.h

# S4 stuff only

nlsconnect.o:	$(CONINCS)
nlsname.o:	$(INC)/string.h $(INC)/ctype.h
nlsestab.o:	$(INC)/stdio.h $(INC)/ctype.h $(INC)/fcntl.h \
		$(INC)/errno.h $(INC)/sys/utsname.h \
		$(INC)/sys/tiuser.h $(LSTINC)/listen.h
putenv.o:


install:	all $(CHKDIR)
		-rm -f $(LIBDIR)/$(LIBNLS)
		install -o -f $(INSDIR) listen
		$(CH)chown $(LSUID) $(INSDIR)/listen
		$(CH)chgrp $(LSGID) $(INSDIR)/listen
		cp $(LIBNLS) $(LIBDIR)
		$(CH)chown $(LIBID) $(LIBDIR)/$(LIBNLS)
		$(CH)chgrp $(LIBID) $(LIBDIR)/$(LIBNLS)

$(CHKDIR):
		mkdir $@
		$(CH)chown $(LSUID) $@
		$(CH)chgrp $(LSGID) $@

clean:
	-rm -f *.o

clobber: clean
	-rm -f listen
	-rm -f libnls.a

FRC:
