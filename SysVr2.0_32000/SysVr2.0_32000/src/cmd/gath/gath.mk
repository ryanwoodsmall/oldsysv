#	gath.mk 1.2 of 10/1/81
#	@(#)gath.mk	1.2

INCLUDE = /usr/include

CC = cc
CCFLAGS = -O

LD = -ld
# LDFLAGS = -x

LIB = lib/librje.a rje/rje.a

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

#------
# send:		Create send in local directory.
#------
send:		$(LIB) send0.o send1.o send2.o send3.o \
		send4.o send5.o send6.o send7.o \
		send8.o send9.o
	$(CC) -O -o send send0.o send1.o send2.o send3.o \
		send4.o send5.o send6.o send7.o \
		send8.o send9.o $(LIB)

install:	send
	cp send /usr/bin/gath
	-chmod 755 /usr/bin/gath
	-chown bin /usr/bin/gath

$(LIB):		lib/*.c rje/*.c
	cd lib; make -f librje.mk librje.a clean
	cd rje; make -f lib.mk install clean

#------
# clean: Remove all .o files.
#------
clean:
	-rm -f send0.o send1.o send2.o send3.o \
		send4.o send5.o send6.o send7.o \
		send8.o send9.o send

clobber:	clean

#------
# Dependencies for individual .o files.
#------
send0.o:	send0.c send.h
send1.o:	send1.c send.h $(INCLUDE)/sys/param.h $(INCLUDE)/sys/stat.h
send2.o:	send2.c send.h
send3.o:	send3.c send.h $(INCLUDE)/ctype.h
send4.o:	send4.c send.h
send5.o:	send5.c send.h $(INCLUDE)/sys/param.h $(INCLUDE)/sys/stat.h
send6.o:	send6.c send.h
send7.o:	send7.c send.h
send8.o:	send8.c send.h
send9.o:	send9.c send.h
