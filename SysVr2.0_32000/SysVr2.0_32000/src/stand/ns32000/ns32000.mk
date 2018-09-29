#   ns32000.mk
INS = :
CFLAGS = -O
EXECS = ls du cat dd od
NSCPGMS = restor tsc

all:	$(EXECS) 5toG iltd sash fsck fsdb mkfs Bto5 5toB mkpt dcutest $(NSCPGMS)

5toG::	5toG.c
	cc -s -o 5toG 5toG.c

iltd::	iltd.c 5toG
	scc -s -o iltd5 bootld.file iltd.c
	./5toG < iltd5 > iltd
iltd::
	$(INS) -f /stand iltd

sash::	sash0.o sash1.o 5toG
	ld -e _start -x -o sash5 ld.file sash0.o sash1.o -l2A -l2
	./5toG < sash5 > sash
sash::
	$(INS) -f /stand sash

sash1.o:
	cc -O -c sash1.c

sash0.o:
	as -o sash0.o sash0.s


$(EXECS)::
	scc -s -O -o $@ execld.file /usr/src/cmd/$@.c
$(EXECS)::
	$(INS) -f /stand $@

$(NSCPGMS)::
	cp $@.ns $@
$(NSCPGMS)::
	$(INS) -f /stand $@

fsdb::
	scc -s -o fsdb execld.file /usr/src/cmd/fsdb/fsdb.c
fsdb::
	$(INS) -f /stand fsdb

fsck::
	scc -s -o fsck execld.file /usr/src/cmd/fsck/fsck.c
fsck::
	$(INS) -f /stand fsck

mkfs::
	scc -s -o mkfs execld.file /usr/src/cmd/mkfs.c
mkfs::
	$(INS) -f /stand mkfs

Bto5::  Bto5.c 5toG
	scc -o Bto5x bootld.file Bto5.c
	./5toG <Bto5x >Bto5
	chmod 775 Bto5
Bto5::
	$(INS) -f /stand Bto5

5toB::  5toB.c 5toG
	scc -o 5toBx bootld.file 5toB.c
	./5toG <5toBx >5toB
	chmod 775 5toB
5toB::
	$(INS) -f /stand 5toB

mkpt:: mkpt.o 5toG
	scc -o mkptx bootld.file mkpt.o
	./5toG <mkptx >mkpt
mkpt::
	$(INS) -f /stand mkpt

dcutest:: dcusaio.o dcutest.o 5toG
	scc -o dcutestx bootld.file dcusaio.o dcutest.o
	./5toG <dcutestx >dcutest
dcutest::
	$(INS) -f /stand dcutest

dcusaio.o: dcusaio.h dcudisk.h
	cc -O -c dcusaio.c

dcutest.o: dcusaio.h
	cc -O -c dcutest.c

install:
	make -f ns32000.mk all INS=/etc/install

clean:
	rm -f iltd.o
	rm -f sash?.o
	rm -f Bto5x 5toBx
	rm -f mkptx mkpt.o
	rm -f dcutestx dcusaio.o dcutest.o

clobber: clean
	rm -f sash sash5
	rm -f $(EXECS) fsck fsdb mkfs
	rm -f $(NSCPGMS) mkpt dcutest
	rm -f iltd iltd5
	rm -f 5toG Bto5 5toB
