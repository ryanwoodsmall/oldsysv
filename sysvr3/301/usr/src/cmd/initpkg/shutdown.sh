#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./shutdown.sh	1.6"

echo "#	Sequence performed to change the init stat of a machine.

#	This procedure checks to see if you are permitted and allows an
#	interactive shutdown.  The actual change of state, killing of
#	processes and such are performed by the new init state, say 0,
#	and its /etc/rc0.

#	Usage:  shutdown [ -y ] [ -g<grace-period> ] [ -i<init-state> ]

#!	chmod +x \${file}

if [ \`pwd\` != / ]
then
	echo \"\$0:  You must be in the / directory to run /etc/shutdown.\"
	exit 1
fi

#	Check the user id.
if [ -x /usr/bin/id ]
then
	eval \`id  |  sed 's/[^a-z0-9=].*//'\`
	if [ \"\${uid:=0}\" -ne 0 ]
	then
	        echo \"\$0:  Only root can run /etc/shutdown.\"
		exit 2
	fi
fi
" >shutdown
if u3b2
then echo "
grace=60
askconfirmation=yes
initstate=s
while [ \$# -gt 0 ]
do
	case \$1 in
	-g[0-9]* )
		grace=\`expr \"\$1\" : '-g\(.*\)'\`
		;;
	-i[Ss0156] )
		initstate=\`expr \"\$1\" : '-i\\(.*\\)'\`
		;;
	-i[234] )
		initstate=\`expr \"\$1\" : '-i\(.*\)'\`
		echo \"\$0:  Initstate \$i is not for system shutdown\"
		exit 1
		;;
	-y )
		askconfirmation=
		;;
	-* )
		echo \"Illegal flag argument '\$1'\"
		exit 1
		;;
	* )
		echo \"Usage:  \$0 [ -y ] [ -g<grace> ] [ -i<initstate> ]\"
		exit 1
	esac
	shift
done

if [ -n \"\${askconfirmation}\" -a -x /etc/ckbupscd ]
	then
	#	Check to see if backups are scheduled at this time
	BUPS=\`/etc/ckbupscd\`
	if [ \"\$BUPS\" != \"\" ]
		then
		echo \"\$BUPS\"
		echo \"Do you wish to abort this shutdown and return to 
command level to do these backups? [y, n] \\\c\"
		read YORN
		if [ \"\$YORN\" = \"y\" -o \"\$YORN\" = \"Y\" ]
			then 
			exit 1
		fi
	fi
fi

if [ -z \"\${TZ}\"  -a  -r /etc/TIMEZONE ]
then
	. /etc/TIMEZONE
fi

echo '\\nShutdown started.    \\\c'
date
echo

sync
cd /

trap \"exit 1\"  1 2 15

a=\"\`who  |  wc -l\`\"
if [ \${a} -gt 1  -a  \${grace} -gt 0 ]
then
	su adm -c /etc/wall<<-!
		The system will be shut down in \${grace} seconds.
		Please log off now.

	!
	sleep \${grace}
fi

/etc/wall <<-!
	THE SYSTEM IS BEING SHUT DOWN NOW ! ! !
	Log off now or risk your files being damaged.

!
sleep \${grace}

if [ \${askconfirmation} ]
then
	echo \"Do you want to continue? (y or n):   \\\c\"
	read b
else
	b=y
fi
if [ \"\$b\" != \"y\" ]
then
	/etc/wall <<-\\!
		False Alarm:  The system will not be brought down.
	!
	echo 'Shut down aborted.'
	exit 1
fi
case \"\${initstate}\" in
s | S )
	. /etc/rc0
esac
/etc/init \${initstate}
" >>shutdown
else echo "

if [ \`pwd\` != / ]
then
	cd /
	echo You must be in the root (/) directory to execute shutdown
	exit 1
fi

	T=/tmp
	trap \"rm -f \$T/\$\$; exit 0\" 0 1 2 15
	
	echo \"\\nSHUTDOWN PROGRAM\\n\"
	date
	echo \"\\n\"
	cd /
	a=\"\`who ^wc -l\`\"
	if [ \$a -gt 1 ]
	then
		echo \"Do you want to send your own message? (y or n):   \\\c\"
		read b
		if [ \"\$b\" = \"y\" ]
		then
			echo \"Type your message followed by ctrl d....\\n\"
			su adm -c /etc/wall
		else
			su adm -c /etc/wall <<!
	PLEASE LOG OFF NOW ! ! !
	System maintenance about to begin.
	All processes will be killed in \$grace seconds.
	!
		fi
		sleep \$grace
	fi
	/etc/wall <<!
	SYSTEM BEING BROUGHT DOWN NOW ! ! !
	!
	if [ \${grace} -ne 0 ]
	then
		sleep 60
	fi
	echo \"\\nBusy out (push down) the appropriate\"
	echo \"phone lines for this system.\\n\"
	echo \"Do you want to continue? (y or n):   \\\c\"
	read b
	if [ \"\$b\" = \"y\" ]
	then
		/usr/lib/acct/shutacct
		echo \"Process accounting stopped.\"
		/etc/errstop
		echo \"Error logging stopped.\"
		echo \"\\nAll currently running processes will now be killed.\\n\"
		/etc/killall
		mount^sed -n -e '/^\/ /d' -e 's/^.* on\(.*\) read.*/umount \1/p' ^ sh -
		/etc/init s
		mount^sed -n -e '/^\/ /d' -e 's/^.* on\(.*\) read.*/umount \1/p' ^ sh -
		sync;sync;sync
		echo \"Wait for \`INIT: SINGLE USER MODE' before halting.\"
		sync;sync;sync;
	else
		echo \"For help, call your system administrator.\"
	fi
" >>shutdown
fi
