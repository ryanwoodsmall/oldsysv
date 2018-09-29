#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dsconfig:dsconfig.sh	1.4"
#
#	script to print Data Storage Configuration. It takes the
#	Simple Admin names in /dev/rSA and maps them to the  /dev/rdsk or
#	/dev/rmt entries and actual location of hardware on the
#	IO bus.
#
#	Usage:  dsconfig [simple admin name ... ]
#


if [ ${#} -eq 0 ]
then
	PATTERN=`ls /dev/rSA 2>/dev/null`
else
	PATTERN=${*}
fi

for DEVICE in ${PATTERN}
do

	BASE=`basename ${DEVICE}`
	DIR=
	SANAME=
	DVCNM=

#
# Simple Admin Name?
#

	COMPARE=`ls /dev/rSA 2>/dev/null`

	for NAME in ${COMPARE}
	do
		if [ "${NAME}" = "${BASE}" ]
		then
			DIR=/dev/rSA
			break
		fi
	done

	if [ "${DIR}" != "/dev/rSA" ]
	then
		echo "ERROR: Option must be a Simple Administration Name"
		echo "       found in /dev/rSA or /dev/SA."
		exit
	fi

#
#	Get device names for /dev/rdsk entries possibly linked to /dev/rSA
#	entries.
#       The next line is changed in response to MR bl87-14201 to correct
#       the CTC problem.  Since a CTC connected to 3B bus will have a entry
#       in /dev/rdsk with minor # equal to "3", it is necessary to search
#       accordingly.     08/20/87 by jzl
#	DVCNMS=`ls /dev/rdsk/c*d*s6* 2>/dev/null`

 	DVCNMS=`ls /dev/rdsk/c*d*s[36]* 2>/dev/null`
	for DVCNM in ${DVCNMS}
	do
		SANAME=`/usr/lbin/samedev ${DVCNM} /dev/rSA/${BASE}`
		if [ -n "${SANAME}" ]
		then
			break
		fi
	done

#
#	If /dev/rSA entry not linked to /dev/rdsk entry, try /dev/rmt
#

	if [ -z "${SANAME}" ]
	then

		DVCNMS=`ls /dev/rmt/c* 2>/dev/null`
		for DVCNM in ${DVCNMS}
		do
			SANAME=`/usr/lbin/samedev ${DVCNM} /dev/rSA/${BASE}`
			if [ -n "${SANAME}" ]
			then
				break
			fi
		done
	fi

#	This portion is added for the cases that a SA name does not correspond
#	to any actual device. Only c?t?d?s? is searched to determine wether
#       it is an extended device.	08-24-87

	if [ -n "${SANAME}" ]
	then
		SANAME=`basename ${SANAME}`
		HAVET=`basename "${DVCNM}"`
		HAVET=`expr "${HAVET}" : '.*\([0-9]t\).*'`
	else
		SANAME=${BASE}
		echo "\nSA: ${SANAME}\nDevice: NOT FOUND"
		continue
	fi

#
#	If it has a 't', it is an extended by device
#

	if [ -n "${HAVET}" ]
	then

  		eval `basename ${DVCNM} | sed -e 's:^c\([0-9]*\)t\([0-9]*\)d\([0-9]*\).*$:SLOT=\1 TC=\2 DRV=\3:'`
		echo "\nSA: ${SANAME}\nDevice: ${DVCNM}"
                echo "\nConfiguration: Slot ${SLOT} Target Controller ${TC} Drive ${DRV}"

	else

  		eval `basename ${DVCNM} | sed -e 's:^c\([0-9]*\)d\([0-9]*\).*$:SLOT=\1 DRV=\2:'`

		SLOT=`expr ${SLOT} - 1`
		echo "\nSA: ${SANAME}\nDEVICE: ${DVCNM}"

		case ${SLOT} in

			-1) echo "\nConfiguration: Integral Floppy Drive"
				;;
			
			0) echo "\nConfiguration: Integral Disk Drive ${DRV}"
				;;
	
			*) echo "\nConfiguration: Slot ${SLOT} Drive ${DRV}"
		esac

	fi

done
