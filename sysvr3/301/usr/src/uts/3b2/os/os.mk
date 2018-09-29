#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:os/os.mk	10.19"
#
#		Copyright 1984 AT&T
#

ROOT = 
STRIP = strip
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
LIBNAME = ../lib.os
DASHG = 
DASHO = -O
CFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG) $(DASHO)
PFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG)
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
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(bio.o): bio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(bitmasks.o): bitmasks.c \
	$(FRC)

$(LIBNAME)(clock.o): clock.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

DISP:	../disp.o $(MASTERD)/disp
	$(MKBOOT) -m $(MASTERD) -d . ../disp.o

../disp.o: disp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)
	$(CC) $(CFLAGS) -c disp.c
	mv disp.o ../disp.o	# save this, but not here
	
$(LIBNAME)(exec.o): exec.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(exit.o): exit.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(fault.o): fault.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(fio.o): fio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(flock.o): flock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/flock.h \
	$(FRC)

$(LIBNAME)(fork.o): fork.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(fstyp.o): fstyp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(getpages.o): getpages.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(getsizes.o): getsizes.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/iobuf.h \
	$(FRC)

$(LIBNAME)(grow.o): grow.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/bitmasks.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sbd.h \
	$(FRC)

$(LIBNAME)(iget.o): iget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(lock.o): lock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/lock.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(machdep.o): machdep.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sit.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(main.o): main.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(malloc.o): malloc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(mmgt.o): mmgt.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(move.o): move.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

$(LIBNAME)(name.o): name.c \
	$(FRC)

$(LIBNAME)(nami.o): nami.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(page.o): page.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(physio.o): physio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(pipe.o): pipe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(prf.o): prf.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/panregs.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/sit.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(probe.o): probe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(region.o): region.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sched.o): sched.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(sdt.o): sdt.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(sig.o): sig.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(slp.o): slp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(space.o): space.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/gdpstr.h \
	$(FRC)

$(LIBNAME)(startup.o): startup.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/boot.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(streamio.o): streamio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(subr.o): subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(swapalloc.o): swapalloc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(swtch.o): swtch.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(sys2.o): sys2.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sys3.o): sys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(sys3b.o): sys3b.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/iu.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/boothdr.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/gate.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/buf.h \
	$(FRC)

$(LIBNAME)(sys4.o): sys4.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/clock.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sysent.o): sysent.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

$(LIBNAME)(text.o): text.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(todc.o): todc.c \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/todc.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(trap.o): trap.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/sit.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/mau.h \
	$(FRC)

$(LIBNAME)(utssys.o): utssys.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(v8nami.o): v8nami.c \
	$(INCRT)/../h/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/../h/systm.h \
	$(INCRT)/../h/inode.h \
	$(INCRT)/../h/mount.h \
	$(INCRT)/../h/dir.h \
	$(INCRT)/../h/user.h \
	$(INCRT)/../h/buf.h \
	$(INCRT)/../h/conf.h \
	$(FRC)
