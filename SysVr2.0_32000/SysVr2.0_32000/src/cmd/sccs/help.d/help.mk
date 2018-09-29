#
#	@(#)help.mk	6.3
#

ROOT = 
HELPLIB = $(ROOT)/usr/lib/help

FILES1 = ad bd cb cm cmds co de default
FILES2 = ge he prs rc un ut vc

all:

install: all
	-mkdir $(HELPLIB)
	-chmod 775 $(HELPLIB)
	-cd $(HELPLIB); rm -f $(FILES2) $(FILES2)
	cp $(FILES1) $(FILES2) $(HELPLIB)
	-cd $(HELPLIB); chmod 664 $(FILES1)	$(FILES2)
	-@cd $(HELPLIB); chgrp bin $(FILES1) $(FILES2) .
	-@cd $(HELPLIB); chown bin $(FILES1) $(FILES2) .

clean:

clobber:
