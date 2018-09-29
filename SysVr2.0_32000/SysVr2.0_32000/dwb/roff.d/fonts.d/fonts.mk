#	troff font tables make file
#
# DSL 2
#	SCCS:  @(#)fonts.mk	1.23

OL = $(ROOT)/
INS = :
INSDIR = ${OL}usr/lib/font
PFILES = makefonts.c
MKFILES = mkfont.c mkfont1.c
FILES =	ft[A-Z] ft[A-Z][A-Z]


makefonts compile all:
	cc -o makefonts makefonts.c
	./makefonts
	-mkdir $(INSDIR)
	$(INS) ${FILES} ${INSDIR}
	if [ "$(INS)" != ":" ]; \
	   then	cd ${INSDIR}; rm -f ftH ftHI ftHM ftFD; \
			ln ftG ftH; ln ftGI ftHI; ln ftGM ftHM; ln ftUD ftFD; \
	fi
	cd ${INSDIR}; chmod 644 $(FILES); $(CH) chgrp bin ${FILES}; chown bin ${FILES};

install:
	for i in $(FILES); \
	do if [ ! -r "$$i" ] || [ -s "`find $$i.c -newer $$i -print`" ]; \
		then $(MAKE) -f fonts.mk INS=cp ROOT=$(ROOT) CH=$(CH) all; \
	   fi; \
	done

mkfont:  mkfont.c mkfont1.c -o mkfont
	${CC} -n -s mkfont.c

clean:
	rm -f makefonts a.out
clobber:  clean
	rm -f ${FILES}
