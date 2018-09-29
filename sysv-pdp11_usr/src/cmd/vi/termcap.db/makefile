# @(#) makefile 1.6@(#)
# reorder gives an editor command for most common terminals
# (in reverse order from n'th to 1'st most commonly used)
# to move them to the front of termcap
#
DESTDIR=
termcap.obj: termcap.src reorder
	ed - termcap.src < reorder
install: termcap.obj
	cp termcap.obj ${DESTDIR}/etc/termcap
	chown bin ${DESTDIR}/etc/termcap
	chgrp bin ${DESTDIR}/etc/termcap
clean:
	rm -f termcap.obj
tabset:	/tmp
	-mkdir ${DESTDIR}/usr/lib/tabset
	cp tabset/* ${DESTDIR}/usr/lib/tabset
	chown bin ${DESTDIR}/usr/lib/tabset ${DESTDIR}/usr/lib/tabset/*
	chgrp bin ${DESTDIR}/usr/lib/tabset ${DESTDIR}/usr/lib/tabset/*
VGRIND=	pr
vgrind:
	${VGRIND} -h "Terminal Capibility Data Base" \
	termcap.src reorder makefile | opr -fx2on1
# VGRIND=	csh /usr/ucb/vgrind
# vgrind:
# 	${VGRIND} -n -h "Terminal Capibility Data Base" \
# 	termcap.src reorder makefile
