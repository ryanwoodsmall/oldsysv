#	@(#).adm.mk	1.15

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
	$(CH)chmod 644 $(CRONTABS)/adm
	$(CH)chgrp sys $(CRONTABS)/adm
	$(TOUCH) 0101000070 $(CRONTABS)/adm
	$(CH)chown root $(CRONTABS)/adm

root::
	cp root $(CRONTABS)/root
	$(CH)chmod 644 $(CRONTABS)/root
	$(CH)chgrp sys $(CRONTABS)/root
	$(TOUCH) 0101000070 $(CRONTABS)/root
	$(CH)chown root $(CRONTABS)/root

sys::
	cp sys $(CRONTABS)/sys
	$(CH)chmod 644 $(CRONTABS)/sys
	$(CH)chgrp sys $(CRONTABS)/sys
	$(TOUCH) 0101000070 $(CRONTABS)/sys
	$(CH)chown root $(CRONTABS)/sys

uucp::
	cp uucp $(CRONTABS)/uucp
	$(CH)chmod 644 $(CRONTABS)/uucp
	$(CH)chgrp sys $(CRONTABS)/uucp
	$(TOUCH) 0101000070 $(CRONTABS)/uucp
	$(CH)chown root $(CRONTABS)/uucp

.proto::
	cp .proto $(LIBCRON)/.proto
	$(CH)chmod 744 $(LIBCRON)/.proto
	$(CH)chgrp sys $(LIBCRON)/.proto
	$(TOUCH) 0101000070 $(LIBCRON)/.proto
	$(CH)chown root $(LIBCRON)/.proto

at.allow::
	cp at.allow $(LIBCRON)/at.allow
	$(CH)chmod 644 $(LIBCRON)/at.allow
	$(CH)chgrp sys $(LIBCRON)/at.allow
	$(TOUCH) 0101000070 $(LIBCRON)/at.allow
	$(CH)chown root $(LIBCRON)/at.allow

cron.allow::
	cp cron.allow $(LIBCRON)/cron.allow
	$(CH)chmod 644 $(LIBCRON)/cron.allow
	$(CH)chgrp sys $(LIBCRON)/cron.allow
	$(TOUCH) 0101000070 $(LIBCRON)/cron.allow
	$(CH)chown root $(LIBCRON)/cron.allow

queuedefs::
	cp queuedefs $(LIBCRON)/queuedefs
	$(CH)chmod 644 $(LIBCRON)/queuedefs
	$(CH)chgrp sys $(LIBCRON)/queuedefs
	$(TOUCH) 0101000070 $(LIBCRON)/queuedefs
	$(CH)chown root $(LIBCRON)/queuedefs


bcheckrc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp bcheckrc.sh $(INSDIR)/bcheckrc;\
	$(CH)chmod 744 $(INSDIR)/bcheckrc;\
	$(CH)chgrp sys $(INSDIR)/bcheckrc;\
	$(TOUCH) 0101000070 $(INSDIR)/bcheckrc;\
	$(CH)chown root $(INSDIR)/bcheckrc

brc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp brc.sh $(INSDIR)/brc;\
	$(CH)chmod 744 $(INSDIR)/brc;\
	$(CH)chgrp sys $(INSDIR)/brc;\
	$(TOUCH) 0101000070 $(INSDIR)/brc;\
	$(CH)chown root $(INSDIR)/brc

checkall:	checkall.sh
	cp checkall.sh $(INSDIR)/checkall
	$(CH)chmod 744 $(INSDIR)/checkall
	$(CH)chgrp bin $(INSDIR)/checkall
	$(TOUCH) 0101000070 $(INSDIR)/checkall
	$(CH)chown bin $(INSDIR)/checkall

checklist::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp checklist $(INSDIR)/checklist;\
	$(CH)chmod 664 $(INSDIR)/checklist;\
	$(CH)chgrp sys $(INSDIR)/checklist;\
	$(TOUCH) 0101000070 $(INSDIR)/checklist;\
	$(CH)chown root $(INSDIR)/checklist

filesave:	filesave.sh
	cp filesave.sh $(INSDIR)/filesave
	$(CH)chmod 744 $(INSDIR)/filesave
	$(CH)chgrp sys $(INSDIR)/filesave
	$(TOUCH) 0101000070 $(INSDIR)/filesave
	$(CH)chown root $(INSDIR)/filesave

gettydefs::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp gettydefs.sh $(INSDIR)/gettydefs;\
	$(CH)chmod 644 $(INSDIR)/gettydefs;\
	$(CH)chgrp sys $(INSDIR)/gettydefs;\
	$(TOUCH) 0101000070 $(INSDIR)/gettydefs;\
	$(CH)chown root $(INSDIR)/gettydefs

group::
	cp group $(INSDIR)/group
	$(CH)chmod 644 $(INSDIR)/group
	$(CH)chgrp sys $(INSDIR)/group
	$(TOUCH) 0101000070 $(INSDIR)/group
	$(CH)chown root $(INSDIR)/group

inittab::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp inittab.sh $(INSDIR)/inittab;\
	$(CH)chmod 644 $(INSDIR)/inittab;\
	$(CH)chgrp sys $(INSDIR)/inittab;\
	$(TOUCH) 0101000070 $(INSDIR)/inittab;\
	$(CH)chown root $(INSDIR)/inittab

ioctl.syscon::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp ioctl.syscon $(INSDIR)/ioctl.syscon;\
	$(CH)chmod 644 $(INSDIR)/ioctl.syscon;\
	$(CH)chgrp sys $(INSDIR)/ioctl.syscon;\
	$(TOUCH) 0101000070 $(INSDIR)/ioctl.syscon;\
	$(CH)chown root $(INSDIR)/ioctl.syscon

master::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp master $(INSDIR)/master;\
	$(CH)chmod 644 $(INSDIR)/master;\
	$(CH)chgrp sys $(INSDIR)/master;\
	$(CH)chown root $(INSDIR)/master

motd:
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	sed 1d motd.sh > $(INSDIR)/motd;\
	$(CH)chmod 644 $(INSDIR)/motd;\
	$(CH)chgrp sys $(INSDIR)/motd;\
	$(CH)chown root $(INSDIR)/motd

passwd::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp passwd $(INSDIR)/passwd;\
	$(CH)chmod 644 $(INSDIR)/passwd;\
	$(CH)chgrp sys $(INSDIR)/passwd;\
	$(TOUCH) 0101000070 $(INSDIR)/passwd;\
	$(CH)chown root $(INSDIR)/passwd

powerfail::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp powerfail.sh $(INSDIR)/powerfail;\
	$(CH)chmod 744 $(INSDIR)/powerfail;\
	$(CH)chgrp sys $(INSDIR)/powerfail;\
	$(TOUCH) 0101000070 $(INSDIR)/powerfail;\
	$(CH)chown root $(INSDIR)/powerfail

profile:	profile.sh
	cp profile.sh $(INSDIR)/profile
	$(CH)chmod 664 $(INSDIR)/profile
	$(CH)chgrp bin $(INSDIR)/profile
	$(TOUCH) 0101000070 $(INSDIR)/profile
	$(CH)chown bin $(INSDIR)/profile

rc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	else cd u3b;\
	fi;\
	cp rc.sh $(INSDIR)/rc;\
	$(CH)chmod 744 $(INSDIR)/rc;\
	$(CH)chgrp sys $(INSDIR)/rc;\
	$(TOUCH) 0101000070 $(INSDIR)/rc;\
	$(CH)chown root $(INSDIR)/rc

shutdown:	shutdown.sh
	cp shutdown.sh $(INSDIR)/shutdown
	$(CH)chmod 744 $(INSDIR)/shutdown
	$(CH)chgrp sys $(INSDIR)/shutdown
	$(TOUCH) 0101000070 $(INSDIR)/shutdown
	$(CH)chown root $(INSDIR)/shutdown

tapesave:	tapesave.sh
	cp tapesave.sh $(INSDIR)/tapesave
	$(CH)chmod 744 $(INSDIR)/tapesave
	$(CH)chgrp sys $(INSDIR)/tapesave
	$(TOUCH) 0101000070 $(INSDIR)/tapesave
	$(CH)chown root $(INSDIR)/tapesave

system::
	-if u3b;\
	then cd u3b;\
		cp system $(INSDIR)/system;\
		$(CH)chmod 644 $(INSDIR)/system;\
		$(CH)chgrp sys $(INSDIR)/system;\
		$(TOUCH) 0101000070 $(INSDIR)/system;\
		$(CH)chown root $(INSDIR)/system;\
	fi

system.mtc11::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc11;\
		$(CH)chmod 644 $(INSDIR)/system.mtc11;\
		$(CH)chgrp sys $(INSDIR)/system.mtc11;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc11;\
		$(CH)chown root $(INSDIR)/system.mtc11;\
	fi

system.mtc12::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc12;\
		$(CH)chmod 644 $(INSDIR)/system.mtc12;\
		$(CH)chgrp sys $(INSDIR)/system.mtc12;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc12;\
		$(CH)chown root $(INSDIR)/system.mtc12;\
	fi

system.un32::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.un32;\
		$(CH)chmod 644 $(INSDIR)/system.un32;\
		$(CH)chgrp sys $(INSDIR)/system.un32;\
		$(TOUCH) 0101000070 $(INSDIR)/system.un32;\
		$(CH)chown root $(INSDIR)/system.un32;\
	fi

uudemon.hr:	uudemon.hr.sh
	cp uudemon.hr.sh $(UUCP)/uudemon.hr
	$(CH)chmod 755 $(UUCP)/uudemon.hr
	$(CH)chgrp bin $(UUCP)/uudemon.hr
	$(TOUCH) 0101000070 $(UUCP)/uudemon.hr
	$(CH)chown uucp $(UUCP)/uudemon.hr

uudemon.day:	uudemon.day.sh
	cp uudemon.day.sh $(UUCP)/uudemon.day
	$(CH)chmod 755 $(UUCP)/uudemon.day
	$(CH)chgrp bin $(UUCP)/uudemon.day
	$(TOUCH) 0101000070 $(UUCP)/uudemon.day
	$(CH)chown uucp $(UUCP)/uudemon.day

uudemon.wk:	uudemon.wk.sh
	cp uudemon.wk.sh $(UUCP)/uudemon.wk
	$(CH)chmod 755 $(UUCP)/uudemon.wk
	$(CH)chgrp bin $(UUCP)/uudemon.wk
	$(TOUCH) 0101000070 $(UUCP)/uudemon.wk
	$(CH)chown uucp $(UUCP)/uudemon.wk

L-devices::
	cp L-devices $(UUCP)/L-devices
	$(CH)chmod 644 $(UUCP)/L-devices
	$(CH)chgrp bin $(UUCP)/L-devices
	$(TOUCH) 0101000070 $(UUCP)/L-devices
	$(CH)chown uucp $(UUCP)/L-devices

L-dialcodes::
	cp L-dialcodes $(UUCP)/L-dialcodes
	$(CH)chmod 644 $(UUCP)/L-dialcodes
	$(CH)chgrp bin $(UUCP)/L-dialcodes
	$(TOUCH) 0101000070 $(UUCP)/L-dialcodes
	$(CH)chown uucp $(UUCP)/L-dialcodes

L.cmds::
	cp L.cmds $(UUCP)/L.cmds
	$(CH)chmod 444 $(UUCP)/L.cmds
	$(CH)chgrp bin $(UUCP)/L.cmds
	$(TOUCH) 0101000070 $(UUCP)/L.cmds
	$(CH)chown uucp $(UUCP)/L.cmds

L.sys::
	cp L.sys $(UUCP)/L.sys
	$(CH)chmod 400 $(UUCP)/L.sys
	$(CH)chgrp bin $(UUCP)/L.sys
	$(TOUCH) 0101000070 $(UUCP)/L.sys
	$(CH)chown uucp $(UUCP)/L.sys

USERFILE::
	cp USERFILE $(UUCP)/USERFILE
	$(CH)chmod 400 $(UUCP)/USERFILE
	$(CH)chgrp bin $(UUCP)/USERFILE
	$(TOUCH) 0101000070 $(UUCP)/USERFILE
	$(CH)chown uucp $(UUCP)/USERFILE

null:
	> $(ROOT)/etc/log/filesave.log
	$(CH)chmod 664 $(ROOT)/etc/log/filesave.log
	$(CH)chgrp sys $(ROOT)/etc/log/filesave.log
	$(TOUCH) 0101000070 $(ROOT)/etc/log/filesave.log
	$(CH)chown root $(ROOT)/etc/log/filesave.log

	> $(ROOT)/etc/mnttab
	$(CH)chmod 644 $(ROOT)/etc/mnttab
	$(CH)chgrp sys $(ROOT)/etc/mnttab
	$(TOUCH) 0101000070 $(ROOT)/etc/mnttab
	$(CH)chown root $(ROOT)/etc/mnttab

	> $(ROOT)/etc/utmp
	$(CH)chmod 664 $(ROOT)/etc/utmp
	$(CH)chgrp bin $(ROOT)/etc/utmp
	$(TOUCH) 0101000070 $(ROOT)/etc/utmp
	$(CH)chown bin $(ROOT)/etc/utmp

	> $(ROOT)/etc/wtmp
	$(CH)chmod 664 $(ROOT)/etc/wtmp
	$(CH)chgrp adm $(ROOT)/etc/wtmp
	$(TOUCH) 0101000070 $(ROOT)/etc/wtmp
	$(CH)chown adm $(ROOT)/etc/wtmp

	> $(ROOT)/usr/adm/pacct
	$(CH)chmod 664 $(ROOT)/usr/adm/pacct
	$(CH)chgrp adm $(ROOT)/usr/adm/pacct
	$(TOUCH) 0101000070 $(ROOT)/usr/adm/pacct
	$(CH)chown adm $(ROOT)/usr/adm/pacct

	> $(ROOT)/usr/adm/sulog
	$(CH)chmod 600 $(ROOT)/usr/adm/sulog
	$(CH)chgrp root $(ROOT)/usr/adm/sulog
	$(TOUCH) 0101000070 $(ROOT)/usr/adm/sulog
	$(CH)chown root $(ROOT)/usr/adm/sulog

	> $(ROOT)/usr/games/advlog
	$(CH)chmod 666 $(ROOT)/usr/games/advlog
	$(CH)chgrp bin $(ROOT)/usr/games/advlog
	$(TOUCH) 0101000070 $(ROOT)/usr/games/advlog
	$(CH)chown bin $(ROOT)/usr/games/advlog

	> $(ROOT)/usr/lib/cron/log
	$(CH)chmod 664 $(ROOT)/usr/lib/cron/log
	$(CH)chgrp bin $(ROOT)/usr/lib/cron/log
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/cron/log
	$(CH)chown bin $(ROOT)/usr/lib/cron/log

	> $(ROOT)/usr/lib/spell/spellhist
	$(CH)chmod 666 $(ROOT)/usr/lib/spell/spellhist
	$(CH)chgrp bin $(ROOT)/usr/lib/spell/spellhist
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/spell/spellhist
	$(CH)chown bin $(ROOT)/usr/lib/spell/spellhist

	> $(ROOT)/usr/lib/uucp/L_stat
	$(CH)chmod 644 $(ROOT)/usr/lib/uucp/L_stat
	$(CH)chgrp bin $(ROOT)/usr/lib/uucp/L_stat
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/uucp/L_stat
	$(CH)chown uucp $(ROOT)/usr/lib/uucp/L_stat

	> $(ROOT)/usr/lib/uucp/L_sub
	$(CH)chmod 644 $(ROOT)/usr/lib/uucp/L_sub
	$(CH)chgrp bin $(ROOT)/usr/lib/uucp/L_sub
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/uucp/L_sub
	$(CH)chown uucp $(ROOT)/usr/lib/uucp/L_sub

	> $(ROOT)/usr/lib/uucp/R_stat
	$(CH)chmod 644 $(ROOT)/usr/lib/uucp/R_stat
	$(CH)chgrp bin $(ROOT)/usr/lib/uucp/R_stat
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/uucp/R_stat
	$(CH)chown uucp $(ROOT)/usr/lib/uucp/R_stat

	> $(ROOT)/usr/lib/uucp/R_sub
	$(CH)chmod 644 $(ROOT)/usr/lib/uucp/R_sub
	$(CH)chgrp bin $(ROOT)/usr/lib/uucp/R_sub
	$(TOUCH) 0101000070 $(ROOT)/usr/lib/uucp/R_sub
	$(CH)chown uucp $(ROOT)/usr/lib/uucp/R_sub

	> $(ROOT)/usr/spool/lp/outputq
	$(CH)chmod 644 $(ROOT)/usr/spool/lp/outputq
	$(CH)chgrp bin $(ROOT)/usr/spool/lp/outputq
	$(TOUCH) 0101000070 $(ROOT)/usr/spool/lp/outputq
	$(CH)chown lp $(ROOT)/usr/spool/lp/outputq

	> $(ROOT)/usr/spool/lp/pstatus
	$(CH)chown lp $(ROOT)/usr/spool/lp/pstatus
	$(CH)chgrp bin $(ROOT)/usr/spool/lp/pstatus
	$(TOUCH) 0101000070 $(ROOT)/usr/spool/lp/pstatus
	$(CH)chmod 644 $(ROOT)/usr/spool/lp/pstatus

	> $(ROOT)/usr/spool/lp/qstatus
	$(CH)chmod 644 $(ROOT)/usr/spool/lp/qstatus
	$(CH)chgrp bin $(ROOT)/usr/spool/lp/qstatus
	$(TOUCH) 0101000070 $(ROOT)/usr/spool/lp/qstatus
	$(CH)chown lp $(ROOT)/usr/spool/lp/qstatus


