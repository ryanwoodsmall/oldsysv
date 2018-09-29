#	@(#)scc.sh	1.2
LIB="-l2A"
ERR="scc: bad conf lib"

CASEB="if pdp11; then LIB='-l2B'; shift; else echo $ERR; exit; fi"

case $1 in
+B)	eval "$CASEB";;
+A)	shift;;
+)	shift; case $1 in
	B)	eval "$CASEB";;
	A)	shift;;
	*)	LIB=;;
	esac;;
+*)	echo $ERR; exit;;
esac

if vax
then
	cc -DSTANDALONE -e start -Z2 -x $* $LIB
else
	cc -DSTANDALONE -Z2 -x $* $LIB
fi
