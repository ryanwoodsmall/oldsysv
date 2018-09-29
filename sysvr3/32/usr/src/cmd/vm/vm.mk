#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.misc:makefile	1.27"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
USR=$(ROOT)/usr
CFLAGS= -O
LDFLAGS= -s
DIRS =	sys bk proc oh oeu xx vinstall
VHOME=$(USR)/vmsys
OHOME=$(USR)/oasys
HELP=$(VHOME)/HELP
OBJECT=$(VHOME)/OBJECTS
STANDARD=$(VHOME)/standard
VDIRS= $(VHOME) $(HELP) $(OBJECT) $(VHOME)/standard  $(VHOME)/standard/FILECABINET $(VHOME)/standard/WASTEBASKET $(VHOME)/standard/pref
ODIRS=$(OHOME)  $(OHOME)/info $(OHOME)/info/OH $(OHOME)/info/OH/externals $(OHOME)/tmp
MKDIR = mkdir

VHELP=T.add T.mod T.rem T.list T.f.propts T.h1.help T.h2.system T.h3.frames T.h41.obj T.h42.type \
T.h43.fold T.h51.file T.h52.other T.h531.pref T.h532.pass T.h534.dir \
T.h535.perm T.h537.off T.h54.serve T.h55.waste T.h56.unix T.h7.cmd \
T.h94.issue  T.instal T.adm\
T.m.found T.m.frmlist  T.m.office T.exit T.sadm cancel cleanup close \
cmd-menu copy create delete display exit find frm-list frm-mgmt goto help move \
next-frm open organize prev-frm print redescribe refresh rename run \
security select time undelete unix update T.mail T.nmail T.smail

VOBJECT= Form.add Form.del Form.find Form.fold Form.mod Form.office Form.org Form.perm \
Form.prin Form.sec Form.send Init.vm Menu.create \
Menu.find Menu.h0.toc Menu.h1.help Menu.h4.obj Menu.h5.off Menu.h53.pr \
Menu.h6.list Menu.h9.prt Menu.install Menu.list Menu.mail Menu.office Menu.pref Menu.print\
Menu.remove Menu.sadm Menu.serve Menu.users Menu.waste Text.disp Text.ed Text.help \
Text.mfhelp Text.run Text.title Text.ver Menu.Tserv Menu.Tmail

SFILES= .profile 
FFILES=FILECABINET/.pref 
WFILES=WASTEBASKET/.pref 
PFILES=pref/.environ pref/.variables
OAFILES=allobjs detect.tab
INFILES=pathalias

all:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		make -f $$d.mk INC="$(INC)" USRLIB="$(USRLIB)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $@;\
		cd ..;\
	done


install:	all vdirs odirs
		@set -e;\
		for d in $(DIRS);\
		do\
			cd $$d;\
			echo Making $@ in $$d subsystem;\
			make -f $$d.mk $@;\
			cd ..;\
		done
		for i in $(VHELP);\
		do\
			install -f $(HELP) HELP/$$i;\
			$(CH) chmod 664 $(HELP)/$$i;\
		done

		for i in $(VOBJECT);\
		do\
			install -f $(OBJECT) OBJECTS/$$i;\
			$(CH) chmod 664 $(OBJECT)/$$i;\
		done

		for i in $(SFILES);\
		do\
			install -f $(STANDARD) standard/$$i;\
			$(CH) chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(FFILES);\
		do\
			install -f $(STANDARD)/FILECABINET standard/$$i;\
			$(CH) chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(WFILES);\
		do\
			install -f $(STANDARD)/WASTEBASKET standard/$$i;\
			$(CH) chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(PFILES);\
		do\
			install -f $(STANDARD)/pref standard/$$i;\
			$(CH) chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(INFILES);\
		do\
			install -f $(OHOME)/info oasys/info/$$i;\
			$(CH) chmod 664 $(OHOME)/info/$$i;\
		done

		for i in $(INFILES);\
		do\
			install -f $(VHOME) $$i;\
			$(CH) chmod 664 $(VHOME)/$$i;\
		done

		for i in $(OAFILES);\
		do\
			install -f $(OHOME)/info/OH/externals oasys/info/OH/externals/$$i;\
			$(CH) chmod 664 $(OHOME)/info/OH/externals/$$i;\
		done


clean:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clean $@;\
		cd ..;\
	done

clobber: clean
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clobber $@;\
		cd ..;\
	done

vdirs: 	$(VDIRS)

odirs:  $(ODIRS)

$(VDIRS):
		$(MKDIR) $@
		$(CH)chmod 775 $@

$(ODIRS):
		$(MKDIR) $@
		$(CH)chmod 775 $@
