#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/toc.d/dtoc.sh	1.3"
:	dtoc [directory name]
:	program to generate a directory table of contents
:
BIN=/usr/bin/graf
if [ "$1" != "" ]
then
	cd $1
fi
if [ "$2" !=  "" ]
then
	j=$2
	i=`echo $2 | sed "s/^0\.//"`
else
	j=0.
	i=0.
fi
ls -al >/tmp/dtoc$$
ftot=`grep -c "^-" </tmp/dtoc$$`
echo "$i\t\"`pwd | sed "s/.*\\///"`\"\t$ftot"
seat=0
for child in `grep "^d" </tmp/dtoc$$ | cut -c40- | sed "/ *\.\.*$/d"`
do
	if [ -r $child ]
	then
:	compose the box markers
		seat=`expr $seat + 1`
		$BIN/dtoc $child  $j$seat.
	fi
done
rm /tmp/dtoc$$
exit
