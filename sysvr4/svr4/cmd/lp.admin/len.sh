#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp.admin:len.sh	1.1"

CSET=`infocmp "$1" | grep "lines#"`

if  [ "$?" != "0" ]
then
	exit 1
fi
echo $CSET | sed -e 's/.*lines#//' -e 's/, .*//' > ${datafile}len
exit 0
