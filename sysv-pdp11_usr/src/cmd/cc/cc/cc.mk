#	@(#)cc.mk	1.4
PCC = pcc
CFLAGS = -O
LDFLAGS = -n -s
YACCRM=-rm
IFLAG = -i
V =
INS = :
INSDIR = /lib
CCDIR = /bin
FFLAG =

all:	$Vc0 $Vc1 $Vc2 $Vcc

compiler:	$Vc0 $Vc1 $Vc2

$Vc0::	c00.o c01.o c02.o c03.o c04.o c05.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $Vc0 c00.o c01.o c02.o c03.o c04.o c05.o $(LIBES)
$Vc0::
	$(INS) $Vc0 $(INSDIR)

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h

$Vc1::	c10.o c11.o c12.o c13.o table.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $Vc1 c10.o c11.o c12.o c13.o table.o $(LIBES)
$Vc1::
	$(INS) $Vc1 $(INSDIR)

c10.o c11.o c12.o c13.o: c1.h
table.o: optable cvopt
	./cvopt <optable >junk.c
	$(CC) -S junk.c
	sed 's/\.data/\.text/' <junk.s >junk.i
	as -o table.o junk.i
	rm junk.i junk.c junk.s


$Vc2::	c20.o c21.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(IFLAG) -o $Vc2 c20.o c21.o $(LIBES)
$Vc2::
	$(INS) $Vc2 $(INSDIR)

c20.o c21.o: c2.h
cvopt:	cvopt.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o cvopt cvopt.c $(LIBES)

$Vcc::	../cc.c
	$(CC) -DCSW=0 $(CFLAGS) $(LDFLAGS) -o $Vcc ../cc.c $(LIBES)
$Vcc::
	$(INS) $Vcc $(CCDIR)

$Vpcc::	../cc.c
	$(PCC) -DCSW=1 '-DCCNAME="pcc"' $(CFLAGS) $(LDFLAGS) -o $Vpcc ../cc.c $(LIBES)
$Vpcc::
	$(INS) $Vpcc $(CCDIR)

install:	all
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) all "INS=/etc/install" "V=$V"

backup:
	cp $(INSDIR)/cpp ocpp
	/etc/install ocpp $(INSDIR)
	cp $(INSDIR)/c0 oc0
	/etc/install oc0 $(INSDIR)
	cp $(INSDIR)/c1 oc1
	/etc/install oc1 $(INSDIR)
	cp $(INSDIR)/c2 oc2
	/etc/install oc2 $(INSDIR)
	cp $(CCDIR)/cc occ
	/etc/install occ $(CCDIR)

promote:
	cp $(INSDIR)/ncpp cpp
	/etc/install cpp $(INSDIR)
	cp $(INSDIR)/nc0 c0
	/etc/install c0 $(INSDIR)
	cp $(INSDIR)/nc1 c1
	/etc/install c1 $(INSDIR)
	cp $(INSDIR)/nc2 c2
	/etc/install c2 $(INSDIR)
	cp $(CCDIR)/ncc cc
	/etc/install cc $(CCDIR)

$Vncc::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=n" "INS=$(INS)" ncc
$Vnc0::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=n" "INS=$(INS)" nc0
$Vnc1::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=n" "INS=$(INS)" nc1
$Vnc2::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=n" "INS=$(INS)" nc2

$Vocc::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=o" "INS=$(INS)" occ
$Voc0::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=o" "INS=$(INS)" oc0
$Voc1::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=o" "INS=$(INS)" oc1
$Voc2::
	$(MAKE) -$(MAKEFLAGS) -f cc.mk IFLAG=$(IFLAG) YACCRM=$(YACCRM) "V=o" "INS=$(INS)" oc2

clean:
	-rm -f *.o cvopt

clobber:	clean
	-rm -f $Vc0 $Vc1 $Vc2 $Vcc $Vpcc
