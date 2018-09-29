# @(#)Makefile	1.2
I =	../../include
L =	../../lib
CPROGS = bel.c gd.c pd.c remcom.c cvrtopt.c
SPROGS = quit.sh yoo.sh
DIRS =	gtop.d ptog.d
LIB =	$(L)/glib.a
BIN =	../../bin

gutil all:	$(CPROGS) $(SPROGS) $(DIRS)
		make BIN=$(BIN) `echo $? | sed "s/\.[a-z]*//g"`

remcom: $(I)/errpr.h remcom.c
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
bel:	bel.c
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
gd:	gd.c $(I)/gsl.h $(I)/gpl.h $(I)/errpr.h $(I)/util.h $(LIB)
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
pd:	pd.c $(I)/errpr.h $(I)/util.h
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
cvrtopt: cvrtopt.c $(I)/errpr.h $(I)/util.h $(I)/setop.h
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@

quit:	quit.sh
	cp quit.sh $(BIN)/quit; touch $@
	chmod 755 $(BIN)/quit
yoo:	yoo.sh
	cp yoo.sh $(BIN)/yoo; touch $@
	chmod 755 $(BIN)/yoo

gtop:	gtop.d $(I)
	-cd gtop.d; make BIN=$(BIN)
ptog:	ptog.d $(I)
	cd ptog.d; make BIN=$(BIN)

