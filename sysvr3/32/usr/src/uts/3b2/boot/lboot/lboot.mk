#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:boot/lboot/makefile	10.5.1.5"

ROOT=
INC= $(ROOT)/usr/include
UINC= $(ROOT)/usr/include
INCLOC= ..
INS=	install
LINT=	lint -n
MAKE=	make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
PR=	pr
STRIP=	strip
SZ=	$(DUMP) -vfh
XCL=	xcl -x -2 -pmini
XREF=	cxref -c -t -s

#TEST=	-DTEST \
	-DDEBUG1 \
	-DDEBUG1a -DDEBUG1b -DDEBUG1c -DDEBUG1d -DDEBUG1e \
	-DDEBUG1f -DDEBUG1g -DDEBUG1h -DDEBUG1i \
	-DDEBUG2

CPPOPT=	-I$(INCLOC) -I$(INC) -Uvax -Uu3b15 -Du3b2 -Updp11 $(TEST) $(MORECPP)
DASHO = -O
CFLAGS=	$(DASHO) $(CPPOPT)
#LDFLAGS=-m
LDFLAGS=
#LBLD=LBLD
LBLD=lbld
FRC=

HEADERS=$(INCLOC)/sys/dir.h \
	$(INCLOC)/sys/filsys.h \
	$(INCLOC)/sys/inode.h \
	$(INCLOC)/sys/iobuf.h \
	$(INCLOC)/sys/param.h \
	io.h \
	lboot.h \
	error.h
ASOURCE=misc.s
CSOURCE=main.c basicio.c clibrary.c dfcstand.c loadunix.c on.c subr.c tables.c
SOURCE=	errortable $(CSOURCE) $(ASOURCE)
OBJECT=	main.o basicio.o clibrary.o dfcstand.o error.o loadunix.o on.o \
	subr.o tables.o misc.o

lboot:	$(LBLD) $(OBJECT) version.o $(FRC)
	@-if [ '$(TEST)' ]; \
	then \
		$(MAKE) version.o; \
		echo '\t$(LD) $(LDFLAGS) $(LBLD) $(OBJECT) version.o -o lboot -lc'; \
		$(LD) $(LDFLAGS) $(LBLD) $(OBJECT) version.o -o lboot -lc; \
		rm -f version.[co]; \
	else \
		echo '\t$(LD) $(LDFLAGS) $(LBLD) $(OBJECT) -o lboot -lc'; \
		$(LD) $(LDFLAGS) $(LBLD) $(OBJECT) -o lboot -lc; \
	fi

lint:	$(HEADERS) $(CSOURCE) FRC
	$(LINT) $(CPPOPT) $(CSOURCE) | \
		sed -e /_0fp/d -e /Sccsid/d -e '/[a-z]_h[ 	]/d' >lint

xref:	$(HEADERS) $(CSOURCE) FRC
	$(XREF) $(CPPOPT) $(CSOURCE) -o xref

error.c:errortable
	-echo '#include <sys/types.h>' >error.c; \
	echo '#include <sys/dir.h>' >>error.c; \
	echo '#include "lboot.h"' >>error.c; \
	echo '#include "error.h"' >>error.c; \
	echo '\nstruct errortab errortab[] = {' >>error.c; \
	echo '/^[ \t]*$$/d' >sed$$$$; \
	echo '/^#/d' >>sed$$$$; \
	echo 's/^[ \t]*//' >>sed$$$$; \
	echo 't reset' >>sed$$$$; \
	echo ': reset' >>sed$$$$; \
	echo 's/^\\([^ \t]*\\)[ \t][ \t]*\\([^ \t]*\\)[ \t][ \t]*\\(".*[^\\\\]"\\)/\t{ \\1, \\2, \\3 },/p' >>sed$$$$; \
	echo 't done' >>sed$$$$; \
	echo 's/^\\([^ \t]*\\)[ \t][ \t]*\\([^ \t]*\\)/\t{ \\1, \\2, NULL },/p' >>sed$$$$; \
	echo ': done' >>sed$$$$; \
	$(CC) -E -P $(CPPOPT) errortable | sed -f sed$$$$ >>error.c; \
	rm -f sed$$$$; \
	echo '\t{ 0 } };' >>error.c

error.h:errortable
	-echo '/^[ \t]*$$/d' >sed$$$$; \
	echo '/^#/d' >>sed$$$$; \
	echo 's/^[ \t]*//' >>sed$$$$; \
	echo 's/[ \t].*//' >>sed$$$$; \
	echo 's/^/__DEF__\t/' >>sed$$$$; \
	echo 's/$$/\t__LINE__/' >>sed$$$$; \
	echo '#define __DEF__ #define\n# 1 "error.h"' >temp$$$$; \
	$(CC) -E -P $(CPPOPT) errortable | sed -f sed$$$$ >>temp$$$$; \
	$(CC) -E -P temp$$$$ >error.h; \
	rm -f temp$$$$ sed$$$$

version.o:version.c
version.c:FRC
	@TEMP=/tmp/$$$$; \
	echo 's/Sun/Sunday/\ns/Mon/Monday/\ns/Tue/Tuesday/\ns/Wed/Wednesday/\ns/Thu/Thursday/\ns/Fri/Friday/\ns/Sat/Saturday/' > $$TEMP; \
	echo 's/Jan/January/\ns/Feb/February/\ns/Mar/March/\ns/Apr/April/\ns/Jun/June/\ns/Jul/July/\ns/Aug/August/\ns/Sep/Sepember/\ns/Oct/October/\ns/Nov/November/\ns/Dec/December/' >> $$TEMP; \
	VERSION=`date '+%r, %a, %h %d, 19%y' | sed -f $$TEMP`; \
	rm -f $$TEMP; \
	echo "\tVERSION=$$VERSION"; \
	echo "char debug_version[] ={\"$$VERSION\"};" >version.c

install: lboot
	$(INS) -f $(ROOT)/lib lboot
	$(STRIP) $(ROOT)/lib/lboot

clean:
	rm -f lboot *.o error.[ch] version.c LISTING tags lint xref

clobber: clean

tags:	$(HEADERS) $(CSOURCE)
	ctags $(HEADERS) $(CSOURCE)

debug:	lboot FRC
	$(DIS) -t .text -da .data -L lboot | newform -i -o -l150 > lboot.s
	$(NM) -x -nef lboot | newform -i -o -l150 > lboot.name
	$(SZ) lboot | newform -i -o -l150 > lboot.map
	($(PR) -h "LBOOT.MAP" lboot.map; \
	$(PR) -h "LBOOT" lboot.s lboot.name) >LISTING
	rm -f lboot.s lboot.name lboot.map

print:	.print
.print:	Makefile $(LBLD) $(INC)/sys/boothdr.h $(HEADERS) $(SOURCE)
	$(XCL) $?
	touch .print

.PRECIOUS: \
	.print

FRC:

#
# Header dependencies
#

basicio.o: basicio.c \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/errno.h \
	$(INCLOC)/sys/filsys.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/ino.h \
	$(INCLOC)/sys/inode.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	io.h \
	lboot.h \
	$(FRC)

clibrary.o: clibrary.c \
	$(UINC)/ctype.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/firmware.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	error.h \
	io.h \
	lboot.h \
	$(FRC)

dfcstand.o: dfcstand.c \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/if.h \
	$(INCLOC)/sys/iobuf.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	error.h \
	io.h \
	lboot.h \
	$(FRC)

error.o: error.c \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	error.h \
	lboot.h \
	$(FRC)

loadunix.o: loadunix.c \
	$(UINC)/a.out.h \
	$(UINC)/aouthdr.h \
	$(UINC)/filehdr.h \
	$(UINC)/linenum.h \
	$(UINC)/nlist.h \
	$(UINC)/reloc.h \
	$(UINC)/scnhdr.h \
	$(UINC)/storclass.h \
	$(UINC)/syms.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/boothdr.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/extbus.h \
	$(INC)/sys/firmware.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	error.h \
	lboot.h \
	$(FRC)

main.o: main.c \
	$(UINC)/a.out.h \
	$(UINC)/aouthdr.h \
	$(UINC)/filehdr.h \
	$(UINC)/linenum.h \
	$(UINC)/nlist.h \
	$(UINC)/reloc.h \
	$(UINC)/scnhdr.h \
	$(UINC)/storclass.h \
	$(UINC)/syms.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/todc.h \
	$(INC)/sys/types.h \
	error.h \
	lboot.h \
	$(FRC)

misc.o: misc.s \
	$(FRC)

on.o: on.c \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	lboot.h \
	$(FRC)

subr.o: subr.c \
	$(UINC)/a.out.h \
	$(UINC)/aouthdr.h \
	$(UINC)/ctype.h \
	$(UINC)/filehdr.h \
	$(UINC)/linenum.h \
	$(UINC)/nlist.h \
	$(UINC)/reloc.h \
	$(UINC)/scnhdr.h \
	$(UINC)/storclass.h \
	$(UINC)/syms.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	error.h \
	lboot.h \
	$(FRC)

tables.o: tables.c \
	$(UINC)/a.out.h \
	$(UINC)/aouthdr.h \
	$(UINC)/filehdr.h \
	$(UINC)/linenum.h \
	$(UINC)/nlist.h \
	$(UINC)/reloc.h \
	$(UINC)/scnhdr.h \
	$(UINC)/storclass.h \
	$(UINC)/syms.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/boothdr.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/csr.h \
	$(INCLOC)/sys/dir.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INCLOC)/sys/param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	error.h \
	lboot.h \
	$(FRC)
