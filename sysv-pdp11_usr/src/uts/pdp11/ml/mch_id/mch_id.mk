# @(#)mch_id.mk	1.1
MCHNAME = ../../mch_id.o
FRC =

FILES = mch.h start.s trap.s fpp.s clist.s\
	copy.s xcopy.s bufio.s misc.s math.s cswitch.s\
	csubr.s backup.s userio.s power.s end.s

all:	$(MCHNAME)

$(MCHNAME):	$(FILES) $(FRC)
	as $(FILES)
	-ld -r -x -o $(MCHNAME) a.out
	-chown bin $(MCHNAME)
	-chgrp bin $(MCHNAME)
	-chmod 664 $(MCHNAME)
	-rm -f a.out

clean:
	-rm -f *.o a.out

clobber:	clean
	-rm -f $(MCHNAME)

FRC:
