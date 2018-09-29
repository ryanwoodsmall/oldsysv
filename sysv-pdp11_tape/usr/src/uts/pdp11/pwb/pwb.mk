# @(#)pwb.mk	1.4
LIBNAME = ../lib3
INCRT = /usr/include
CFLAGS = -O -I$(INCRT)
FRC =

FILES =\
	$(LIBNAME)(pcl.o)\
	$(LIBNAME)(prof.o)

all:	$(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)
	strip $(LIBNAME)
	-chown bin $(LIBNAME)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

$(LIBNAME)(pcl.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/pcl.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/map.h\
	$(FRC)

$(LIBNAME)(prof.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/map.h\
	prfintr.s\
	$(FRC)
	$(CC) -c $(CFLAGS) prof.c prfintr.s
	-ld -x -r prof.o prfintr.o
	mv a.out prof.o
	ar rv $(LIBNAME) prof.o
	-rm -f prof.o prfintr.o
