# @(#)whatis.sh	1.1
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

case $1 in
	-*o*) opflg=1; shift;;
esac

if test $# -le 0
then
	ls  | pr -l`ls  | wc -l | af "x/5" | ceil` -5 -t -w72
	cat < whatis | $DEV
else
	for i in $*
	do if test -r $i
		then
			if test $opflg
				then
					echo "\n$i"
					cat < $i | sed -n "/^Option/,\$p" | $DEV
					echo 
				else cat < $i | $DEV
			fi
		else echo "\ndon't know what $i is"
		fi
	done
fi
