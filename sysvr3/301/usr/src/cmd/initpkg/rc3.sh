#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc3.sh	1.11"

if u3b2
then echo "
#	\"Run Commands\" executed when the system is changing to init state 3,
#	same as state 2 (multi-user) but with remote file sharing.
set \`who -r\`
if [ -d /etc/rc3.d ]
then
	for f in /etc/rc3.d/K*
	{
		if [ -s \${f} ]
		then
			/bin/sh \${f} stop
		fi
	}

	for f in /etc/rc3.d/S*
	{
		if [ -s \${f} ]
		then
			/bin/sh \${f} start
		fi
	}
fi
if [ \$9 = 'S' ]
then
	echo '
The system is ready.'
fi
" >rc3
else
echo "
#	\"Run Commands\" executed when the system is changing to init state 3,
#	configuration is site dependant
" >rc3
fi
