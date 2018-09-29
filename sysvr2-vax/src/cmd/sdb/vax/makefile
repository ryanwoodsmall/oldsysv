#	@(#) makefile: 1.10 7/26/84
ROOT	=

# -DFLEXNAMES for flexnames code
FLEX	=

# -DDEBUG for debug code
DEBUG	=

LDFLAGS	=

INC	= /usr/include
INCSYS	= $(INC)/sys
COMMON	= $(ROOT)/usr/src/cmd/sdb/com
UBIN	= $(ROOT)/usr/bin

CC	= cc
CFLAGS	=
DEFLIST	= $(FLEX) $(DEBUG)
INCLIST	= -I$(COMMON) -I$(INC) -I$(INCSYS)
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
CCLD_CMD= $(CC) $(LDFLAGS) $(CFLAGS)

# headers included by head.h
HINC	= $(INC)/setjmp.h $(COMMON)/bio.h $(COMMON)/mode.h		\
	  $(COMMON)/machdep.h $(INC)/stdio.h $(INC)/termio.h

# headers included by machdep.h
MINC	= $(INCSYS)/param.h $(INCSYS)/types.h $(INC)/signal.h		\
	  $(INCSYS)/sysmacros.h $(INCSYS)/dir.h $(INCSYS)/user.h	\
	  $(INCSYS)/errno.h $(INC)/a.out.h $(INCSYS)/reg.h		\
	  $(INCSYS)/stat.h

# headers included by a.myout.h
A.OUTINC= $(INC)/filehdr.h $(INC)/aouthdr.h $(INC)/scnhdr.h		\
	  $(INC)/reloc.h $(INC)/linenum.h $(INC)/syms.h

MACH.H	= $(MINC) $(A.OUTINC)
HEAD.H	= $(HINC) $(MACH.H)

DIS.O	= bits.o extn.o tbls.o utls.o

OBJS	= access.o bio.o debug.o decode.o display.o docomm.o fio.o	\
	  machdep.o main.o message.o opset.o optab.o pcs.o prvar.o re.o	\
	  runpcs.o setup.o sub.o symt.o udef.o version.o xeq.o $(DIS.O)


all:	sdb

sdb:	$(OBJS)
	$(CCLD_CMD) -s -o sdb $(OBJS)

install:	all
	/etc/install -n $(ROOT)/usr/bin sdb

clean:
	rm -f $(OBJS)

clobber:	clean
	rm -f sdb

########
access.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/access.c
		$(CC_CMD) $(COMMON)/access.c

bio.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/bio.c
		$(CC_CMD) $(COMMON)/bio.c

debug.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/debug.c
		$(CC_CMD) $(COMMON)/debug.c

decode.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/decode.c
		$(CC_CMD) $(COMMON)/decode.c

display.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/display.c
		$(CC_CMD) $(COMMON)/display.c

docomm.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/docomm.c
		$(CC_CMD) $(COMMON)/docomm.c

fio.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/fio.c
		$(CC_CMD) $(COMMON)/fio.c

machdep.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/machdep.c
		$(CC_CMD) $(COMMON)/machdep.c

main.o:		$(COMMON)/head.h $(HEAD.H) $(INC)/fcntl.h $(COMMON)/main.c
		$(CC_CMD) $(COMMON)/main.c

message.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/message.c
		$(CC) -S $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMMON)/message.c
		ed - < $(COMMON)/:rofix message.s
		$(CC) -c message.s
		rm message.s

opset.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/opset.c
		$(CC_CMD) $(COMMON)/opset.c

optab.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/optab.c
		$(CC) -S $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMMON)/optab.c
		ed - < $(COMMON)/:rofix optab.s
		$(CC) -c optab.s
		rm optab.s

pcs.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/pcs.c
		$(CC_CMD) $(COMMON)/pcs.c

prvar.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/prvar.c
		$(CC_CMD) $(COMMON)/prvar.c

re.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/re.c
		$(CC_CMD) $(COMMON)/re.c

runpcs.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/runpcs.c
		$(CC_CMD) $(COMMON)/runpcs.c

setup.o:	$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/setup.c
		$(CC_CMD) $(COMMON)/setup.c

sub.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/sub.c
		$(CC_CMD) $(COMMON)/sub.c

symt.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(COMMON)/symt.c
		$(CC_CMD) $(COMMON)/symt.c

udef.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/udef.c
		$(CC_CMD) $(COMMON)/udef.c

version.o:	$(COMMON)/version.c
		$(CC_CMD) $(COMMON)/version.c

xeq.o:		$(COMMON)/head.h $(HEAD.H) $(COMMON)/coff.h $(INC)/fcntl.h $(COMMON)/xeq.c
		$(CC_CMD) $(COMMON)/xeq.c


bits.o:		$(INC)/stdio.h dis.h $(COMMON)/sgs.h
		$(CC_CMD) bits.c

extn.o:		$(INC)/stdio.h dis.h
		$(CC_CMD) extn.c

tbls.o:		dis.h
		$(CC_CMD) tbls.c

utls.o:		$(INC)/stdio.h dis.h $(COMMON)/sgs.h
		$(CC_CMD) utls.c
