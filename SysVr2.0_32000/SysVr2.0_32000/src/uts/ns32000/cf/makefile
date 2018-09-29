# @(#)Makefile	6.3
SYS = unix
NODE = unix
REL = 2.0v1
VER = sys32
MACH = NS-32016

INCRT = /usr/include
INS = /etc/install
INSDIR = /
CFLAGS = -O -I$(INCRT)
FRC =
NAME = $(SYS)$(VER)
TYPE = ns32000

all:	init ../$(NAME)
fast:	../$(NAME)
	@-chown bin conf.c low.s conf.o low.o linesw.o name.o
	@-chgrp bin conf.c low.s conf.o low.o linesw.o name.o
	@-chmod 664 conf.o low.o linesw.o name.o

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

../$(NAME): low.o conf.o ../lib[0-9] ../locore.o linesw.o name.o
	-/bin/ld -o ../$(NAME) ld.file -e start -x ../locore.o \
		 conf.o low.o linesw.o ../lib[0-9] name.o
	@-chmod 755 ../$(NAME)
	@-chown bin ../$(NAME)
	@-chgrp bin ../$(NAME)

low.o:	low.s

conf.o:		conf.c

low.s conf.c:	$(NAME).d
	config $(NAME).d
	$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		name.c

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
	-rm -f ../$(NAME) low.s conf.c

conf.o:\
	config.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/space.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/acct.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/map.h\
	$(INCRT)/sys/callo.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/err.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/opt.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/pfdat.h\
	$(INCRT)/sys/tuneable.h\
	$(INCRT)/sys/swap.h\
	$(INCRT)/sys/init.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/trace.h\
	$(INCRT)/sys/ipc.h\
	$(INCRT)/sys/msg.h\
	$(INCRT)/sys/sem.h\
	$(INCRT)/sys/shm.h\
	$(INCRT)/sys/flock.h\
	$(FRC)

name.o:\
	$(INCRT)/sys/utsname.h\
	$(FRC)
	@-$(CC) $(CFLAGS) -c \
		-DSYS=\"`expr $(SYS) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		name.c

low.o:\
	$(FRC)

linesw.o:\
	config.h\
	$(INCRT)/sys/conf.h\
	$(FRC)

install:	all
	$(INS) -f $(INSDIR) "../$(SYS)$(VER)"

FRC:
