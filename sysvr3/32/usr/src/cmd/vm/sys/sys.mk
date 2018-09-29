#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm:sys/sys.mk	1.3"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libsys.a
HEADER1=../inc
INCLUDE=	-I$(HEADER1) 

AR=		ar
CFLAGS = 	-O

$(LIBRARY):  \
		$(LIBRARY)(backslash.o)\
		$(LIBRARY)(expand.o) \
		$(LIBRARY)(exit.o) \
		$(LIBRARY)(filename.o) \
		$(LIBRARY)(getaltenv.o) \
		$(LIBRARY)(getepenv.o) \
		$(LIBRARY)(memshift.o) \
		$(LIBRARY)(parent.o) \
		$(LIBRARY)(spawn.o) \
		$(LIBRARY)(strCcmp.o) \
		$(LIBRARY)(strsave.o) \
		$(LIBRARY)(tempfiles.o) \
		$(LIBRARY)(terrmess.o) \
		$(LIBRARY)(terror.o) \
		$(LIBRARY)(varappend.o) \
		$(LIBRARY)(varcreate.o) \
		$(LIBRARY)(vardelete.o)  \
		$(LIBRARY)(vargrow.o)

$(LIBRARY)(backslash.o):	backslash.c $(HEADER1)/wish.h 

$(LIBRARY)(expand.o):	exit.c $(HEADER1)/wish.h $(HEADER1)/moremacros.h

$(LIBRARY)(exit.o):	exit.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(filename.o):	filename.c

$(LIBRARY)(getaltenv.o):	getaltenv.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(getepenv.o):	getepenv.c $(HEADER1)/wish.h $(HEADER1)/moremacros.h

$(LIBRARY)(memshift.o):	memshift.c $(HEADER1)/wish.h

$(LIBRARY)(parent.o):	parent.c $(HEADER1)/wish.h

$(LIBRARY)(spawn.o):	spawn.c

$(LIBRARY)(strCcmp.o):	strCcmp.c 

$(LIBRARY)(strsave.o):	strsave.c $(HEADER1)/wish.h

$(LIBRARY)(tempfiles.o):	tempfiles.c $(HEADER1)/wish.h $(HEADER1)/retcodes.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h

$(LIBRARY)(terrmess.o):	terrmess.c $(HEADER1)/wish.h $(HEADER1)/terror.h

$(LIBRARY)(terror.o):	terror.c $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/terror.h $(HEADER1)/retcodes.h

$(LIBRARY)(varappend.o):	varappend.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h 

$(LIBRARY)(varcreate.o):	varcreate.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h 

$(LIBRARY)(vardelete.o):	vardelete.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h 

$(LIBRARY)(vargrow.o):	vargrow.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h 

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

####### Standard makefile targets ########

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

profile:	libprof.a
#
# PROFILING: ---------------------------------------------
#  method 0: ignore the exit and suffer the conseqences
#
VERDICT = libp0

libp0.a:  libsys.a noeggs.o
	/bin/rm -f $@; cp libsys.a $@;
	ar d $@ exit.o
	ar ruv $@ noeggs.o
noeggs.c: exit.c
	sed 's/^exit/_noeggs/' $? >$@
#	
#  method 1: Rename the profiling and unix exits with
#      one extra leading under-bar.
# the library itself may not be profiled, but it doesnt
#   need the "exit" module; "exit" has to be recrafted
#   from csu/mcrt0.s
#
# VERDICT = libp1
libprof.a:	$(VERDICT).a
	/bin/rm -f $@; ln $? $@
#
# mcrt0 is viewmaster's profiling "crt0" file
# 
libp1.a:	libsys.a mcrt0.o vexit.o
	cp libsys.a $@
	ar ruv $@ vexit.o
#
#  machine, libc, environment tools
#
MACH	= u3b
LIBC	= /usr/src/lib/libc
M_LIB	= ${LIBC}/${MACH}
M4	= m4 ${M_LIB}/m4.def	
MCDEF	= ${M_LIB}/mcount.def
#
# turn exit into _exit and _exit into __exit
#
CRT0	= ${M_LIB}/csu/mcrt0.s
MCOUNT	= ${M_LIB}/crt/mcount.s
mcrt0.s: ${CRT0} 
	sed 's/exit/_exit/g' $?|$(M4) $(MCDEF) ->$@
vexit.s: ${M_LIB}/sys/exit.s
	sed 's/exit/_exit/g' $? |$(M4) $(MCDEF) -> $@

#
#  method 2. define a cleanup routine from the 
#   local exit routine and the unix cleanup (stdio/flsbuf.c)
#
libp2.a:	libsys.a v_clean.o
	/bin/rm -f $@; cp libsys.a $@; ar d libsys.a exit.0
	ar ruv libsys.a v_clean.o
#
#  ONLY when L*PROFiling, 
# FACE Cleanup ={ port/stdio/flsbuf.c exit.c }{ ... }.
# where _cleanup => __cleanup
#  &    exit     => _cleanup
#		
v_clean.c:	exit.c ${LIBC}/port/stdio/flsbuf.c sys.mk
	(echo "void _cleanup();" ;\
	 sed -e 's/^exit.*/cleanup(n)/'\
	     -e '/_exit/d' exit.c;\
	 echo "void";\
	 sed -n '/cleanup/,/}/p' ${LIBC}/port/stdio/flsbuf.c;\
	)|\
	 sed -e 's/cleanup/_&/g' >$@
#

.PRECIOUS:	$(LIBRARY)
