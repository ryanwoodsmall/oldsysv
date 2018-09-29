#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:u3b2/makefile	1.8"
ROOT	=

# -DFLEXNAMES for flexnames code
FLEX	=

# -DDEBUG=<debug level> for debug code
DEBUG	=

LDFLAGS	=

INC	= $(ROOT)/usr/include
INCSWP	= $(INC)
INCSYS	= $(INC)/sys
INCSWPS	= $(INCSWP)/sys
COMMON	= $(ROOT)/usr/src/cmd/sdb/com
UBIN	= $(ROOT)/usr/bin

CC	= cc
CFLAGS	= -O
DEFLIST	= $(FLEX) $(DEBUG)
INCLIST	= -I$(COMMON) -I$(INCSWP) -I$(INCSWPS)
CC_CMD	= $(CC) -c  $(CFLAGS) $(DEFLIST) $(INCLIST)
CCLD_CMD= $(CC) $(LDFLAGS) $(CFLAGS) $(FFLAG)

# headers included by head.h
HINC	= $(INC)/setjmp.h $(COMMON)/bio.h $(COMMON)/mode.h		\
	  $(COMMON)/machdep.h $(INC)/stdio.h $(INC)/termio.h

# headers included by machdep.h
MINC	= $(INCSWPS)/param.h $(INCSWPS)/types.h $(INC)/signal.h		\
	  $(INCSWPS)/sysmacros.h $(INCSYS)/dir.h $(INCSWPS)/user.h	\
	  $(INCSYS)/errno.h $(COMMON)/a.myout.h $(INCSWPS)/reg.h		\
	  $(INCSYS)/stat.h

# headers included by a.myout.h
A.OUTINC= $(INC)/filehdr.h $(INC)/aouthdr.h $(INC)/scnhdr.h		\
	  $(INC)/reloc.h $(INC)/linenum.h $(COMMON)/mysyms.h

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
	cp sdb.sh sdb
	install -n $(ROOT)/usr/bin sdb
	install -n $(ROOT)/usr/lib sdbp	
	install -n $(ROOT)/usr/lib sdbs

clean:
	rm -f $(OBJS)

clobber:	clean
	rm -f sdb sdbp sdbs

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
		$(CC_CMD) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMMON)/message.c

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
