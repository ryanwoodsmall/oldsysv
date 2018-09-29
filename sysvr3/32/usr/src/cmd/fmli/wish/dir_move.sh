#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.wish:src/wish/dir_move.sh	1.1"

cd $1 || exit 1

if [ "$3"x != x ]
then path=$2/$3
else path=$2/`basename $1`
fi

mkdir $path || exit 1

find . -print | cpio -pmud $path 2>/dev/null || exit 1

if [ `basename $0` = "dir_move" ]
then cd `dirname $1` && rm -rf $name 2>/dev/null || exit 2
fi

exit 0
