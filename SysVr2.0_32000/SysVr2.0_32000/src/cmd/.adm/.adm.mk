

ROOT =
LIB = $(ROOT)//usr/lib
UUCP = $(LIB)/uucp
CRONTABS = $(ROOT)/usr/spool/cron/crontabs
LIBCRON = $(LIB)/cron
INSDIR = $(ROOT)/etc
TOUCH=:

all::	.proto L-devices L-dialcodes L.cmds L.sys adm at.allow checkall \
	cron.allow filesave group motd profile queuedefs root \
	shutdown sys tapesave uucp uudemon.day uudemon.hr uudemon.wk \
	bcheckrc brc checklist gettydefs inittab master passwd powerfail \
	rc system system.mtc11 system.mtc12 system.un32 \
	USERFILE ioctl.syscon null

clobber:

install:
	make -f .adm.mk $(ARGS)

adm::
	cp adm $(CRONTABS)/adm
	chmod 644 $(CRONTABS)/adm
	chgrp sys $(CRONTABS)/adm
	$(TOUCH) 0101000070 $(CRONTABS)/adm
	chown root $(CRONTABS)/adm

root::
	cp root $(CRONTABS)/root
	chmod 644 $(CRONTABS)/root
	chgrp sys $(CRONTABS)/root
	$(TOUCH) 0101000070 $(CRONTABS)/root
	chown root $(CRONTABS)/root

sys::
	cp sys $(CRONTABS)/sys
	chmod 644 $(CRONTABS)/sys
	chgrp sys $(CRONTABS)/sys
	$(TOUCH) 0101000070 $(CRONTABS)/sys
	chown root $(CRONTABS)/sys

uucp::
	cp uucp $(CRONTABS)/uucp
	chmod 644 $(CRONTABS)/uucp
	chgrp sys $(CRONTABS)/uucp
	$(TOUCH) 0101000070 $(CRONTABS)/uucp
	chown root $(CRONTABS)/uucp

.proto::
	cp .proto $(LIBCRON)/.proto
	chmod 744 $(LIBCRON)/.proto
	chgrp sys $(LIBCRON)/.proto
	$(TOUCH) 0101000070 $(LIBCRON)/.proto
	chown root $(LIBCRON)/.proto

at.allow::
	cp at.allow $(LIBCRON)/at.allow
	chmod 644 $(LIBCRON)/at.allow
	chgrp sys $(LIBCRON)/at.allow
	$(TOUCH) 0101000070 $(LIBCRON)/at.allow
	chown root $(LIBCRON)/at.allow

cron.allow::
	cp cron.allow $(LIBCRON)/cron.allow
	chmod 644 $(LIBCRON)/cron.allow
	chgrp sys $(LIBCRON)/cron.allow
	$(TOUCH) 0101000070 $(LIBCRON)/cron.allow
	chown root $(LIBCRON)/cron.allow

queuedefs::
	cp queuedefs $(LIBCRON)/queuedefs
	chmod 644 $(LIBCRON)/queuedefs
	chgrp sys $(LIBCRON)/queuedefs
	$(TOUCH) 0101000070 $(LIBCRON)/queuedefs
	chown root $(LIBCRON)/queuedefs


bcheckrc::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp bcheckrc.sh $(INSDIR)/bcheckrc;\
	chmod 744 $(INSDIR)/bcheckrc;\
	chgrp sys $(INSDIR)/bcheckrc;\
	$(TOUCH) 0101000070 $(INSDIR)/bcheckrc;\
	chown root $(INSDIR)/bcheckrc

brc::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp brc.sh $(INSDIR)/brc;\
	chmod 744 $(INSDIR)/brc;\
	chgrp sys $(INSDIR)/brc;\
	$(TOUCH) 0101000070 $(INSDIR)/brc;\
	chown root $(INSDIR)/brc

checkall:	checkall.sh
	cp checkall.sh $(INSDIR)/checkall
	chmod 744 $(INSDIR)/checkall
	chgrp bin $(INSDIR)/checkall
	$(TOUCH) 0101000070 $(INSDIR)/checkall
	chown bin $(INSDIR)/checkall

checklist::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp checklist $(INSDIR)/checklist;\
	chmod 664 $(INSDIR)/checklist;\
	chgrp sys $(INSDIR)/checklist;\
	$(TOUCH) 0101000070 $(INSDIR)/checklist;\
	chown root $(INSDIR)/checklist

filesave:	filesave.sh
	cp filesave.sh $(INSDIR)/filesave
	chmod 744 $(INSDIR)/filesave
	chgrp sys $(INSDIR)/filesave
	$(TOUCH) 0101000070 $(INSDIR)/filesave
	chown root $(INSDIR)/filesave

gettydefs::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp gettydefs.sh $(INSDIR)/gettydefs;\
	chmod 644 $(INSDIR)/gettydefs;\
	chgrp sys $(INSDIR)/gettydefs;\
	$(TOUCH) 0101000070 $(INSDIR)/gettydefs;\
	chown root $(INSDIR)/gettydefs

group::
	cp group $(INSDIR)/group
	chmod 644 $(INSDIR)/group
	chgrp sys $(INSDIR)/group
	$(TOUCH) 0101000070 $(INSDIR)/group
	chown root $(INSDIR)/group

inittab::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp inittab.sh $(INSDIR)/inittab;\
	chmod 644 $(INSDIR)/inittab;\
	chgrp sys $(INSDIR)/inittab;\
	$(TOUCH) 0101000070 $(INSDIR)/inittab;\
	chown root $(INSDIR)/inittab

ioctl.syscon::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp ioctl.syscon $(INSDIR)/ioctl.syscon;\
	chmod 644 $(INSDIR)/ioctl.syscon;\
	chgrp sys $(INSDIR)/ioctl.syscon;\
	$(TOUCH) 0101000070 $(INSDIR)/ioctl.syscon;\
	chown root $(INSDIR)/ioctl.syscon

master::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp master $(INSDIR)/master;\
	chmod 644 $(INSDIR)/master;\
	chgrp sys $(INSDIR)/master;\
	chown root $(INSDIR)/master

motd:	motd.sh
	cp motd.sh $(INSDIR)/motd
	chmod 644 $(INSDIR)/motd
	chgrp sys $(INSDIR)/motd
	chown root $(INSDIR)/motd

passwd::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp passwd $(INSDIR)/passwd;\
	chmod 644 $(INSDIR)/passwd;\
	chgrp sys $(INSDIR)/passwd;\
	$(TOUCH) 0101000070 $(INSDIR)/passwd;\
	chown root $(INSDIR)/passwd

powerfail::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp powerfail.sh $(INSDIR)/powerfail;\
	chmod 744 $(INSDIR)/powerfail;\
	chgrp sys $(INSDIR)/powerfail;\
	$(TOUCH) 0101000070 $(INSDIR)/powerfail;\
	chown root $(INSDIR)/powerfail

profile:	profile.sh
	cp profile.sh $(INSDIR)/profile
	chmod 664 $(INSDIR)/profile
	chgrp bin $(INSDIR)/profile
	$(TOUCH) 0101000070 $(INSDIR)/profile
	chown bin $(INSDIR)/profile

rc::
	-if /bin/vax;\
	then cd vax;\
	elif /bin/ns32000;\
	then cd ns32000;\
	elif /bin/pdp11;\
	then cd pdp11;\
	elif /bin/u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp rc.sh $(INSDIR)/rc;\
	chmod 744 $(INSDIR)/rc;\
	chgrp sys $(INSDIR)/rc;\
	$(TOUCH) 0101000070 $(INSDIR)/rc;\
	chown root $(INSDIR)/rc

shutdown:	shutdown.sh
	cp shutdown.sh $(INSDIR)/shutdown
	chmod 744 $(INSDIR)/shutdown
	chgrp sys $(INSDIR)/shutdown
	$(TOUCH) 0101000070 $(INSDIR)/shutdown
	chown root $(INSDIR)/shutdown

tapesave:	tapesave.sh
	cp tapesave.sh $(INSDIR)/tapesave
	chmod 744 $(INSDIR)/tapesave
	chgrp sys $(INSDIR)/tapesave
	$(TOUCH) 0101000070 $(INSDIR)/tapesave
	chown root $(INSDIR)/tapesave

system::
	-if /bin/u3b;\
	then cd u3b;\
		cp system $(INSDIR)/system;\
		chmod 644 $(INSDIR)/system;\
		chgrp sys $(INSDIR)/system;\
		$(TOUCH) 0101000070 $(INSDIR)/system;\
		chown root $(INSDIR)/system;\
	fi

system.mtc11::
	-if /bin/u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc11;\
		chmod 644 $(INSDIR)/system.mtc11;\
		chgrp sys $(INSDIR)/system.mtc11;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc11;\
		chown root $(INSDIR)/system.mtc11;\
	fi

system.mtc12::
	-if /bin/u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc12;\
		chmod 644 $(INSDIR)/system.mtc12;\
		chgrp sys $(INSDIR)/system.mtc12;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc12;\
		chown root $(INSDIR)/system.mtc12;\
	fi

system.un32::
	-if /bin/u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.un32;\
		chmod 644 $(INSDIR)/system.un32;\
		chgrp sys $(INSDIR)/system.un32;\
		$(TOUCH) 0101000070 $(INSDIR)/system.un32;\
		chown root $(INSDIR)/system.un32;\
	fi

uudemon.hr:	uudemon.hr.sh
	cp uudemon.hr.sh $(UUCP)/uudemon.hr
	chmod 755 $(UUCP)/uudemon.hr
	chgrp bin $(UUCP)/uudemon.hr
	$(TOUCH) 0101000070 $(UUCP)/uudemon.hr
	chown uucp $(UUCP)/uudemon.hr

uudemon.day:	uudemon.day.sh
	cp uudemon.day.sh $(UUCP)/uudemon.day
	chmod 755 $(UUCP)/uudemon.day
	chgrp bin $(UUCP)/uudemon.day
	$(TOUCH) 0101000070 $(UUCP)/uudemon.day
	chown uucp $(UUCP)/uudemon.day

uudemon.wk:	uudemon.wk.sh
	cp uudemon.wk.sh $(UUCP)/uudemon.wk
	chmod 755 $(UUCP)/uudemon.wk
	chgrp bin $(UUCP)/uudemon.wk
	$(TOUCH) 0101000070 $(UUCP)/uudemon.wk
	chown uucp $(UUCP)/uudemon.wk

L-devices::
	cp L-devices $(UUCP)/L-devices
	chmod 644 $(UUCP)/L-devices
	chgrp bin $(UUCP)/L-devices
	$(TOUCH) 0101000070 $(UUCP)/L-devices
	chown uucp $(UUCP)/L-devices

L-dialcodes::
	cp L-dialcodes $(UUCP)/L-dialcodes
	chmod 644 $(UUCP)/L-dialcodes
	chgrp bin $(UUCP)/L-dialcodes
	$(TOUCH) 0101000070 $(UUCP)/L-dialcodes
	chown uucp $(UUCP)/L-dialcodes

L.cmds::
	cp L.cmds $(UUCP)/L.cmds
	chmod 444 $(UUCP)/L.cmds
	chgrp bin $(UUCP)/L.cmds
	$(TOUCH) 0101000070 $(UUCP)/L.cmds
	chown uucp $(UUCP)/L.cmds

L.sys::
	cp L.sys $(UUCP)/L.sys
	chmod 400 $(UUCP)/L.sys
	chgrp bin $(UUCP)/L.sys
	$(TOUCH) 0101000070 $(UUCP)/L.sys
	chown uucp $(UUCP)/L.sys

USERFILE::
	cp USERFILE $(UUCP)/USERFILE
	chmod 400 $(UUCP)/USERFILE
	chgrp bin $(UUCP)/USERFILE
	$(TOUCH) 0101000070 $(UUCP)/USERFILE
	chown uucp $(UUCP)/USERFILE

null:
	> /etc/log/filesave.log
	chmod 664 /etc/log/filesave.log
	chgrp sys /etc/log/filesave.log
	$(TOUCH) 0101000070 /etc/log/filesave.log
	chown root /etc/log/filesave.log

	> /etc/mnttab
	chmod 644 /etc/mnttab
	chgrp sys /etc/mnttab
	$(TOUCH) 0101000070 /etc/mnttab
	chown root /etc/mnttab

	> /etc/utmp
	chmod 664 /etc/utmp
	chgrp bin /etc/utmp
	$(TOUCH) 0101000070 /etc/utmp
	chown bin /etc/utmp

	> /etc/wtmp
	chmod 664 /etc/wtmp
	chgrp adm /etc/wtmp
	$(TOUCH) 0101000070 /etc/wtmp
	chown adm /etc/wtmp

	> /usr/adm/pacct
	chmod 664 /usr/adm/pacct
	chgrp adm /usr/adm/pacct
	$(TOUCH) 0101000070 /usr/adm/pacct
	chown adm /usr/adm/pacct

	> /usr/adm/sulog
	chmod 600 /usr/adm/sulog
	chgrp root /usr/adm/sulog
	$(TOUCH) 0101000070 /usr/adm/sulog
	chown root /usr/adm/sulog

	> /usr/games/advlog
	chmod 666 /usr/games/advlog
	chgrp bin /usr/games/advlog
	$(TOUCH) 0101000070 /usr/games/advlog
	chown bin /usr/games/advlog

	> /usr/lib/cron/log
	chmod 664 /usr/lib/cron/log
	chgrp bin /usr/lib/cron/log
	$(TOUCH) 0101000070 /usr/lib/cron/log
	chown bin /usr/lib/cron/log

	> /usr/lib/spell/spellhist
	chmod 666 /usr/lib/spell/spellhist
	chgrp bin /usr/lib/spell/spellhist
	$(TOUCH) 0101000070 /usr/lib/spell/spellhist
	chown bin /usr/lib/spell/spellhist

	> /usr/lib/uucp/L_stat
	chmod 644 /usr/lib/uucp/L_stat
	chgrp bin /usr/lib/uucp/L_stat
	$(TOUCH) 0101000070 /usr/lib/uucp/L_stat
	chown uucp /usr/lib/uucp/L_stat

	> /usr/lib/uucp/L_sub
	chmod 644 /usr/lib/uucp/L_sub
	chgrp bin /usr/lib/uucp/L_sub
	$(TOUCH) 0101000070 /usr/lib/uucp/L_sub
	chown uucp /usr/lib/uucp/L_sub

	> /usr/lib/uucp/R_stat
	chmod 644 /usr/lib/uucp/R_stat
	chgrp bin /usr/lib/uucp/R_stat
	$(TOUCH) 0101000070 /usr/lib/uucp/R_stat
	chown uucp /usr/lib/uucp/R_stat

	> /usr/lib/uucp/R_sub
	chmod 644 /usr/lib/uucp/R_sub
	chgrp bin /usr/lib/uucp/R_sub
	$(TOUCH) 0101000070 /usr/lib/uucp/R_sub
	chown uucp /usr/lib/uucp/R_sub

	> /usr/spool/lp/outputq
	chmod 644 /usr/spool/lp/outputq
	chgrp bin /usr/spool/lp/outputq
	$(TOUCH) 0101000070 /usr/spool/lp/outputq
	chown lp /usr/spool/lp/outputq

	> /usr/spool/lp/pstatus
	chown lp /usr/spool/lp/pstatus
	chgrp bin /usr/spool/lp/pstatus
	$(TOUCH) 0101000070 /usr/spool/lp/pstatus
	chmod 644 /usr/spool/lp/pstatus

	> /usr/spool/lp/qstatus
	chmod 644 /usr/spool/lp/qstatus
	chgrp bin /usr/spool/lp/qstatus
	$(TOUCH) 0101000070 /usr/spool/lp/qstatus
	chown lp /usr/spool/lp/qstatus


