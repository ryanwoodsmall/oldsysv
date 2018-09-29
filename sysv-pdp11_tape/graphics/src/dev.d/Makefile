# @(#)Makefile	1.1
LIB =	../../../lib/glib.a
BIN =	../../../bin
DIRS =	lolib uplib hp7220.d tek4000.d

dev all:: lolib; cd $?; make LIB=$(LIB)
dev all:: uplib; cd $?; make LIB=$(LIB)
dev all:: hp7220.d; cd $?; make BIN=$(BIN) LIB=$(LIB)
dev all:: tek4000.d; cd $?; make BIN=$(BIN) LIB=$(LIB)
