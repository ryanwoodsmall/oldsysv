#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright (c) 1986 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm:vinstall/servmgmt.sh	1.3"

#menu# manage services (add, modify, delete)
#help#
#help#	This subcommand allows you to add, modify or delete
#help#	service menu information.

VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
export VMSYS
cmdlist='
	1 add
	2 modify
	3 delete
	4 list
'
flags="-qq -k$$"
trap exit 1 2 15

echo "
This procedure is used to add, modify, and delete entries from 
the FACE Services Menu

Type 'q' at any time to quit the present operation.
If a '?' appears as a choice, type '?' for help.

If a default appears in the question type <RETURN> for the default."

while true
do
	CMD=`checklist ${flags} -fep -Dadd "
Enter the operation you want to perform:
${cmdlist}
(default add)[q]:  " ${cmdlist}`

	case "$CMD" in
	add|1)
		/bin/sh /usr/admin/menu/packagemgmt/FACE/addserv
		;;
	modify|2)
		/bin/sh /usr/admin/menu/packagemgmt/FACE/modserv
		;;
	delete|3)
		/bin/sh /usr/admin/menu/packagemgmt/FACE/delserv
		;;
	list|4)
		servlist=`$VMSYS/bin/listserv -l VMSYS`
		echo "
This is the current list of Services known to FACE:"
	echo "${servlist}" | pr -t -w0 -5
		;;
	*)
		break;
		;;
	esac

done
