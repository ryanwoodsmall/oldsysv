#	@(#)vcf.mk	1.2
ASM = scc
INS = /etc/install
.c.o:
	$(ASM) -c $*.c

MFILES = mtpr.m psl.m trap.m uba.m
SFILES = misc.s random.s scb.s trap.s
OFILES = macheck.o trap.o intr.o ckipl.o
HTESTS = ckdz.o ckkmc.o ckdh.o ckdn.o

all:	vers.h locore.o $(OFILES) $(HTESTS)
	cat vers.h cf.c >sysconf.c
	$(ASM) -o vcf locore.o sysconf.c $(OFILES) $(HTESTS)
	$(INS) -f /stand vcf
	rm sysconf.c vers.h vcf

locore.o:
	cat $(MFILES) $(SFILES) >locore.s
	as -o a.out locore.s
	-ld -r -x -o locore.o a.out
	-rm -f a.out locore.s

vers.h:
	echo "#define VERS \"`date +%m/%d/%y\ \ %H:%M`\"" >vers.h
	rm -f sysconf.o

clean:
	-rm -f *.o vers.h

clobber: clean
	-rm -f vcf
