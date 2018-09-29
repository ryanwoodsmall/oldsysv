#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin3b2:etc/Makefile	1.6"
#	Makefile for etc
#	Generated Wed May  1 14:03:24 EDT 1985 from skeleton makefile:
#  sadmin3b2:etc/SKELMakefile  1.1 \
/sccs/src/cmd/sadmin3b2/etc/s.SKELMakefile

ROOT = /

DIR = $(ROOT)/etc

CMDS = bzapunix 

DIRS = rc.d 

PARTS = $(CMDS) bupsched checklist coredirs fstab rc.d/autoconfig rc.d/disks \
rc.d/sysetup system 

ALL:		$(PARTS)

install:	$(PARTS) $(DIR) remove
	umask 022;  ls $(PARTS)  |  cpio -pdum $(DIR)
	cd $(DIR);  chmod go-w,u+w $(PARTS)
	cd $(DIR);  chmod +x $(CMDS)

$(DIR):
	mkdir $(DIR);  chmod 755 $(DIR)

remove:
	cd $(DIR);  rm -f $(PARTS)
	-cd $(DIR);  rmdir $(DIRS)

partslist:
	@echo Makefile SKELMakefile $(PARTS)  |  tr ' ' '\012'  |  sort
	
product:
	@echo $(PARTS)  |  tr ' ' '\012'  |  sed "s;^;$(DIR)/;"

productdir:
	@echo $(DIR)
	@echo $(DIRS)  |  tr ' ' '\012'  |  sed "s;^;$(DIR)/;"

clean:

clobber:

srcaudit:
	@(	find * -print;  \
		ls -d $(DIRS) $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	../../sadmin/tools/genmakefile SKELMakefile
	cp Makefile OMakefile
	../../sadmin/tools/genmakefile SKELMakefile *Makefile >Makefile
	rm -f OMakefile
