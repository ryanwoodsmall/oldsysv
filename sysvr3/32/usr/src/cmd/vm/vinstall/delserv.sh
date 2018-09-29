#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm:vinstall/delserv.sh	1.1"

trap 'rm -rf $VMSYS/OBJECTS/.Lserve ; exit 0' 1 2 15
flags="-qq -k$$"; export flags

# set VMSYS so that Menu.serve file can be updated if installed.
VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
export VMSYS

if [ ! -d "${VMSYS}" ]
then
	echo >&2 "Can't find home directory for vmsys"
	sleep 3
	exit 2
fi

if [ ! -f ${VMSYS}/OBJECTS/.Lserve ]
then
	>${VMSYS}/OBJECTS/.Lserve
else
	echo "Can't update Menu.serve file it is LOCKED!!!"
	exit 2
fi

echo "\015"
while [ "${ans}" != "q" -a "${ans}" != "n" ]
do

	servlist=`${VMSYS}/bin/listserv -l VMSYS`
	if [ -z "${servlist}" ]
	then
		echo "There are no services to delete\n"
		rm -f $VMSYS/OBJECTS/.Lserve
		exit 0
	else
		servlist=`echo "${servlist}" | pr -t -w0 -5`
	fi

	choice=`checklist ${flags} -fep "This is the current list of services:

${servlist}

Select a service to delete[q]:" ${servlist}`
	case "${choice}" in
	*)
		mname=${choice}
		;;
	esac	

	echo "\\nPlease wait updating $VMSYS/OBJECTS/Menu.serve"
	${VMSYS}/bin/delserve "${mname}" "VMSYS"
	if [ "${?}" != "0" ]
	then
		echo "Failure to delete service ${mname}"
	fi

	servlist=`${VMSYS}/bin/listserv -l VMSYS`
	if [ -z "${servlist}" ]
	then
		echo "There are no more Services to delete \\n"
		rm -f $VMSYS/OBJECTS/.Lserve
		exit 0
	else
		while true
		do
			echo "Would you like to delete another Service[y,n,q]? \\c"
			read ans
			case "${ans}" in
			y|Y|yes|YES)
				break
				;;
			n|N|no|NO)
				ans=n
				break
				;;
			q)
				ans=q
				break
				;;
			*)
				echo  "Invalid response"
				;;
			esac
		done
	fi
done
rm -rf ${VMSYS}/OBJECTS/.Lserve
exit 0
