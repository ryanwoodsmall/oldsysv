#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ct:ct.mk	2.12"
#
#	ct make file
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

INSDIR = $(ROOT)/usr/bin
UUDIR=../uucp
LDFLAGS = -s

# OLD=-o to save old files upon install
INS=cpset
# If you system does not have "cpset"
#  use the one in the uucp source directory and use the
#  next INS=../uucp/Cpset line instead of the INS=cpset line above.
# INS=../uucp/Cpset

CTDIR = .
CFLAGS = -O -DSTANDALONE -DSMALL -I$(UUDIR)	# for smaller a.outs
#CFLAGS = -O -DSTANDALONE -I$(UUDIR)

TLILIB = -lnsl_s
# DKLIB = -ldk
# DKLIB no longer used, datakit files are included in uucp
#LIBS= -lc_s $(TLILIB) $(DKLIB)

LIBS= -lc_s $(TLILIB) 

CTOBJS =  ct.o callers.o getargs.o line.o uucpdefs.o ulockf.o\
	 conn.o interface.o sysfiles.o strsave.o strecpy.o stoa.o

CTSRC =  ct.c $(UUDIR)/callers.c $(UUDIR)/getargs.c \
	$(UUDIR)/line.c $(UUDIR)/uucpdefs.c $(UUDIR)/ulockf.c \
	 $(UUDIR)/conn.c $(UUDIR)/interface.c $(UUDIR)/sysfiles.c \
	 $(UUDIR)/strsave.c $(UUDIR)/strecpy.c $(UUDIR)/stoa.c

ODK=dkbreak.o dkdial.o dkminor.o dtnamer.o dkerr.o 
LDK=$(UUDIR)/dkbreak.c $(UUDIR)/dkdial.c $(UUDIR)/dkminor.c \
	$(UUDIR)/dtnamer.c $(UUDIR)/dkerr.c

ct:	copyright $(CTOBJS) $(ODK)
	$(CC) $(CFLAGS) $(CTOBJS) $(ODK) $(LIBS) $(LDFLAGS) -o ct

all:	install clobber

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

install:	copyright ct
	$(INS) $(OLD) ct $(INSDIR)/ct 4111 root uucp

clean:
	-rm -f *.o

clobber: clean
	rm -f ct

callers.o:	$(UUDIR)/callers.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h \
		$(UUDIR)/dk.h
	$(CC) -c $(CFLAGS) $(UUDIR)/callers.c

getargs.o:	$(UUDIR)/getargs.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/getargs.c
 
line.o:		$(UUDIR)/line.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/line.c

uucpdefs.o:	$(UUDIR)/uucpdefs.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/uucpdefs.c

ulockf.o:	$(UUDIR)/ulockf.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/ulockf.c

conn.o:		$(UUDIR)/conn.c
	$(CC) -c $(CFLAGS) $(UUDIR)/conn.c
 
interface.o:	$(UUDIR)/interface.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(CC) -c $(CFLAGS) $(UUDIR)/interface.c
 
sysfiles.o:	$(UUDIR)/sysfiles.c  $(UUDIR)/sysfiles.h
	$(CC) -c $(CFLAGS) $(UUDIR)/sysfiles.c
 
strsave.o:	$(UUDIR)/strsave.c
	$(CC) -c $(CFLAGS) $(UUDIR)/strsave.c
 
strecpy.o:	$(UUDIR)/strecpy.c
	$(CC) -c $(CFLAGS) $(UUDIR)/strecpy.c
 
stoa.o:	$(UUDIR)/stoa.c
	$(CC) -c $(CFLAGS) $(UUDIR)/stoa.c

dkbreak.o: $(UUDIR)/dkbreak.c $(UUDIR)/dk.h $(UUDIR)/sysexits.h
	$(CC) -c $(CFLAGS) $(UUDIR)/dkbreak.c
	
dkdial.o: $(UUDIR)/dkdial.c $(UUDIR)/dk.h $(UUDIR)/sysexits.h
	$(CC) -c $(CFLAGS) $(UUDIR)/dkdial.c
	
dkminor.o: $(UUDIR)/dkminor.c  $(UUDIR)/dk.h $(UUDIR)/sysexits.h
	$(CC) -c $(CFLAGS) $(UUDIR)/dkminor.c
	
dtnamer.o: $(UUDIR)/dtnamer.c $(UUDIR)/dk.h $(UUDIR)/sysexits.h
	$(CC) -c $(CFLAGS) $(UUDIR)/dtnamer.c
	
dkerr.o: $(UUDIR)/dkerr.c $(UUDIR)/dk.h $(UUDIR)/sysexits.h 
	$(CC) -c $(CFLAGS) $(UUDIR)/dkerr.c
