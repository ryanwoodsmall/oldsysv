#	rjeload.sh 1.4 of 3/17/82
#	@(#)rjeload.sh	1.4

#
#	rjeload - this program takes the kmc device as an
#	argument.  This device is "connected" to the vpm
#	top device specified in the lines file via vpmset.
#	The simple name of the kmc device is assumed, and /dev/ is
#	prepended to the name.  The RJE script (/lib/rjeproto) is
#	loaded (into the kmc device specified) and started by
#	vpmstart.
#

if [ $# -ne 1 ]
then
	echo "Usage: $0 kmcdev"
	exit 1
fi
LINES=/usr/rje/lines
NAME=`expr //$0 : '.*/\(.*\)load'`
KMC=/dev/$1

#	*** Loop until proper line is found ***

while read line
do
	set $line

	#	*** $3 is the directory	***
	#	*** $4 is the prefix	***
	#	*** $5 is the device	***

	if [ "$NAME" = "$4" ]
	then
		cd $3
		VPMT=/dev/$5
		echo "$NAME: $VPMT $KMC"
		/etc/vpmset -d $VPMT 2> /dev/null
		/etc/vpmset $VPMT $KMC 0
		if [ $? -ne 0 ]
		then
			echo "vpmset failed, load aborted."
			exit 1
		fi
		/etc/vpmstart $KMC 6 /lib/rjeproto
		LOGNAME=rje ./${NAME}init
		exit 1
	fi
done < $LINES
if [ $? -eq 0 ]
then
	echo "Can't find $NAME in $LINES"
	exit 1
fi
