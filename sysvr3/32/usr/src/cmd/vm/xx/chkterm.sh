#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm:xx/chkterm.sh	1.1"

if [ $# = 0 ]
then
	echo Usage: chkterm [terminfo] [term]
	exit 2
fi
terminfo=${TERMINFO:-/usr/lib/terminfo}
term=${1}
dir=`echo ${term} | cut -c1`
if [ -f  "${terminfo}/${dir}/${term}" ]
then
	exit 0
else
	exit 1
fi
