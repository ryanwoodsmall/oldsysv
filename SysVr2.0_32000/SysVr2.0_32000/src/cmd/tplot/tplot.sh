################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	@(#)tplot.sh	1.1

case $1 in
-T*)	t=$1
	shift ;;
*)	t=-T$TERM
esac
case $t in
-T450)	exec /usr/lib/t450 $*;;
-T300)	exec /usr/lib/t300 $*;;
-T300S|-T300s)	exec /usr/lib/t300s $*;;
-Tver)	exec /usr/lib/vplot $*;;
-Ttek|-T4014)	exec /usr/lib/t4014 $* ;;
*)  echo terminal type not known 1>&2; exit 1
esac
