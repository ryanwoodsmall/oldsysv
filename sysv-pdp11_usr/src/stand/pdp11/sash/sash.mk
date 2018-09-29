#	@(#)sash.mk	1.3
INS = :

sash:	sash0.o sash1.o
	ld -o sash sash0.o sash1.o /usr/lib/lib2.a /usr/lib/lib2A.a
	rm -r sash0.o sash1.o
	$(INS) -f /stand sash

sash1.o:	sash1.c
	scc -c sash1.c

clobber:
	rm -f sash
