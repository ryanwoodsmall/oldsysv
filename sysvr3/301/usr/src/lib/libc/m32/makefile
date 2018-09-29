#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:makefile	1.43"
#
#  makefile for libc/m32
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

.SUFFIXES: .p

M4=m4 m4.def
CC=cc
CFLAGS=-O
PROF=
NONPROF=
INCLIBC=../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
crt/cerror.o	  crt/mcount.o	    csu/crt0.o	      csu/mcrt0.o	\
csu/fcrt0.o	  csu/fmcrt0.o	    \
csu/crt1.o	  csu/fcrt1.o       csu/fmcrt1.o      csu/mcrt1.o	\
csu/crtn.o	  \
gen/abs.o	  gen/atof.o        \
gen/biglitpow.o	  gen/cuexit.o      gen/dtop.o	      \
gen/ecvt.o	  gen/fakcu.o	    gen/frexp.o	      gen/gen_def.o     \
gen/ldexp.o	  gen/m32_data.o    \
gen/ltostr.o	  gen/memcpy.o	    gen/ptod.o 	      gen/setjmp.o	\
gen/strcpy.o      gen/strcmp.o	    gen/fptrap.o      \
gen/strlen.o	  sys/access.o      sys/acct.o	      \
sys/alarm.o	  sys/chdir.o	    sys/chmod.o	      sys/chown.o	\
sys/chroot.o	  sys/close.o	    sys/creat.o	      sys/dup.o		\
sys/execl.o	  sys/execle.o	    sys/execv.o       \
sys/execve.o	  sys/exit.o	    sys/fcntl.o	      sys/fork.o	\
sys/fstat.o	  sys/getegid.o	    sys/geteuid.o     sys/getgid.o	\
sys/getpid.o	  sys/getppid.o	    sys/getuid.o      sys/gtty.o	\
sys/ioctl.o	  sys/kill.o	    sys/link.o	      sys/lseek.o	\
sys/mknod.o	  sys/mount.o	    sys/msgsys.o      sys/nice.o	\
sys/open.o	  sys/pause.o	    sys/pipe.o	      sys/profil.o	\
sys/plock.o	  sys/uadmin.o	    \
sys/ptrace.o	  sys/read.o	    sys/sbrk.o	      sys/semsys.o	\
sys/putmsg.o      sys/getmsg.o      sys/poll.o \
sys/setgid.o	  sys/setpgrp.o	    sys/setuid.o      sys/shmsys.o	\
sys/signal.o	  sys/stat.o	    sys/stime.o	      sys/stty.o	\
sys/sync.o	  sys/sys3b.o	    sys/syscall.o     sys/time.o	\
sys/times.o	  sys/ulimit.o	    sys/umask.o	      sys/umount.o	\
sys/uname.o	  sys/unlink.o	    sys/ustat.o	      sys/utime.o	\
sys/wait.o	  sys/write.o	    \
sys/advfs.o       sys/rdebug.o      sys/rfstart.o     \
sys/rfstop.o      sys/rfsys.o	    sys/rmount.o      sys/rumount.o     \
sys/unadvfs.o     \
sys/fstatfs.o	  sys/getdents.o    sys/mkdir.o	      \
sys/rmdir.o	  sys/statfs.o      sys/sysfs.o       \
fp/faddd.o	  fp/fmuld.o	    fp/fdivd.o	      fp/fcmpd.o 	\
fp/fnegd.o	  fp/fltod.o	    fp/fltos.o	  	      		\
fp/fcmps.o	  fp/fdtos.o 	    \
fp/fstod.o	  fp/ftdtol.o	    fp/ftdtou.o	      \
fp/ftstol.o	  fp/ftstou.o	    \
fp/dround.o	  fp/dspecial.o     \
fp/fadds.o	  fp/fmuls.o	    fp/fdivs.o	      \
fp/fnegs.o	  fp/sround.o	    fp/sspecial.o     \
fp/fp_data.o      fp/fp_def.o	    \
fp/fpstart.o	  fp/fpstart0.o	    fp/fpstart1.o     \
fp/getflts0.o	  fp/getflts1.o	    fp/getflth0.o     fp/getflth1.o	\
fp/fpgetmask.o	  fp/fpsetmask.o    fp/fpgetrnd.o     fp/fpsetrnd.o	\
fp/fpgetsticky.o  fp/fpsetsticky.o  fp/isnand.o	      \
print/doprnt.o	  print/fprintf.o   print/printf.o    print/sprintf.o \
print/vfprintf.o  print/vprintf.o   print/vsprintf.o


POBJECTS=\
crt/cerror.p	  crt/mcount.p	    csu/crt0.p	      csu/mcrt0.p	\
csu/fcrt0.p	  csu/fmcrt0.p	    \
csu/crt1.p	  csu/fcrt1.p	    csu/fmcrt1.p      csu/mcrt1.p	\
gen/abs.p	  gen/atof.p	    \
gen/biglitpow.p   gen/cuexit.p	    gen/dtop.p        \
gen/ecvt.p	  \
gen/fakcu.p	  gen/frexp.p	    gen/ldexp.p       \
gen/ltostr.p	  gen/memcpy.p	    gen/ptod.p	      gen/setjmp.p 	\
gen/strcpy.p      gen/strcmp.p	    gen/fptrap.p      \
gen/strlen.p	  sys/access.p      sys/acct.p	      \
sys/alarm.p	  sys/chdir.p	    sys/chmod.p	      sys/chown.p	\
sys/chroot.p	  sys/close.p	    sys/creat.p	      sys/dup.p		\
sys/execl.p	  sys/execle.p	    sys/execv.p       \
sys/execve.p	  sys/exit.p	    sys/fcntl.p	      sys/fork.p	\
sys/fstat.p	  sys/getegid.p	    sys/geteuid.p     sys/getgid.p	\
sys/getpid.p	  sys/getppid.p	    sys/getuid.p      sys/gtty.p	\
sys/ioctl.p	  sys/kill.p	    sys/link.p	      sys/lseek.p	\
sys/mknod.p	  sys/mount.p	    sys/msgsys.p      sys/nice.p	\
sys/open.p	  sys/pause.p	    sys/pipe.p	      sys/profil.p	\
sys/plock.p	  \
sys/ptrace.p	  sys/read.p	    sys/sbrk.p	      sys/semsys.p	\
sys/putmsg.p      sys/getmsg.p      sys/poll.p      \
sys/setgid.p	  sys/setpgrp.p	    sys/setuid.p      sys/shmsys.p	\
sys/signal.p	  sys/stat.p	    sys/stime.p	      sys/stty.p	\
sys/sync.p	  sys/sys3b.p	    sys/syscall.p     sys/time.p	\
sys/times.p	  sys/ulimit.p	    sys/umask.p	      sys/umount.p	\
sys/uname.p	  sys/unlink.p	    sys/ustat.p	      sys/utime.p	\
sys/advfs.p       sys/rdebug.p      \
sys/rfstart.p     sys/rfstop.p      sys/rmount.p      sys/rumount.p     \
sys/rfsys.p	  sys/unadvfs.p     \
sys/fstatfs.p     sys/getdents.p    sys/mkdir.p       \
sys/rmdir.p	  sys/statfs.p      sys/sysfs.p 	\
sys/wait.p	  sys/write.p	    sys/uadmin.p      \
fp/faddd.p	  fp/fmuld.p	    fp/fdivd.p	      fp/fcmpd.p 	\
fp/fnegd.p	  fp/fltod.p	    fp/fltos.p	      \
fp/fcmps.p	  fp/fdtos.p 	    \
fp/fstod.p	  fp/ftdtol.p	    fp/ftdtou.p       \
fp/ftstol.p	  fp/ftstou.p	    \
fp/dround.p	  fp/dspecial.p     \
fp/fadds.p	  fp/fmuls.p	    fp/fdivs.p	      \
fp/fnegs.p	  fp/sround.p	    fp/sspecial.p     \
fp/fpstart.p	  fp/fpstart0.p	    fp/fpstart1.p     \
fp/getflts0.p	  fp/getflts1.p	    fp/getflth0.p     fp/getflth1.p	\
fp/fpgetmask.p	  fp/fpsetmask.p    fp/fpgetrnd.p     fp/fpsetrnd.p	\
fp/fpgetsticky.p  fp/fpsetsticky.p  fp/isnand.p       \
print/doprnt.p	  print/fprintf.p   print/printf.p    print/sprintf.p \
print/vfprintf.p  print/vprintf.p   print/vsprintf.p


all:
	$(PROF)make -e $(IGN) prof NONPROF=@#
	$(NONPROF)make -e $(IGN) nonprof PROF=@#

nonprof: $(OBJECTS)

prof: $(POBJECTS)

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS)  -c $*.c
	$(NONPROF)mv $(*F).o $*.o
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p

.s.o .s.p:
	$(NONPROF)@echo $*.s:
	$(NONPROF)$(M4) $(SDEFLIST) -DMCOUNT=#  $*.s   > $(*F).s   
	$(NONPROF)$(CC) $(DEFLIST) $(CFLAGS) -c $(*F).s && mv $(*F).o $*.o
	$(PROF)@echo $*.s:
	$(PROF)$(M4) mcount.def  $*.s   > $(*F).s 
	$(PROF)$(CC) $(DEFLIST) $(CFLAGS) -c -p $(*F).s  && mv $(*F).o $*.p
	-rm $(*F).s

archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	# figure out the correct ordering for all the archive modules except
	#	cuexit and fakcu, which must go at end of archive.
	mv cuexit.o cuexit.x
	mv fakcu.o fakcu.x
	mv fpstart.o fpstart.x
	mv fpstart0.o fpstart0.x
	mv signal.o signal.x
	mv getflth0.o getflth0.x
	mv getflth1.o getflth1.x
	mv getflts0.o getflts0.x
	mv getflts1.o getflts1.x
	mv kill.o kill.x
	mv getpid.o getpid.x
	mv cerror.o cerror.x
	mv fpsetmask.o fpsetmask.x
	$(LORDER) *.o | tsort >objlist
	mv cuexit.x cuexit.o
	mv fakcu.x fakcu.o
	mv fpstart.x fpstart.o
	mv fpstart0.x fpstart0.o
	mv signal.x signal.o
	mv getflth0.x getflth0.o
	mv getflth1.x getflth1.o
	mv getflts0.x getflts0.o
	mv getflts1.x getflts1.o
	mv kill.x kill.o
	mv getpid.x getpid.o
	mv cerror.x cerror.o
	mv fpsetmask.x fpsetmask.o
	#
	# build the archive with the modules in correct order.
	$(AR) q ../lib.libc getflts1.o getflth1.o `cat objlist` \
	signal.o fpstart.o fpstart0.o getflth0.o getflts0.o \
	fpsetmask.o kill.o getpid.o cerror.o cuexit.o fakcu.o 
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
	if [ "$(PROF)" != "@#" ]; \
	then \
	$(PROF)$(AR) q ../libp.libc getflts1.o getflth1.o `cat objlist` \
	signal.o fpstart.o fpstart0.o getflth0.o getflts0.o \
	fpsetmask.o kill.o getpid.o cerror.o cuexit.o fakcu.o  ; \
	fi

clean:
	-rm -f *.o
	-rm -f *.p
	-rm -f *.s

clobber: clean
	-rm -f */*.o
	-rm -f */*.p
