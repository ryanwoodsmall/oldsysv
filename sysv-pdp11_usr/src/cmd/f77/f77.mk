#	@(#)f77.mk	1.6
TESTDIR = .
FRC =
INSDIR =
LDFLAGS = -s
FFLAG =
IFLAG = -i

all:
	-if u3b ; then cd u3b ; make 3b ; exit ; \
	elif vax ; then cd vax ; else cd pdp11 ; fi ; \
	make -f f77.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR)
	-if vax ; then cd vax ; make -f pcc.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) fort ; \
	elif u3b ; then cd u3b ; make -f pcc.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) fort ; fi
install:
	-if u3b ; then cd u3b ; make 3b ; make install ; exit ; \
	elif vax ; then cd vax ; else cd pdp11 ; fi ; \
	make -f f77.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) install
	-if vax ; then cd vax ; make -f pcc.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) finstall ; \
	elif u3b ; then cd u3b ; make -f pcc.mk finstall ; fi
clean:
	-if u3b ; then cd u3b ; make clean ; exit ; \
	elif vax ; then cd vax ; else cd pdp11 ; fi ; \
	make -f f77.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) clean
	-if vax ; then cd vax ; make -f pcc.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) clean ; \
	elif u3b ; then cd u3b ; make -f pcc.mk shrink ; fi
clobber:
	-if u3b ; then cd u3b ; make clobber ; make -f pcc.mk clobber ; exit ; \
	elif vax ; then cd vax ; else cd pdp11 ; fi ; \
	make -f f77.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) clobber
	-if vax ; then cd vax ; make -f pcc.mk LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) TESTDIR=$(TESTDIR) clobber ; fi
