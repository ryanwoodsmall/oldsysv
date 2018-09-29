#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.oh:src/oh/oh.mk	1.15"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=liboh.a
HEADER1=../inc
INCLUDE= -I$(HEADER1)
DFLAGS=	-DWISH
CFLAGS=	-O
AR=	ar


$(LIBRARY): \
		$(LIBRARY)(action.o) \
		$(LIBRARY)(alias.o) \
		$(LIBRARY)(cmd.o) \
		$(LIBRARY)(detab.o) \
		$(LIBRARY)(detect.o) \
		$(LIBRARY)(dispfuncs.o) \
		$(LIBRARY)(evstr.o) \
		$(LIBRARY)(externoot.o) \
		$(LIBRARY)(fm_mn_par.o) \
		$(LIBRARY)(getval.o) \
		$(LIBRARY)(helptext.o) \
		$(LIBRARY)(if_ascii.o) \
		$(LIBRARY)(if_dir.o) \
		$(LIBRARY)(if_init.o) \
		$(LIBRARY)(if_exec.o) \
		$(LIBRARY)(if_form.o) \
		$(LIBRARY)(if_help.o) \
		$(LIBRARY)(if_menu.o) \
		$(LIBRARY)(ifuncs.o) \
		$(LIBRARY)(is_objtype.o) \
		$(LIBRARY)(misc.o) \
		$(LIBRARY)(namecheck.o) \
		$(LIBRARY)(nextpart.o) \
		$(LIBRARY)(obj_to_opt.o) \
		$(LIBRARY)(obj_to_par.o) \
		$(LIBRARY)(odftread.o) \
		$(LIBRARY)(odikey.o) \
		$(LIBRARY)(oh_init.o) \
		$(LIBRARY)(ootpart.o) \
		$(LIBRARY)(ootread.o) \
		$(LIBRARY)(opt_rename.o) \
		$(LIBRARY)(optab.o) \
		$(LIBRARY)(optabfuncs.o) \
		$(LIBRARY)(ott_mv.o) \
		$(LIBRARY)(partab.o) \
		$(LIBRARY)(partabfunc.o) \
		$(LIBRARY)(pathtitle.o) \
		$(LIBRARY)(path_to_vp.o) \
		$(LIBRARY)(pathott.o) \
		$(LIBRARY)(scram.o) \
		$(LIBRARY)(slk.o) \
		$(LIBRARY)(suffuncs.o) \
		$(LIBRARY)(typefuncs.o) \
		$(LIBRARY)(typetab.o)

$(LIBRARY)(action.o):	action.c $(HEADER1)/wish.h ../inc/token.h ../inc/var_arrays.h $(HEADER1)/if_menu.h $(HEADER1)/moremacros.h

$(LIBRARY)(alias.o):	alias.c $(HEADER1)/wish.h ../inc/token.h ../inc/slk.h ../inc/actrec.h ../inc/ctl.h $(HEADER1)/moremacros.h

$(LIBRARY)(cmd.o):	cmd.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h ./fm_mn_par.h $(HEADER1)/moremacros.h $(HEADER1)/eval.h

$(LIBRARY)(detab.o):	detab.c $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/detabdefs.h

$(LIBRARY)(detect.o):	detect.c $(HEADER1)/var_arrays.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/detabdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/parse.h

$(LIBRARY)(dispfuncs.o):	dispfuncs.c $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/partabdefs.h $(HEADER1)/moremacros.h

$(LIBRARY)(externoot.o):	externoot.c $(HEADER1)/tsys.h

$(LIBRARY)(fm_mn_par.o):	fm_mn_par.c ./fm_mn_par.h $(HEADER1)/wish.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/eval.h $(HEADER1)/vtdefs.h $(HEADER1)/actrec.h $(HEADER1)/token.h $(HEADER1)/moremacros.h

$(LIBRARY)(getval.o):	getval.c ./fm_mn_par.h ./objform.h $(HEADER1)/wish.h $(HEADER1)/form.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/eval.h $(HEADER1)/winp.h $(HEADER1)/token.h

$(LIBRARY)(helptext.o):	helptext.c

$(LIBRARY)(evstr.o):	evstr.c $(HEADER1)/wish.h $(HEADER1)/form.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/eval.h $(HEADER1)/winp.h $(HEADER1)/token.h ./fm_mn_par.h $(HEADER1)/moremacros.h

$(LIBRARY)(if_ascii.o):	if_ascii.c $(HEADER1)/wish.h $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/typetab.h $(HEADER1)/obj.h $(HEADER1)/retcds.h $(HEADER1)/procdefs.h

$(LIBRARY)(if_dir.o):	if_dir.c $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/ctl.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/typetab.h $(HEADER1)/moremacros.h

$(LIBRARY)(if_init.o):	if_init.c ./fm_mn_par.h $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/var_arrays.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/color_pair.h $(HEADER1)/attrs.h $(HEADER1)/moremacros.h

$(LIBRARY)(if_exec.o):	if_exec.c $(HEADER1)/wish.h $(HEADER1)/terror.h

$(LIBRARY)(if_form.o):	if_form.c ./fm_mn_par.h ./objform.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/token.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/form.h $(HEADER1)/terror.h $(HEADER1)/var_arrays.h $(HEADER1)/typetab.h $(HEADER1)/winp.h $(HEADER1)/eval.h $(HEADER1)/moremacros.h $(HEADER1)/message.h

$(LIBRARY)(if_help.o):	if_help.c ./fm_mn_par.h ./objhelp.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/form.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/typetab.h $(HEADER1)/winp.h $(HEADER1)/token.h $(HEADER1)/moremacros.h

$(LIBRARY)(if_menu.o):	if_menu.c ./fm_mn_par.h ./objmenu.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/var_arrays.h $(HEADER1)/terror.h $(HEADER1)/if_menu.h $(HEADER1)/typetab.h $(HEADER1)/moremacros.h

$(LIBRARY)(ifuncs.o):	ifuncs.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/obj.h $(HEADER1)/optabdefs.h $(HEADER1)/retcds.h $(HEADER1)/windefs.h $(HEADER1)/moremacros.h $(HEADER1)/token.h

$(LIBRARY)(is_objtype.o):	is_objtype.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/detabdefs.h $(HEADER1)/typetab.h

$(LIBRARY)(misc.o):	misc.c $(HEADER1)/tsys.h $(HEADER1)/moremacros.h

$(LIBRARY)(namecheck.o):	namecheck.c $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/partabdefs.h

$(LIBRARY)(nextpart.o):	nextpart.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(obj_to_opt.o):	obj_to_opt.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(obj_to_par.o):	obj_to_par.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/optabdefs.h

$(LIBRARY)(odftread.o):	odftread.c $(HEADER1)/terror.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/detabdefs.h $(HEADER1)/retcds.h $(HEADER1)/mio.h

$(LIBRARY)(odikey.o):	odikey.c $(HEADER1)/var_arrays.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/moremacros.h

$(LIBRARY)(oh_init.o):	oh_init.c $(HEADER1)/wish.h $(HEADER1)/typetab.h

$(LIBRARY)(ootpart.o):	ootpart.c $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/mess.h

$(LIBRARY)(ootread.o):	ootread.c $(HEADER1)/terror.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/ifuncdefs.h $(HEADER1)/mess.h $(HEADER1)/mio.h

$(LIBRARY)(opt_rename.o):	opt_rename.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(optab.o):	optab.c $(HEADER1)/tsys.h $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h

$(LIBRARY)(optabfuncs.o):	optabfuncs.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(ott_mv.o):	ott_mv.c $(HEADER1)/tsys.h $(HEADER1)/if_menu.h $(HEADER1)/typetab.h $(HEADER1)/partabdefs.h

$(LIBRARY)(partab.o):	partab.c $(HEADER1)/tsys.h $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(partabfunc.o):	partabfunc.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(pathtitle.o):	pathtitle.c $(HEADER1)/wish.h

$(LIBRARY)(path_to_vp.o):	path_to_vp.c  $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/obj.h

$(LIBRARY)(pathott.o):	pathott.c $(HEADER1)/typetab.h

$(LIBRARY)(scram.o):	scram.c $(HEADER1)/typetab.h $(HEADER1)/obj.h $(HEADER1)/partabdefs.h $(HEADER1)/parse.h $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/exception.h $(HEADER1)/token.h $(HEADER1)/vtdefs.h $(HEADER1)/winp.h $(HEADER1)/retcds.h $(HEADER1)/moremacros.h

$(LIBRARY)(slk.o):	slk.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h ./fm_mn_par.h $(HEADER1)/moremacros.h

$(LIBRARY)(suffuncs.o):	suffuncs.c

$(LIBRARY)(typefuncs.o):	typefuncs.c $(HEADER1)/var_arrays.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/partabdefs.h $(HEADER1)/moremacros.h

$(LIBRARY)(typetab.o):	typetab.c $(HEADER1)/var_arrays.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/partabdefs.h $(HEADER1)/moremacros.h

.c.a:
	$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets #####

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
