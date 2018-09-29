#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rmount.sh	1.8"

if u3b2
then echo "#	mount remote resources
#		/etc/rmount <flags> <resource> <mountpoint> <fstype>

TIME=60		# delay between attempts
RETRIES=0	# number of attempts before exit (0 means try forever)
USAGE=\"Usage: \$0 -d[r] <resource> <mountpoint> [fstype]\"

if [ \$# -lt 3 ]
then 
	echo \${USAGE} 
	exit 1
fi
case \"\$1\" in
	\"-d\" | \"-rd\" | \"-dr\")
		;;
	*)
	echo \"invalid flag \\\"\$1\\\": \${USAGE}\"
	exit 1;;
esac
#			is it already mounted?
/etc/mount | /bin/grep \" \$2 \"
if [ \$? -eq 0 ]
then
	exit 0
fi

if [ \"\$4\" != \"\" ]
then fstype=\"-f \$4\"
fi
i=1
while true
do
	/etc/mount \$1 \${fstype} \$2 \$3 >/dev/null 2>&1
	if [ \$? = 0 ]
	then
		exit 0
	fi
	if [ \${RETRIES} -gt 0 ]
	then
		RETRIES=\`expr \${RETRIES} - 1\`
		if [ \${RETRIES} -le 0 ]
		then
			exit 1
		fi
	fi
	sleep \${TIME}
#	if name server disappears, give up (no longer running rfs)
	ps -e | grep nserve >/dev/null 2>&1
	if [ \$? != 0 ]
	then 
		exit 0
	fi

done
" >rmount
fi
