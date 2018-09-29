#	@(#)adb.mk	1.4
#
.SUFFIXES:
.SUFFIXES: .o .c .e .r .f .y .yr .ry .ey .ye .l .s

CFLAGS = -O
AS = as -

TESTDIR = .
INS = /etc/install -i -n /bin 
INSDIR =

BIN = /bin
USRBIN = /usr/bin
LIBA = lib/liba
LIBM = lib/libm
LIBC = lib/libc
LIBS = lib/libS
INCRT = /usr/include

FRC =


all:	$(TESTDIR)/adb

$(TESTDIR)/adb:\
	access.o\
	command.o\
	expr.o\
	extern.o\
	format.o\
	input.o\
	main.o\
	opcode.o\
	output.o\
	pcs.o\
	print.o\
	runpcs.o\
	setup.o\
	sym.o\
	/$(LIBC).a\
	$(INCRT)/time.h\
	$(INCRT)/mon.h\
	$(INCRT)/stdio.h\
	$(INCRT)/varargs.h\
	$(INCRT)/ctype.h\
	$(INCRT)/values.h\
	$(INCRT)/errno.h\
	$(INCRT)/termio.h\
	$(FRC)
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/adb *.o

access.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


command.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


expr.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


extern.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


format.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


input.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


main.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


opcode.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


output.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


pcs.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


print.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


runpcs.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


setup.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)


sym.o:\
	defs.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/a.out.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/sys/text.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/tty.h\
	$(INCRT)/sys/termio.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/reg.h\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	mode.h\
	functs.h\
	extern.h\
	sys.h\
	lib.h\
	machine.h\
	$(FRC)
	$(CC) $(CFLAGS) -DFLEXNAMES -c sym.c


FRC:

install:	all
		$(INS) $(TESTDIR)/adb $(INSDIR)
clean:
		-rm -f *.o

clobber:	clean
		-rm -f $(TESTDIR)/adb
