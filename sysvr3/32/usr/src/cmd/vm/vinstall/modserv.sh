#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm:vinstall/modserv.sh	1.3"

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
#Request user to enter the type Service Menu Name
servnam() {
	echo "\015"
	mnamelen=10
	while true 
	do
		mname=`checkre ${flags} -H'
		This "name" defines the Menu item name for the Service Menu.
		The name specified here must be 10 chracters or shorter.' \
		-fe "Enter The Menu item name for the Service Menu: " \
		'.' 'You must enter a value.' \
		'^[A-Za-z0-9_]*$' 'Answer contains an illegal character.' \
		"^.\{1,${mnamelen}\}$" "Answer is too long.
		No more than ${mnamelen} characters are permitted."`

		break
	done
}

# Request user to enter command:
servcom () {
	echo "\015"
	command=
	while [ -z "${command}" ]
	do
		echo "Please enter command invoked from Service Menu: \\c"
		read command
		if [ "${command}" = "q" ]
		then
			rm -f $VMSYS/OBJECTS/.Lserve
			exit 0
		fi

		${VMSYS}/bin/chexec "${command}"
		if [ "${?}" != "0" ]
		then
			echo "Warning ${command} doesn't exist !!! \\n"
		fi
	done
}

while [ "${ans}" != "q" -a "${ans}" != "n" ]
do

	servlist=`${VMSYS}/bin/listserv -l VMSYS`
	if [ -z "${servlist}" ]
	then
		echo "There are no services to modify\n"
		rm -f $VMSYS/OBJECTS/.Lserve
		exit 0
	else
		servlist=`echo "${servlist}" | pr -t -w0 -5`
	fi

	choice=`checklist ${flags} -fep "This is the current list of services:

${servlist}

Select a service to modify[q]:" ${servlist}`
	case "${choice}" in
	*)
		mname=${choice}
		command=`tail -1 $VMSYS/bin/${mname}.ins`
		ocommand=${command}
		term="\$TERM"
		echo "\015"
		echo "Terminal type set will be ${term} default set for the user"
		path="\$HOME/FILECABINET"
		echo "\015"
		echo "Working Directory will be the user's \$HOME/FILECABINET"
		;;
	esac	

# list values and give the user a chance to change
	select=" "
	while [ "${select}" != "" ]
	do
		echo "\\nThese are the values chosen for the following:"
		echo "\t1 Service Name: ${mname}"
		echo "\t2 Service Command: ${command}"
		echo "\015"
		echo "Please select a number or <return> for no change: \c"
		read select
		case "${select}" in
		"1")
			servnam
			if [ "${choice}" != "${mname}" ]
			then
				if [ -f ${VMSYS}/bin/${mname}.ins ]
				then
					echo "${mname} already exists \\n"
					servnam
				fi
			fi
			;;
		"2")
			servcom
			;;
		"q")
			rm -f $VMSYS/OBJECTS/.Lserve
			exit 0
			;;
		"")
			select=""
			;;
		*)
			echo "Invalid selection Please try again"
			;;
		esac
	done

	echo "\\nPlease wait updating $VMSYS/OBJECTS/Menu.serve"
	${VMSYS}/bin/modserv "$term" "$mname" "$command" "$path" "VMSYS" "$term" "$choice" "$ocommand" "$path"
		if [ "${?}" = "0" ]
		then
			chown vmsys ${VMSYS}/bin/${mname}.ins
			chgrp vm ${VMSYS}/bin/${mname}.ins
		else
			echo "Failure to create service for ${mname}"
		fi

	while true
	do
		echo "Would you like to modify another Service[y,n,q]? \\c"
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
done
rm -rf ${VMSYS}/OBJECTS/.Lserve
exit 0
