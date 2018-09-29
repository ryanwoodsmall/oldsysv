################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	sa1.sh 1.2 of 5/10/84
#	@(#)sa1.sh	1.2
DATE=`date +%d`
ENDIR=/usr/lib/sa
DFILE=/usr/adm/sa/sa$DATE
cd $ENDIR
if [ $# = 0 ]
then
	$ENDIR/sadc 1 1 $DFILE
else
	$ENDIR/sadc $* $DFILE
fi
