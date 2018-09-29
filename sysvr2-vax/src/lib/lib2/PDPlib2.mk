#	@(#)PDPlib2.mk	1.2
INS = /etc/install
CFLAGS = -O

SYS =
OBJS  = SYS.o mount.o tty.o chdir.o \
	sbrk.o time.o stat.o lseek.o gtty.o stty.o isatty.o \
	getuid.o getgid.o nice.o read.o creat.o close.o \
	open.o write.o stime.o fstat.o tell.o umount.o \
	umask.o chmod.o getargv.o access.o mknod.o ustat.o \
	exit.o PDPtpdmp.o PDPhtdmp.o

DEVS  = PDPht.o PDPgd.o rk.o rp.o tm.o
LIBS  = lib2.a
LMODS = srt0.o lib2.a

all:	stest
stest:
	-if test "$(SYS)" != ""; then make -f $(SYS)lib2.mk comp SYS=$(SYS); \
	elif vax ; then make -f VAXlib2.mk comp SYS=VAX ; \
	make -f lib2A.mk all;\
	elif pdp11 ; then make -f PDPlib2.mk comp SYS=PDP ;\
	make -f lib2A.mk all; \
	make -f lib2B.mk all ; fi

comp:	$(LMODS)

srt0.o:	PDPsrt0.s
	as -o srt0.o PDPsrt0.s
	cp srt0.o /lib/crt2.o
	-chown bin /lib/crt2.o
	-chgrp bin /lib/crt2.o
	-chmod 664 /lib/crt2.o
	rm -f srt0.o

lib2.a:	$(OBJS) $(DEVS)
	rm -f A*.o B*.o C*.o D*.o
	cp /lib/libc.a lib2.a
	ar d lib2.a acct.o alarm.o cerror.o chown.o chroot.o dup.o
	ar d lib2.a execl.o execle.o execv.o execve.o execvp.o fcntl.o fork.o
	ar d lib2.a getpass.o getpid.o getppid.o kill.o link.o mktemp.o
	ar d lib2.a pause.o pipe.o popen.o profil.o ptrace.o
	ar d lib2.a setgid.o setpgrp.o setuid.o signal.o sleep.o sync.o
	ar d lib2.a syscall.o system.o tmpfile.o times.o ulimit.o uname.o
	ar d lib2.a unlink.o utime.o wait.o
	mkdir tmp
	mv *.o tmp
	ar x lib2.a
	cp tmp/*.o .
	rm -rf tmp
	rm -f lib2.a
	-ar rv lib2.a `lorder *.o|sed 's/^ *//'|\
	sort|comm -23 - order.out|cat order.in -|tsort`
	ar q lib2.a flsbuf.o
	strip lib2.a

$(OBJS):	

$(DEVS):	
	cc -c $<

install:	all
	$(INS) -n /usr/lib lib2.a
	make -f lib2A.mk install
	-if pdp11; then make -f lib2B.mk install; fi
clean:
	rm -f *.o
clobber: clean
	rm -f lib2.a
	make -f lib2A.mk clobber
	-if pdp11 ; then make -f lib2B.mk clobber; fi
