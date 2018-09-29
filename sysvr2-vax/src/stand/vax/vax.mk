# @(#)vax.mk	1.7
INS = :
CFLAGS = -O
EXECS = ls du cat dd od 

all:	$(EXECS) iltd sash vrmhead boots conflp \
	fsck format fsdb vcf
iltd::	iltd.o	gdboot2b.o
	scc -s -o iltd execld.file iltd.o gdboot2b.o
iltd::
	$(INS) -f /stand iltd

sash::	vrmhead

sash::	sash0.o sash1.o
	ld -e start -x ld.file sash0.o sash1.o \
		/usr/lib/lib2A.a /usr/lib/lib2.a -o sash
	./vrmhead sash sash.strip
	mv sash.strip sash
sash::
	$(INS) -f /stand sash

sash1.o:
	cc -O -c sash1.c

sash0.o:
	as -o sash0.o sash0.s

vrmhead::	vrmhead.c
	cc -s $(CFLAGS) -o vrmhead vrmhead.c
vrmhead::
	$(INS) -f /stand vrmhead

boots::
	cd boots; make -f boots.mk all INS=$(INS)

conflp::
	cd conflp; make -f conflp.mk all INS=$(INS)

$(EXECS)::
	scc -s -O -o $@ execld.file /usr/src/cmd/$@.c
$(EXECS)::
	$(INS) -f /stand $@

format::
	scc -s -o format execld.file cmd/format.c
format::
	$(INS) -f /stand format


fsdb::
	scc -s -o fsdb execld.file /usr/src/cmd/fsdb/fsdb.c
fsdb::
	$(INS) -f /stand fsdb

fsck::
	scc -s -o fsck execld.file /usr/src/cmd/fsck/fsck.c
fsck::
	$(INS) -f /stand fsck
vcf::
	cd cmd/vcf; make -f vcf.mk all clean

gdboot2b.o:
	cd boots; make -f boots.mk gdboot2b.o
	cp boots/gdboot2b.o .

install:
	make -f vax.mk all INS=/etc/install

clean:
	rm -f iltd.o gdboot2b.o
	rm -f sash?.o

clobber: clean
	rm -f sash
	rm -f vrmhead
	rm -f $(EXECS) format fsck fsdb
	cd boots; make -f boots.mk clobber
	rm -f iltd
