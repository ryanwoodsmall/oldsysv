#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:locking.sh	1.3"

if [ -f /tmp/*.${table} ]
then
	echo "Someone else is updating the ${table} table.  ${info}"
	echo "will be ${action} the ${place} when the ${table} table
is no longer"
	echo "in use.  If you hit break while waiting, ${info} will not be"
	echo "${action} the ${place}."
	while [ -f /tmp/*.${table} ]
	do
		continue
	done
fi
