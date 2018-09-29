#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:os/os.mk	10.19.8.5"
#
#		Copyright 1984 AT&T
#

ROOT = 
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
LIBNAME = ../lib.os
DASHO = -O
CFLAGS= -I$(INC) -DINKERNEL $(MORECPP) $(DASHO)
PFLAGS= -I$(INC) -DINKERNEL $(MORECPP)
FRC =

FILES =\
	$(LIBNAME)(acct.o) \
	$(LIBNAME)(bio.o) \
	$(LIBNAME)(bitmasks.o) \
	$(LIBNAME)(clock.o) \
	$(LIBNAME)(exec.o) \
	$(LIBNAME)(exit.o) \
	$(LIBNAME)(fault.o) \
	$(LIBNAME)(fio.o) \
	$(LIBNAME)(flock.o) \
	$(LIBNAME)(fork.o) \
	$(LIBNAME)(getpages.o) \
	$(LIBNAME)(getsizes.o) \
	$(LIBNAME)(grow.o) \
	$(LIBNAME)(iget.o) \
	$(LIBNAME)(lock.o) \
	$(LIBNAME)(machdep.o) \
	$(LIBNAME)(main.o) \
	$(LIBNAME)(malloc.o) \
	$(LIBNAME)(mmgt.o) \
	$(LIBNAME)(move.o) \
	$(LIBNAME)(nami.o) \
	$(LIBNAME)(page.o) \
	$(LIBNAME)(physio.o) \
	$(LIBNAME)(pipe.o) \
	$(LIBNAME)(prf.o) \
	$(LIBNAME)(probe.o) \
	$(LIBNAME)(region.o) \
	$(LIBNAME)(sched.o) \
	$(LIBNAME)(sdt.o) \
	$(LIBNAME)(sig.o) \
	$(LIBNAME)(slp.o) \
	$(LIBNAME)(space.o) \
	$(LIBNAME)(startup.o) \
	$(LIBNAME)(streamio.o) \
	$(LIBNAME)(subr.o) \
	$(LIBNAME)(swapalloc.o) \
	$(LIBNAME)(swtch.o) \
	$(LIBNAME)(sys2.o) \
	$(LIBNAME)(sys3.o) \
	$(LIBNAME)(sys3b.o) \
	$(LIBNAME)(sys4.o) \
	$(LIBNAME)(sysent.o) \
	$(LIBNAME)(text.o) \
	$(LIBNAME)(todc.o) \
	$(LIBNAME)(trap.o) \
	$(LIBNAME)(utssys.o) \
	$(LIBNAME)(fstyp.o)


#
# Files for avoiding optimization
#

NOPFILES = \
	prf


all:	name.o DISP pick  $(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

# included for development, should be removed for distribution

pick:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o ; \
	fi

$(LIBNAME):	$(FILES)
	ar ruv $(LIBNAME) *.o ;
	rm -f *.o ;

name.o:
	$(CC) $(CFLAGS) -c name.c \
		-DREL=`expr '"$(REL)' : '\(..\{0,8\}\)'`\" \
		-DVER=`expr '"$(VER)' : '\(..\{0,8\}\)'`\"
	mv name.o ..

.c.a:
	@case $* in \
	$(NOPFILES))\
		echo "\t$(CC) $(PFLAGS) -c $<";\
		$(CC) $(PFLAGS) -c $<;\
		exit $$?;\
		;;\
	getsizes) \
		echo "\t$(CC) $(PFLAGS) -c -g $<"; \
		$(CC) $(PFLAGS) -c -g $<; \
		if [ $$? -ne 0 ]; \
		then \
			exit $$?; \
		fi; \
		echo "\tcp $*.o ../$*.o"; \
		cp $*.o ../$*.o; \
		exit $$?; \
		;; \
	*)\
		echo "\t$(CC) $(CFLAGS) -c $<";\
		$(CC) $(CFLAGS) -c $<;\
		exit $$?;\
		;;\
	esac

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:
	-rm -f $(LIBNAME)
	ar qc $(LIBNAME)

#
# Header dependencies
#

$(LIBNAME)(acct.o): acct.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/file.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(FRC)

$(LIBNAME)(bio.o): bio.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/rbuf.h \
	$(FRC)

$(LIBNAME)(bitmasks.o): bitmasks.c \
	$(FRC)

$(LIBNAME)(clock.o): clock.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/callo.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/var.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/map.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/inline.h \
	$(FRC)

DISP:	../disp.o $(MASTERD)/disp
	$(MKBOOT) -m $(MASTERD) -d . ../disp.o

../disp.o: disp.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/var.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)
	$(CC) $(CFLAGS) -c disp.c
	mv disp.o ../disp.o	# save this, but not here

$(LIBNAME)(exec.o): exec.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/map.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/var.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/message.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(exit.o): exit.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/map.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/var.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(fault.o): fault.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/var.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/getpages.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(fio.o): fio.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/file.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/open.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(flock.o): flock.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/user.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(FRC)

$(LIBNAME)(fork.o): fork.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/map.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(fstyp.o): fstyp.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(getpages.o): getpages.c \
	$(INC)/sys/types.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/getpages.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(getsizes.o): getsizes.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/callo.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/var.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/msg.h \
	$(INC)/sys/sem.h \
	$(INC)/sys/shm.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/fs/s5fblk.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/adv.h \
	$(INC)/sys/nserve.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/comm.h \
	$(INC)/sys/dirent.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/message.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/stream.h \
	$(FRC)

$(LIBNAME)(grow.o): grow.c \
	$(INC)/sys/types.h \
	$(INC)/sys/bitmasks.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/var.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/sbd.h \
	$(FRC)

$(LIBNAME)(iget.o): iget.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/file.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/var.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/open.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/rbuf.h \
	$(FRC)

$(LIBNAME)(lock.o): lock.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/lock.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(machdep.o): machdep.c \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sit.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/user.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/var.h \
	$(INC)/sys/message.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(main.o): main.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/var.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(malloc.o): malloc.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/map.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(FRC)

$(LIBNAME)(mmgt.o): mmgt.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/map.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(move.o): move.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/file.h \
	$(INC)/sys/systm.h \
	$(FRC)

$(LIBNAME)(name.o): name.c \
	$(FRC)

$(LIBNAME)(nami.o): nami.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(page.o): page.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/var.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/map.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(physio.o): physio.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/map.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/getpages.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(pipe.o): pipe.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/file.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(prf.o): prf.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/panregs.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/sit.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(probe.o): probe.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(region.o): region.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/var.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/shm.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/message.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sched.o): sched.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/var.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/getpages.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/buf.h \
	$(FRC)

$(LIBNAME)(sdt.o): sdt.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/var.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(sig.o): sig.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/file.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/var.h \
	$(INC)/sys/map.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/message.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(slp.o): slp.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/user.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/map.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(space.o): space.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/acct.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/rbuf.h \
	$(FRC)

$(LIBNAME)(startup.o): startup.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/boot.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/var.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(streamio.o): streamio.c \
	$(INC)/sys/types.h \
	$(INC)/sys/file.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/strstat.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/var.h \
	$(INC)/sys/poll.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(subr.o): subr.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/var.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(FRC)

$(LIBNAME)(swapalloc.o): swapalloc.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/getpages.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/map.h \
	$(INC)/sys/open.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/file.h \
	$(FRC)

$(LIBNAME)(swtch.o): swtch.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/var.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(sys2.o): sys2.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/message.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/var.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/poll.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sys3.o): sys3.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/file.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/statfs.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/var.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/open.h \
	$(INC)/sys/message.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(FRC)

$(LIBNAME)(sys3b.o): sys3b.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/boothdr.h \
	$(INC)/sys/uadmin.h \
	$(INC)/sys/map.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/gate.h \
	$(INC)/sys/var.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/fstyp.h \
	$(FRC)

$(LIBNAME)(sys4.o): sys4.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/var.h \
	$(INC)/sys/clock.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sysent.o): sysent.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/systm.h \
	$(FRC)

$(LIBNAME)(text.o): text.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

$(LIBNAME)(todc.o): todc.c \
	$(INC)/sys/sbd.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/todc.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(trap.o): trap.c \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/sit.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/var.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/mau.h \
	$(FRC)

$(LIBNAME)(utssys.o): utssys.c \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/uadmin.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/ustat.h \
	$(INC)/sys/statfs.h \
	$(INC)/sys/file.h \
	$(INC)/sys/var.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC)
