#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#
#ident	"@(#)vm:xx/xx.mk	1.11"
#

INC=$(ROOT)/usr/include
USR=$(ROOT)/usr
USRLIB=$(ROOT)/usr/lib
LIBOH	=	../oh/liboh.a
LIBPROC	=	../proc/libproc.a
LIBSYS	=	../sys/libsys.a
LIBOEU	=	../oeu/liboeu.a
LIBBK	=	../bk/libbk.a

MKDIR	=	mkdir
VBIN=$(USR)/vmsys/bin
OABIN = $(USR)/oasys/bin
HEADER1=../inc
INCLUDE=	-I$(HEADER1)

LIBS = $(LIBOH) $(LIBOEU) $(LIBPROC) $(LIBSYS) 
DIRS= $(USR)/vmsys $(USR)/oasys

CFLAGS= -O 

LDFLAGS =  -s

OCMDS =	termtest setenv oeupkg btoa identify

VCMDS = face suspend dir_move dir_copy dir_creat creaserve listserv delserve chexec chkterm basename mnucheck modserv

CMDS = $(VCMDS) $(OCMDS)

face: face.o
	$(CC) $(LDFLAGS) -o $@ face.o -lPW

face.o:	face.c 

suspend: suspend.o $(LIBPROC)
	$(CC) $(LDFLAGS) -o $@ suspend.o $(LIBPROC)

suspend.o: suspend.c $(HEADER1)/wish.h

creaserve: creaserve.o
	$(CC) $(LDFLAGS) -o $@ creaserve.o 

creaserve.o: creaserve.c $(HEADER1)/wish.h $(HEADER1)/exception.h

delserve: delserve.o
	$(CC) $(LDFLAGS) -o $@ delserve.o 

delserve.o: delserve.c $(HEADER1)/wish.h $(HEADER1)/exception.h

listserv: listserv.o
	$(CC) $(LDFLAGS) -o $@ listserv.o 

listserv.o: listserv.c $(HEADER1)/wish.h $(HEADER1)/exception.h

modserv: modserv.o
	$(CC) $(LDFLAGS) -o $@ modserv.o 

modserv.o: modserv.c $(HEADER1)/wish.h $(HEADER1)/exception.h

mnucheck: mnucheck.o
	$(CC) $(LDFLAGS) -o $@ mnucheck.o 

mnucheck.o: mnucheck.c $(HEADER1)/wish.h

btoa	: btoa.o
	$(CC) $(LDFLAGS) -o $@ btoa.o

btoa.o	: btoa.c


OEUPKG = oeupkg.o $(LIBOEU) $(LIBBK) $(LIBOH) $(LIBSYS)
oeupkg:	$(OEUPKG)
	$(CC) $(LDFLAGS) -o $@ $(OEUPKG)

oeupkg.o:	oeupkg.c $(HEADER1)/tsys.h $(HEADER1)/terror.h $(HEADER1)/retcds.h $(HEADER1)/parse.h

termtest:	termtest.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h $(LIBSYS) $(HEADER1)/moremacros.h
	$(CC) $(CFLAGS) $(LDFLAGS)  $(INCLUDE) -o $@ termtest.c $(LIBSYS) -lPW

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

.o:
	$(CC) $(LDFLAGS) -o $@ $?

.sh:
	/bin/rm -f $@
	/bin/cp $< $@
	$(CH)chmod 755 $@

###### Standard Makefile Targets ######

all:	$(CMDS)

install: all dirs $(VBIN) $(OABIN)
	@set +e;\
	for f in $(VCMDS);\
	do\
		install -f $(VBIN) $$f;\
		$(CH) chmod 755 $(VBIN)/$$f;\
	done;\
	for f in $(OCMDS);\
	do\
		install -f $(OABIN) $$f;\
		$(CH) chmod 755 $(OABIN)/$$f;\
	done

dirs:	$(DIRS)

$(DIRS):
	$(MKDIR) $@ 
	$(CH)chmod 755 $@

$(VBIN) $(OABIN):
	$(MKDIR) $@ 
	$(CH)chmod 755 $@

clean: 
	@set +e;\
	for f in $(VCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	for f in $(OCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	/bin/rm -f *.o

clobber:	clean
