#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)adm:u3b5/powerfail.sh	1.2"
echo "POWER FAIL RC"

# **** Insert powerfail commands here
# Normally you would check the run state if you have states other
# than zero and execute the commands necessary to reload devices.
echo "Check all the disks and breakers"
uname
