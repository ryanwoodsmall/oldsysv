#	@(#)mmtest.mk	1.1
INS = :

mmtest:	mmtest.o mmmch.o
	ld -o mmtest mmmch.o mmtest.o
	rm -f *.o
	$(INS) -f /stand mmtest

clobber:
	rm -f mmtest
