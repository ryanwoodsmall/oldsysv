#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm:xx/chexec.sh	1.2"

if [ $# = 0 ]
then
	echo Usage: whereis [ls options] [commandname] ...
	exit 2
fi

cmd=${1}
if [ -f  "${cmd}" ]
then
	exit 0
fi

dirs=". /bin /usr/bin /usr/lbin /etc"
case $PATH in
	:*) dirs1=. ;;
esac
IFS="$IFS:"
for i in $PATH $CDPATH
do
	dirs1="$dirs1 $i"
done
IFS=" 	
"
dirs="$dirs1 $dirs"
list=
#
#	Take out duplicates
#
for i in $dirs
do
	a=no
	for j in $list
	do
		case $i in
		$j)
			a=yes
			;;
		esac
	done
	case $a in
	no)
		list="$list $i"
		;;
	esac
done
for j in "$@"
do
	a=no
	for i in $list
	do
		filelist="$filelist $i/$j"
	done
	for i in $filelist
	do
		if [ -f $i ]
		then
			a=flag
		fi
	done
	if [ "$a" != flag ]
	then
		echo "Warning cannot find $j"
		exit 1
	fi
	filelist=""
done
