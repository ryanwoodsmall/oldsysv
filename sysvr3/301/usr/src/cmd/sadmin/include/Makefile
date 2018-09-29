#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin:include/Makefile	2.2"
#	Makefile for include
#	Generated Tue Oct 30 22:56:21 EST 1984 from skeleton makefile:
#  @(#)SKELMakefile  2.1  /sccs/src/cmd/sadmin/include/s.SKELMakefile

PARTS = cmderr.h stddef.h switch.h 

ALL:		$(PARTS)

install:	remove

remove:

clean clobber:

partslist:
	@echo Makefile SKELMakefile $(PARTS)  |  tr ' ' '\012'  |  sort
	
product:

productdir:

srcaudit:
	@(	find * -print;  \
		ls -d $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	../tools/genmakefile SKELMakefile
	cp Makefile OMakefile
	../tools/genmakefile SKELMakefile *Makefile >Makefile
	rm -f OMakefile
