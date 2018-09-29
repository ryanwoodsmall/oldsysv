#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./fstab.sh	1.8"

if u3b2
then echo "# format: mountdev fs [-[rd]] [fstype]
# This file is used by /etc/mount, /etc/mountall, and /etc/rmountall

/dev/dsk/c1d0s2 /usr
" >fstab
fi
