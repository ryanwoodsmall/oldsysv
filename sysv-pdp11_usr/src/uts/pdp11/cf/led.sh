# @(#)led.sh	1.4
#	$1	name of output
#	$2	name of segment file
#	$3	type, either 'i' or 'id'

OUTF=$1
SEGF=$2
TYPE=$3

if [ $# -ne 3 ]
then
	echo "usage: led.sh output input type"
	exit 2
fi

case $TYPE in

i)	CMD="/bin/ld -x low.o ../mch_i.o ../lib0 conf.o linesw.o\
		../lib[1-9] name.o -o $OUTF"
	echo $CMD
	$CMD
	chmod 755 $OUTF
	chown bin $OUTF
	chgrp bin $OUTF
	exit
	;;

id)	CMD="/bin/ld -ix low.o ../mch_id.o ../lib0 conf.o linesw.o\
		../lib[1-9] name.o"
	echo $CMD
	$CMD
	size a.out
	CMD="/etc/sysfix a.out $OUTF"
	echo $CMD
	$CMD
	chmod 755 $OUTF
	chown bin $OUTF
	chgrp bin $OUTF
	rm -f a.out
	exit
	;;

ov)
	;;

*)	echo type must be "'i', 'id' or 'ov'"
	exit
esac

#	Read SEGF and build a sensible command line for ovld

TMPF=/tmp/led$$
SEGT=/tmp/segf$$
DTAB=/tmp/dtab$$
flag=0

while read l		# split SEGF into a DTAB and a real SEGF
do
	if [ "$l" = "*" ]
	then
		flag=1
	elif [ $flag -eq 0 ]
	then
		echo $l >> $DTAB
	else
		echo $l >> $SEGT
	fi
done < $SEGF

SEGF=$SEGT		# Note: from this point on, SEGF is really a temp file.

while read l		# Read SEGF and determine if the modules to be exported
do			# have been configured.  If not, toss 'em away.
			# For aliases, separate alternatives with '|'.
			# For multi-word patterns, escape inter-word white
			# space with '\'.
	for f in $l
	do
		x=`fgrep $f $DTAB | sed 's/.* //'`  # courtesy of echo above,
		egrep "$x" conf.c > /dev/null
		if [ $? -eq 0 ]
		then
			echo "$f \c" >> $TMPF
		fi
	done
	echo >> $TMPF
done < $SEGF

ed - $TMPF <<!		# Unfortunately, we have to clean this guy up.
	H
	g/ \$/s///
	g/^\$/d
	w
	q
!

cp $TMPF $SEGF

mods=`cat $SEGF`
mods=`echo $mods`
rem="rm -f $mods"
trap "$rem; exit" 1 2 3 15	# Handle interrupts and such

for lib in ../lib[1-9]	# extract the object modules we need
do
	ar xv $lib $mods 2> /dev/null
done

ed - $SEGF <<!		# now change SEGF into a segment spec for ovld
	H
	1,\$s/^/-Z /
	1,\$s/\$/ /
	1,\$j
	s/\$/ -L /
	w
	q
!
			# build the ovld line
CMD="/etc/ovld -iF low.o ../mch_ov.o `cat $SEGF` ../lib0 conf.o linesw.o\
	../lib[1-9] name.o"
echo $CMD		# and do it!
$CMD
CMD="/etc/sysfix a.out $OUTF"
echo $CMD
$CMD
chmod 755 $OUTF
chown bin $OUTF
chgrp bin $OUTF
rm -f a.out
$rem $TMPF $SEGF $DTAB
