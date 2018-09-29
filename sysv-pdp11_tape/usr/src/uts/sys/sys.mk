SRC=.
INC=/usr/include/sys
CPLN=cp
all: $(INC)/acct.h \
$(INC)/crtctl.h \
$(INC)/csi.h \
$(INC)/csierrs.h \
$(INC)/csikmc.h \
$(INC)/dir.h \
$(INC)/dmk.h \
$(INC)/elog.h \
$(INC)/erec.h \
$(INC)/err.h \
$(INC)/errno.h \
$(INC)/fblk.h \
$(INC)/file.h \
$(INC)/ino.h \
$(INC)/ioctl.h \
$(INC)/ipc.h \
$(INC)/kmc.h \
$(INC)/lapbtr.h \
$(INC)/lock.h \
$(INC)/lprio.h \
$(INC)/mount.h \
$(INC)/msg.h \
$(INC)/nc.h \
$(INC)/pcl.h \
$(INC)/sem.h \
$(INC)/signal.h \
$(INC)/st.h \
$(INC)/stat.h \
$(INC)/stermio.h \
$(INC)/sysmacros.h \
$(INC)/termio.h \
$(INC)/times.h \
$(INC)/trace.h \
$(INC)/ttold.h \
$(INC)/utsname.h \
$(INC)/vp.h \
$(INC)/vpmt.h \
$(INC)/x25.h \
$(INC)/x25macro.h \
$(INC)/x25packet.h \
$(INC)/x25rpt.h \
$(INC)/x25u.h \
$(INC)/x25user.h
$(INC)/acct.h: $(SRC)/acct.h
	$(CPLN) $(SRC)/acct.h $(INC)
$(INC)/crtctl.h: $(SRC)/crtctl.h
	$(CPLN) $(SRC)/crtctl.h $(INC)
$(INC)/csi.h: $(SRC)/csi.h
	$(CPLN) $(SRC)/csi.h $(INC)
$(INC)/csierrs.h: $(SRC)/csierrs.h
	$(CPLN) $(SRC)/csierrs.h $(INC)
$(INC)/csikmc.h: $(SRC)/csikmc.h
	$(CPLN) $(SRC)/csikmc.h $(INC)
$(INC)/dir.h: $(SRC)/dir.h
	$(CPLN) $(SRC)/dir.h $(INC)
$(INC)/dmk.h: $(SRC)/dmk.h
	$(CPLN) $(SRC)/dmk.h $(INC)
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
$(INC)/ioctl.h: $(SRC)/ioctl.h
	$(CPLN) $(SRC)/ioctl.h $(INC)
$(INC)/ipc.h: $(SRC)/ipc.h
	$(CPLN) $(SRC)/ipc.h $(INC)
$(INC)/kmc.h: $(SRC)/kmc.h
	$(CPLN) $(SRC)/kmc.h $(INC)
$(INC)/lapbtr.h: $(SRC)/lapbtr.h
	$(CPLN) $(SRC)/lapbtr.h $(INC)
$(INC)/lock.h: $(SRC)/lock.h
	$(CPLN) $(SRC)/lock.h $(INC)
$(INC)/lprio.h: $(SRC)/lprio.h
	$(CPLN) $(SRC)/lprio.h $(INC)
$(INC)/mount.h: $(SRC)/mount.h
	$(CPLN) $(SRC)/mount.h $(INC)
$(INC)/msg.h: $(SRC)/msg.h
	$(CPLN) $(SRC)/msg.h $(INC)
$(INC)/nc.h: $(SRC)/nc.h
	$(CPLN) $(SRC)/nc.h $(INC)
$(INC)/pcl.h: $(SRC)/pcl.h
	$(CPLN) $(SRC)/pcl.h $(INC)
$(INC)/sem.h: $(SRC)/sem.h
	$(CPLN) $(SRC)/sem.h $(INC)
$(INC)/signal.h: $(SRC)/signal.h
	$(CPLN) $(SRC)/signal.h $(INC)
$(INC)/st.h: $(SRC)/st.h
	$(CPLN) $(SRC)/st.h $(INC)
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
$(INC)/vp.h: $(SRC)/vp.h
	$(CPLN) $(SRC)/vp.h $(INC)
$(INC)/vpmt.h: $(SRC)/vpmt.h
	$(CPLN) $(SRC)/vpmt.h $(INC)
$(INC)/x25.h: $(SRC)/x25.h
	$(CPLN) $(SRC)/x25.h $(INC)
$(INC)/x25macro.h: $(SRC)/x25macro.h
	$(CPLN) $(SRC)/x25macro.h $(INC)
$(INC)/x25packet.h: $(SRC)/x25packet.h
	$(CPLN) $(SRC)/x25packet.h $(INC)
$(INC)/x25rpt.h: $(SRC)/x25rpt.h
	$(CPLN) $(SRC)/x25rpt.h $(INC)
$(INC)/x25u.h: $(SRC)/x25u.h
	$(CPLN) $(SRC)/x25u.h $(INC)
$(INC)/x25user.h: $(SRC)/x25user.h
	$(CPLN) $(SRC)/x25user.h $(INC)
