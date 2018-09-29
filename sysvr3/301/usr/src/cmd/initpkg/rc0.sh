#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc0.sh	1.13"

if u3b2
then echo "
#	\"Run Commands\" for init state 0
#	Leaves the system in a state where it is safe to turn off the power
#	or go to firmware.

stty sane tab3 2>/dev/null
echo 'The system is coming down.  Please wait.'

#	The following segment is for historical purposes.
#	There should be nothing in /etc/shutdown.d.
if [ -d /etc/shutdown.d ]
then
	for f in /etc/shutdown.d/*
	{
		if [ -s \$f ]
		then
			/bin/sh \${f}
		fi
	}
fi
#	End of historical section

if [ -d /etc/rc0.d ]
then
	for f in /etc/rc0.d/K*
	{
		if [ -s \${f} ]
		then
			/bin/sh \${f} stop
		fi
	}

#	system cleanup functions ONLY (things that end fast!)	

	for f in /etc/rc0.d/S*
	{
		if [ -s \${f} ]
		then
			/bin/sh \${f} start
		fi
	}
fi

trap \"\" 15
kill -15 -1
sleep 10
/etc/killall  9
sleep 10
sync;sync;sync
/etc/umountall
stty sane 2>/dev/null
sync;  sync
echo '
The system is down.'
sync
" >rc0
fi
