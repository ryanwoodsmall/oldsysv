# @(#)sys.mk	6.1
SRC=.
INC=/usr/include/sys
CPLN=cp
all: $(INC)/acct.h \
$(INC)/crtctl.h \
$(INC)/dir.h \
$(INC)/elog.h \
$(INC)/erec.h \
$(INC)/err.h \
$(INC)/errno.h \
$(INC)/fblk.h \
$(INC)/file.h \
$(INC)/ino.h \
$(INC)/ipc.h \
$(INC)/lock.h \
$(INC)/lprio.h \
$(INC)/mount.h \
$(INC)/msg.h \
$(INC)/sem.h \
$(INC)/signal.h \
$(INC)/stat.h \
$(INC)/stermio.h \
$(INC)/sysmacros.h \
$(INC)/termio.h \
$(INC)/times.h \
$(INC)/trace.h \
$(INC)/ttold.h \
$(INC)/utsname.h
$(INC)/acct.h: $(SRC)/acct.h
	$(CPLN) $(SRC)/acct.h $(INC)
$(INC)/crtctl.h: $(SRC)/crtctl.h
	$(CPLN) $(SRC)/crtctl.h $(INC)
$(INC)/dir.h: $(SRC)/dir.h
	$(CPLN) $(SRC)/dir.h $(INC)
$(INC)/elog.h: $(SRC)/elog.h
	$(CPLN) $(SRC)/elog.h $(INC)
$(INC)/erec.h: $(SRC)/erec.h
	$(CPLN) $(SRC)/erec.h $(INC)
$(INC)/err.h: $(SRC)/err.h
	$(CPLN) $(SRC)/err.h $(INC)
$(INC)/errno.h: $(SRC)/errno.h
	$(CPLN) $(SRC)/errno.h $(INC)
$(INC)/fblk.h: $(SRC)/fblk.h
	$(CPLN) $(SRC)/fblk.h $(INC)
$(INC)/file.h: $(SRC)/file.h
	$(CPLN) $(SRC)/file.h $(INC)
$(INC)/ino.h: $(SRC)/ino.h
	$(CPLN) $(SRC)/ino.h $(INC)
$(INC)/ipc.h: $(SRC)/ipc.h
	$(CPLN) $(SRC)/ipc.h $(INC)
$(INC)/lock.h: $(SRC)/lock.h
	$(CPLN) $(SRC)/lock.h $(INC)
$(INC)/lprio.h: $(SRC)/lprio.h
	$(CPLN) $(SRC)/lprio.h $(INC)
$(INC)/msg.h: $(SRC)/msg.h
	$(CPLN) $(SRC)/msg.h $(INC)
$(INC)/mount.h: $(SRC)/mount.h
	$(CPLN) $(SRC)/mount.h $(INC)
$(INC)/sem.h: $(SRC)/sem.h
	$(CPLN) $(SRC)/sem.h $(INC)
$(INC)/signal.h: $(SRC)/signal.h
	$(CPLN) $(SRC)/signal.h $(INC)
$(INC)/stat.h: $(SRC)/stat.h
	$(CPLN) $(SRC)/stat.h $(INC)
$(INC)/stermio.h: $(SRC)/stermio.h
	$(CPLN) $(SRC)/stermio.h $(INC)
$(INC)/sysmacros.h: $(SRC)/sysmacros.h
	$(CPLN) $(SRC)/sysmacros.h $(INC)
$(INC)/termio.h: $(SRC)/termio.h
	$(CPLN) $(SRC)/termio.h $(INC)
$(INC)/times.h: $(SRC)/times.h
	$(CPLN) $(SRC)/times.h $(INC)
$(INC)/trace.h: $(SRC)/trace.h
	$(CPLN) $(SRC)/trace.h $(INC)
$(INC)/ttold.h: $(SRC)/ttold.h
	$(CPLN) $(SRC)/ttold.h $(INC)
$(INC)/utsname.h: $(SRC)/utsname.h
	$(CPLN) $(SRC)/utsname.h $(INC)
