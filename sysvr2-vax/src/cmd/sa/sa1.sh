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
