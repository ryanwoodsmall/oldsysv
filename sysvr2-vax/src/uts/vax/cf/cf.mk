# @(#)cf.mk	6.7
SYS = unix
NODE = unix
REL = 2.0v2
VER = gdht
MACH = vax-780

INCRT = /usr/include
INS = /etc/install
INSDIR = /
CFLAGS = -O -I$(INCRT)
FRC =
NAME = $(SYS)$(VER)
TYPE = vax

all:	init ../$(NAME)
fast:	../$(NAME)
	@-chown bin conf.c univec.c conf.o univec.o linesw.o name.o
	@-chgrp bin conf.c univec.c conf.o univec.o linesw.o name.o
	@-chmod 664 conf.o univec.o linesw.o name.o

init:
	cd /usr/src/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)"
	@-$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		name.c

../$(NAME): univec.o conf.o ../lib[0-9] ../locore.o linesw.o name.o
	-/bin/ld -o ../$(NAME) ld.file -e start -x ../locore.o \
		 conf.o univec.o linesw.o ../lib[0-9] name.o
	@-chmod 755 ../$(NAME)
	@-chown bin ../$(NAME)
	@-chgrp bin ../$(NAME)

clean:
	cd /usr/src/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clean
	-rm -f *.o

clobber:	clean
	cd /usr/src/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clobber
	-rm -f ../$(NAME) univec.c conf.c

conf.o:\
	config.h\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/callo.h\
	$(INCRT)/sys/csi.h\
	$(INCRT)/sys/csihdw.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dmk.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/err.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/io.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/nsc.h\
	$(INCRT)/sys/nscdev.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/space.h\
	$(INCRT)/sys/bx25.space.h\
	$(INCRT)/sys/bx25sl.h\
	$(INCRT)/sys/st.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/trace.h\
	$(INCRT)/sys/uba.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/vpmt.h\
	$(INCRT)/sys/x25.h\
	$(INCRT)/sys/em.h\
	$(FRC)

name.o:\
	$(INCRT)/sys/utsname.h\
	$(FRC)

univec.o:\
	$(FRC)

linesw.o:\
	config.h\
	$(INCRT)/sys/conf.h\
	$(FRC)

install:	all
	$(INS) -f $(INSDIR) "../$(SYS)$(VER)"

FRC:
