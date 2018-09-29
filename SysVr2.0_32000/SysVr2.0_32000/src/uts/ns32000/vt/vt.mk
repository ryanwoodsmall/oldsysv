# @(#)vt.mk	6.1
LIBNAME = ../lib4
INCRT = /usr/include
CFLAGS = -O -I$(INCRT)
FRC =
FILES =\
	$(LIBNAME)(ds40.o)\
	$(LIBNAME)(hp45.o)\
	$(LIBNAME)(tec.o)\
	$(LIBNAME)(tex.o)\
	$(LIBNAME)(vt100.o)\
	$(LIBNAME)(vt61.o)

all:	$(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)
	-chown bin $(LIBNAME)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

$(LIBNAME)(ds40.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
$(LIBNAME)(hp45.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
$(LIBNAME)(tec.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
$(LIBNAME)(tex.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
$(LIBNAME)(vt100.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
$(LIBNAME)(vt61.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/crtctl.h\
	$(FRC)
FRC:
