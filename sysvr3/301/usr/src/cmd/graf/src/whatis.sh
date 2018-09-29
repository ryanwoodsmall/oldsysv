#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/whatis.sh	1.5"
: "<: t-4 d :>"
: whatis [-o] [name ...]
:

BIN=/usr/bin/graf
cd /usr/lib/graf/whatis

set - `$BIN/cvrtopt $@`

if test -x /usr/bin/${TERM=cat}
then DEV=$TERM
else DEV=cat
fi
if test $DEV = "4014"
	then
		DEV="4014 -n"
	fi

case $1 in
	-*o*) opflg=1; shift;;
esac

if test $# -le 0
then
	ls  | pr -l`ls  | wc -l | af "x/5" 2>/dev/null| ceil` -5 -t -w72
	sed -n -e "2,\$p" whatis | $DEV
else
	for i in $*
	do if test -r $i
		then
			if test $opflg
				then
					echo "\n$i"
					cat < $i | sed -n "/^Option/,\$p" | $DEV
					echo 
				else sed -n -e "2,\$p" $i | $DEV
			fi
		else echo "\ndon't know what $i is"
		fi
	done
fi
