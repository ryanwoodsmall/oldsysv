#	sed make file
#	SCCS:	@(#)sed.mk	1.7

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/sed
INS = :
REL = current
CSID = -r`gsid sed $(REL)`
MKSID = -r`gsid sed.mk $(REL)`
LIST = lp
INSDIR = $(OL)bin
IFLAG = -n
B10 =
CFLAGS = -O $(B10)
LDFLAGS = -s $(IFLAG)
SOURCE = sed.h sed0.c sed1.c
FILES = sed0.o sed1.o
MAKE = make

compile all: sed
	:

sed:	$(FILES)
	$(CC) $(LDFLAGS) -o sed $(FILES)
	$(INS) $(INSDIR) sed

$(FILES)::  sed.h
	:

install:
	$(MAKE) -f sed.mk INS="install -f" OL=$(OL)

build:	bldmk
	get -p $(CSID) s.sed.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p $(MKSID) s.sed.mk > $(RDIR)/sed.mk

listing:
	pr sed.mk $(SOURCE) | $(LIST)
listmk: ;  pr sed.mk | $(LIST)

edit:
	get -e -p s.sed.src | ntar -g

delta:
	ntar -p $(SOURCE) > sed.src
	delta s.sed.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.sed.mk
mkdelta: ;  delta s.sed.mk

clean:
	  rm -f $(FILES)

clobber:  clean
	  rm -f sed

delete:	clobber
	rm -f $(SOURCE)
