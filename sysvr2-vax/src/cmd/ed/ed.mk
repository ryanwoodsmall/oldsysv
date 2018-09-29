#	ed make file
#	SCCS:	@(#)ed.mk	1.10

ROOT =
OL = $(ROOT)/
INSDIR = $(OL)bin
IFLAG = -n
B10 = 
CFLAGS = -O -DCRYPT $(B10)
LDFLAGS = -s $(IFLAG)

compile all: ed
	:

ed:	ed.c
	$(CC) -S $(CFLAGS) ed.c
	-if pdp11; then sh ./edfun ed.s; else true; fi
	$(CC) $(LDFLAGS) -o ed ed.s

install: ed
	ED=`basename \`pwd\`` ; \
	cpset ed $(INSDIR)/$${ED} ; \
	rm -f $(INSDIR)/r$${ED} ; \
	ln $(INSDIR)/$${ED} $(INSDIR)/r$${ED}

clean:
	  rm -f ed.s ed.o

clobber:  clean
	  rm -f ed
