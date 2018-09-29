#	@(#)adb.mk	1.2
TESTDIR = .
FRC =
INSDIR =
CFLAGS = -O
LDFLAGS=
IFLAG = -i

all:
	-if vax ;\
	then cd vax; make -f adb.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) ; \
	elif ns32000 ;\
	then cd ns32000; make -f adb.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) ; \
	else cd pdp11; make -f adb.mk FFLAG="$(FFLAG)" IFLAG="$(IFLAG)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) ; fi
install:
	-if vax ;\
	then cd vax; make -f adb.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) install; \
	elif ns32000 ;\
	then cd ns32000; make -f adb.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) install; \
	else cd pdp11; make -f adb.mk FFLAG="$(FFLAG)" IFLAG="$(IFLAG)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) install; fi
clean:
	-if vax ; then cd vax; make -f adb.mk clean TESTDIR=$(TESTDIR);\
	elif ns32000 ; then cd ns32000; make -f adb.mk clean TESTDIR=$(TESTDIR);\
	else cd pdp11; make -f adb.mk FFLAG="$(FFLAG)" IFLAG="$(IFLAG)" clean TESTDIR=$(TESTDIR); fi
clobber:
	-if vax ; then cd vax; make -f adb.mk clobber TESTDIR=$(TESTDIR); \
	elif ns32000 ; then cd ns32000; make -f adb.mk clobber TESTDIR=$(TESTDIR); \
	else cd pdp11; make -f adb.mk FFLAG="$(FFLAG)" IFLAG="$(IFLAG)" clobber TESTDIR=$(TESTDIR); fi
