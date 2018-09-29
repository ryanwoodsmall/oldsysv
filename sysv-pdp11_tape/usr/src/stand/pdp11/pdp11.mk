#	@(#)pdp11.mk	1.2
INS = :
CFLAGS = -O
EXECS = ls du cat dd od 

all:	$(EXECS) boot sashp iltd fsdb mmtst fsck

sashp:
	cd sash; make -f sash.mk INS=$(INS)

boot:
	cd boots; make -f boots.mk all INS=$(INS)

$(EXECS)::
	scc -s -O -o $@ /usr/src/cmd/$@.c
	$(INS) -f /stand $@

iltd::
	cd iload; make -f iltd.mk install INS=$(INS)

fsdb::
	scc -s -o fsdb /usr/src/cmd/fsdb/fsdb.c
fsdb::
	$(INS) -f /stand fsdb

fsck::
	scc -s -o fsck /usr/src/cmd/fsck/fsck.c
fsck::
	$(INS) -f /stand fsck

mmtst:
	cd mmtest; make -f mmtest.mk INS=$(INS)
install:
	make -f pdp11.mk all INS=/etc/install

clean:
	rm -f *.o

clobber: clean
	rm -f $(EXECS) iltd fsdb
	cd iload; make -f iltd.mk clobber
	cd boots; make -f boots.mk clobber
	cd sash; make -f sash.mk clobber
	cd mmtest; make -f mmtest.mk clobber
