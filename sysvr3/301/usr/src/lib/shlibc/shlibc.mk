#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)shlibc:shlibc.mk	1.13"
#
#
# makefile for shared libc
# 
#
# there will be no profiled objects with shared libraries.
#

SGS=
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
LORDER=$(PFX)lorder
ROOT=
LIB=$(ROOT)/lib
SHLIB=$(ROOT)/shlib
DEFLIST=
SDEFLIST=
MKSHLIB=mkshlib

# List of non-shared objects from libc-port
NONSH1=\
../port/gen/a64l.o	../port/gen/abort.o	../port/gen/assert.o 	\
../port/gen/bsearch.o	../port/gen/clock.o	../port/gen/closedir.o	\
../port/gen/ctype.o	../port/gen/ctime.o	../port/gen/dial.o	\
../port/gen/drand48.o	../port/gen/errlst.o	../port/gen/execvp.o	\
../port/gen/ftok.o	../port/gen/getcwd.o	../port/gen/getgrgid.o	\
../port/gen/ftw.o	../port/gen/getgrent.o	../port/gen/getpw.o	\
../port/gen/getgrnam.o	../port/gen/getlogin.o	../port/gen/getpwent.o	\
../port/gen/getpwnam.o	../port/gen/getpwuid.o	../port/gen/getut.o	\
../port/gen/hsearch.o	../port/gen/l3.o 	../port/gen/l64a.o	\
../port/gen/lfind.o	../port/gen/lsearch.o	../port/gen/modf.o	\
../port/gen/mon.o	../port/gen/nlist.o	../port/gen/opendir.o	\
../port/gen/perror.o	../port/gen/putenv.o	../port/gen/putpwent.o	\
../port/gen/qsort.o	../port/gen/rand.o	../port/gen/readdir.o	\
../port/gen/seekdir.o	../port/gen/sleep.o	../port/gen/ssignal.o	\
../port/gen/strcspn.o 	../port/gen/strdup.o	../port/gen/strpbrk.o	\
../port/gen/strspn.o	../port/gen/strtod.o	../port/gen/strtok.o	\
../port/gen/strtol.o	../port/gen/swab.o	../port/gen/tell.o	\
../port/gen/telldir.o	../port/gen/tfind.o 	../port/gen/tsearch.o   \
../port/gen/ttyname.o	../port/gen/ttyslot.o	../port/stdio/clrerr.o	\
../port/stdio/ctermid.o	../port/stdio/cuserid.o	../port/stdio/data.o	\
../port/stdio/doscan.o	../port/stdio/fdopen.o	../port/stdio/ftell.o	\
../port/stdio/getpass.o	../port/stdio/popen.o	../port/stdio/rew.o	\
../port/stdio/scanf.o	../port/stdio/setvbuf.o	../port/stdio/system.o	\
../port/stdio/tempnam.o	../port/stdio/tmpfile.o	../port/stdio/tmpnam.o	\
../port/sys/lockf.o

# List of non-shared objects from libc-m32
NONSH2=\
../m32/crt/mcount.o	../m32/fp/fpstart.o	../m32/fp/fpstart0.o	\
../m32/fp/getflth1.o	../m32/gen/fptrap.o	../m32/gen/cuexit.o	\
../m32/gen/fakcu.o	../m32/gen/setjmp.o	../m32/print/vfprintf.o	\
../m32/print/vprintf.o	../m32/print/vsprintf.o	../m32/sys/acct.o	\
../m32/sys/advfs.o	../m32/sys/alarm.o	../m32/sys/chown.o	\
../m32/sys/chroot.o	../m32/sys/dup.o	../m32/sys/execl.o	\
../m32/sys/execle.o	../m32/sys/execv.o	../m32/sys/execve.o	\
../m32/sys/exit.o	../m32/sys/fork.o	../m32/sys/fstatfs.o	\
../m32/sys/getdents.o	../m32/sys/getegid.o	../m32/sys/geteuid.o	\
../m32/sys/getgid.o	../m32/sys/getmsg.o	../m32/sys/getppid.o	\
../m32/sys/getuid.o	../m32/sys/gtty.o	../m32/sys/link.o	\
../m32/sys/mkdir.o	../m32/sys/mknod.o	../m32/sys/mount.o	\
../m32/sys/msgsys.o	../m32/sys/nice.o	../m32/sys/pause.o	\
../m32/sys/pipe.o	../m32/sys/plock.o	../m32/sys/poll.o	\
../m32/sys/profil.o	../m32/sys/ptrace.o	../m32/sys/putmsg.o	\
../m32/sys/rdebug.o	../m32/sys/rfstart.o	../m32/sys/rfstop.o	\
../m32/sys/rfsys.o	../m32/sys/rmdir.o	../m32/sys/rmount.o	\
../m32/sys/rumount.o	../m32/sys/semsys.o	../m32/sys/setgid.o	\
../m32/sys/setpgrp.o	../m32/sys/setuid.o	\
../m32/sys/shmsys.o	../m32/sys/statfs.o	\
../m32/sys/stime.o	../m32/sys/stty.o	../m32/sys/sync.o	\
../m32/sys/sys3b.o	../m32/sys/syscall.o	../m32/sys/sysfs.o	\
../m32/sys/times.o	../m32/sys/uadmin.o	../m32/sys/ulimit.o	\
../m32/sys/umask.o	../m32/sys/umount.o	../m32/sys/unadvfs.o	\
../m32/sys/uname.o	../m32/sys/ustat.o	../m32/sys/wait.o

all: 
	if  u3b15 || u3b5 || u3b2; then make -e -f shlibc.mk shared \
		PROF=@# MACHINE=m32 DEFLIST=-DM32 SDEFLIST=-DSHLIB ; fi

shared:
	#
	# compile portable library modules
	cd port; make -e CC=$(CC)
	#
	# compile machine-dependent library modules
	cd $(MACHINE); make -e CC=$(CC) 
	#
	# create the host and target modules
	-rm -f host target
	$(PFX)$(MKSHLIB) -q -s spec -t target -h host
	#
	# build the mixed archive in the temporary directory "shobject"
	-rm -rf shobject
	mkdir shobject
	cd shobject; make -e -f ../shlibc.mk mixed
	# 
	# remove temprary directory "shobject"
	-rm -rf shobject
mixed:
	#
	# extract from the host all the object files and
	# reorder the objects in the same way they are in libc.a
	#
	$(AR) -x ../host
	#
	# copy to the current directory the non-shared object files
	cp $(NONSH1) .
	cp $(NONSH2) .
	#
	# preserve libc.a object order
	mv cuexit.o             cuexit.x
	mv fakcu.o   		fakcu.x
	mv fpstart.o            fpstart.x
	mv fpstart0.o	        fpstart0.x
	mv signal.o             signal.x
	mv getflth0.o 	        getflth0.x
	mv getflth1.o	        getflth1.x
	mv getflts1.o	        getflts1.x
	mv kill.o               kill.x
	mv getpid.o             getpid.x
	mv cerror.o             cerror.x
	mv gen_def.o	        gen_def.x
	mv fpsetmask.o		fpsetmask.x
	$(LORDER) *.o  |tsort > objlist
	mv cuexit.x	        cuexit.o
	mv fakcu.x 		fakcu.o
	mv fpstart.x 		fpstart.o
	mv fpstart0.x 		fpstart0.o
	mv signal.x 		signal.o
	mv getflth0.x 		getflth0.o
	mv getflth1.x 		getflth1.o
	mv getflts1.x 		getflts1.o
	mv kill.x 		kill.o
	mv getpid.x 		getpid.o
	mv cerror.x 		cerror.o
	mv gen_def.x 		gen_def.o
	mv fpsetmask.x 		fpsetmask.o
	# create mixed archive
	-rm -f ../libc_s.a
	$(AR) q ../libc_s.a  getflth1.o `cat objlist`  \
	signal.o fpstart.o fpstart0.o getflth0.o getflts1.o fpsetmask.o \
	kill.o getpid.o cerror.o gen_def.o cuexit.o fakcu.o hst*
	# clean up
move:
	# First change the name of the old shared library target 
	# module  so that the target just created can be installed
	# Move the shared archive into the correct directory
	# Move the target file into the correct directory
	#
	-mv $(SHLIB)/libc_s $(SHLIB)/libc_s-`date +%j\%H\%M`
	cp libc_s.a $(LIB)/libc_s.a 
	cp target $(SHLIB)/libc_s 

install:all move

clean:
	#
	# remove intermediate files
	-rm -rf *.o hst* *.x objlist 
	cd port ;  make clean
	if u3b15 || u3b5 || u3b2 ;   then cd m32 ;   make clean ; fi
	#
clobber:
	#
	# 
	#
	-rm -rf $(SHLIB)/libc_s-* 
	-rm -rf target libc_s.a
	-rm -rf *.o lib*.libc lib*.contents obj* hst* *.x
	cd port; make clobber;
	if u3b15 || u3b5 || u3b2; then cd m32 ; make clobber ; fi
	# done
	#
