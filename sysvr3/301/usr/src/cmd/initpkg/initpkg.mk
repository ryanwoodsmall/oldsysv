#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./initpkg.mk	1.10"

ROOT =
INSDIR = $(ROOT)/etc
TOUCH=$(CH)/bin/touch

SCRIPTS= bcheckrc brc inittab fstab rstab mountall rc0 rc2 rc3 \
	rmount rmountall rumountall shutdown umountall
DIRECTORIES= init.d rc0.d rc2.d rc3.d

all:	scripts directories
	echo "**** initpkg.mk complete ****"

scripts: $(SCRIPTS)

directories: $(DIRECTORIES)

clean:

clobber: clean
	rm -f $(SCRIPTS)

install:
	make -f initpkg.mk $(ARGS)

bcheckrc::
	
	sh bcheckrc.sh $(ROOT)
	cp bcheckrc $(INSDIR)/bcheckrc
	$(CH)chmod 744 $(INSDIR)/bcheckrc
	$(CH)chgrp sys $(INSDIR)/bcheckrc
	$(TOUCH) 0101000070 $(INSDIR)/bcheckrc
	$(CH)chown root $(INSDIR)/bcheckrc

brc::
	sh brc.sh $(ROOT)
	cp brc $(INSDIR)/brc
	$(CH)chmod 744 $(INSDIR)/brc
	$(CH)chgrp sys $(INSDIR)/brc
	$(TOUCH) 0101000070 $(INSDIR)/brc
	$(CH)chown root $(INSDIR)/brc

fstab::
	sh fstab.sh  $(ROOT)
	cp fstab $(INSDIR)/fstab
	$(CH)chmod 664 $(INSDIR)/fstab
	$(CH)chgrp sys $(INSDIR)/fstab
	$(TOUCH) 0101000070 $(INSDIR)/fstab
	$(CH)chown root $(INSDIR)/fstab

rstab::
	sh rstab.sh  $(ROOT)
	cp rstab $(INSDIR)/rstab
	$(CH)chmod 664 $(INSDIR)/rstab
	$(CH)chgrp sys $(INSDIR)/rstab
	$(TOUCH) 0101000070 $(INSDIR)/rstab
	$(CH)chown root $(INSDIR)/rstab

inittab::
	sh inittab.sh $(ROOT)
	cp inittab $(INSDIR)/inittab
	$(CH)chmod 644 $(INSDIR)/inittab
	$(CH)chgrp sys $(INSDIR)/inittab
	$(TOUCH) 0101000070 $(INSDIR)/inittab
	$(CH)chown root $(INSDIR)/inittab

powerfail::
	sh powerfail.sh $(ROOT)
	cp powerfail $(INSDIR)/powerfail
	$(CH)chmod 744 $(INSDIR)/powerfail
	$(CH)chgrp sys $(INSDIR)/powerfail
	$(TOUCH) 0101000070 $(INSDIR)/powerfail
	$(CH)chown root $(INSDIR)/powerfail

mountall::
	sh mountall.sh $(ROOT)
	cp mountall $(INSDIR)/mountall
	$(CH)chmod 744 $(INSDIR)/mountall
	$(CH)chgrp sys $(INSDIR)/mountall
	$(TOUCH) 0101000070 $(INSDIR)/mountall
	$(CH)chown root $(INSDIR)/mountall

rc0::
	sh rc0.sh $(ROOT)
	cp rc0 $(INSDIR)/rc0
	$(CH)chmod 744 $(INSDIR)/rc0
	$(CH)chgrp sys $(INSDIR)/rc0
	$(TOUCH) 0101000070 $(INSDIR)/rc0
	$(CH)chown root $(INSDIR)/rc0

rc2::
	sh rc2.sh $(ROOT)
	cp rc2 $(INSDIR)/rc2
	$(CH)chmod 744 $(INSDIR)/rc2
	$(CH)chgrp sys $(INSDIR)/rc2
	$(TOUCH) 0101000070 $(INSDIR)/rc2
	$(CH)chown root $(INSDIR)/rc2

rc3::
	sh rc3.sh $(ROOT)
	cp rc3 $(INSDIR)/rc3
	$(CH)chmod 744 $(INSDIR)/rc3
	$(CH)chgrp sys $(INSDIR)/rc3
	$(TOUCH) 0101000070 $(INSDIR)/rc3
	$(CH)chown root $(INSDIR)/rc3

rmount::
	sh rmount.sh $(ROOT)
	cp rmount $(INSDIR)/rmount
	$(CH)chmod 744 $(INSDIR)/rmount
	$(CH)chgrp sys $(INSDIR)/rmount
	$(TOUCH) 0101000070 $(INSDIR)/rmount
	$(CH)chown root $(INSDIR)/rmount

rmountall::
	sh rmountall.sh $(ROOT)
	cp rmountall $(INSDIR)/rmountall
	$(CH)chmod 744 $(INSDIR)/rmountall
	$(CH)chgrp sys $(INSDIR)/rmountall
	$(TOUCH) 0101000070 $(INSDIR)/rmountall
	$(CH)chown root $(INSDIR)/rmountall

rumountall::
	sh rumountall.sh $(ROOT)
	cp rumountall $(INSDIR)/rumountall
	$(CH)chmod 744 $(INSDIR)/rumountall
	$(CH)chgrp sys $(INSDIR)/rumountall
	$(TOUCH) 0101000070 $(INSDIR)/rumountall
	$(CH)chown root $(INSDIR)/rumountall

shutdown::
	sh shutdown.sh $(ROOT)
	cp shutdown $(INSDIR)/shutdown
	$(CH)chmod 744 $(INSDIR)/shutdown
	$(CH)chgrp sys $(INSDIR)/shutdown
	$(TOUCH) 0101000070 $(INSDIR)/shutdown
	$(CH)chown root $(INSDIR)/shutdown

umountall::
	sh umountall.sh $(ROOT)
	cp umountall $(INSDIR)/umountall
	$(CH)chmod 744 $(INSDIR)/umountall
	$(CH)chgrp sys $(INSDIR)/umountall
	$(TOUCH) 0101000070 $(INSDIR)/umountall
	$(CH)chown root $(INSDIR)/umountall

init.d::
	cd ./init.d; \
	sh :mk.init.d.sh ROOT=$(ROOT) CH=$(CH)

rc0.d::	
	cd ./rc0.d; \
	sh :mk.rc0.d.sh ROOT=$(ROOT) CH=$(CH)

rc2.d::	
	cd ./rc2.d; \
	sh :mk.rc2.d.sh ROOT=$(ROOT) CH=$(CH)

rc3.d::	
	cd ./rc3.d; \
	sh :mk.rc3.d.sh ROOT=$(ROOT) CH=$(CH)
