#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin3b2:admin/Makefile	1.4"
#	Makefile for admin
#	Generated Tue Apr 23 19:01:19 EST 1985 from skeleton makefile:
#ident  "sadmin3b2:admin/SKELMakefile  1.2"

ADMINHOME = $(ROOT)/usr/admin

CMDS = 

DIRECTORIES = menu menu/diskmgmt menu/diskmgmt/harddisk menu/filemgmt \
menu/filemgmt/bupsched menu/machinemgmt 

PARTS = $(CMDS) bupsched menu/diskmgmt/format menu/diskmgmt/harddisk/DESC \
menu/diskmgmt/harddisk/display menu/diskmgmt/harddisk/makehdfsys \
menu/diskmgmt/harddisk/partitioning menu/diskmgmt/harddisk/rmdisk \
menu/filemgmt/bupsched/DESC menu/filemgmt/bupsched/schedcheck \
menu/filemgmt/bupsched/schedmsg menu/machinemgmt/autold \
menu/machinemgmt/floppykey 

ALL: \
		$(PARTS)

install: \
		$(PARTS) remove
	: 'Must be run by root or the owner of /usr (sys)'
	-umask 022;  mkdir $(ADMINHOME);  cd $(ADMINHOME); \
	  mkdir $(DIRECTORIES)
	umask 022;  ls $(PARTS)  |  cpio -pd $(ADMINHOME)

remove:
	-cd $(ADMINHOME);  rm -f $(PARTS)
	-cd $(ADMINHOME);  \
	  rmdir `echo $(DIRECTORIES)  |  tr ' ' '\012'  |  sort -r`
	-rmdir $(ADMINHOME)

clean clobber:

partslist:
	@echo Makefile SKELMakefile $(PARTS)  |  tr ' ' '\012'  |  sort

product:
	@echo $(PARTS)  |  tr ' ' '\012'  | \
		sed 's;^;$(ADMINHOME)/;'

productdir:
	@echo $(ADMINHOME)
	@echo $(DIRECTORIES)  |  tr ' ' '\012'  | \
		sed 's;^;$(ADMINHOME)/;'

srcaudit:
	@(	find * -print;  \
		ls -d $(DIRECTORIES) $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	../../sadmin/tools/genmakefile SKELMakefile
	cp Makefile OMakefile
	../../sadmin/tools/genmakefile SKELMakefile *Makefile  >Makefile
	rm -f OMakefile
