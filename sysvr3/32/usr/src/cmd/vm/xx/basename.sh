#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm:xx/basename.sh	1.1"
AQQQ=${1-.}
AQQQ=`expr //$AQQQ : "\(.*\)\/$" \| $AQQQ`
BQQQ=`expr //$AQQQ : '.*/\(.*\)'`
expr $BQQQ : "\(.*\)$2$" \| $BQQQ
