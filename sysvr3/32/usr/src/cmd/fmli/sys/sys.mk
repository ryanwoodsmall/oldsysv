#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.sys:src/sys/sys.mk	1.12"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libsys.a
HEADER1=../inc
INCLUDE=	-I$(HEADER1) 

AR=		ar
CFLAGS = 	-O

$(LIBRARY):  \
		$(LIBRARY)(actrec.o) \
		$(LIBRARY)(ar_mfuncs.o) \
		$(LIBRARY)(backslash.o) \
		$(LIBRARY)(chgenv.o) \
		$(LIBRARY)(chgepenv.o) \
		$(LIBRARY)(coproc.o) \
		$(LIBRARY)(copyfile.o) \
		$(LIBRARY)(estrtok.o) \
		$(LIBRARY)(evfuncs.o) \
		$(LIBRARY)(eval.o) \
		$(LIBRARY)(evstr.o) \
		$(LIBRARY)(exit.o) \
		$(LIBRARY)(expand.o) \
		$(LIBRARY)(filename.o) \
		$(LIBRARY)(genfind.o) \
		$(LIBRARY)(getaltenv.o) \
		$(LIBRARY)(getepenv.o) \
		$(LIBRARY)(io.o) \
		$(LIBRARY)(itoa.o) \
		$(LIBRARY)(memshift.o) \
		$(LIBRARY)(mencmds.o) \
		$(LIBRARY)(nstrcat.o) \
		$(LIBRARY)(onexit.o) \
		$(LIBRARY)(parent.o) \
		$(LIBRARY)(putaltenv.o) \
		$(LIBRARY)(readfile.o) \
		$(LIBRARY)(scrclean.o) \
		$(LIBRARY)(spawn.o) \
		$(LIBRARY)(strappend.o) \
		$(LIBRARY)(strCcmp.o) \
		$(LIBRARY)(stream.o) \
		$(LIBRARY)(strsave.o) \
		$(LIBRARY)(tempfiles.o) \
		$(LIBRARY)(terrmess.o) \
		$(LIBRARY)(terror.o) \
		$(LIBRARY)(varappend.o) \
		$(LIBRARY)(varchkapnd.o) \
		$(LIBRARY)(varcreate.o) \
		$(LIBRARY)(vardelete.o) \
		$(LIBRARY)(vargrow.o) \
		$(LIBRARY)(varinsert.o) \
		$(LIBRARY)(varshrink.o) \
		$(LIBRARY)(watch.o)

$(LIBRARY)(actrec.o):	actrec.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/menudefs.h $(HEADER1)/ctl.h $(HEADER1)/terror.h $(HEADER1)/message.h

$(LIBRARY)(ar_mfuncs.o):	ar_mfuncs.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h

$(LIBRARY)(backslash.o):	backslash.c $(HEADER1)/wish.h

$(LIBRARY)(chgenv.o):	chgenv.c

$(LIBRARY)(chgepenv.o):	chgepenv.c $(HEADER1)/wish.h

$(LIBRARY)(coproc.o):	coproc.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/var_arrays.h $(HEADER1)/eval.h $(HEADER1)/moremacros.h

$(LIBRARY)(copyfile.o):	copyfile.c $(HEADER1)/wish.h $(HEADER1)/exception.h

$(LIBRARY)(estrtok.o):	estrtok.c

$(LIBRARY)(evfuncs.o):	evfuncs.c $(HEADER1)/wish.h $(HEADER1)/eval.h $(HEADER1)/ctl.h $(HEADER1)/moremacros.h

$(LIBRARY)(eval.o):	eval.c $(HEADER1)/wish.h $(HEADER1)/eval.h $(HEADER1)/terror.h

$(LIBRARY)(evstr.o):	evstr.c $(HEADER1)/wish.h $(HEADER1)/eval.h

$(LIBRARY)(exit.o):	exit.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(expand.o):	expand.c $(HEADER1)/wish.h $(HEADER1)/moremacros.h

$(LIBRARY)(filename.o):	filename.c

$(LIBRARY)(genfind.o):	genfind.c $(HEADER1)/partabdefs.h $(HEADER1)/typetab.h $(HEADER1)/wish.h $(HEADER1)/var_arrays.h $(HEADER1)/eval.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
	$(AR) rv $@ genfind.o
	/bin/rm -f genfind.o

$(LIBRARY)(getaltenv.o):	getaltenv.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(getepenv.o):	getepenv.c $(HEADER1)/wish.h $(HEADER1)/moremacros.h

$(LIBRARY)(io.o):	io.c $(HEADER1)/wish.h $(HEADER1)/eval.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h

$(LIBRARY)(itoa.o):	itoa.c

$(LIBRARY)(memshift.o):	memshift.c $(HEADER1)/wish.h

$(LIBRARY)(mencmds.o):	mencmds.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/procdefs.h $(HEADER1)/eval.h $(HEADER1)/typetab.h $(HEADER1)/ctl.h $(HEADER1)/moremacros.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
	$(AR) rv $@ mencmds.o
	/bin/rm -f mencmds.o

$(LIBRARY)(nstrcat.o):	nstrcat.c $(HEADER1)/wish.h

$(LIBRARY)(onexit.o):	onexit.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(parent.o):	parent.c $(HEADER1)/wish.h

$(LIBRARY)(putaltenv.o):	putaltenv.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h $(HEADER1)/moremacros.h

$(LIBRARY)(readfile.o):	readfile.c $(HEADER1)/wish.h $(HEADER1)/terror.h

$(LIBRARY)(scrclean.o):	scrclean.c

$(LIBRARY)(spawn.o):	spawn.c

$(LIBRARY)(strappend.o):	strappend.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(strCcmp.o):	strCcmp.c

$(LIBRARY)(stream.o):	stream.c $(HEADER1)/wish.h $(HEADER1)/token.h

$(LIBRARY)(strsave.o):	strsave.c $(HEADER1)/wish.h

$(LIBRARY)(tempfiles.o):	tempfiles.c $(HEADER1)/wish.h $(HEADER1)/retcodes.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h

$(LIBRARY)(terrmess.o):	terrmess.c $(HEADER1)/wish.h $(HEADER1)/terror.h

$(LIBRARY)(terror.o):	terror.c $(HEADER1)/wish.h $(HEADER1)/message.h $(HEADER1)/vtdefs.h $(HEADER1)/terror.h $(HEADER1)/retcodes.h

$(LIBRARY)(varappend.o):	varappend.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(varchkapnd.o):	varchkapnd.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(varcreate.o):	varcreate.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(vardelete.o):	vardelete.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(vargrow.o):	vargrow.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(varinsert.o):	varinsert.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(varshrink.o):	varshrink.c $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(watch.o):	watch.c $(HEADER1)/wish.h


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
