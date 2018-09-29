#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/gutil.d/yoo.sh	1.3"
trap "rm /tmp/yoo$$; exit 1" 2 3
cat - > /tmp/yoo$$
mv /tmp/yoo$$ $1
