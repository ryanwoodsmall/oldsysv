#	@(#)Makefile	1.10 -- SCCS command build and installation

HDR = ../hdr
#	Directory containing SCCS header files
LIBS = ../lib/comobj.a ../lib/mpwlib.a ../lib/cassi.a ../lib/cassi.a
#	SCCS common routine archive
BIN = ../sbin
#	Directory where executable SCCS commands are stored
B02 =
B12 =
#	u370 base register flags
CFLAGS = -O -c
#	Flags used by C compiler
LDFLAGS = -s -n -i
#	Load flags used by C compiler
SYSLIBS = -lPW
#	Libraries used during loading

ARGS =

CMDS = admin bdiff cdc comb delta get help prs \
	rmdel sact sccsdiff unget val vc what

DOTC1 = admin.c bdiff.c rmchg.c comb.c delta.c get.c
DOTC2 = help.c prs.c sccsdiff.sh unget.c val.c vc.c what.c

all:	$(CMDS)

admin:	admin.o $(LIBS)
	$(CC) $(LDFLAGS) admin.o $(LIBS) $(SYSLIBS) -o $(BIN)/admin 
	-chmod 775 $(BIN)/admin

admin.o:	admin.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B12) admin.c

bdiff:	bdiff.o $(LIBS)
	$(CC) $(LDFLAGS) bdiff.o $(LIBS) $(SYSLIBS) -o $(BIN)/bdiff 
	-chmod 775 $(BIN)/bdiff

bdiff.o:	bdiff.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) bdiff.c

cdc:	rmchg
	-rm -f $(BIN)/cdc
	ln $(BIN)/rmchg $(BIN)/cdc

comb:	comb.o $(LIBS)
	$(CC) $(LDFLAGS) comb.o $(LIBS) $(SYSLIBS) -o $(BIN)/comb 
	-chmod 775 $(BIN)/comb

comb.o:		comb.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) comb.c

delta:	delta.o $(LIBS)
	$(CC) $(LDFLAGS) delta.o $(LIBS) $(SYSLIBS) -o $(BIN)/delta 
	-chmod 775 $(BIN)/delta

delta.o:	delta.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) delta.c

get:	get.o $(LIBS)
	$(CC) $(LDFLAGS) get.o $(LIBS) $(SYSLIBS) -o $(BIN)/get 
	-chmod 775 $(BIN)/get

get.o:		get.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) get.c

help:	help.o $(LIBS)
	$(CC) $(LDFLAGS) help.o $(LIBS) $(SYSLIBS) -o $(BIN)/help 
	-chmod 775 $(BIN)/help

help.o:		help.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) help.c

prs:	prs.o $(LIBS)
	$(CC) $(LDFLAGS) prs.o $(LIBS) $(SYSLIBS) -o $(BIN)/prs 
	-chmod 775 $(BIN)/prs

prs.o:		prs.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B12) prs.c

rmchg:	rmchg.o $(LIBS)
	$(CC) $(LDFLAGS) rmchg.o $(LIBS) $(SYSLIBS) -o $(BIN)/rmchg 
	-chmod 775 $(BIN)/rmchg

rmchg.o:	rmchg.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) -DFHAND=\"../lib  rmchg.c

rmdel:	rmchg
	-rm -f $(BIN)/rmdel
	ln $(BIN)/rmchg $(BIN)/rmdel

sact:	unget
	-rm -f $(BIN)/sact
	ln $(BIN)/unget $(BIN)/sact

sccsdiff:	sccsdiff.sh
	cp sccsdiff.sh $(BIN)/sccsdiff
	-chmod 775 $(BIN)/sccsdiff

sccsdiff.sh:
	get  -p s.sccsdiff.sh > sccsdiff.sh

unget:	unget.o $(LIBS)
	$(CC) $(LDFLAGS) unget.o $(LIBS) $(SYSLIBS) -o $(BIN)/unget 
	-chmod 775 $(BIN)/unget

unget.o:	unget.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) unget.c

val:	val.o $(LIBS)
	$(CC) $(LDFLAGS) val.o $(LIBS) $(SYSLIBS) -o $(BIN)/val 
	-chmod 775 $(BIN)/val

val.o:	val.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) $(B02) val.c

vc:	vc.o $(LIBS)
	$(CC) $(LDFLAGS) vc.o $(LIBS) $(SYSLIBS) -o $(BIN)/vc 
	-chmod 775 $(BIN)/vc

vc.o:	vc.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) vc.c

what:	what.o $(LIBS)
	$(CC) $(LDFLAGS) what.o $(LIBS)  $(SYSLIBS) -o $(BIN)/what 
	-chmod 775 $(BIN)/what

what.o:		what.c $(HDR)/defines.h $(HDR)/had.h
	$(CC) $(CFLAGS) what.c

$(LIBS):
	cd ../lib; $(MAKE)

owner:
	-@if [ "$(ARGS)" ]; \
	then cd $(BIN); chgrp bin $(ARGS); chown bin $(ARGS); \
	else cd $(BIN); chgrp bin $(CMDS); chown bin $(CMDS); \
	fi

clean:
	-rm -f *.o
	-rm -f $(BIN)/rmchg

clobber:	clean

Bigclob:	clobber
	-rm -f $(DOTC1)
	-rm -f $(DOTC2)
