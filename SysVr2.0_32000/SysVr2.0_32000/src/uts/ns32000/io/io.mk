# @(#)io.mk	6.6
LIBNAME = ../lib2
INCRT = /usr/include
CFLAGS = -O -I$(INCRT)
C2 = /lib/c2
FRC =

FILES =\
	$(LIBNAME)(tt0.o)\
	$(LIBNAME)(tt1.o)\
	$(LIBNAME)(tty.o)\
	$(LIBNAME)(sxt.o)\
	$(LIBNAME)(clist.o)\
	$(LIBNAME)(cons.o)\
	$(LIBNAME)(dc.o)\
	$(LIBNAME)(err.o)\
	$(LIBNAME)(mem.o)\
	$(LIBNAME)(sys.o)\
	$(LIBNAME)(partab.o)\
	$(LIBNAME)(sio.o)\
	$(LIBNAME)(nscphys.o)\
	$(LIBNAME)(pio.o)\
	$(LIBNAME)(lp.o)\
	$(LIBNAME)(tc.o)



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

$(LIBNAME)(clist.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/tty.h\
	$(FRC)
$(LIBNAME)(cons.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/cons.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/page.h\
	$(FRC)
$(LIBNAME)(dc.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dcu.h\
	$(INCRT)/sys/disk.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)
$(LIBNAME)(err.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/utsname.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/erec.h\
	$(FRC)
$(LIBNAME)(lp.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/lprio.h\
	$(FRC)
$(LIBNAME)(mem.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)
$(LIBNAME)(nscphys.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/mmu.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)
$(LIBNAME)(sio.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/sio.h\
	$(FRC)
$(LIBNAME)(sxt.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/seg.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/ttold.h\
	$(INCRT)/sys/sxt.h\
	$(FRC)
$(LIBNAME)(sys.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/proc.h\
	$(FRC)
$(LIBNAME)(partab.o):\
	$(FRC)
$(LIBNAME)(tc.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/mtio.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)
$(LIBNAME)(tt0.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/crtctl.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)
$(LIBNAME)(tt1.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)
$(LIBNAME)(tty.o):\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/ttold.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)
$(LIBNAME)(pio.o):\
	$(INCRT)/sys/page.h\
	$(INCRT)/sys/mmu.h\
	$(FRC)

FRC:
