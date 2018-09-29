# @(#)cf.mk	1.4
SYS = unix
NODE = unix
REL = sysV
VER = gdht
MACH = pdp11-70

TYPE = id
SEGF = SEGF

INS = /etc/install
INSDIR = /
INCRT = /usr/include
SRC = /usr/src
CFLAGS = -O -I$(INCRT)
FRC =
NAME = $(SYS)$(VER)

all:	init ../$(NAME)
	@-chown bin conf.c low.s conf.o low.o linesw.o name.o
	@-chgrp bin conf.c low.s conf.o low.o linesw.o name.o
	@-chmod 664 conf.o low.o linesw.o name.o

init:
	@if [ $(TYPE) = i ] ;\
	then \
		echo UNIX 5.0 requires separate I/D ;\
		exit 1 ; \
	fi
	cd $(SRC)/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)"
	@-$(CC) $(CFLAGS) -c -DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" name.c

../$(NAME): low.o conf.o ../mch_$(TYPE).o ../lib[0-9] linesw.o name.o
	-./led.sh ../$(NAME) $(SEGF) $(TYPE)

clean:
	cd $(SRC)/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clean
	-rm -f *.o

clobber:	clean
	cd $(SRC)/uts; make -f uts.mk "INS=$(INS)" "INSDIR=$(INSDIR)" \
		"INCRT=$(INCRT)" "FRC=$(FRC)" "SYS=$(SYS)" \
		"NODE=$(NODE)" "REL=$(REL)" "VER=$(VER)" "MACH=$(MACH)"\
		"TYPE=$(TYPE)" clobber
	-rm -f *.o

conf.o:\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/callo.h\
	$(INCRT)/sys/csi.h\
	$(INCRT)/sys/csihdw.h\
	$(INCRT)/sys/da.h\
	$(INCRT)/sys/dmk.h\
	$(INCRT)/sys/du.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/err.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/io.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/maus.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/nsc.h\
	$(INCRT)/sys/opt.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/pcl.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/space.h\
	$(INCRT)/sys/st.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/sys/trace.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/vp.h\
	$(INCRT)/sys/vpmt.h\
	$(INCRT)/sys/x25.h\
	$(FRC)

linesw.o:\
	$(INCRT)/sys/conf.h\
	$(FRC)

low.o:\
	$(FRC)

name.o:\
	$(INCRT)/sys/utsname.h\
	$(FRC)

install:	all
	$(INS) -f $(INSDIR) "../$(SYS)$(VER)"

FRC:
