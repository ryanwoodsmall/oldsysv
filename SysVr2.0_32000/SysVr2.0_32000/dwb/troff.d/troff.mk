#	makefile for (di) troff.  Also builds subproducts - typesetter
#		drivers, fonts, rasters, etc.
#
# DSL 2.
#	SCCS @(#)troff.mk	1.11

OL = $(ROOT)/
CFLAGS= -O
LDFLAGS = -s
IFLAG = -i
CFILES=n1.c n2.c n3.c n4.c n5.c t6.c n7.c n8.c n9.c t10.c ni.c nii.c hytab.c suftab.c
HFILES=tdef.h v.h d.h s.h dev.h
TFILES=n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o ni.o nii.o hytab.o suftab.o
INS = :
INSDIR = $(OL)usr/bin

all:	troff tc

troff:	$(TFILES)
	$(CC) $(LDFLAGS) -o troff $(IFLAG) $(TFILES) 
	$(INS) troff $(INSDIR)
	cd $(INSDIR); chmod 755 troff; $(CH) chgrp bin troff; chown bin troff

n1.o:	tdef.h d.h v.h s.h ext.h
n2.o:	tdef.h d.h v.h s.h ext.h
n3.o:	tdef.h d.h v.h s.h ext.h
n4.o:	tdef.h d.h v.h s.h ext.h
n5.o:	tdef.h d.h v.h s.h ext.h
t6.o:	tdef.h d.h v.h s.h dev.h ext.h
n7.o:	tdef.h d.h v.h s.h ext.h
n8.o:	tdef.h ext.h
n9.o:	tdef.h d.h v.h ext.h
t10.o:	tdef.h d.h v.h dev.h ext.h
ni.o:	tdef.h
nii.o:	tdef.h d.h v.h s.h
hytab.o:	hytab.c
suftab.o:	suftab.c

tc:	tc.o draw.o
	$(CC) $(LDFLAGS) -o tc tc.o draw.o -lm
	$(INS) tc $(INSDIR)
	cd $(INSDIR); chmod 755 tc; $(CH) chgrp bin tc; chown bin tc

hc:	hc.o draw.o
	$(CC) $(LDFLAGS) -o hc hc.o draw.o -lm
	$(INS) hc $(INSDIR)
	cd $(INSDIR); chmod 755 hc; $(CH) chgrp bin hc; chown bin hc

ta:	ta.o draw.o
	$(CC) $(LDFLAGS) -o ta ta.o draw.o -lm
	$(INS) ta $(INSDIR)
	cd $(INSDIR); chmod 755 ta; $(CH) chgrp bin ta; chown bin ta

tc.o:	dev.h
hc.o:	dev.h
ta.o:	dev.h

makedev:	makedev.c dev.h
	cc $(LDFLAGS) -o makedev makedev.c

install:
	$(MAKE) -f troff.mk INS=cp all ROOT=$(ROOT) CH=$(CH)
	cd devX97;  $(MAKE) -f devX97.mk install ROOT=$(ROOT) CH=$(CH)
	cd devaps;  $(MAKE) -f devaps.mk install ROOT=$(ROOT) CH=$(CH)
	cd devi10;  $(MAKE) -f devi10.mk install ROOT=$(ROOT) CH=$(CH)
tcinstall:  ;  $(MAKE) -f troff.mk INS=cp tc ROOT=$(ROOT) CH=$(CH)
hcinstall:  ;  $(MAKE) -f troff.mk INS=cp hc ROOT=$(ROOT) CH=$(CH)
tainstall:  ;  $(MAKE) -f troff.mk INS=cp ta ROOT=$(ROOT) CH=$(CH)

clean:	hcclean taclean tcclean
	rm -f $(TFILES) draw.o
	cd devaps;  $(MAKE) -f devaps.mk clean
	cd devi10;  $(MAKE) -f devi10.mk clean
	cd devX97;  $(MAKE) -f devX97.mk clean
hcclean:  ;  rm -f hc.o
taclean:  ;  rm -f ta.o
tcclean:  ;  rm -f tc.o

clobber:	hcclobber taclobber tcclobber
	rm -f $(TFILES) draw.o
	rm -f troff tc makedev
	cd devaps;  $(MAKE) -f devaps.mk clobber
	cd devi10;  $(MAKE) -f devi10.mk clobber
	cd devX97;  $(MAKE) -f devX97.mk clobber
hcclobber:	hcclean
	rm -f hc
taclobber:	taclean
	rm -f ta
tcclobber:	tcclean
	rm -f tc
