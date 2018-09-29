#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright (c) 1986 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.vinstall:src/vinstall/usermgmt.sh	1.4"

#menu# manage users (set up a FACE environment)
#help#
#help#	This subcommand allows you to add 
#help#	a FACE for a given login.

flags="-qq -k$$"
trap exit 1 2 15

echo "
This procedure is used to add a FACE environment for a Unix user.

Type 'q' at any time to quit the present operation.
If a '?' appears as a choice, type '?' for help."

/bin/sh /usr/admin/menu/packagemgmt/FACE/viewsetup
