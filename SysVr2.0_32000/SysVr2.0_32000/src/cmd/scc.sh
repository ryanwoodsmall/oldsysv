################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
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
elif ns32000
then
	cc -DSTANDALONE -e start -Z2 -x $* -l2 $LIB
else
	cc -DSTANDALONE -Z2 -x $* $LIB
fi
