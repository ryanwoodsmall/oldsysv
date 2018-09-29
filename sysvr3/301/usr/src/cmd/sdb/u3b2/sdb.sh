#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:u3b2/sdb.sh	1.3"
#sdb shell - used to determine if on a swapping or paging system and
#            select the correct sdb (sdbp: paging or sdbs: swapping)
case `uname -r` in
	2.0*)	/usr/lib/sdbs $@ ;;	#sdb for swapping systems

	*)	/usr/lib/sdbp $@ ;;	#sdb for paging systems
esac
