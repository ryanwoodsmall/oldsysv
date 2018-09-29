#	rjeload3b.sh 1.3 of 3/17/82
#	@(#)rjeload3b.sh	1.3

#	rjeload vpm-bottom - the bottom device is "connected" to
#	the vpm top device specified in the lines file via vpmset.
#	The simple name of the devices are assumed, and /dev/ is
#	prepended to the names.  The RJE script (/lib/rjeproto) is
#	loaded into the bottom device specified.
#

if [ $# -gt 2 ]
then
	echo "Usage: $0 vpmbdev"
	exit 1
fi
LINES=/usr/rje/lines
NAME=`expr //$0 : '.*/\(.*\)load'`
VPMB=/dev/$1

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
		echo "$NAME: $VPMT $VPMB"
		/etc/vpmset -d $VPMT 2> /dev/null
		/etc/vpmset $VPMT $VPMB
		if [ $? -ne 0 ]
		then
			echo "vpmset failed, load aborted."
			exit 1
		fi
		if [ -z "$V_35" ]
		then
			/etc/vpmstart $VPMB 6 /lib/rjeproto
		else
			/etc/vpmstart $VPMB 7 /lib/rjeproto
		fi
		LOGNAME=rje ./${NAME}init
		exit 1
	fi
done < $LINES
if [ $? -eq 0 ]
then
	echo "Can't find $NAME in $LINES"
	exit 1
fi
