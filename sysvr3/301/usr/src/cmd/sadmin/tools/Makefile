#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin:tools/Makefile	2.4"
#	Makefile for tools used in building sadmin
#	Generated Tue Oct 30 21:36:09 EST 1984 from skeleton makefile:
#  @(#)SKELMakefile  1.4  /sccs/src/cmd/sadmin/tools/s.SKELMakefile

ROOT = /

CMDS = cmdsused genmakefile mkprod.cpio mkproto stats subcommands 

PARTS = $(CMDS) INSTALL Makefile RUN SKELMakefile 

ALL:		$(PARTS)
	chmod go-w $(PARTS)
	chmod +x $(CMDS)

install:

remove:

partslist:	$(PARTS)
	@echo $(PARTS)  |  tr ' ' '\012'  |  sort
	
product productdir:

clean:

clobber:

srcaudit:
	@(	find * -print;  \
		ls -d $(DIRECTORIES) $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	genmakefile SKELMakefile ALL
	cp Makefile OMakefile
	./genmakefile SKELMakefile OMakefile >Makefile
	rm -f OMakefile
