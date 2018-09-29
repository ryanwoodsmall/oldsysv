#	nroff terminal driving tables make file
#
# DSL 2.
#	SCCS: @(#)terms.mk	1.26

OL = $(ROOT)/
INSDIR = ${OL}usr/lib/term
TFILES = code.300 code.lp tab2631.c tab2631-c.c tab2631-e.c tab300.c \
	 tab300-12.c tab300s.c tab300s-12.c tab37.c tab382.c \
	 tab4000a.c tab450.c tab450-12.c tab832.c taba1.c tablp.c tabtn300.c \
	 tabX.c tabX97.c
PFILES = maketerms.c terms.hd
FILES = tab2631 tab2631-c tab2631-e tab300 tab300-12 tab300s \
	tab300s-12 tab37 tab382 tab4000a tab450 \
	tab450-12 tab832 taba1 tablp tabtn300 tabX tabX97
IFILES = $(FILES) tab300S tab300S-12 tab4000A

B03 =

maketerms compile all:  terms.hd
	cc -DPART1 ${B03} -o maketerms maketerms.c
	./maketerms
	cc -DPART2 ${B03} -o maketerms maketerms.c
	./maketerms
	cc -DPART3 ${B03} -o maketerms maketerms.c
	./maketerms
	-mkdir $(INSDIR)
	$(INS) ${FILES} ${INSDIR}
	if [ "$(INS)" != ":" ]; \
	   then	cd ${INSDIR}; rm -f tab300S tab300S-12 tab4000A; \
			ln tab300s tab300S; ln tab300s-12 tab300S-12; ln tab4000a tab4000A; \
	fi
	cd ${INSDIR}; chmod 644 $(IFILES); $(CH) chgrp bin $(IFILES); chown bin $(IFILES)

install:
	for i in $(FILES); \
	do if [ ! -r "$$i" ] || [ -s "`find $$i.c -newer $$i -print`" ]; \
		then $(MAKE) -f terms.mk INS=cp ROOT=$(ROOT) CH=$(CH) all; \
	   fi; \
	done


clean:
	rm -f maketerms
clobber:  clean
	rm -f ${FILES}
