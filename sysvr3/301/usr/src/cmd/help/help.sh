#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:help.sh	1.11"

#	Remove Temporary Files
trap "rm -f ${session_log:=/tmp/log$$} ; exit" 0 1 2 3 15

#	Export Variables
export DESCRIPTIONS OPTIONS EXAMPLES GLOSSARY SCREENS FETCH LIST \
	HELP_FILE  ROOT RESPONSE_TABLE DISPLAY_TABLE KEYWORDS_TABLE HELP

#	Initialize Variables
HELP="/usr/lib/help"
DESCRIPTIONS="${HELP}/db/descriptions.a"
OPTIONS="${HELP}/db/options.a"
EXAMPLES="${HELP}/db/examples.a"
GLOSSARY="${HELP}/db/glossary.a"
SCREENS="${HELP}/db/screens.a"
DISPLAY_TABLE="${HELP}/db/tables/display"
RESPONSE_TABLE="${HELP}/db/tables/responses"
KEYWORDS_TABLE="${HELP}/db/tables/keywords"
SYSTEM_LOG="${HELP}/HELPLOG"
FETCH="${HELP}/fetch"
LIST="${HELP}/list"
A=`expr //${0} : "\(.*\)\/$" \| ${0}`
NAME=`expr //$A : '.*/\(.*\)'`

if [ -z "$TERM" ]
then
	TERM=450
	export TERM
fi

#	Create Session Log if System Log is Writable
if [ -w "${SYSTEM_LOG}" ]
then
	LOG_FILE="${session_log}"
	echo "\n\nlogin=${LOGNAME}\tuname=${UNAME:=`uname`}\tdate=`date`" >${LOG_FILE}
fi

#	Set Up Variables for Call to Interaction Handler
if [ "${NAME}" = 'help' -a ${#} -ne 0 ]
then
	NAME=${1} ; shift
fi
case "${NAME}" in
help)
	HELP_FILE='help_foot'
	;;
starter)
	;;
usage)
	RESPONSE=''
	if getopts d:e:o: i
	then
		case "${i}" in
		d)	NAME=getdesc ; RESPONSE=${OPTARG} ; dflag=1 ;;
		e)	NAME=getexample ; RESPONSE=${OPTARG} ; eflag=1 ;;
		o)	NAME=getoption ; RESPONSE=${OPTARG} ; oflag=1 ;;
		\?)	echo "\n\n ${1}  is not a legal option." > /dev/tty
			echo "Legal options include -d , -e , or -o.\n" > /dev/tty
			if [ ! -t 1 ]
			then
				exit 1
			else
				echo "Do you wish to enter the usage component of the help facility now (y/n)? \c" > /dev/tty
			fi
			read ANS
			if [ "${ANS}" != "y" -a "${ANS}" != "yes" ]	
			then
				exit 0
			fi
			NAME=usage ;;
		esac
	else
		NAME=getdesc; RESPONSE="${1}"
	fi

	if [ -n "${RESPONSE}" ]
	then
		case ${NAME} in
		getdesc)
			if [ -z "`${LIST} ${DESCRIPTIONS} ${RESPONSE} 2>/dev/null`" ]
			then
				error='1'
			fi
			;;
		getexample)
			if [ -z "`${LIST} ${EXAMPLES} ${RESPONSE} 2>/dev/null`" ]
			then
				error='1'
			fi
			;;
		getoption)
			if [ -z "`${LIST} ${OPTIONS} ${RESPONSE} 2>/dev/null`" ]
			then
				error='1'
			fi
			;;
		*)
			error='1'
			;;
		esac
	else

		# If help is not being redirected and an error occurs, then
		# an error message is printed to the screen.  Otherwise, the
		# usage screen is piped or redirected to the file or command.
		if [ ! -t 1 ]
		then
			if [ "${dflag}" = 1 -o "${eflag}" = 1 -o "${oflag}" = 1 ]
			then
				echo "Missing command name argument\n" > /dev/tty
				exit 1
			else
				NAME=usage
			fi
		else
			NAME=usage
		fi
	fi
	if [ -n "${error}" ]
	then
		echo "\n\n ${RESPONSE}  is not an included command\n\n" > /dev/tty
		if [ ! -t 1 ]
		then
			exit 1
		else
			echo "Do you wish to enter the usage component of the help facility now (y/n)? \c" > /dev/tty
			read ANS
			if [ "${ANS}" != "y"  -a "${ANS}" != "yes" ]	
			then
				exit 0
			fi
			NAME=usage
		fi
	fi
	;;
locate)
	if [ -n "${1}" ]
	then
		NAME=keysrch RESPONSE="${*}"
	fi
	;;
glossary)
	if [ -n "${*}" ]
	then
		term=`grep "^${*}:glossary" ${RESPONSE_TABLE} | sed 's/.*://'`
		if [ -n "${term}" ]
		then
			NAME=define RESPONSE=${term}
		else
			echo "\n\n ${*}  is not included in the glossary.\n\n" > /dev/tty
			if [ ! -t 1 ]
			then
				exit 1
			else
				echo "Do you wish to enter the glossary component of the help facility now (y/n)? \c" > /dev/tty
				read ANS
				if [ "${ANS}" != "y" -a "${ANS}" != "yes" ]	
				then
					exit 0
				fi
			fi
		fi
	fi
	;;
*)
	# This if statement hooks into the sccs help command.  If help gets
	# an option that it can't recognize, it assumes the option is valid
	# for the sccs help command. 

	if [ -x /usr/lib/help/lib/help2 ]
	then
		/usr/lib/help/lib/help2 ${NAME} ${*} && exit 0
	fi

	NAME=help
	echo "\n\n\nThis is the UNIX system help facility.\n\n\n"> /dev/tty
	echo "The facility is invoked at shell level by typing:\n\n"> /dev/tty
	echo "\n        help        or"> /dev/tty
	echo "\n        [help] glossary [term]        or"> /dev/tty
	echo "\n        [help] locate [keyword1 keyword2 ...]        or"> /dev/tty
	echo "\n        [help] starter        or"> /dev/tty
        echo "\n        [help] usage [command_name]\n\n\n"> /dev/tty
	if [ ! -t 1 ]
	then
		exit 1
	else
		echo "Do you wish to enter the help facility now (y/n)? \c" > /dev/tty
	fi
	read ANS
	if [ "${ANS}" != "y" -a "${ANS}" != "yes" ]
	then
		exit 0
	fi
	NAME=help
	;;
esac

#	Execute the Interaction Handler
if [ ${SCROLL:=yes} = 'yes' ]
then
	${HELP}/interact -s -d "${DISPLAY_TABLE}" -r "${RESPONSE_TABLE}" -l "${LOG_FILE}" ${NAME} "${RESPONSE}"
else
	${HELP}/interact -d "${DISPLAY_TABLE}" -r "${RESPONSE_TABLE}" -l "${LOG_FILE}" ${NAME} "${RESPONSE}"
fi

#	Update System Log if it is Writable
if [ -w "${SYSTEM_LOG}" -a -s "${LOG_FILE}" ]
then
	echo "login=${LOGNAME}\tuname=${UNAME}\tdate=`date`" >>${LOG_FILE}
	cat ${LOG_FILE} >>${SYSTEM_LOG}
fi
