#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfuadmin:rfuadmin.sh	1.11"

#if u3b2
echo "#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	\"@(#)/usr/bin/rfuadmin.sl 1.2 3.0 10/07/85 50322\"

# executed by rfudaemon on request from annother system.
# this is a filter that alllows other systems to execute
# selected commands. This example is used by the fumount
# command when a remote system is going down.
# System administrators may expand the case list if it is
# desired to add to the remote command list.


if [ -d /usr/adm/log ]
then
	LOG=/usr/adm/log/rfuadmin.log
else
	LOG=/usr/adm/rfuadmin.log
fi
echo \`date\` \$* >>\$LOG
case \$1 in
'fuwarn' | 'disconnect' | 'fumount')
	TYPE=\$1
	RESRC=\$2
	if [ \"\$3\" = \"0\" ]
	then
		GRACE=NOW!
	else
		GRACE=\"in \$3 seconds.\"
	fi
	;;
'error')
	echo \$* >>\$LOG
	echo \$* >/dev/console 2>&1
	exit 1
	;;
*)
echo \"unexpected command \\\"\$*\\\"\">>\$LOG
	echo \"unexpected command for \$0 \\\"\$*\\\"\">/dev/console 2>&1
	exit 1
	;;
esac

		# RESRC is of the form domain.resource.

R=\`echo \$RESRC | sed -e 's/\\..*//'\`	# domain of the resource
D=\`/usr/bin/dname\`				# this domain
RESRC2=\`echo \$RESRC | sed -e 's/.*\\.//'\`	# resource name only

M=\`/etc/mount  |
	while read fs dummy1 dev mode mode2 dummy2
	do
		if [ \"\${dev}\" = \"\${RESRC}\" ]
		then
				# if the full name is in the mount table,
				# it is unique.
			echo \$fs \$dummy1 \$dev \$mode \$mode2
			exit 0
		else
			# otherwise,
			# if the domain of this machine is the same
			# as the that of the resource, it may be
			# mounted without the domain name specified.
			# Must be careful here, cuz if the resource
			# is 'XXX', we may find 'other_domain.XXX'
			# as well as 'XXX' in the mount table.

			if [ \"\$R\" = \"\$D\" ]
			then
					# the domain is the same
				if [ \"\${RESRC2}\" = \"\${dev}\" ]
				then
					echo \$fs \$dummy1 \$dev \$mode \$mode2
					exit 0
				fi
			fi
		fi
	done\`
if [ \"\$M\" = \"\" ]
then
	exit 0		# it's not mounted
fi
set \$M
			# \$1 is mountpoint
			# \$3 is 'domain.resource' or 'resource'
			# \$4 is 'read/write/remote' or 'read'
case \${TYPE} in

#		The fumount(1M) warning from a host
'fuwarn')
	echo \"\$1\" is being removed from the system \$GRACE.>>\$LOG
	/etc/wall <<!
'\$1' is being removed from the system \$GRACE
!
	exit 0
	;;

'disconnect' | 'fumount')
	if [ \"\$TYPE\" = \"disconnect\" ]
	then
	echo \"\$1\" has been disconnected from the system.>>\$LOG
		/etc/wall <<!
'\$1' has been disconnected from the system.
!
	fi
	/etc/fuser -k \$3 >>\$LOG 2>&1
	echo umount -d \$3 >>\$LOG
	/etc/umount -d \$3
		# for automatic remount, ...
	sleep 10
	echo \$4 | grep write >/dev/null 2>&1
	if [ \$? = 0 ]
	then 
		w=\"\"
	else
		w=\"r\"
	fi
	sh /etc/rmount -d\$w \$3 \$1 &
	exit 0
	;;

esac

" >rfuadmin
#endif
