#	@(#)makefile	1.1
#
# makefile for libc/port/x25
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O

OBJECTS=\
mdevno.o     mmdevno.o     x25llnk.o     x25ppvc.o

POBJECTS=\
mdevno.p     mmdevno.p     x25llnk.p     x25ppvc.p

nonprof: $(OBJECTS)

prof: $(POBJECTS)

x25llnk.o x25llnk.p x25ppvc.o x25ppvc.p: /usr/include/sys/csi.h
x25llnk.o x25llnk.p: /usr/include/sys/dmk.h
x25llnk.o x25llnk.p x25ppvc.o x25ppvc.p: /usr/include/sys/map.h
x25llnk.o x25llnk.p x25ppvc.o x25ppvc.p: /usr/include/sys/nc.h
mdevno.o mdevno.p \
	mmdevno.o mmdevno.p: /usr/include/sys/stat.h
mdevno.o mdevno.p \
	mmdevno.o mmdevno.p x25llnk.o x25llnk.p \
	x25ppvc.o x25ppvc.p: /usr/include/sys/types.h
x25llnk.o x25llnk.p x25ppvc.o x25ppvc.p: /usr/include/sys/x25.h
x25ppvc.o x25ppvc.p: /usr/include/sys/x25u.h
mdevno.o mdevno.p mmdevno.o mmdevno.p \
	x25llnk.o x25llnk.p \
	x25ppvc.o x25ppvc.p: /usr/include/x25lib.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c    && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c
