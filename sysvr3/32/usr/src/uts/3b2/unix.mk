#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:unix.mk	10.11.1.5"

ROOT = 
DIS = dis
NM = nm
SIZE = size
STRIP = strip
INC=$(ROOT)/usr/include

LIBS= lib.os lib.io
NODE = kernel
REL = 
VER = 
NAME = $(NODE)$(VER)

INS=install
INSDIR=.
MKBOOT = mkboot
MASTERD=./master.d
DASHO = -O
CFLAGS= $(DASHO) -I$(INC) -DINKERNEL -DREM $(MORECPP)
FRC =

all:FRC
	cd ml; $(MAKE) -f ml.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../os; $(MAKE) -f os.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../fs; $(MAKE) -f fs.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../nudnix; $(MAKE) -f nudnix.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../io; $(MAKE) -f io.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../debug; $(MAKE) -f debug.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	cd ../boot; $(MAKE) -f boot.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	wait
	$(MAKE) -f unix.mk NAME=$(NAME) unix

machine:FRC
	cd ml; $(MAKE) -f ml.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

system:FRC
	cd os; $(MAKE) -f os.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

drivers:FRC
	cd io; $(MAKE) -f io.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

debug:FRC
	cd debug; $(MAKE) -f debug.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

boot:FRC
	cd boot; $(MAKE) -f boot.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

nudnix:FRC
	cd nudnix; $(MAKE) -f nudnix.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" 

fs:FRC
	cd fs; $(MAKE) -f fs.mk "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)"

unix:	$(LIBS) vuifile $(MASTERD)/kernel
	-rm -f $(NAME)
	$(LD) -r -o $(NAME) -u ttopen -u reclock vuifile start.o gate.o \
		locore.o name.o getsizes.o $(LIBS)
	$(MKBOOT) -m $(MASTERD) -d . -k $(NAME);
	@echo $(NAME) made.

clean:
	cd ml; $(MAKE) -f ml.mk clean
	cd os; $(MAKE) -f os.mk clean
	cd nudnix; $(MAKE) -f nudnix.mk clean
	cd io; $(MAKE) -f io.mk clean
	cd fs; $(MAKE) -f fs.mk clean
	cd debug; $(MAKE) -f debug.mk clean
	cd boot; $(MAKE) -f boot.mk clean
	-rm -f *.o

clobber: clean
	cd ml; $(MAKE) -f ml.mk clobber
	cd os; $(MAKE) -f os.mk clobber
	cd io; $(MAKE) -f io.mk clobber
	cd nudnix; $(MAKE) -f nudnix.mk clobber
	cd fs; $(MAKE) -f fs.mk clobber
	cd debug; $(MAKE) -f debug.mk clobber
	cd boot; $(MAKE) -f boot.mk clobber
	-rm -f $(NAME) $(NAME).* ld.out load.*


install:all
#	$(INS) -f $(INSDIR) "$(NAME)"
	cd boot; $(MAKE) -f boot.mk install "FRC=$(FRC)" "DASHO=$(DASHO)" "MORECPP=$(MORECPP)" "INC=$(INC)" ; \
	wait


FRC:
