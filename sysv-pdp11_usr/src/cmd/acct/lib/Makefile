#	@(#)Makefile	1.2 of 3/31/82	#
TESTDIR = .
LIB = a.a
CFLAGS = -O
INCDIR = ..
FRC =

FILES= \
	$(LIB)(pnpsplit.o) \
	$(LIB)(copyn.o) \
	$(LIB)(devtolin.o) \
	$(LIB)(expand.o) \
	$(LIB)(lintodev.o) \
	$(LIB)(namtouid.o) \
	$(LIB)(tmless.o) \
	$(LIB)(tmsecs.o) \
	$(LIB)(uidtonam.o) \
	$(LIB)(substr.o) \
	$(LIB)(regex.o) \
	$(LIB)(regcmp.o)

all:	$(LIB) $(FRC)

$(LIB):	$(FILES)
	@echo lib $(LIB) is now up to date

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIB)

FRC:

.c.a:
	$(CC) $(CFLAGS) -I$(INCDIR) -c $<
	ar rv $@ $*.o
	rm -f $*.o

