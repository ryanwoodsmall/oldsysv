#	@(#)c2.mk	1.5
CC=cc
CFLAGS=-O
CTAGS=ctags
LINT=lint
LIB=-ll
MIP=.
PPFLAGS=-I. -I${MIP}
CSRC=local.c oplook.c supp.c w1.c w2.c w3.c
COBJ=inter.o optim.o local.o oplook.o supp.o w1.o w2.o w3.o
RSRC=local.l oplook.c supp.c w1.c w2.c w3.c
MSRC=$(MIP)/inter.c $(MIP)/optim.c

all:	c2

c2:		$(COBJ)
		$(CC) $(CFLAGS) $(COBJ) $(LIB) -o c2

install:	c2
		/etc/install -f /lib c2

optim.h:	defs $(MIP)/optim.h

optim.o:	optim.h $(MIP)/optim.c
		$(CC) $(PPFLAGS) $(CFLAGS) -c $(MIP)/optim.c

inter.o:	optim.h $(MIP)/inter.c
		$(CC) $(PPFLAGS) $(CFLAGS) -c $(MIP)/inter.c

local.o:	optim.h local.c

oplook.o:	optim.h oplook.c instrs.h

supp.o:		optim.h supp.c

w1.o:		optim.h w1.c

w2.o:		optim.h w2.c

w3.o:		optim.h w3.c

local.c:	local.l
		$(LEX) $(LFLAGS) local.l
		-@mv -f lex.yy.c local.c

clean:
	-rm -f $(COBJ) local.c core c2 lint.out cxref.out cflow.out tags *.O

clobber: clean
	-rm -f c2

tags:	$(RSRC) optim.h $(MSRC)
	$(CTAGS) defs extdec.h $(RSRC) $(MIP)/optim.h $(MSRC)

lint.out:	$(CSRC)
		$(LINT) $(LINTFLAGS) $(PPFLAGS) $(CSRC) >lint.out

cflow.out:	$(CSRC)
		cflow -r $(PPFLAGS) $(CSRC) >cflow.out

cxref.out:	$(CSRC)
		cxref -c $(PPFLAGS) $(CSRC) >cxref.out

.c.o:;	$(CC) $(PPFLAGS) $(CFLAGS) -c $*.c
