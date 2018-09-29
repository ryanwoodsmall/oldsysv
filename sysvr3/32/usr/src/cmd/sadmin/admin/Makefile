#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sadmin:admin/Makefile	2.12.1.1"
#	Makefile for admin
#    @(#)SKELMakefile  2.3  /sccs/src/cmd/sadmin/admin/s.SKELMakefile

ADMINHOME = $(ROOT)/usr/admin
BIN = $(ROOT)/usr/bin

CMDS = checkfsys makefsys mountfsys powerdown setup sysadm umountfsys 

C_CMD = shade

DIRECTORIES = menu menu/diagnostics menu/diskmgmt menu/filemgmt \
menu/machinemgmt menu/packagemgmt menu/softwaremgmt menu/syssetup \
menu/ttymgmt menu/usermgmt menu/usermgmt/modgroup menu/usermgmt/moduser 

PARTS = $(CMDS) gettyvalues menu/DESC menu/diagnostics/DESC \
menu/diagnostics/diskrepair menu/diagnostics/diskreport menu/diskmgmt/DESC \
menu/diskmgmt/checkfsys menu/diskmgmt/cpdisk menu/diskmgmt/erase \
menu/diskmgmt/makefsys menu/diskmgmt/mountfsys menu/diskmgmt/umountfsys \
menu/filemgmt/DESC menu/filemgmt/backup menu/filemgmt/diskuse \
menu/filemgmt/fileage menu/filemgmt/filesize menu/filemgmt/restore \
menu/filemgmt/store menu/machinemgmt/DESC menu/machinemgmt/firmware \
menu/machinemgmt/powerdown menu/machinemgmt/reboot menu/machinemgmt/whoson \
menu/packagemgmt/DESC menu/softwaremgmt/DESC menu/softwaremgmt/install \
menu/softwaremgmt/installpkg menu/softwaremgmt/listpkg \
menu/softwaremgmt/removepkg menu/softwaremgmt/runpkg \
menu/softwaremgmt/uninstall menu/syssetup/DESC menu/syssetup/admpasswd \
menu/syssetup/datetime menu/syssetup/nodename menu/syssetup/setup \
menu/syssetup/syspasswd menu/ttymgmt/DESC menu/ttymgmt/lineset \
menu/ttymgmt/mklineset menu/ttymgmt/modtty menu/usermgmt/DESC \
menu/usermgmt/addgroup menu/usermgmt/adduser menu/usermgmt/delgroup \
menu/usermgmt/deluser menu/usermgmt/lsgroup menu/usermgmt/lsuser \
menu/usermgmt/modadduser menu/usermgmt/modgroup/DESC \
menu/usermgmt/modgroup/chgname menu/usermgmt/moduser/DESC \
menu/usermgmt/moduser/chgloginid menu/usermgmt/moduser/chgpasswd \
menu/usermgmt/moduser/chgshell profile.dot unixadmin 

ALL: \
		$(PARTS) $(C_CMD) passwd

passwd:	\
		$(CMDS)
	echo $(CMDS)  |  tr ' ' '\012'  | \
	  sed 's;.*;&::0:0:administration login:$(ADMINHOME):/usr/bin/&;' \
>passwd

install: \
		$(PARTS) $(BIN) $(C_CMD) remove passwd
	: 'Must be run by root or the owner of /usr and $(BIN) (sys)'
	@echo '\nThe following entries must be added to /etc/passwd:'
	@cat passwd
	@echo "\\nSee the "passwd" file in `pwd`\\n"
	-umask 022;  if test ! -d $(ADMINHOME); then mkdir $(ADMINHOME); fi;  cd $(ADMINHOME)
	for dir in $(DIRECTORIES); do if test ! -d $$dir; then mkdir $$dir; fi; done
	umask 022;  ls $(PARTS)  |  cpio -pd $(ADMINHOME)
	cd $(ADMINHOME);  chmod u+xw,go-xw $(CMDS)
	cd $(ADMINHOME);  ln -f profile.dot .profile
	install -f $(BIN) shade
	echo $(CMDS) | tr ' ' '\012' | \
	  xargs -i -t ln -f $(BIN)/shade $(BIN)/{}

$(C_CMD): 
	$(CC) -s -o shade shade.c

remove:
	if test -d $(ADMINHOME); then cd $(ADMINHOME);  rm -f $(PARTS) .profile; \
	A=`echo $(DIRECTORIES)  |  tr ' ' '\012'  |  sort -r`; \
	for dir in $$A; do if test -d $$dir; then if test `ls $$dir | wc -l` -eq 0; then rmdir $$dir; fi; fi; done;  \
	cd $(BIN); \
	if test `ls $(ADMINHOME) | wc -l` -eq 0; then rmdir $(ADMINHOME); fi; \
	fi
	cd $(BIN);  rm -f $(CMDS)

$(BIN):
	mkdir $(BIN)
	chmod 755 $(BIN)

clean clobber:
	rm -f passwd shade $(BIN)/shade

partslist:
	@echo Makefile SKELMakefile $(PARTS)  |  tr ' ' '\012'  |  sort

product:
	@echo .profile $(PARTS)  |  tr ' ' '\012'  | \
		sed 's;^;$(ADMINHOME)/;'
	@echo $(CMDS)  |  tr ' ' '\012'  | \
		sed 's;.*;$(BIN)/&	--linked to each other--;'

productdir:
	@echo $(BIN)
	@echo $(ADMINHOME)
	@echo $(DIRECTORIES)  |  tr ' ' '\012'  | \
		sed 's;^;$(ADMINHOME)/;'

srcaudit:
	@(	find * -print;  \
		ls -d $(DIRECTORIES) $(PARTS) SKELMakefile Makefile  \
	)  |  sort  |  uniq -u  |  sed 's/$$/	unexpected/'

newmakefile:	../tools/genmakefile SKELMakefile
	cp Makefile OMakefile
	../tools/genmakefile SKELMakefile *Makefile passwd >Makefile
	rm -f OMakefile
