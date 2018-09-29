#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./brc.sh	1.7"

if u3b2
then echo "if [ -f /etc/advtab ]
then
	mv /etc/advtab /etc/oadvtab
fi
>/etc/advtab
chmod 644 /etc/advtab
" >>brc
fi
