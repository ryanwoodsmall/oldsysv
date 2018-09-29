#	The ID keywords for the sdb/vax/makefile are @(#) makefile: 1.6 3/21/83
CFLAGS=	-O -DDEBUG
CFLAGS=	-O -DDEBUG=2
CFLAGS=	-O 
CC = cc
TESTDIR = .
INS = /etc/install -n /usr/bin
INSDIR =

DIS.C=	opcode.c output.c
DIS.O=	opcode.o output.o

OBJS=	main.o re.o fio.o bio.o decode.o docomm.o sub.o  \
	access.o pcs.o runpcs.o setup.o message.o \
	symt.o display.o prvar.o xeq.o version.o \
	opset.o optab.o machdep.o udef.o \
	debug.o	${DIS.O}

COM= ../com
SOURCE=	$(COM)/main.c $(COM)/re.c $(COM)/fio.c $(COM)/bio.c $(COM)/decode.c  \
	$(COM)/docomm.c $(COM)/sub.c $(COM)/access.c $(COM)/pcs.c  \
	$(COM)/runpcs.c $(COM)/setup.c $(COM)/message.c  \
	$(COM)/symt.c $(COM)/display.c $(COM)/prvar.c $(COM)/xeq.c  \
	$(COM)/version.c $(COM)/opset.c $(COM)/optab.c  \
	$(COM)/machdep.c $(COM)/udef.c $(COM)/debug.c 

CSOURCE= $(COM)/machdep.h $(COM)/access.c $(COM)/machdep.c $(COM)/opset.c \
	$(COM)/optab.c $(COM)/runpcs.c $(COM)/setup.c $(COM)/xeq.c

all: sdb

sdb:	$(OBJS)
	$(CC) $(CFLAGS) -o sdb $(OBJS)

INC= /usr/include
INCS= $(INC)/sys
HINC= $(INC)/setjmp.h $(INC)/stdio.h $(INC)/termio.h $(INC)/sgtty.h
MINC= $(INCS)/param.h $(INCS)/types.h $(INCS)/sysmacros.h \
	$(INCS)/dir.h $(INCS)/user.h $(INCS)/seg.h \
	$(INC)/errno.h $(INC)/signal.h $(INCS)/signal.h

COFF.H= $(COM)/coff.h $(COM)/sgs.h $(INC)/a.out.h
MACHDEP.H= $(COM)/machdep.h $(MINC)
MODE.H= $(COM)/mode.h
HEAD.H= $(COM)/head.h $(COM)/bio.h $(MODE.H) $(MACHDEP.H) $(HINC)


decode.o:	$(HEAD.H) $(COM)/decode.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/decode.c

docomm.o:	$(HEAD.H) $(COFF.H) $(COM)/docomm.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/docomm.c

main.o:		$(HEAD.H) $(COM)/main.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/main.c

sub.o:		$(HEAD.H) $(COFF.H) $(COM)/sub.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/sub.c

re.o:		$(HEAD.H) $(COM)/re.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/re.c

fio.o:		$(HEAD.H) $(COM)/fio.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/fio.c

bio.o:		$(COM)/bio.h $(COM)/bio.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/bio.c

access.o:	$(HEAD.H) $(COM)/access.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/access.c

pcs.o:		$(HEAD.H) $(COM)/pcs.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/pcs.c

runpcs.o:	$(HEAD.H) $(COM)/runpcs.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/runpcs.c

xeq.o:		$(HEAD.H) $(COFF.H) $(COM)/xeq.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/xeq.c

setup.o:	$(HEAD.H) $(COFF.H) $(COM)/setup.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/setup.c

message.o:	$(MODE.H) $(COM)/message.c
	$(CC) $(CFLAGS) $(FLEX) -S $(COM)/message.c
	ed - <$(COM)/:rofix message.s
	as -o message.o message.s
	rm message.s

symt.o:		$(HEAD.H) $(COFF.H) $(COM)/symt.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/symt.c

display.o:	$(HEAD.H) $(COFF.H)  $(COM)/display.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/display.c

prvar.o:	$(HEAD.H) $(COFF.H) $(COM)/prvar.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/prvar.c

version.o:	$(COM)/version.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/version.c

opset.o:	$(HEAD.H) $(COM)/opset.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/opset.c

optab.o:	$(MODE.H) $(MACHDEP.H)  $(COM)/optab.c
	$(CC) $(CFLAGS) $(FLEX) -S $(COM)/optab.c
	ed - <$(COM)/:rofix optab.s
	as -o optab.o optab.s
	rm optab.s
machdep.o:	$(HEAD.H) $(COFF.H) $(COM)/machdep.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/machdep.c

udef.o:		$(HEAD.H) $(COFF.H) $(COM)/udef.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/udef.c


output.o:	defs.h $(MODE.H)
	$(CC) $(CFLAGS) $(FLEX) -c output.c
opcode.o:	defs.h $(MODE.H)
	$(CC) $(CFLAGS) $(FLEX) -c opcode.c

debug.o:	$(COM)/head.h $(COFF.H) $(COM)/debug.c
		$(CC) -c $(CFLAGS) $(FLEX) $(COM)/debug.c

install:	all
		cp sdb sdb.temp
		mv sdb.temp sdb
		$(INS) $(TESTDIR)/sdb $(INSDIR)

clean :
		-rm -f $(OBJS)

clobber:	clean
		-rm -f $(TESTDIR)/sdb

