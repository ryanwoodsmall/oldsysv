#
#	@(#)help.mk	6.5
#

ROOT = 
HELPLIB = $(ROOT)/usr/lib/help

FILES1 = ad bd cb cm cmds co de default
FILES2 = ge he prs rc un ut vc

all:

install: all
	-mkdir $(HELPLIB)
	$(CH)-chmod 775 $(HELPLIB)
	-cd $(HELPLIB); rm -f $(FILES2) $(FILES2)
	cp $(FILES1) $(FILES2) $(HELPLIB)
	-cd $(HELPLIB); $(CH)chmod 664 $(FILES1)	$(FILES2)
	-@cd $(HELPLIB); $(CH)chgrp bin $(FILES1) $(FILES2) .
	-@cd $(HELPLIB); $(CH)chown bin $(FILES1) $(FILES2) .

clean:

clobber:
