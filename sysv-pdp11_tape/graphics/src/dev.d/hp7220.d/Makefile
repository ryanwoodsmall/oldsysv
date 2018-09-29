# @(#)Makefile	1.1
I =	../../../include
DIRS =	hpd.d
LIB =	../../../lib/glib.a
BIN =	../../../bin

hp7220 all:	$(DIRS)
		make BIN=$(BIN) `echo $? | sed "s/\..//g"`
		touch hp7220

hpd:	$(LIB) hpd.d  $(I)
	cd hpd.d; make BIN=$(BIN)
