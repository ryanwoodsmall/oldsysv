#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rmount.sh	1.9.2.1"

if u3b2
then echo "#	mount remote resources
/usr/nserve/rmount \$*
" >rmount
fi
