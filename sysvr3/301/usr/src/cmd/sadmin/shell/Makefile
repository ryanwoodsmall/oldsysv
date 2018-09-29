#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin:shell/Makefile	2.3"
#	Makefile for shell
#	Generated Mon Aug 19 17:26:19 EDT 1985 from skeleton makefile:
#  sadmin:shell/SKELMakefile  1.4  /sccs/src/cmd/sadmin/shell/s.SKELMakefile

ROOT = /

DIR = $(ROOT)/usr/lbin

CMDS = admerr agefile disklabel diskumount drivename filecheck labelfsname \
largest lightenfs mktable ncpio oldfile rmjunk samedev selectdevice \
selpattern spacewatch 

PARTS = $(CMDS) restorefiles 

ALL:		$(PARTS)

install:	$(PARTS) $(DIR) remove
	umask 022;  ls $(PARTS)  |  cpio -pdum $(DIR)
	cd $(DIR);  chmod go-w,u+w $(PARTS)
	cd $(DIR);  chmod +x $(CMDS)

$(DIR):
	mkdir $(DIR);  chmod 755 $(DIR)

remove:
	cd $(DIR);  rm -f $(PARTS)

partslist:
	@echo Makefile SKELMakefile $(PARTS)  |  tr ' ' '\012'  |  sort
	
product:
	@echo $(PARTS)  |  tr ' ' '\012'  |  sed "s;^;$(DIR)/;"

productdir:
	@echo $(DIR)

clean:

clobber:

srcaudit:
	@(	find * -print;  \
		ls -d $(DIRECTORIES) $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	../tools/genmakefile SKELMakefile
	cp Makefile OMakefile
	../tools/genmakefile SKELMakefile *Makefile >Makefile
	rm -f OMakefile
