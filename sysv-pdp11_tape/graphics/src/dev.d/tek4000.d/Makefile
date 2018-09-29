# @(#)Makefile	1.2
I =	../../../include
CPROGS = erase.c hardcopy.c tekset.c 
DIRS =	td.d ged.d
LIB =	../../../lib/glib.a
BIN =	../../../bin

tek4000 all:	$(CPROGS) $(DIRS) tek
		make BIN=$(BIN) `echo $? | sed "s/\..//g"`
		touch tek4000

erase:	erase.c
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
hardcopy: hardcopy.c
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@
tekset: tekset.c
	BIN=$(BIN) sh $(SHFLAG) ./cco $@; touch $@

tek:	$(LIB) lib $(I) include
	cd lib; make
	touch tek
td:	$(LIB) td.d $(I)
	cd td.d; make BIN=$(BIN)
ged:	$(LIB) lib ged.d include $(I)
	cd ged.d; make BIN=$(BIN); 
