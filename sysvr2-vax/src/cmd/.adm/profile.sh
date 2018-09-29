
#	@(#)profile.sh	1.6

trap "" 1 2 3
export TZ LOGNAME
TZ=EST5EDT
case "$0" in
-sh | -rsh)
	trap : 1 2 3
	echo "UNIX System V Release `uname -r|cut -dv -f1` `uname -m` Version `uname -r|cut -dv -f2`"
	uname -n
	echo "Copyright (c) 1984 AT&T Technologies, Inc.\nAll Rights Reserved\n"
	cat /etc/motd
	trap "" 1 2 3
	if mail -e
	then echo "you have mail"
	fi
	if [ $LOGNAME != root ]
	then
		news -n
	fi
	;;
-su)
	:
	;;
esac
trap 1 2 3
