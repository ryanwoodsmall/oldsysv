################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	@(#)errstop.sh	1.1
if [ ! -r ${1-/unix} ]
then
	echo namelist not readable
	exit 1
fi

a=`ps -e -n ${1-/unix}|grep errdemon|grep -v grep|sed -e "s/\(......\).*/kill -15 \1 > \/dev\/null /"`
if [ "$a" ]
then
	if eval $a
	then
		:
	else
		echo Not root
	fi
else
	echo No demon active
	exit 1
fi
exit 0
