#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-3b2:cmd-3b2.mk	1.5"
#
# cmd-3b2.mk
#
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Du3b2
LDFLAGS = -s
INS = install
FRC =

DFILES =\
	absunix.o \
	dlabelit.o \
	dswap.o \
	fsys.o \
	pdinfo.o \
	scat.o \
	pnewboot.o \
	ttyset.o

NOSHFILES =\
	pnewboot |\
	absunix

SHFILES =\
	dlabelit |\
	dswap |\
	fsys |\
	pdinfo |\
	scat |\
	ttyset


FILES = absunix dlabelit dswap fsys pdinfo scat pnewboot ttyset 
all:  mkfs $(DFILES) cpio lbin

install:  all 
	cp absunix \
	   dlabelit \
	   dswap \
	   fsys \
	   pdinfo \
	   scat \
	   pnewboot \
	   ttyset	$(ROOT)/mkfs/inst/bin


.c.o:
	@-case $* in \
		$(SHFILES))\
			echo "\t$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $* $<";\
			$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $* $<;\
			;;\
		$(NOSHFILES))\
			echo "\t$(CC) $(CFLAGS) $(LDFLAGS) -o $* $<";\
			$(CC) $(CFLAGS) $(LDFLAGS) -o $* $<;\
			;;\
		*)\
			;;\
		esac


mkfs:
		if [ ! -d $(ROOT)/mkfs ];then mkdir $(ROOT)/mkfs; fi;


cpio:
		find inst -print | cpio -pdum $(ROOT)/mkfs

lbin:
		make -f lbin.mk install

clean:
	-rm -f *.o

clobber:    clean
	-rm -f $(FILES)

FRC:

#
# Header dependencies
#

absunix.o: absunix.c \
	$(INCRT)/aouthdr.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/scnhdr.h \
	$(FRC)

dlabelit.o: dlabelit.c \
	$(INCRT)/sys/param.h \
        $(INCRT)/sys/types.h \
	$(INCRT)/signal.h \
	$(INCRT)/sys/filsys.h \
	$(FRC)



dswap.o: dswap.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)



fsys.o: fsys.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/filsys.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)



pdinfo.o: pdinfo.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)


soat.o: scat.c \
	$(INCRT)/fcntl.h
	$(FRC)


ttyset.o: ttyset.c \
	$(INCRT)/sys/termio.h \
	$(INCRT)/termio.h \
	$(FRC)

pnewboot.o: pnewboot.c \
	$(INCRT)/a.out.h \
	$(INCRT)/aouthdr.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/linenum.h \
	$(INCRT)/nlist.h \
	$(INCRT)/reloc.h \
	$(INCRT)/scnhdr.h \
	$(INCRT)/storclass.h \
	$(INCRT)/syms.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)
