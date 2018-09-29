#	@(#)makefile	1.2
#
# makefile for libc/pdp11
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#
# The environment variable EXSIM should be set to "-" in the make
# command line by the reassignment EXSIM=-  if the pdp11 Floating
# Point Simulator is to be excluded from the library.
#

.SUFFIXES: .p

M4=m4 m4.def
CC=cc
CFLAGS=-O
PROF=
EXSIM=@:

OBJECTS=\
crt/aldiv.o       crt/almul.o       crt/alrem.o       crt/auldiv.o	\
crt/aulrem.o      crt/cerror.o      crt/cret.o        crt/csav.o	\
crt/csv.o         crt/ldiv.o        crt/lmul.o        crt/lrem.o	\
crt/mcount.o      crt/udiv.o        crt/uldiv.o       crt/ulrem.o	\
csu/crt0.o        csu/fcrt0.o       csu/fmcrt0.o      csu/mcrt0.o	\
gen/abort.o       gen/cuexit.o      gen/drand48.o     \
gen/fp.o          gen/frexp.o       gen/ldexp.o       gen/ldfps.o	\
gen/modf.o        gen/setjmp.o      gen/tolower.o     gen/toupper.o	\
print/cvt.o       print/doprnt.o    print/ffltpr.o    print/fltpr.o	\
print/strout.o    sys/access.o      \
sys/acct.o        sys/alarm.o       sys/chdir.o       sys/chmod.o	\
sys/chown.o       sys/chroot.o      sys/close.o       sys/creat.o	\
sys/dup.o         sys/execl.o       sys/execle.o      sys/execv.o	\
sys/execve.o      sys/exit.o        sys/fcntl.o       sys/fork.o	\
sys/fstat.o       sys/getegid.o     sys/geteuid.o     sys/getgid.o	\
sys/getpid.o      sys/getppid.o     sys/getuid.o      sys/gtty.o	\
sys/ioctl.o       sys/kill.o        sys/link.o        sys/lseek.o	\
sys/maus.o        sys/mknod.o       sys/mount.o       sys/nice.o	\
sys/open.o        sys/pause.o       sys/pipe.o        sys/plock.o	\
sys/profil.o      sys/ptrace.o      sys/read.o        sys/sbrk.o	\
sys/setgid.o      sys/setpgrp.o     sys/setuid.o      sys/signal.o	\
sys/stat.o        sys/stime.o       sys/stty.o        sys/sync.o	\
sys/syscall.o     sys/time.o        sys/times.o       sys/ulimit.o	\
sys/umask.o       sys/umount.o      sys/uname.o       sys/unlink.o	\
sys/ustat.o       sys/utime.o       sys/wait.o        sys/write.o

POBJECTS=\
crt/aldiv.p       crt/almul.p       crt/alrem.p       crt/auldiv.p	\
crt/aulrem.p      crt/cerror.p      crt/cret.p        crt/csav.p	\
crt/csv.p         crt/ldiv.p        crt/lmul.p        crt/lrem.p	\
crt/mcount.p      crt/udiv.p        crt/uldiv.p       crt/ulrem.p	\
csu/crt0.p        csu/fcrt0.p       csu/fmcrt0.p      csu/mcrt0.p	\
gen/abort.p       gen/cuexit.p      gen/drand48.p     \
gen/fp.p          gen/frexp.p       gen/ldexp.p       gen/ldfps.p	\
gen/modf.p        gen/setjmp.p      gen/tolower.p     gen/toupper.p	\
print/cvt.p       print/doprnt.p    print/ffltpr.p    print/fltpr.p	\
print/strout.p    sys/access.p      \
sys/acct.p        sys/alarm.p       sys/chdir.p       sys/chmod.p	\
sys/chown.p       sys/chroot.p      sys/close.p       sys/creat.p	\
sys/dup.p         sys/execl.p       sys/execle.p      sys/execv.p	\
sys/execve.p      sys/exit.p        sys/fcntl.p       sys/fork.p	\
sys/fstat.p       sys/getegid.p     sys/geteuid.p     sys/getgid.p	\
sys/getpid.p      sys/getppid.p     sys/getuid.p      sys/gtty.p	\
sys/ioctl.p       sys/kill.p        sys/link.p        sys/lseek.p	\
sys/maus.p        sys/mknod.p       sys/mount.p       sys/nice.p	\
sys/open.p        sys/pause.p       sys/pipe.p        sys/plock.p	\
sys/profil.p      sys/ptrace.p      sys/read.p        sys/sbrk.p	\
sys/setgid.p      sys/setpgrp.p     sys/setuid.p      sys/signal.p	\
sys/stat.p        sys/stime.p       sys/stty.p        sys/sync.p	\
sys/syscall.p     sys/time.p        sys/times.p       sys/ulimit.p	\
sys/umask.p       sys/umount.p      sys/uname.p       sys/unlink.p	\
sys/ustat.p       sys/utime.p       sys/wait.p        sys/write.p

all:
	make -e $(IGN) nonprof
	$(PROF)make -e $(IGN) prof NONPROF=@#

nonprof: $(OBJECTS)

prof: $(POBJECTS)

.c.o .c.p:
	@echo $*.c:
	$(NONPROF)$(CC) $(CFLAGS) -c $*.c  && mv $(*F).o $*.o
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c  && mv $(*F).o $*.p

.s.o .s.p:
	@echo $*.s:
	$(NONPROF)$(M4) -DMCOUNT='/'  $*.s   > $(*F).s   && \
	$(NONPROF)$(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.o
	$(PROF)$(M4) mcount.def  $*.s   > $(*F).s   && \
	$(PROF)$(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.p
	-rm $(*F).s

archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	# Delete the Floating Point Simulator if it is to be excluded.
	$(EXSIM) rm fp.o
	$(EXSIM) echo Floating Point Simulator Excluded
	#
	# figure out the correct ordering for all the archive modules,
	#	and leave the ordered list of modules names in "objlist".
	sh ../pdp11/make.order
	#
	# build the archive with the modules in correct order.
	$(AR) q ../lib.libc `cat objlist`
	$(STRIP) ../lib.libc
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
	$(PROF)$(AR) q ../libp.libc `cat objlist`
	$(PROF)$(STRIP) ../libp.libc

clean:
	-rm -f *.[ops]

clobber: clean
	-rm -f */*.[op]
