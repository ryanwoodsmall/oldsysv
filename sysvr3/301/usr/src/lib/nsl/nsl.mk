#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)libnsl:nsl/nsl.mk	1.10"
# 
# Network services library
#

ROOT=
USRLIB=$(ROOT)/usr/lib
LIB=$(ROOT)/shlib
INCRT=$(ROOT)/usr/include
CFLAGS= -I$(INCRT) -O 

LIBOBJS= t_accept.o t_bind.o t_connect.o t_error.o t_close.o\
	 t_getinfo.o t_getstate.o t_listen.o t_look.o\
	 t_rcv.o t_rcvconnect.o t_rcvdis.o t_snd.o t_snddis.o\
	 t_unbind.o t_optmgmt.o\
	 t_rcvudata.o t_rcvuderr.o t_sndudata.o t_sndrel.o t_rcvrel.o\
	 t_alloc.o t_free.o t_open.o t_sync.o\
	 _dummy.o _errlst.o _data.o _conn_util.o _utility.o\
	 __free_def.o __calloc_def.o __perror_def.o __strlen_def.o\
	 __write_def.o __ioctl_def.o __putmsg_def.o __getmsg_def.o\
	 __errno_def.o __memcpy_def.o __fcntl_def.o __sigset_def.o\
	 __open_def.o __close_def.o __ulimit_def.o

INCLUDES=  	$(INCRT)/sys/param.h\
		$(INCRT)/sys/types.h\
		$(INCRT)/sys/errno.h\
		$(INCRT)/sys/stream.h\
		$(INCRT)/sys/stropts.h\
		$(INCRT)/sys/tihdr.h\
		$(INCRT)/sys/timod.h\
		$(INCRT)/sys/tiuser.h\
		$(INCRT)/sys/signal.h\
		./_import.h


all:      _spec $(LIB)/libnsl_s $(USRLIB)/libnsl_s.a

$(LIB)/libnsl_s: _spec $(LIBOBJS)
	-rm -f libnsl_s;
	$(PFX)mkshlib -s _spec -t libnsl_s

$(USRLIB)/libnsl_s.a: _spec $(LIBOBJS)
	-rm -f libnsl_s.a;
	$(PFX)mkshlib -s _spec -h libnsl_s.a -n -t libnsl_s

$(LIBOBJS):	$(INCLUDES)

install:  all
	install -f $(USRLIB) libnsl_s.a;
	install -o -f $(LIB) libnsl_s
	$(CH)chown bin $(USRLIB)/libnsl_s.a
	$(CH)chown bin $(LIB)/libnsl_s
	$(CH)chgrp bin $(USRLIB)/libnsl_s.a
	$(CH)chgrp bin $(LIB)/libnsl_s
	$(CH)chmod 664 $(USRLIB)/libnsl_s.a
	$(CH)chmod 775 $(LIB)/libnsl_s

clean:
	-rm -f *.o

clobber:	clean
	-rm -f libnsl_s.a 
	-rm -f libnsl_s 
