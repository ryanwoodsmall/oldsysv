#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# Copyright (c) 1986 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.vinstall:src/vinstall/viewsetup.sh	1.5"
trap 'exit 0' 1 2 15
flags="-qq -k$$"
loginlen=8	# Maximum login ID length
minid=100	# Minimum user and group ID

# check if VMSYS has value and if so, is it a directory and accessible
VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
OASYS=`sed -n -e '/^oasys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
if [ ! -d "${VMSYS}" -o ! -x "${VMSYS}" ]
then
	echo "\\tERROR: ${VMSYS},"
	echo "\\tdirectory does not exist or is not accessible!!"
	exit 1
fi
if [ ! -d "${OASYS}" -o ! -x "${OASYS}" ]
then
	echo "\\tERROR: ${VMSYS},"
	echo "\\tdirectory does not exist or is not accessible!!"
	exit 1
fi
while true
do
	LOGINID=`checkre ${flags} -H'
	This is the "name" that the computer uses to identify the user.
	It also is used to identify data that belongs to the user.' \
		-fe "Enter user's login ID [?, q]:  " \
		-R ':' '":" is an illegal character.' \
		-r '.' 'You must enter a value.'`
	uid=`sed -n "/^${LOGINID}:/s/[^:]*:[^:]*:\([0-9]*\):.*/\1/p" /etc/passwd`
	if [ -z "${uid}" ]
	then
		echo "\\t'${LOGINID}' does not exist.  
This is the current list:"	
	     cut -d: -f1 /etc/passwd | sort | pr -t -w80 -6
		continue 
	elif [ "${uid}" -lt ${minid} ]
	then
		echo "\\tThis login has user ID ${uid}, which is less than ${minid}
	I will not modify this login ID."
		continue
	fi

echo "\n"
choicelist="yes no"
	service=`checklist ${flags} -fep "Sysadm Privileges:
${choicelist}

Select an answer for sysadm privilege for ${LOGINID} [q]:" ${choicelist}`

	${VMSYS}/bin/vsetup "${LOGINID}" "${service}"
	if checkyn ${flags} -f 'Do you want to add FACE environment for another login?'
	then 
		continue
	else
		break
	fi
done
exit 0
