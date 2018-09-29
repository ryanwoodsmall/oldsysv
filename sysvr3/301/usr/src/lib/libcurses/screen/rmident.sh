#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:screen/rmident.sh	1.3"
H='curses.ext maketerm.ed compiler.h curses.form curshdr.h ns_curses.h object.h otermcap.h print.h unctrl.h uparm.h'
for i in $H
do 
	echo de-identing $i
	mv $i zzy_$i
	sed '/ident/d' < zzy_$i > $i
	rm -f zzy_$i
done
