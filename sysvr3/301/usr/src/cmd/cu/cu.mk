#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cu:cu.mk	2.5"
#
#
# ***************************************************************
# *	Copyright (c) 1984 AT&T Technologies, Inc.		*
# *                 All Rights Reserved				*
# *	THIS IS UNPUBLISHED PROPRIETARY SOURCE			*
# *	CODE OF AT&T TECHNOLOGIES, INC.				*
# *	The copyright notice above does not			*
# *	evidence any actual or intended				*
# *	publication of such source code.			*
# ***************************************************************
#

OWNER=uucp
GRP=daemon
BIN=$(ROOT)/usr/bin
UUDIR=../uucp
LDFLAGS = -s

CUDIR = .
CFLAGS = -O -DSTANDALONE -I$(UUDIR)
CFLAGS = -O -DSTANDALONE -DSMALL -I$(UUDIR) #for smaller a.outs
LIBS= -lnsl_s

#	Use the following lines for DATAKIT
# CFLAGS = -O -DSTANDALONE -I$(UUDIR) -DDATAKIT
# CFLAGS = -O -DDATAKIT -DSTANDALONE -DSMALL -I$(UUDIR) #for smaller a.outs
# LIBS=-ldk

# OLD=-o to save old files upon install
INS=cpset
# If you system does not have "cpset"
#  use the one in the uucp source directory and use the
#  next INS=../uucp/Cpset line instead of the INS=cpset line above.
# INS=../uucp/Cpset

CUOBJS =  cu.o callers.o getargs.o culine.o uucpdefs.o ulockf.o\
	 conn.o altconn.o interface.o strsave.o sysfiles.o strecpy.o

CUSRC =  cu.c altconn.c culine.c $(UUDIR)/callers.c\
	$(UUDIR)/conn.c $(UUDIR)/uucpdefs.c $(UUDIR)/ulockf.c \
	$(UUDIR)/getargs.c $(UUDIR)/interface.c $(UUDIR)/strsave.c \
	$(UUDIR)/sysfiles.c $(UUDIR)/strecpy.c

cu:	copyright $(CUOBJS)
	$(CC) $(CFLAGS) $(CUOBJS) $(LIBS) $(LDFLAGS) -o cu

all:	 cu

copyright:
	@echo "\n\n**********************************************"
	@echo "* Copyright (c) 1984 AT&T Technologies, Inc. *"
	@echo "*           All Rights Reserved              *"
	@echo "* THIS IS UNPUBLISHED PROPRIETARY SOURCE     *"
	@echo "* CODE OF AT&T TECHNOLOGIES, INC.            *"
	@echo "* The copyright notice above does not        *"
	@echo "* evidence any actual or intended            *"
	@echo "* publication of such source code.           *"
	@echo "**********************************************\n\n"

install:	copyright all
	$(INS) $(OLD) cu $(BIN)/cu 4111 uucp sys

clean:
	-rm -f *.o

clobber: clean
	rm -f cu

callers.o:	$(UUDIR)/callers.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/callers.c

getargs.o:	$(UUDIR)/getargs.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/getargs.c
 
uucpdefs.o:	$(UUDIR)/uucpdefs.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/uucpdefs.c

ulockf.o:	$(UUDIR)/ulockf.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/ulockf.c

conn.o:		$(UUDIR)/conn.c
	$(CC) -c $(CFLAGS) $(UUDIR)/conn.c

interface.o:	$(UUDIR)/interface.c  $(UUDIR)/uucp.h
	$(CC) -c $(CFLAGS) $(UUDIR)/interface.c

strsave.o:	$(UUDIR)/strsave.c 
	$(CC) -c $(CFLAGS) $(UUDIR)/strsave.c

sysfiles.o:	$(UUDIR)/sysfiles.c  $(UUDIR)/sysfiles.h
	$(CC) -c $(CFLAGS) $(UUDIR)/sysfiles.c

strecpy.o:	$(UUDIR)/strecpy.c
	$(CC) -c $(CFLAGS) $(UUDIR)/strecpy.c
