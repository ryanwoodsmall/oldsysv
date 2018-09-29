#       @(#)troff.sh	1.1
dev=aps
oflags= newargs=

for i
do
	case $i in
	-Tcat)	dev=cat ;;
	-Taps)	dev=aps ;;
	-T*)	echo invalid option $i;  exit 1 ;;
	-c*)	cm=`echo $i|sed -e s/c/m/`
		newargs="$newargs $cm"
		oflags=$oflags$i  ;;
	-b|-k*|-p*|-g|-w)	oflags=$i$oflags ;;
	*)	newargs="$newargs $i"  ;;
	esac
done

case $dev in

cat)
	exec otroff $*
	;;

aps)
	if [ "-b" = "$oflags" ]
	then
		echo '-b no longer supported'
		exit 1
	fi
	if [ -n "$oflags" ]
	then
		echo "options -c, -k, -b, -w, -g and -p are no longer supported"
	fi
	exec troff $newargs
	;;

esac
