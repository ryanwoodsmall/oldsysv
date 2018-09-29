#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:boot/olboot/makefile	10.5"


ROOT =
INC = $(ROOT)/usr/include
INCRT = ../..
INCLOC = ..
CFLAGS = -I$(INCLOC) -I$(INCRT) -I$(INC) -Uu3b -Uvax -Du3b2 -Updp11 $(DBO)
STRIP = strip
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
LIBNAME = libfm.a
INS = install
FRC =

.c.a:
	$(CC) $(CFLAGS) -c $<
	ar rv $@ $*.o
	-rm -f $*.o

LFILES = \
	$(LIBNAME)(basicio.o)\
	$(LIBNAME)(findfile.o)\
	$(LIBNAME)(findfs.o)\
	$(LIBNAME)(loadfile.o)\
	$(LIBNAME)(loadprog.o)

all: olboot

install: olboot
	$(STRIP) olboot
	$(INS) -f $(ROOT)/lib olboot
	$(INS) -f $(ROOT)/lib $(LIBNAME)

olboot: lbld\
	misc.o\
	lboot.o\
	$(LIBNAME)\
	$(FRC)
	$(LD) $(LDFLAG) misc.o lboot.o lbld -o olboot $(LIBNAME) -lc

$(LIBNAME): $(LFILES)

clean:
	rm -f olboot *.o

clobber: clean
	rm -f $(LIBNAME)

print:
	pr -n lboot.c lbld lboot.dis lboot.name lboot.size | \
		opr -f hole -txr -p land

debug: olboot
	$(SIZE) olboot > olboot.size
	$(DIS) -L olboot > olboot.dis
	$(NM) -nefx olboot > olboot.name
	$(STRIP) olboot

FRC:


#
# Header dependencies
#

lboot.o: lboot.c \
	$(INC)/a.out.h \
	$(INC)/aouthdr.h \
	$(INC)/filehdr.h \
	$(INC)/linenum.h \
	$(INC)/nlist.h \
	$(INC)/reloc.h \
	$(INC)/scnhdr.h \
	$(INC)/storclass.h \
	$(INC)/syms.h \
	$(INCRT)/sys/boot.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/immu.h \
	$(INCLOC)/sys/inode.h \
	$(INCLOC)/sys/iobuf.h \
	$(INCRT)/sys/lboot.h \
	$(INCRT)/sys/nvram.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(FRC)

$(LIBNAME)(basicio.o): basicio.c \
	$(INCRT)/sys/boot.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/ino.h \
	$(INCLOC)/sys/inode.h \
	$(INCLOC)/sys/iobuf.h \
	$(INCRT)/sys/lboot.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(FRC)

$(LIBNAME)(findfile.o): findfile.c \
	$(INCLOC)/sys/dir.h \
	$(INCRT)/sys/firmware.h \
	$(INCLOC)/sys/inode.h \
	$(INCRT)/sys/lboot.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)

$(LIBNAME)(findfs.o): findfs.c \
	$(INCRT)/sys/boot.h \
	$(INCRT)/sys/elog.h \
	$(INCLOC)/sys/filsys.h \
	$(INCRT)/sys/firmware.h \
	$(INCLOC)/sys/inode.h \
	$(INCLOC)/sys/iobuf.h \
	$(INCRT)/sys/lboot.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)

$(LIBNAME)(loadfile.o): loadfile.c \
	$(INC)/a.out.h \
	$(INC)/aouthdr.h \
	$(INC)/filehdr.h \
	$(INC)/linenum.h \
	$(INC)/nlist.h \
	$(INC)/reloc.h \
	$(INC)/scnhdr.h \
	$(INC)/storclass.h \
	$(INC)/syms.h \
	$(INCRT)/sys/firmware.h \
	$(INCLOC)/sys/inode.h \
	$(INCRT)/sys/lboot.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)

$(LIBNAME)(loadprog.o): loadprog.c \
	$(INC)/a.out.h \
	$(INC)/aouthdr.h \
	$(INC)/filehdr.h \
	$(INC)/linenum.h \
	$(INC)/nlist.h \
	$(INC)/reloc.h \
	$(INC)/scnhdr.h \
	$(INC)/storclass.h \
	$(INC)/syms.h \
	$(INCRT)/sys/firmware.h \
	$(INCLOC)/sys/inode.h \
	$(INCRT)/sys/lboot.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)

misc.o: misc.s \
	$(FRC)

