
#	@(#)Makefile	1.12 -- SCCS library build

LIB1 = comobj.a
LIB2 = mpwlib.a
LIB3 = cassi.a
LIBS = $(LIB1) $(LIB2) $(LIB3)
INSTR =
FHAND = `pwd`

DOTC1 = chkid.c chksid.c del_ba.c date_ba.c dodelt.c
DOTC2 = del_ab.c dofile.c dohist.c doie.c dolist.c eqsid.c
DOTC3 = flushto.c newstats.c permiss.c logname.c pf_ab.c
DOTC4 = date_ab.c rdmod.c setup.c sid_ab.c sid_ba.c sidtoser.c
DOTC5 = sinit.c stats_ab.c fmterr.c getline.c putline.c auxf.c
DOTC6 = xcreat.c xmsg.c fmalloc.c
DOTC7 = gf.c cmrcheck.c deltack.c error.o 

GRPA =	$(LIB1)(chkid.o) \
	$(LIB1)(chksid.o) \
	$(LIB1)(del_ba.o) \
	$(LIB1)(date_ba.o) \
	$(LIB1)(dodelt.o)

GRPB =	$(LIB1)(del_ab.o) \
	$(LIB1)(dofile.o) \
	$(LIB1)(dohist.o) \
	$(LIB1)(doie.o) \
	$(LIB1)(dolist.o) \
	$(LIB1)(eqsid.o)

GRPC =	$(LIB1)(flushto.o) \
	$(LIB1)(newstats.o) \
	$(LIB1)(permiss.o) \
	$(LIB1)(logname.o) \
	$(LIB1)(pf_ab.o)

GRPD =	$(LIB1)(date_ab.o) \
	$(LIB1)(rdmod.o) \
	$(LIB1)(setup.o) \
	$(LIB1)(sid_ab.o) \
	$(LIB1)(sid_ba.o) \
	$(LIB1)(sidtoser.o)

GRPE =	$(LIB1)(sinit.o) \
	$(LIB1)(stats_ab.o) \
	$(LIB1)(fmterr.o) \
	$(LIB1)(getline.o) \
	$(LIB1)(putline.o) \
	$(LIB1)(auxf.o)

GRPF =	$(LIB2)(xcreat.o) \
	$(LIB2)(xmsg.o) \
	$(LIB2)(fmalloc.o)

GRPG = $(LIB3)(gf.o) \
	$(LIB3)(cmrcheck.o) \
	$(LIB3)(deltack.o) \
	$(LIB3)(error.o)

GRPH = $(LIB3)(filehand.o)

all:	$(LIBS)
	@echo "\t** Libraries are up to date\n"

.c.a:;

.c~.a:
	$(GET) $(GFLAGS) -p $< > $*.c

.PRECIOUS:	$(LIBS)

$(LIB1)::	$(GRPA)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB1) $?
		rm -f $?

$(LIB1)::	$(GRPB)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB1) $?
		rm -f $?

$(LIB1)::	$(GRPC)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB1) $?
		rm -f $?

$(LIB1)::	$(GRPD)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB1) $?
		rm -f $?

$(LIB1)::	$(GRPE)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB1) $?
		rm -f $?

$(LIB3)::   $(GRPG)
			$(CC) $(CFLAGS) -c  -DFHMAKE=\"$(FHAND) $(?:.o=.c)
			ar r $(LIB3) $?
			rm -f $?

$(LIB3)::   filehand.o
			ar r $(LIB3) filehand.o
			rm -f filehand.o
			
$(LIB1)::	$(GRPA) $(GRPB) $(GRPC) $(GRPD) $(GRPE)
		-chmod 664 $@
		-if  pdp11  ; \
		then strip $@; \
		else strip -r $@; \
		     ar ts $@; \
		fi
		-@if [ "$(INSTR)" ]; \
		then chgrp bin $@; chown bin $@; \
		fi

$(LIB2)::	$(GRPF)
		$(CC) $(CFLAGS) -c $(?:.o=.c)
		ar r $(LIB2) $?
		rm -f $?
		-chmod 664 $@
		-if  pdp11 ; \
		then strip $@; \
		else strip -r $@; \
		     ar ts $@; \
		fi
		-@if [ "$(INSTR)" ]; \
		then chgrp bin $@; chown bin $@; \
		fi

$(LIB3)::	$(GRPG) $(GRPH)
		-chmod 664 $@
		if pdp11  ; \
		then strip $@; \
		else strip -r $@; \
		     ar ts $@; \
		fi
		-@if [ "$(INSTR)" ]; \
		then chgrp bin $@; chown bin $@; \
		fi

filehand.o: filehand.c filehand.h
		make -f fhand.mk PARMS=-DXALLOC 

deltack.o: 	deltack.c $(HDR)/defines.h filehand.h
	$(CC) $(CFLAGS) -DFHMAKE=\"$(FHAND) $(LIBDIR)/deltack.c $(LIBES)
	
error.o:	error.c
	$(CC) $(CFLAGS) error.c  $(LIBES)
	
gf.o:  gf.c filehand.h
	$(CC) $(CFLAGS) -DFHMAKE=\"$(FHAND)  gf.c
		
cmrcheck.o: cmrcheck.c filehand.h
	$(CC) $(CFLAGS) -DFHMAKE=\"$(FHAND) cmrcheck.c

clean:
	rm -f *.o
	rm -f *.a

clobber:	clean

Bigclob:	clobber
	-rm -f $(DOTC1)
	-rm -f $(DOTC2)
	-rm -f $(DOTC3)
	-rm -f $(DOTC4)
	-rm -f $(DOTC5)
	-rm -f $(DOTC6)
