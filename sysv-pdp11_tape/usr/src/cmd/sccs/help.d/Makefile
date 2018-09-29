#	@(#)Makefile	1.4 -- SCCS helpfile installation

HELPLIB = /usr/lib/help

FILES1 = ad bd cb cm cmds co de default
FILES2 = ge he prs rc un ut vc

helpfiles:
	-mkdir $(HELPLIB)
	-chmod 775 $(HELPLIB)
	cp $(FILES1) \
		$(FILES2) $(HELPLIB)
	-cd $(HELPLIB); chmod 664 $(FILES1) \
					$(FILES2)
	-@cd $(HELPLIB); chgrp bin $(FILES1) \
					$(FILES2) .
	-@cd $(HELPLIB); chown bin $(FILES1) \
					$(FILES2) .
