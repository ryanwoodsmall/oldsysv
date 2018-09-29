# @(#)ttoc.sh	1.1
: ttoc mm-file
:
LIB=/usr/lib/graf
BIN=/usr/bin/graf

trap "rm -f /tmp/ttoc*.$$" 1 2 3
set - `cvrtopt $@`

while test -n "$1"
do
	case $1 in
		-)
			break
			;;
		-*)
			echo "!! ttoc:  `expr $1 : '-\(.\)'`?" 1>&2
			;;
		*)
			break
			;;
	esac
	shift
done

if	test -z "$1" -o "$1" = "-"
then
	cat - > /tmp/ttoc0.$$
	file=/tmp/ttoc0.$$
else
	file=$1
fi

if test -f $file 
then
	count=`grep -c "^.TC" $file`
	if test $count -eq 0
	then
		ed - $file < $LIB/ttoc.d/ed.notoc > /tmp/ttoc.$$
	else
		ed - $file < $LIB/ttoc.d/ed.toc > /tmp/ttoc.$$
	fi
	nroff -mm -n -rB1 -rC3 /tmp/ttoc.$$ | yoo /tmp/ttoc.$$
	ed - /tmp/ttoc.$$ < $LIB/ttoc.d/ed.ttoc.t
else
echo "!! ttoc:  cannot open $file" 1>&2
fi
rm -f /tmp/ttoc*.$$
