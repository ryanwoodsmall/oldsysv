# @(#)pwb.mk	6.1
LIBNAME = ../lib3
INCRT = /usr/include
CFLAGS = -O -I$(INCRT)
FRC =

FILES =\
	$(LIBNAME)(pcl.o)\
	$(LIBNAME)(prof.o)

IFILES =\
	pcl

all:	$(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)
	-chown bin $(LIBNAME)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)

.c.a:
	@-case $* in \
	$(IFILES))\
		echo "\t$(CC) -I$(INCRT) -S $<";\
		$(CC) -I$(INCRT) -S $<;\
		echo "\t/lib/c2 -y $*.s $*.os";\
		/lib/c2 -y $*.s $*.os;\
		echo "\tas -o $*.o $*.os";\
		as -o $*.o $*.os;\
		echo "\trm -f $*.s $*.os";\
		rm -f $*.s $*.os;\
		;;\
	*)\
		echo "\t$(CC) -c $(CFLAGS) $<";\
		$(CC) -c $(CFLAGS) $<;\
		;;\
	esac
	ar rv $@ $*.o
	-rm -f $*.o

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
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/psl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/buf.h\
	$(FRC)

FRC:
