################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	@(#)uuto.sh	1.3
mysys=`uname -n`
mesg="Usage: $0 [-m -p] files remote!user"
if test  $# -lt 1
	then
		echo $mesg
		exit
fi
trap "trap '' 0; exit" 0 1 2 13 15
copy=0
a=""
#	get options
while true
do
	case $1 in
	 -m | -r* | -x*)	a="$a $1" ;;
	 -p)	copy=1 ;;
	 -d | -n*)	;;
	 -*)	echo $mesg; exit;;
	 *)	break ;;
	esac
	shift
done
#	get file names
while test $#  -gt 1
do
	if test -r "$1" -a -f "$1"
		then f="$f $1"
	elif test -r "$1" -a -d "$1"
		then d="$d $1"
		elif test "$UUP" = ""
		then echo "$1: file/directory not found"; exit
	fi
	shift
done
#	the recipient arg: remote!user
remote=`expr $1 : '\(.*\)!'`
user=`expr $1 : '.*!\(.*\)' \| $1`
if test 1 -eq $copy
then a="$a -C"
fi
	a="$a -d -n$user"
error=1
if test -n "$d" -a -n "$user"
then
	for i in $d
	do
	( cd $i; UUP="$UUP/$i" uuto * $1 )
	error=0
	done
fi
if test -n "$f" -a -n "$user"
then
	uucp $a $f $remote!~/receive/$user/$mysys$UUP/
	error=0
fi
if test $error = 1
then 
	echo $mesg
	exit 2
fi
